#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QMouseEvent>
#include <QOpenGLWidget>

struct Tool;
class Frame;
class Layer;

QT_BEGIN_NAMESPACE
namespace Ui {
class ImageCanvas;
}
QT_END_NAMESPACE

class ImageCanvas : public QOpenGLWidget
{
    Q_OBJECT

    //NOTE: parent must be a QScrollArea!

private:
    Ui::ImageCanvas* ui;
    int scaling= 5;
    QGraphicsScene scene;
    QBrush brush;
    QPen pen;
    QMenu* context_menu= nullptr;
    Qt::MouseButton mouse_down_button= Qt::NoButton;
    bool mouse_has_moved= false;
    QPointF mouse_last_pos;
    QPointF mouse_last_global_pos;
    Layer* current_layer;
    QImage* image;
    QImage selection;
    QImage floating_layer;
    QRect rect_selection= QRect(0,0,0,0);
    Frame* current_frame= nullptr;
    int current_layer_index= 0;
    Tool* current_tool;
    QSize tilegrid_size= QSize(8, 8);

    const int max_scaling= 32;
    const QList<QRgb> selection_palette= {
        0x00000000, 0x800080FF, 0x8000FFFF, 0xC0000000, 0xC0FFFFFF,
    };

    bool show_tilegrid= true;
    bool show_pixelgrid= false;

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void paintEvent(QPaintEvent* event);

    void PanToMouse(QPoint mouse_global_pos);
    void PaintGrid(QPainter* painter);
    void PaintTempSelection(QPainter* painter);

public:
    ImageCanvas(QScrollArea* parent, Frame* frame);

    void Redraw();
    void ZoomIn();
    void ZoomOut();

    void Plot(int x, int y, int color, int radius=1);
    void PlotSelection(int x, int y, bool include, int radius=1);
    void DrawPencil(QPoint pos, bool primary= true);
    void DrawSelectionPencil(QPoint pos, bool include);
    void PickColor(QPoint pos, bool primary= true);
    void RectangleSelect(QPoint pos, bool include= true); //Call on mouse release

    const QImage* Image() const { return image; }
    QWidget* Widget() { return this->Widget(); }
    bool IsRectSelection() { return rect_selection.width() != 0 && rect_selection.height() != 0; }

    //void SetImage(QImage* image);
    void SetFrame(Frame* frame);
    void SetCurrentLayerIndex(int index);
    void SetTileGridSize(QSize size) { tilegrid_size= size; }
    void EnableTileGrid(bool enable) { show_tilegrid= enable; Redraw(); }
    void EnablePixelGrid(bool enable) { show_pixelgrid= enable; Redraw(); }
};

#endif // CANVAS_H
