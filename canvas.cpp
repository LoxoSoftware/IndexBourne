#include "canvas.h"
#include "project.h"
#include "mainwindow.h"
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <queue>

#define CANVAS_BORDER_W                 2
#define TILECANVASX_TO_PIXEL(x)         (((x-x/(image->width()*scaling))/scaling))
#define TILECANVASY_TO_PIXEL(y)         (((y-y/(image->height()*scaling))/scaling))

extern Project* current_project;
extern MainWindow* main_window;

ImageCanvas::ImageCanvas(QScrollArea* parent, Frame* frame)
{
    setParent(parent);
    setStyleSheet("background-image: linear-gradient(to bottom, aqua, navy);");
    parent->setWidget(this);
    setMouseTracking(true);
    parent->setWidgetResizable(true);

    if (current_layer_index >= frame->LayerCount())
        current_layer_index= 0;

    current_tool= new Tool;

    SetFrame(frame);

    Redraw();
    show();
}

void ImageCanvas::UpdateMode()
{
    if (!current_project || !this->image)
        return;

    *current_tool= current_project->CurrentTool();

    if (current_tool->type == Tool_Transform)
    {
        rect_selection= GetSelectionBoundaries().normalized();
        if (rect_selection.size() == QSize(0,0))
        {
            main_window->UpdateTransformStatus(false);
            return;
        }
        main_window->UpdateTransformStatus(true);

        //Copy selection into floating layer
        floating_layer= this->image->copy(rect_selection);
        //Mask out pixels which are not selected
        for (int iy=0; iy<floating_layer.height(); iy++)
        {
            if (iy+rect_selection.y() < 0 || iy+rect_selection.y() >= this->image->height())
                continue;

            uint8_t* sl_img= floating_layer.scanLine(iy);
            uint8_t* sl_sel= selection.scanLine(iy+rect_selection.y());

            for (int ix=0; ix<floating_layer.width(); ix++)
            {
                if (ix+rect_selection.x() < 0 || ix+rect_selection.x() >= this->image->width())
                    continue;

                if (!sl_sel[ix+rect_selection.x()])
                    sl_img[ix]= 0;
            }
        }
    }
    else if (current_tool->type & (Tool_RectSelect | Tool_FloodSelect | Tool_PencilSelect))
        DiscardFloatingLayer(true);
    else
        ApplyFloatingLayer();

    Redraw();
}

void ImageCanvas::ApplyFloatingLayer(bool opaque, bool record)
{
    if (floating_layer.isNull() || selection.isNull() || rect_selection.isNull() || !this->image)
    {
        DiscardFloatingLayer();
        return;
    }

    for (int iy=0; iy<floating_layer.height(); iy++)
    {
        if (iy+rect_selection.y() < 0 || iy+rect_selection.y() >= this->image->height())
            continue;

        uint8_t* sl_src= floating_layer.scanLine(iy);
        uint8_t* sl_dest= this->image->scanLine(iy+rect_selection.y());

        for (int ix=0; ix<floating_layer.width(); ix++)
        {
            if (ix+rect_selection.x() < 0 || ix+rect_selection.x() >= this->image->width())
                continue;

            if (sl_src[ix] || opaque)
                sl_dest[ix+rect_selection.x()]= sl_src[ix];
        }
    }

    DiscardFloatingLayer();

    if (record && current_project)
        current_project->CurrentFrame()->PushNewSnapshot(
            new UndoSnapshot(Undocmd_DiffImage, current_layer_index, current_layer));
}

void ImageCanvas::DiscardFloatingLayer(bool keep_selection)
{
    if (main_window)
        main_window->UpdateTransformStatus(false);

    floating_layer= QImage();
    if (!keep_selection)
    {
        selection.fill(0);
        rect_selection= QRect();
    }

    Redraw();
}

void ImageCanvas::Redraw()
{
    if (!image)
        return;

    scene.clear();

    setMinimumSize(image->width()*scaling,
                   image->height()*scaling);
    setMaximumSize(this->minimumSize());
    scene.setSceneRect(this->rect());

    QPixmap pix;
    QGraphicsPixmapItem* item;

    //Draw each layer
    for (int il=0; il<current_frame->LayerCount(); il++)
    {
        if (!current_frame->LayerAt(il)->visible)
        {
            if (il) continue;
            else
            {
                QImage voidimg= QImage(current_frame->ImageSize(), QImage::Format_Indexed8);
                voidimg.setColorTable(current_project->Palette());
                voidimg.fill(0);
                pix= QPixmap::fromImage(voidimg);
            }
        }
        else
            pix= QPixmap::fromImage(current_frame->LayerAt(il)->image);
        // if (il == current_layer_index && floating_layer != QImage())
        // {
        //     QImage tempmask= floating_layer.copy(image->rect());
        //     pix.setMask(QBitmap::fromImage(tempmask.createMaskFromColor(0)));
        // }
        item= new QGraphicsPixmapItem(pix);
        item->setScale(scaling);
        scene.addItem(item);

        //Draw floating selection
        if (il == current_layer_index && floating_layer != QImage())
        {
            pix= QPixmap::fromImage(floating_layer);
            item= new QGraphicsPixmapItem(pix);
            item->setOffset(rect_selection.x(), rect_selection.y());
            item->setScale(scaling);
            scene.addItem(item);
        }
    }

    //Draw selection
    if (current_tool)
    if (current_tool->type != Tool_Transform)
    {
        pix= QPixmap::fromImage(this->selection);
        item= new QGraphicsPixmapItem(pix);
        item->setScale(scaling);
        scene.addItem(item);
    }

    this->repaint();
}

void ImageCanvas::PaintGrid(QPainter* painter)
{
    QPen pen_tg_out= QPen(QColor(0,0,0, IsRectSelection() ? 128 : 255));
    pen_tg_out.setWidth(3);
    QPen pen_tg_in= QPen(QColor(255,255,255, IsRectSelection() ? 128 : 255));
    pen_tg_in.setWidth(1);
    QPen pen_pg= QPen(QColor(1,1,1));
    pen_pg.setWidth(1);

    if (show_pixelgrid && scaling > 4)
    {
        painter->setPen(pen_pg);
        for (int iy=0; iy<height(); iy+=scaling)
            //Horizontal lines
            painter->drawLine(QLineF(0, iy, width(), iy));
        for (int ix=0; ix<width(); ix+=scaling)
            //Vertical lines
            painter->drawLine(QLineF(ix, 0, ix, height()));
    }
    if (show_tilegrid)
    {
        for (int iy=0; iy<height(); iy+=tilegrid_size.height()*scaling)
        {
            //Horizontal lines
            painter->setPen(pen_tg_out);
            painter->drawLine(QLineF(0, iy, width(), iy));
            painter->setPen(pen_tg_in);
            painter->drawLine(QLineF(0, iy, width(), iy));
        }
        for (int ix=0; ix<width(); ix+=tilegrid_size.width()*scaling)
        {
            //Vertical lines
            painter->setPen(pen_tg_out);
            painter->drawLine(QLineF(ix, 0, ix, height()));
            painter->setPen(pen_tg_in);
            painter->drawLine(QLineF(ix, 0, ix, height()));
        }
    }
}

void ImageCanvas::PaintTempSelection(QPainter* painter)
{
    if (!IsRectSelection())
        return;

    QPen pen_sel_out;
    QPen pen_sel_in;
    switch (current_tool->type)
    {
    case Tool_RectSelect:
        pen_sel_out.setColor(QColor(32,32,0,255));
        pen_sel_in.setColor(QColor(255,255,0,255));
        pen_sel_in.setStyle(Qt::DashLine);
        break;
    case Tool_Transform:
        pen_sel_out.setColor(QColor(0,32,32,255));
        pen_sel_in.setColor(QColor(0,255,255,255));
        pen_sel_in.setStyle(Qt::SolidLine);
        break;
    default:
        return;
    }
    pen_sel_out.setWidth(3);
    pen_sel_in.setWidth(1);

    QRect disp_rect= QRect(
        rect_selection.x()*scaling, rect_selection.y()*scaling,
        rect_selection.width()*scaling, rect_selection.height()*scaling );

    painter->setPen(pen_sel_out);
    painter->drawRect(disp_rect);
    painter->setPen(pen_sel_in);
    painter->drawRect(disp_rect);
}

// void ImageCanvas::SetImage(QImage* image)
// {
//     this->image= image;
//     Redraw();
// }

void ImageCanvas::SetFrame(Frame* frame)
{
    if (!frame)
    {
        this->image= nullptr;
        return;
    }
    if (frame->LayerCount() <= 0)
    {
        this->image= nullptr;
        return;
    }

    if (current_layer_index >= frame->LayerCount())
        current_layer_index= 0;

    this->current_layer= frame->LayerAt(current_layer_index);
    this->image= &current_layer->image;
    this->selection= QImage(frame->ImageSize(), QImage::Format_Indexed8);
    this->selection.setColorTable(selection_palette);
    this->current_frame= frame;
    DiscardFloatingLayer();
}

void ImageCanvas::SetCurrentLayerIndex(int index)
{
    this->current_layer= current_frame->LayerAt(index);
    this->image= &current_layer->image;
    this->current_layer_index= index;
}

void ImageCanvas::ZoomIn()
{
    if (scaling < max_scaling)
    {
        scaling++;
        Redraw();
    }
}

void ImageCanvas::ZoomOut()
{
    if (scaling > 1)
    {
        scaling--;
        Redraw();
    }
}

void ImageCanvas::Plot(int x, int y, int color, int radius)
{   
    for (int iy=-radius/2.f; iy<radius/2.f; iy++)
        for (int ix=-radius/2.f; ix<radius/2.f; ix++)
        {
            if (x+ix < 0 || x+ix >= image->width()
                || y+iy < 0 || y+iy >= image->height())
                continue;
            image->setPixel(x+ix, y+iy, color);
        }
}

void ImageCanvas::PlotSelection(int x, int y, bool include, int radius)
{
    for (int iy=-radius/2.f; iy<radius/2.f; iy++)
        for (int ix=-radius/2.f; ix<radius/2.f; ix++)
        {
            if (x+ix < 0 || x+ix >= image->width()
                || y+iy < 0 || y+iy >= image->height())
                continue;
            if (include)
                selection.setPixel(x+ix, y+iy, ((x+y+ix+iy)&1)+1);
            else
                selection.setPixel(x+ix, y+iy, 0);
        }
}

void ImageCanvas::DrawPencil(QPoint pos, bool primary)
{
    int pixx= TILECANVASX_TO_PIXEL(pos.x());
    int pixy= TILECANVASY_TO_PIXEL(pos.y());

    if (primary)
        Plot(pixx, pixy, current_project->PaltableAIndex(), current_tool->diameter_a);
    else
        Plot(pixx, pixy, current_project->PaltableBIndex(), current_tool->diameter_b);

    Redraw();
}

void ImageCanvas::DrawSelectionPencil(QPoint pos, bool include)
{
    int tilex= TILECANVASX_TO_PIXEL(pos.x());
    int tiley= TILECANVASY_TO_PIXEL(pos.y());

    PlotSelection(tilex, tiley, include, include ? current_tool->diameter_b : current_tool->diameter_a);

    Redraw();
}

void ImageCanvas::PickColor(QPoint pos, bool primary)
{
    int pixx= TILECANVASX_TO_PIXEL(pos.x());
    int pixy= TILECANVASY_TO_PIXEL(pos.y());

    if (pixx < 0 || pixx >= image->width()
        || pixy < 0 || pixy >= image->height())
        return;

    int color_picked= 0, tcol;
    for (int il=0; il<current_frame->LayerCount(); il++)
    {
        if (!current_frame->LayerAt(il)->visible)
            continue;

        tcol= current_frame->LayerAt(il)->image.pixelIndex(pixx, pixy);
        if (tcol)
            color_picked= tcol;
    }

    if (primary)
        current_project->SetPaltableAPosition(QPoint(color_picked%PALETTE_W, color_picked/PALETTE_W));
    else
        current_project->SetPaltableBPosition(QPoint(color_picked%PALETTE_W, color_picked/PALETTE_W));
}

void ImageCanvas::RectangleSelect(QRect rect, bool include)
{
    for (int iy=rect.y(); iy<=rect.bottom(); iy++)
        for (int ix=rect.x(); ix<=rect.right(); ix++)
            PlotSelection(ix, iy, include, 1);

    Redraw();
}

QImage ImageCanvas::GetFloodMap(QImage img, QPoint pos)
{
    using namespace std;

    QImage result= QImage(img.size(), QImage::Format_Indexed8);
    result.setColorTable((palette_t){0,1});
    result.fill(0);

    int old_color = img.pixelIndex(pos.x(), pos.y());
    int new_color = (old_color+1)&0xFF;
    vector<pair<int, int>> dir = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    queue<pair<int, int>> q;

    if (old_color == new_color)
        return result;

    q.push({pos.x(), pos.y()});

    // Change the color of the starting pixel
    img.setPixel(pos.x(), pos.y(), new_color);
    result.setPixel(pos.x(), pos.y(), 1);

    // Perform Breadth-First Search
    while (!q.empty())
    {
        pair<int, int> front = q.front();
        int x = front.first, y = front.second;
        q.pop();

        // Traverse all 4 directions
        for (pair<int, int>& it : dir)
        {
            int nx = x + it.first;
            int ny = y + it.second;

            // Check boundary conditions and color match
            if (nx >= 0 && nx < img.width() &&
                ny >= 0 && ny < img.height() &&
                img.pixelIndex(nx, ny) == old_color)
            {
                img.setPixel(nx, ny, new_color);
                result.setPixel(nx, ny, 1);
                q.push({nx, ny});
            }
        }
    }

    return result;
}

void ImageCanvas::FloodSelect(QPoint pos)
{
    int pixx= TILECANVASX_TO_PIXEL(pos.x());
    int pixy= TILECANVASY_TO_PIXEL(pos.y());

    QImage result= GetFloodMap(*this->image, QPoint(pixx, pixy));

    for (int iy=0; iy<selection.height(); iy++)
    {
        uint8_t* sl_mask= result.scanLine(iy);

        for (int ix=0; ix<selection.width(); ix++)
            if (sl_mask[ix])
                PlotSelection(ix, iy, true);
    }

    Redraw();
}

void ImageCanvas::FillSelection(int color)
{
    for (int iy=0; iy<selection.height(); iy++)
    {
        uint8_t* sl_mask= selection.scanLine(iy);
        uint8_t* sl_dest= this->image->scanLine(iy);

        for (int ix=0; ix<selection.width(); ix++)
            if (sl_mask[ix])
                sl_dest[ix]= color;
    }

    current_frame->PushNewSnapshot(new UndoSnapshot(Undocmd_DiffImage, current_layer_index, current_layer));
    Redraw();
}

QRect ImageCanvas::GetSelectionBoundaries()
{
    QPoint min_start= QPoint(999999, 999999);
    QPoint max_end= QPoint(0, 0);
    bool is_selected= false;

    for (int iy=0; iy<selection.height(); iy++)
    {
        uint8_t* sl= selection.scanLine(iy);

        for (int ix=0; ix<selection.width(); ix++)
        {
            if (sl[ix] == 0)
                continue;

            is_selected= true;

            if (ix < min_start.x())
                min_start.setX(ix);
            if (iy < min_start.y())
                min_start.setY(iy);
            if (ix > max_end.x())
                max_end.setX(ix);
            if (iy > max_end.y())
                max_end.setY(iy);
        }
    }

    if (is_selected)
        return QRect(min_start, max_end);
    else
        return QRect(0,0,0,0);
}

void ImageCanvas::mousePressEvent(QMouseEvent* event)
{
    //event->accept();

    *current_tool= current_project->CurrentTool();

    mouse_down_button= event->button();
    mouse_last_pos= event->pos();
#if QT_VERSION_MAJOR > 5
    mouse_last_global_pos= event->globalPosition();
#else
    mouse_last_global_pos= event->globalPos();
#endif

    if (mouse_down_button == Qt::MiddleButton)
        this->setCursor(Qt::ClosedHandCursor);

    if (event->modifiers()& Qt::ControlModifier)
    {
        switch(current_tool->type)
        {
        case Tool_Pencil:
            //Quick color picker
            if (mouse_down_button & (Qt::LeftButton | Qt::RightButton))
                PickColor(event->pos(), mouse_down_button == Qt::LeftButton);
            break;
        case Tool_FloodSelect:
            FloodSelect(event->pos());
            break;
        default:
            break;
        }
    }
    else
    {
        switch (current_tool->type)
        {
        case Tool_FloodSelect:
            selection.fill(0);
            FloodSelect(event->pos());
            break;
        default:
            mouseMoveEvent(event);
            break;
        }
    }

    if (rect_selection.size() == QSize(0,0) && current_tool->type == Tool_Transform)
    {
        RectangleSelect(image->rect(), true);
        UpdateMode();
    }

    mouse_has_moved= false;
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (mouse_down_button | (Qt::LeftButton | Qt::RightButton))
    {
        switch (current_tool->type)
        {
        case Tool_Pencil:
            current_project->CurrentFrame()->PushNewSnapshot(new UndoSnapshot(Undocmd_DiffImage, current_layer_index, current_layer));
            break;
        case Tool_RectSelect:
            if (!(event->modifiers()&Qt::ControlModifier))
                //Hold CTRL to select multiple regions
                selection.fill(0);
            RectangleSelect(rect_selection, event->button()&Qt::LeftButton);
            if (!( event->modifiers()&Qt::ControlModifier || event->modifiers()&Qt::ShiftModifier ))
                current_project->SetCurrentToolType(Tool_Transform);
            break;
        default:
            break;
        }
    }

    if (mouse_down_button == Qt::MiddleButton)
        this->setCursor(Qt::ArrowCursor);

    mouse_down_button= Qt::NoButton;
    mouse_has_moved= false;
}

void ImageCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (mouse_down_button & (Qt::LeftButton | Qt::RightButton))
    {
        switch(current_tool->type)
        {
        case Tool_Pencil:
            DrawPencil(event->pos(), mouse_down_button == Qt::LeftButton);
            break;
        case Tool_Eyedropper:
            PickColor(event->pos(), mouse_down_button == Qt::LeftButton);
            break;
        case Tool_PencilSelect:
            DrawSelectionPencil(event->pos(), mouse_down_button == Qt::LeftButton);
            break;
        case Tool_RectSelect:
            rect_selection.setX(TILECANVASX_TO_PIXEL(mouse_last_pos.x()));
            rect_selection.setY(TILECANVASY_TO_PIXEL(mouse_last_pos.y()));
            rect_selection.setWidth(TILECANVASX_TO_PIXEL(event->pos().x())-rect_selection.x());
            rect_selection.setHeight(TILECANVASY_TO_PIXEL(event->pos().y())-rect_selection.y());
            this->repaint();
            break;
        default:
            break;
        }
    }

    if (current_tool->type == Tool_Transform && !(mouse_down_button & Qt::MiddleButton))
    {
        if (rect_selection.contains(event->pos().x()/scaling, event->pos().y()/scaling))
            this->setCursor(Qt::SizeAllCursor);
        else
            this->setCursor(Qt::ArrowCursor);

        if (mouse_down_button == Qt::LeftButton)
#if QT_VERSION_MAJOR > 5
            MoveFloatLayerToMouse(event->globalPosition().toPoint());
#else
            MoveFloatLayerToMouse(event->globalPos());
#endif
    }

    if (mouse_down_button == Qt::MiddleButton)
    {
#if QT_VERSION_MAJOR > 5
        PanToMouse(event->globalPosition().toPoint());
#else
        PanToMouse(event->globalPos());
#endif
    }
}

void ImageCanvas::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers()& Qt::ControlModifier)
    {
        if (event->angleDelta().y() >= 15)
        {
            ZoomIn();
        }
        if (event->angleDelta().y() <= -15)
        {
            ZoomOut();
        }
    }
}

void ImageCanvas::paintEvent(QPaintEvent* event)
{
    QPainter painter= QPainter(this);
    scene.render(&painter);
    PaintGrid(&painter);
    PaintTempSelection(&painter);
}

void ImageCanvas::PanToMouse(QPoint mouse_global_pos)
{
    int diffx= mouse_global_pos.x()-mouse_last_global_pos.x();
    int diffy= mouse_global_pos.y()-mouse_last_global_pos.y();

    this->setGeometry(x()+diffx, y()+diffy, width(), height());
    mouse_last_global_pos= mouse_global_pos;
}

void ImageCanvas::MoveFloatLayerToMouse(QPoint mouse_global_pos)
{
    if (rect_selection.size() == QSize(0,0))
        return;

    int diffx= (mouse_global_pos.x()-mouse_last_global_pos.x())/scaling;
    int diffy= (mouse_global_pos.y()-mouse_last_global_pos.y())/scaling;

    if (!diffy && !diffx)
        return;

    rect_selection.setRect(rect_selection.x()+diffx, rect_selection.y()+diffy,
                           rect_selection.width(), rect_selection.height());
    //move the selection
    QImage old_sel= selection;
    selection.fill(0);
    for (int iy= 0; iy < selection.height(); iy++)
    {
        if (iy < diffy || iy-diffy >= selection.height())
            continue;

        uint8_t* sl_src= old_sel.scanLine(iy-diffy);
        uint8_t* sl_dest= selection.scanLine(iy);

        for (int ix= 0; ix < selection.width(); ix++)
        {
            if (ix < diffx || ix-diffx >= selection.width())
                continue;

            sl_dest[ix]= sl_src[ix-diffx];
        }
    }

    Redraw();

    mouse_last_global_pos= mouse_global_pos;
}