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

    if (frame->LayerCount() > 0)
        this->image= frame->Layer(current_layer_index);
    else
        this->image= nullptr;
    this->current_frame= frame;
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

    for (int il=0; il<current_frame->LayerCount(); il++)
    {
        QPixmap pix= QPixmap::fromImage(*current_frame->Layer(il));
        QGraphicsPixmapItem* item= new QGraphicsPixmapItem(pix);
        item->setScale(scaling);
        scene.addItem(item);
    }

    this->repaint();
}

void ImageCanvas::PaintGrid(QPainter* painter)
{
    QPen pen_tg_out= QPen(QColor(0,0,0));
    pen_tg_out.setWidth(3);
    QPen pen_tg_in= QPen(QColor(255,255,255));
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

    this->image= frame->Layer(current_layer_index);
    this->current_frame= frame;
}

void ImageCanvas::SetCurrentLayerIndex(int index)
{
    this->image= current_frame->Layer(index);
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

    Redraw();
}

void ImageCanvas::DrawPencil(QPoint pos, bool primary)
{
    int tilex= TILECANVASX_TO_PIXEL(pos.x());
    int tiley= TILECANVASY_TO_PIXEL(pos.y());

    if (primary)
        Plot(tilex, tiley, current_project->PaltableAIndex(), current_tool->diameter_a);
    else
        Plot(tilex, tiley, current_project->PaltableBIndex(), current_tool->diameter_b);
}

void ImageCanvas::PickColor(QPoint pos, bool primary)
{
    int tilex= TILECANVASX_TO_PIXEL(pos.x());
    int tiley= TILECANVASY_TO_PIXEL(pos.y());

    if (tilex < 0 || tilex >= image->width()
        || tiley < 0 || tiley >= image->height())
        return;

    int color_picked= image->pixelIndex(tilex, tiley);

    if (primary)
        current_project->SetPaltableAPosition(QPoint(color_picked%PALETTE_W, color_picked/PALETTE_W));
    else
        current_project->SetPaltableBPosition(QPoint(color_picked%PALETTE_W, color_picked/PALETTE_W));
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
    //Save snapshot
    if (mouse_down_button | (Qt::LeftButton | Qt::RightButton))
    {
        if (current_tool->type == Tool_Pencil)
            current_project->CurrentFrame()->PushNewSnapshot(new UndoSnapshot(Undocmd_DiffImage, current_layer_index, image));
    }

    mouse_down_button= Qt::NoButton;
    mouse_has_moved= false;

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
}

void ImageCanvas::PanToMouse(QPoint mouse_global_pos)
{
    int finalx= mouse_global_pos.x()-mouse_last_global_pos.x();
    int finaly= mouse_global_pos.y()-mouse_last_global_pos.y();

    this->setGeometry(x()+finalx, y()+finaly, width(), height());
    mouse_last_global_pos= mouse_global_pos;
}