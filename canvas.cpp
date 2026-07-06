#include "canvas.h"
#include "project.h"
#include <QGraphicsPixmapItem>
#include <QMessageBox>

#define CANVAS_BORDER_W                 2
#define TILECANVASX_TO_PIXEL(x)         (((x-x/(image->width()*scaling))/scaling))
#define TILECANVASY_TO_PIXEL(y)         (((y-y/(image->height()*scaling))/scaling))

extern Project* current_project;

ImageCanvas::ImageCanvas(QScrollArea* parent, Frame* frame)
{
    setParent(parent);
    setStyleSheet("background-image: linear-gradient(to bottom, aqua, navy);");
    parent->setWidget(this);
    setMouseTracking(true);
    parent->setWidgetResizable(true);

    if (current_layer_index >= frame->LayerCount())
        current_layer_index= 0;

    SetFrame(frame);

    this->current_tool= new Tool();

    Redraw();
    show();
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
        item= new QGraphicsPixmapItem(pix);
        item->setScale(scaling);
        scene.addItem(item);
    }

    //Draw selection
    pix= QPixmap::fromImage(this->selection);
    item= new QGraphicsPixmapItem(pix);
    item->setScale(scaling);
    scene.addItem(item);

    this->repaint();
}

void ImageCanvas::PaintGrid(QPainter* painter)
{
    QPen pen_tg_out= QPen(QColor(0,0,0, rectangle_selecting ? 96 : 255));
    pen_tg_out.setWidth(3);
    QPen pen_tg_in= QPen(QColor(255,255,255, rectangle_selecting ? 96 : 255));
    pen_tg_in.setWidth(1);
    QPen pen_pg= QPen(QColor(1,1,1));
    pen_pg.setWidth(1);

    if (show_pixelgrid && scaling > 4)
    {
        painter->setPen(pen_pg);
        for (int iy=0; iy<height(); iy+=scaling)
            for (int ix=0; ix<width(); ix+=scaling)
            {
                painter->drawLine(QLineF(ix, iy, width(), iy));
                painter->drawLine(QLineF(ix, iy, ix, height()));
            }
    }
    if (show_tilegrid)
        for (int iy=0; iy<height(); iy+=tilegrid_size.height()*scaling)
            for (int ix=0; ix<width(); ix+=tilegrid_size.width()*scaling)
            {
                painter->setPen(pen_tg_out);
                painter->drawLine(QLineF(ix, iy, width(), iy));
                painter->drawLine(QLineF(ix, iy, ix, height()));
                painter->setPen(pen_tg_in);
                painter->drawLine(QLineF(ix, iy, width(), iy));
                painter->drawLine(QLineF(ix, iy, ix, height()));
            }

}

void ImageCanvas::PaintTempSelection(QPainter* painter)
{
    if (!rectangle_selecting)
        return;

    QPen pen_tg_out= QPen(QColor(32,32,0,255));
    pen_tg_out.setWidth(3);
    QPen pen_tg_in= QPen(QColor(255,255,0,255));
    pen_tg_in.setWidth(1);
    pen_tg_in.setStyle(Qt::DashLine);

    QRect disp_rect= QRect(
        temp_rect_selection.x()*scaling, temp_rect_selection.y()*scaling,
        temp_rect_selection.width()*scaling, temp_rect_selection.height()*scaling );

    painter->setPen(pen_tg_out);
    painter->drawRect(disp_rect);
    painter->setPen(pen_tg_in);
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
    this->selection.fill(0);
    this->current_frame= frame;
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
        tcol= current_frame->LayerAt(il)->image.pixelIndex(pixx, pixy);
        if (tcol)
            color_picked= tcol;
    }

    if (primary)
        current_project->SetPaltableAPosition(QPoint(color_picked%PALETTE_W, color_picked/PALETTE_W));
    else
        current_project->SetPaltableBPosition(QPoint(color_picked%PALETTE_W, color_picked/PALETTE_W));
}

void ImageCanvas::RectangleSelect(QPoint pos, bool include)
{
    for (int iy=temp_rect_selection.y(); iy<=temp_rect_selection.bottom(); iy++)
        for (int ix=temp_rect_selection.x(); ix<=temp_rect_selection.right(); ix++)
            PlotSelection(ix, iy, include, 1);

    Redraw();
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

    if (event->modifiers()& Qt::ControlModifier && current_tool->type == Tool_Pencil)
    {
        //Quick color picker
        if (mouse_down_button & (Qt::LeftButton | Qt::RightButton))
            PickColor(event->pos(), mouse_down_button == Qt::LeftButton);
    }
    else
    {
        mouseMoveEvent(event);
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
            if (!( event->modifiers()&Qt::ControlModifier || event->modifiers()&Qt::ShiftModifier ))
                current_project->SetCurrentToolType(Tool_Transform);
            RectangleSelect(event->pos(), event->button()&Qt::LeftButton);
            Redraw();
            break;
        default:
            break;
        }
    }

    mouse_down_button= Qt::NoButton;
    mouse_has_moved= false;
    rectangle_selecting= false;

    this->setCursor(Qt::ArrowCursor);
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
            rectangle_selecting= true;
            temp_rect_selection.setX(TILECANVASX_TO_PIXEL(mouse_last_pos.x()));
            temp_rect_selection.setY(TILECANVASY_TO_PIXEL(mouse_last_pos.y()));
            temp_rect_selection.setWidth(TILECANVASX_TO_PIXEL(event->pos().x())-temp_rect_selection.x());
            temp_rect_selection.setHeight(TILECANVASY_TO_PIXEL(event->pos().y())-temp_rect_selection.y());
            this->repaint();
            break;
        default:
            break;
        }
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
    int finalx= mouse_global_pos.x()-mouse_last_global_pos.x();
    int finaly= mouse_global_pos.y()-mouse_last_global_pos.y();

    this->setGeometry(x()+finalx, y()+finaly, width(), height());
    mouse_last_global_pos= mouse_global_pos;
}