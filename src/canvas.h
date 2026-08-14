/*
    IndexBourne - an image editor for indexed mode
    Copyright (C) 2026  LoxoSoftware

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
    typedef enum
    {
        TransMode_None = 0,
        TransMode_Move,
        TransMode_Vertical,
        TransMode_Horizontal,
        TransMode_AllAxis,
    } transform_mode_t;

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
    QImage* current_layer;
    QImage* image;
    QImage selection;
    QImage selection_old;
    QImage floating_layer;
    QImage floating_layer_old= QImage();
    QImage clipboard_image= QImage();
    QImage clipboard_mask= QImage();
    bool is_transfer_source_moved= false;
    QRect rect_selection= QRect(0,0,0,0);
    Frame* current_frame= nullptr;
    int current_layer_index= 0;
    Tool* current_tool= nullptr;
    transform_mode_t transform_mode= TransMode_None;
    bool transform_grabbing_right= false;
    bool transform_grabbing_bottom= false;
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
    void MoveFloatLayerToMouse(QPoint mouse_global_pos);
    void ResizeFloatLayerToMouse(QPoint mouse_global_pos);
    void PaintGrid(QPainter* painter);
    void PaintTempSelection(QPainter* painter);
    void UpdateSelectionContentWithImage(QImage content);

public:
    ImageCanvas(QScrollArea* parent, Frame* frame);

    void UpdateMode();
    void UpdateCurrentTool();
    void Redraw();
    void ZoomIn();
    void ZoomOut();

    void Plot(int x, int y, int color, int radius=1);
    void PlotSelection(int x, int y, bool include, int radius=1);
    void DrawPencil(QPoint pos, bool primary= true);
    void DrawSelectionPencil(QPoint pos, bool include);
    void PickColor(QPoint pos, bool primary= true);
    void RectangleSelect(QRect rect, bool include= true); //Call on mouse release
    void FloodSelect(QPoint pos);
    void FillSelection(int color);
    void FlipSelection(bool horizontal, bool vertical);
    void InvertSelectionRegion();
    void TransferToFloatingLayer(bool keep, QImage* src= nullptr, QImage* mask= nullptr, QRect src_rect= QRect());
    void ApplyFloatingLayer(bool opaque= true, bool record= true);
    void DiscardFloatingLayer(bool keep_selection= false, bool auto_undo= false);
    void CopySelected();
    void Paste();

    const QImage* Image() const { return image; }
    QWidget* Widget() { return this->Widget(); }
    bool IsRectSelection() { return rect_selection.width() != 0 && rect_selection.height() != 0; }
    QRect GetSelectionBoundaries();
    QImage GetFloodMap(QImage img, QPoint pos);

    //void SetImage(QImage* image);
    void SetFrame(Frame* frame);
    void SetCurrentLayerIndex(int index);
    void SetTileGridSize(QSize size) { tilegrid_size= size; }
    void EnableTileGrid(bool enable) { show_tilegrid= enable; Redraw(); }
    void EnablePixelGrid(bool enable) { show_pixelgrid= enable; Redraw(); }
};

#endif // CANVAS_H
