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
    InsertLayer();
    ClearHistory();
    PushNewSnapshot();
}

void Frame::InsertLayerAt(int pos)
{
    QImage new_layer= QImage(this->image_size, QImage::Format_Indexed8);
    new_layer.setColorTable(current_project->Palette());
    new_layer.fill(0);
    this->insert(pos, new_layer);
    PushNewSnapshot();
}

void Frame::SetImageSize(QSize size)
{
    for (int i=0; i<this->size(); i++)
    {
        this->replace(i, this->at(i).copy(0 ,0, size.width(), size.height()));
    }

    this->image_size= size;
    ClearHistory();
    PushNewSnapshot();
}

void Frame::PushNewSnapshot()
{
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

    layergroup_t layers;
    for (QImage il : *this)
    {
        il.setColorTable(current_project->Palette());
        layers+= il;
    }

    history.insert(history_index-1, layers);
}

void Frame::Undo()
{
    if (history_index <= 1)
        return;
    history_index--;
    for (int i=0; i<this->size(); i++)
        this->replace(i, history[history_index-1].at(i));
    current_project->SetPalette(this->first().colorTable(), false);
}

void Frame::Redo()
{
    if (history_index >= history.size())
        return;
    history_index++;
    for (int i=0; i<this->size(); i++)
        this->replace(i, history[history_index-1].at(i));
    current_project->SetPalette(this->first().colorTable(), false);
}

void Frame::ClearHistory()
{
    history_index= 0;
    history.clear();
}

Project::Project(MainWindow* parent, QSize size, QRgb bit_depth)
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

    this->main_window= parent;
    this->canvas= new ImageCanvas(main_window->CanvasContainer(), CurrentFrame());

    this->dckPaletteEdit= parent->PalettePanelPtr();
    this->dckToolPanel= parent->ToolPanelPtr();

    if (!current_project)
        current_project= this;
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

    *(CurrentLayer())= img.copy(0, 0, CurrentLayer()->width(), CurrentLayer()->height());

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

    CurrentFrame()->PushNewSnapshot();
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
            unsigned char* sl_src= CurrentFrame()->Layer(il)->scanLine(iy);
            unsigned char* sl_dest= result.scanLine(iy);

            for (int ix=0; ix<ImageSize().width(); ix++)
                sl_dest[ix]= sl_src[ix];
        }
    }

    return result;
}

void Project::SetCurrentFrameIndex(int frame)
{
    this->current_frame= frame;
    SetPalette(palette_t(), false);
}

void Project::SetPalette(const palette_t new_palette, bool recursive)
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
            QImage* layer= timeline[ifr].Layer(il);
            layer->setColorTable(palette);
        }
    }

    if (UiPalettePanel())
        UiPalettePanel()->Update();
    if (Canvas())
        Canvas()->Redraw();
}

void Project::SetPaletteFast(palette_t new_palette)
{
    this->palette= new_palette;
    if (UiPalettePanel())
        UiPalettePanel()->Update();
}

void Project::SetImageSize(QSize size)
{
    for (int ifr=0; ifr<timeline.size(); ifr++)
        timeline[ifr].SetImageSize(size);

    this->image_size= size;
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

    for (int ifr=0; ifr<timeline.size(); ifr++)
    {
        for (int il=0; il<timeline[ifr].LayerCount(); il++)
        {
            QImage* layer= timeline[ifr].Layer(il);
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

            layer->setColorTable(palette);
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