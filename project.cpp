#include "mainwindow.h"
#include "project.h"
#include <QScrollArea>
#include <QMessageBox>

extern Project* current_project;

Frame::Frame(Project* parent, QSize size)
{
    if (!parent)
        throw "Frame::Frame -- parent is null";

    this->image_size= size;
    if (!current_project)
        current_project= parent;

    //QImage* new_layer= InsertLayer();
    ClearHistory();
    //PushNewSnapshot(new UndoSnapshot(Undocmd_AddLayer, 0, new_layer));
}

Layer* Frame::InsertLayerAt(int pos, bool record)
{
    Layer new_layer= Layer(this->image_size);
    new_layer.image.setColorTable(current_project->Palette());
    new_layer.image.fill(0);

    if (record || !LayerCount())
        PushNewSnapshot(new UndoSnapshot(LayerCount()>0? Undocmd_AddLayer : Undocmd_DiffImage, pos, &new_layer));

    this->insert(pos, new_layer);

    return (Layer*)&this->at(pos);
}

void Frame::RemoveLayer(int pos, bool record)
{
    if (this->size() < 2)
    {
        LayerAt(0)->image.fill(0);
        return;
    }

    if (record)
        PushNewSnapshot(new UndoSnapshot(Undocmd_RmLayer, pos, LayerAt(pos)));
    this->remove(pos);

    if (current_project->CurrentLayerIndex() >= this->size())
        current_project->SetCurrentLayerIndex(this->size()-1);
}

void Frame::ReplaceLayer(Layer img, int index)
{
    this->replace(index, img);
}

void Frame::SetImageSize(QSize size)
{
    this->image_size= size;
    ClearHistory();

    for (int il=0; il<this->size(); il++)
    {
        Layer new_layer= Layer(this->at(il).image.copy(0, 0, size.width(), size.height()));
        this->replace(il, new_layer);
    }
    PushNewSnapshot(new UndoSnapshot(Undocmd_DiffImage, 0, LayerAt(0)));
}

Layer* Frame::MergeLayerDown(int index)
{
    if (index >= LayerCount() || index < 1)
        return LayerAt(index);

    PushNewSnapshot(new UndoSnapshot(Undocmd_DiffImage, index-1, LayerAt(index-1)));

    //Layer* old_layer= new Layer(LayerAt(index-1)->image);

    for (int iy=0; iy<ImageSize().height(); iy++)
    {
        uchar* sl_src= LayerAt(index)->image.scanLine(iy);
        uchar* sl_dest= LayerAt(index-1)->image.scanLine(iy);

        for (int ix=0; ix<ImageSize().width(); ix++)
            if (sl_src[ix])
                sl_dest[ix]= sl_src[ix];
    }

    RemoveLayer(index, true);
    index--;

    PushNewSnapshot(new UndoSnapshot(Undocmd_DiffImage, index, LayerAt(index)));
    return LayerAt(index);
}

void Frame::PushNewSnapshot(UndoSnapshot* snap)
{
    //TODO: Discard all oldest snapshots until the oldest is a DiffImage

    if (history_index < HISTORY_MAX)
    {
        //Delete all the newer snapshots after the index
        for (int i=history_index; i<history.size(); )
            history.removeLast();

        history_index++;
    }
    else
    {
        //Delete oldest snapshot and push new one
        history.removeFirst();
        history_index= HISTORY_MAX;
    }

    snap->layer_data.image.setColorTable(current_project->Palette());

    history.insert(history_index-1, *snap);
}

void Frame::Undo()
{
    if (history_index <= 1)
        history_index= 1;
    else
        history_index--;

    //The last element is the current state, so we don't want to restore it
    UndoSnapshot* snap= &history[history_index-1];

    Layer* restored_layer;
    switch (snap->cmd)
    {
    case Undocmd_DiffImage:
        this->replace(snap->layer_ind, snap->layer_data);
        current_project->SetPalette(snap->layer_data.image.colorTable(), false);
        break;
    case Undocmd_AddLayer:
        this->RemoveLayer(snap->layer_ind, false);
        break;
    case Undocmd_RmLayer:
        restored_layer= this->InsertLayerAt(snap->layer_ind, false);
        *restored_layer= snap->layer_data; //Restore last image before removal
        break;
    default:
        break;
    }

    current_project->Canvas()->Redraw();
    current_project->UpdateLayerPanel();
}

void Frame::Redo()
{
    if (history_index >= history.size()-1)
    {
        history_index= history.size();
        if (history[history_index-1].cmd != Undocmd_DiffImage)
            return;
    }
    else
        history_index++;
    UndoSnapshot* snap= &history[history_index-1];

    Layer* restored_layer;
    switch (snap->cmd)
    {
    case Undocmd_DiffImage:
        this->replace(snap->layer_ind, snap->layer_data);
        current_project->SetPalette(snap->layer_data.image.colorTable(), false);
        current_project->SetCurrentLayerIndex(snap->layer_ind);
        break;
    case Undocmd_AddLayer:
        restored_layer= this->InsertLayerAt(snap->layer_ind, false);
        *restored_layer= snap->layer_data;
        current_project->SetCurrentLayerIndex(snap->layer_ind);
        break;
    case Undocmd_RmLayer:
        this->RemoveLayer(snap->layer_ind, false);
        if (snap->layer_ind-1 >= LayerCount())
            current_project->SetCurrentLayerIndex(LayerCount());
        else
            current_project->SetCurrentLayerIndex(snap->layer_ind-1);
        break;
    default:
        break;
    }

    current_project->Canvas()->Redraw();
}

void Frame::ClearHistory()
{
    history_index= 0;
    history.clear();
}

Layer* Frame::LayerAt(int layer)
{
    Layer* img= (Layer*)&this->at(layer);
    palette_t pal= current_project->Palette();

    if (layer != 0)
        pal[0]= qRgba(0,0,0,0);

    img->image.setColorTable(pal);

    return img;
}

Project::Project(MainWindow* parent, QSize size)
{
    if (!parent)
        throw "Project::Project -- parent is null";
    if (!parent->CanvasContainer())
        throw "Project::Project -- canvas container is null";

    this->image_size= size;

    this->palette.clear();
    for (int i=0; i<PALETTE_W*PALETTE_H; i++)
    {
        if (i == 0)
            this->palette += QRgb(0xFFFF00FF);
        else
            this->palette += QRgb(0xFF000000);
    }

    InsertFrame();
    SetCurrentFrameIndex(0);
    CurrentFrame()->InsertLayer();

    this->main_window= parent;
    this->canvas= new ImageCanvas(main_window->CanvasContainer(), CurrentFrame());

    this->dckPaletteEdit= parent->PalettePanelPtr();
    this->dckToolPanel= parent->ToolPanelPtr();
    this->dckLayerPanel= parent->LayerPanelPtr();

    if (!current_project)
        current_project= this;

    parent->UpdateWindowTitle();
    UpdateLayerPanel();
    SetPalette(); //Update palette panel
}

bool Project::ImportBitmap(QImage img, consent_t canvas_resize, consent_t import_palette)
{
    if (img.format() != QImage::Format_Indexed8 || img.colorCount() < 1)
    {
        QMessageBox::critical(this->main_window, "Error", "Imported image must be a PNG or BMP in 8-bit indexed format."
                                             "\nRGB format images are not supported at the moment");
        return false;
    }

    if (canvas_resize != Consent_No)
    {
        QMessageBox::StandardButton answer= QMessageBox::Yes;
        if (canvas_resize == Consent_Ask)
            answer= QMessageBox::question(this->main_window, "Resize canvas?",
                                                            "Do you wish to resize the canvas to fit the new image?");
        if (answer == QMessageBox::Yes)
            this->SetImageSize(img.size());
    }

    palette_t old_palette= this->Palette();

    CurrentLayer()->image= img.copy(0, 0, CurrentLayer()->image.width(), CurrentLayer()->image.height());

    if (import_palette != Consent_No)
    {
        QMessageBox::StandardButton answer= QMessageBox::Yes;
        if (import_palette == Consent_Ask)
            answer= QMessageBox::question(this->main_window, "Import palette?",
                                           "Do you wish to replace the palette?");
        if (answer == QMessageBox::Yes)
            SetPalette(img.colorTable());
        else
            SetPalette(old_palette);
    }
    else
        SetPalette(old_palette);

    CurrentFrame()->PushNewSnapshot(new UndoSnapshot(Undocmd_AddLayer, CurrentLayerIndex(), CurrentLayer()));
    Canvas()->Redraw();

    return true;
}

QImage Project::RenderBitmap()
{
    QImage result= QImage(this->ImageSize(), QImage::Format_Indexed8);
    result.setColorTable(this->Palette());

    for (int il=0; il<CurrentFrame()->LayerCount(); il++)
    {
        for (int iy=0; iy<ImageSize().height(); iy++)
        {
            unsigned char* sl_src= CurrentFrame()->LayerAt(il)->image.scanLine(iy);
            unsigned char* sl_dest= result.scanLine(iy);

            for (int ix=0; ix<ImageSize().width(); ix++)
                if (sl_src[ix] != 0 || il == 0)
                    sl_dest[ix]= sl_src[ix];
        }
    }

    return result;
}

void Project::SetCurrentLayerIndex(int layer)
{
    current_layer= layer;
    this->Canvas()->SetCurrentLayerIndex(layer);
    UpdateLayerPanel();
}

void Project::SetCurrentFrameIndex(int frame)
{
    this->current_frame= frame;
    if (!CurrentFrame())
        return;
    SetPalette(palette_t(), false);
    if (Canvas())
        this->Canvas()->SetFrame(CurrentFrame());
}

void Project::SetPalette(palette_t new_palette, bool recursive)
{
    if (new_palette != palette_t())
    {
        this->palette.clear();
        this->palette= new_palette;
    }

    int start_frame= recursive? 0 : CurrentFrameIndex();
    int end_frame= recursive? timeline.size() : CurrentFrameIndex()+1;

    for (int ifr=start_frame; ifr<end_frame; ifr++)
    {
        for (int il=0; il<timeline[ifr].LayerCount(); il++)
        {
            Layer* layer= timeline[ifr].LayerAt(il);
            palette_t temp_pal= this->palette;

            if (il != 0)
                temp_pal[0]= qRgba(0,0,0,0);

            layer->image.setColorTable(temp_pal);
        }
    }

    if (UiPalettePanel())
        UiPalettePanel()->Update();
    if (Canvas())
        Canvas()->Redraw();
}

void Project::SetImageSize(QSize size)
{
    for (int ifr=0; ifr<timeline.size(); ifr++)
        timeline[ifr].SetImageSize(size);

    this->image_size= size;

    if (Canvas())
        Canvas()->Redraw();
}

void Project::SetFileName(QString filename)
{
    this->filename= filename;
    this->main_window->UpdateWindowTitle();
}

void Project::SetPaltableAPosition(QPoint pos)
{
    if (pos == paltable_Bpos) //Swap indexes if the selection ends up in the same place
        paltable_Bpos= paltable_Apos;
    paltable_Apos= pos;
    UiPalettePanel()->on_UpdateAPosition(pos);
}

void Project::SetPaltableBPosition(QPoint pos)
{
    if (pos == paltable_Apos) //Swap indexes if the selection ends up in the same place
        paltable_Apos= paltable_Bpos;
    paltable_Bpos= pos;
    UiPalettePanel()->on_UpdateBPosition(pos);
}

void Project::InsertFrameAt(int pos)
{
    timeline.insert(pos, Frame(this, this->image_size));
}

void Project::SwapColorIndex(int index_a, int index_b)
{
    if (index_a >= palette.size() || index_b >= palette.size())
    {
        QMessageBox::critical(main_window, "Error", "Selected index A or B is out of palette bounds");
        return;
    }

    QRgb tempcol= palette[index_a];
    palette[index_a]= palette[index_b];
    palette[index_b]= tempcol;

    if (index_a && index_b) //Swapping color 0 would break the transparency
    for (int ifr=0; ifr<timeline.size(); ifr++)
    {
        for (int il=0; il<timeline[ifr].LayerCount(); il++)
        {
            QImage* layer= &(timeline[ifr].LayerAt(il)->image);
            QImage old_image= *layer;

            for (int iy=0; iy<layer->height(); iy++)
            {
                for (int ix=0; ix<layer->width(); ix++)
                {
                    if (old_image.pixelIndex(ix, iy) == index_a)
                        layer->setPixel(ix, iy, index_b);
                    if (old_image.pixelIndex(ix, iy) == index_b)
                        layer->setPixel(ix, iy, index_a);
                }
            }
        }
    }

    SetPalette(palette_t(), false);
}

void Project::FillPaletteLinear(int index_a, int index_b, QRgb color)
{
    for (int i=index_a; i<=index_b && i<palette.size(); i++)
        palette[i]= color;
    SetPalette();
}

void Project::FillPaletteRect(QPoint pos_a, QPoint pos_b, QRgb color)
{
    for (int iy=pos_a.y(); iy<=pos_b.y(); iy++)
        for (int ix=pos_a.x(); ix<=pos_b.x(); ix++)
        {
            if (ix+iy*PALETTE_W >= palette.size())
                continue;
            palette[ix+iy*PALETTE_W]= color;
        }
    SetPalette();
}

void Project::SwapLayerIndex(int index_a, int index_b)
{
    if (index_a >= CurrentFrame()->LayerCount() || index_b >= CurrentFrame()->LayerCount())
    {
        QMessageBox::critical(main_window, "Error", "Selected index A or B is out of bounds");
        return;
    }

    for (int ifr=0; ifr<timeline.size(); ifr++)
    {
        Layer templayer= *FrameAt(ifr)->LayerAt(index_a);
        FrameAt(ifr)->ReplaceLayer(*FrameAt(ifr)->LayerAt(index_b), index_a);
        FrameAt(ifr)->ReplaceLayer(templayer, index_b);
    }

    SetPalette(palette_t(), false);
}