#ifndef PROJECT_H
#define PROJECT_H

#include <QImage>
#include <QGraphicsView>
#include "canvas.h"
#include "palette_panel.h"
#include "tool_panel.h"
#include "layer_panel.h"

#define PALETTE_W   16
#define PALETTE_H   16
#define HISTORY_MAX 32

class Project;
class Layer;

typedef QList<Layer> layergroup_t;
typedef QList<QRgb> palette_t;

typedef enum
{
    Undocmd_DiffImage,
    Undocmd_AddLayer,
    Undocmd_RmLayer,
} undocommand_t;

class Layer
{
public:
    Layer(QString name="")
    {
        if (name != "") this->name= name;
        this->image= QImage(8, 8, QImage::Format_Indexed8);
    }
    Layer(QSize size, QString name="")
    {
        if (name != "") this->name= name;
        this->image= QImage(size, QImage::Format_Indexed8);
    }
    Layer(QImage src, QString name="")
    {
        if (name != "") this->name= name;
        this->image= src;
    }
    QImage image;
    QString name= "layer";
    bool visible= true;
    uchar opacity= 255;
};

class UndoSnapshot
{
public:
    UndoSnapshot(undocommand_t cmd, int layer, Layer* image= nullptr)
    {
        this->cmd= cmd;
        this->layer= layer;
        this->image= image? *image : Layer(QSize(8,8));
    };

    Layer image= Layer(QSize(8,8));
    undocommand_t cmd;
    int layer;
};

class Frame : protected layergroup_t
{
private:
    QSize image_size;
    QList<UndoSnapshot> history;
    int history_index= 0;

public:
    Frame(Project* parent, QSize size);

    Layer* InsertLayerAt(int pos, bool record= true);
    Layer* InsertLayer(bool record= true) { return InsertLayerAt(this->size(), record); }
    void RemoveLayer(int layer, bool record= true);
    void ReplaceLayer(Layer img, int index);
    void SetImageSize(QSize size);
    void Undo();
    void Redo();
    void ClearHistory();
    void PushNewSnapshot(UndoSnapshot* snapshot);

    Layer* LayerAt(int layer);
    const QSize ImageSize() const { return image_size; }
    const int LayerCount() const { return this->size(); }
    int HistorySize() { return history.size(); }
    int HistoryIndex() { return history_index; }
};

typedef enum
{
    Tool_Pencil,
    Tool_Eyedropper,
    Tool_Rectangle,
    Tool_Ellipsoid,
    Tool_FilledRectangle,
    Tool_FilledEllipsoid,
    Tool_FloodFill,
    Tool_RectSelect,
    Tool_Transform,
} tooltype_t;

typedef struct Tool
{
    tooltype_t type= Tool_Pencil;
    int diameter_a= 1;
    int diameter_b= 1;
    QRect rect= QRect(0,0,1,1);
} Tool;

typedef enum
{
    Consent_No = 0,
    Consent_Ask = 1,
    Consent_Force = 2,
} consent_t;

typedef enum
{
    Format_8bpp,
    Format_4bpp,
} bppformat_t;

class Project
{

protected:
    QList<Frame> timeline;
    palette_t palette= palette_t();
    QSize image_size;
    int current_layer= 0;
    int current_frame= 0;
    QPoint paltable_Apos= QPoint(1,0);
    QPoint paltable_Bpos= QPoint(0,0);
    bppformat_t image_bpp= Format_8bpp;
    MainWindow* main_window= nullptr;
    ImageCanvas* canvas= nullptr;
    PalettePanel** dckPaletteEdit= nullptr;
    ToolPanel** dckToolPanel= nullptr;
    LayerPanel** dckLayerPanel= nullptr;
    QString filename= "";
    QString shared_palette_filename= "";
    bool is_saved= true;

    PalettePanel* UiPalettePanel() { return dckPaletteEdit ? *dckPaletteEdit : nullptr; }
    ToolPanel* UiToolPanel() { return dckToolPanel ? *dckToolPanel : nullptr; }
    LayerPanel* UiLayerPanel() { return dckLayerPanel ? *dckLayerPanel : nullptr; }

public:
    Project(MainWindow* parent, QSize size= QSize(64,64));
    bool LoadProject(QString filename); //Returns true on success
    bool SaveProject(QString filename); //Returns true on success
    bool ImportBitmap(QImage img, consent_t canvas_resize=Consent_Ask, consent_t import_palette=Consent_Ask); //Returns true on success
    QImage RenderBitmap();
    QImage RenderPalette();

    Frame* CurrentFrame() { return (Frame*)&(timeline.at(current_frame)); }
    Frame* FrameAt(int frame) { return (Frame*)&(timeline.at(frame)); }
    Layer* CurrentLayer() { return CurrentFrame()->LayerAt(current_layer); }
    const int CurrentFrameIndex() const { return current_frame; }
    const int CurrentLayerIndex() const { return current_layer; }
    const int Frames() const { return timeline.size(); }
    const QSize ImageSize() const { return image_size; }
    const palette_t Palette() const { return palette; }
    const bppformat_t BppFormat() const { return image_bpp; }
    const QString FileName() const { return filename; }
    ImageCanvas* Canvas() { return canvas; }
    MainWindow* ParentWindow() { return main_window; }
    const QPoint PaltableAPosition() const { return paltable_Apos; }
    const QPoint PaltableBPosition() const { return paltable_Bpos; }
    const int PaltableAIndex() const { return paltable_Apos.x()+paltable_Apos.y()*PALETTE_W; }
    const int PaltableBIndex() const { return paltable_Bpos.x()+paltable_Bpos.y()*PALETTE_W; }
    Tool CurrentTool() { return UiToolPanel()->GetCurrentTool(); }

    void SetCurrentFrameIndex(int frame);
    void SetCurrentLayerIndex(int layer);
    void SetPaltableAPosition(QPoint pos);
    void SetPaltableBPosition(QPoint pos);
    void SetImageSize(QSize size);
    void SetPalette(palette_t palette= palette_t(), bool recursive= false);
    void SetBppFormat(bppformat_t fmt);
    void SetFileName(QString filename) { this->filename= filename; }
    void InsertFrameAt(int pos);
    void InsertFrame() { InsertFrameAt(timeline.size()); }
    void RemoveFrame(int frame);
    void SwapColorIndex(int index_a, int index_b);
    void FillPaletteLinear(int index_a, int index_b, QRgb color);
    void FillPaletteRect(QPoint pos_a, QPoint pos_b, QRgb color);
    void UpdateLayerPanel() { UiLayerPanel()->Update(); }
    void SwapLayerIndex(int index_a, int index_b);
};

#endif // PROJECT_H
