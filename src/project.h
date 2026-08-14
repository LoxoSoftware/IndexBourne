#ifndef PROJECT_H
#define PROJECT_H

#include <QImage>
#include <QGraphicsView>
#include "src/canvas.h"
#include "src/palette_panel.h"
#include "src/tool_panel.h"
#include "src/layer_panel.h"

#define PALETTE_W       16
#define PALETTE_H       16
#define HISTORY_MAX     32

#define _PRJ_FOREACH_FRAME  for (int ifr=0; ifr<timeline.size(); ifr++)

class Project;

typedef QList<QImage> layergroup_t;
typedef QList<QRgb> palette_t;

typedef enum
{
    Undocmd_DiffImage,
    Undocmd_AddLayer,
    Undocmd_RmLayer,
} undocommand_t;

class UndoSnapshot
{
public:
    UndoSnapshot(undocommand_t cmd, int layer_ind, QImage* layer_data= nullptr)
    {
        this->cmd= cmd;
        this->layer_ind= layer_ind;
        if (layer_data)
            this->layer_data= *layer_data;
    };

    QImage layer_data= QImage(8, 8, QImage::Format_Indexed8);
    undocommand_t cmd;
    int layer_ind;
};

class Frame : protected layergroup_t
{
private:
    QSize image_size;
    QList<UndoSnapshot> history;
    layergroup_t init_state;
    int history_index= 0;

public:
    Frame(Project* parent, QSize size);

    // ---- NOTE: Only the Project class should interface with these lower level methods ----
    QImage* InsertLayerAt(int pos, bool record= true);
    QImage* InsertLayer(bool record= true) { return InsertLayerAt(this->size(), record); }
    void RemoveLayer(int layer, bool record= true);
    // ---- ----
    void ReplaceLayer(QImage img, int index);
    void SetPalette(palette_t palette, bool record= false);
    void SetImageSize(QSize size);
    void SwapLayers(int index_a, int index_b);
    QImage* MergeLayerDown(int index, bool record= true);
    void Undo();
    void Redo();
    void ClearHistory();
    void RestoreInitialState();
    void PushNewSnapshot(UndoSnapshot* snapshot);

    QImage RenderBitmap();

    QImage* LayerAt(int layer);
    const QSize ImageSize() const { return image_size; }
    const int LayerCount() const { return this->size(); }
    int HistorySize() { return history.size(); }
    int HistoryIndex() { return history_index; }
};

typedef enum
{
    Tool_Pencil         = (1<<0),
    Tool_Eyedropper     = (1<<1),
    Tool_Rectangle      = (1<<2),
    Tool_Ellipsoid      = (1<<3),
    Tool_FloodFill      = (1<<4),
    Tool_FloodSelect    = (1<<5),
    Tool_RectSelect     = (1<<6),
    Tool_PencilSelect   = (1<<7),
    Tool_Transform      = (1<<8),
} tooltype_t;

typedef struct Tool
{
    tooltype_t type= Tool_Pencil;
    int diameter_a= 1;
    int diameter_b= 1;
    bool opaque_apply_mode= true;
    bool force_integer_scale= false;
    bool contour= false;
    bool fill= true;
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

class LayerProps
{
public:
    bool visible= true;
    QString name= ""; //Leave blank for automatic name "layer X"
};

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
    QList<LayerProps> layer_info;

    PalettePanel* UiPalettePanel() { return dckPaletteEdit ? *dckPaletteEdit : nullptr; }
    ToolPanel* UiToolPanel() { return dckToolPanel ? *dckToolPanel : nullptr; }
    LayerPanel* UiLayerPanel() { return dckLayerPanel ? *dckLayerPanel : nullptr; }

public:
    Project(MainWindow* parent, QSize size= QSize(64,64));
    bool LoadProject(QString filename); //Returns true on success
    bool SaveProject(QString filename); //Returns true on success
    bool ImportBitmap(QImage img, consent_t canvas_resize=Consent_Ask, consent_t import_palette=Consent_Ask); //Returns true on success
    bool SavePalette(QString filename, bool interactive= true); //Returns true on success
    bool LoadPalette(QString filename, bool interactive= true); //Returns true on success
    bool GoExport(QWidget* parent); //Returns true on success

    Frame* CurrentFrame() { return (Frame*)&(timeline.at(current_frame)); }
    Frame* FrameAt(int frame) { return (Frame*)&(timeline.at(frame)); }
    QImage* CurrentLayer() { return CurrentFrame()->LayerAt(current_layer); }
    const int CurrentFrameIndex() const { return current_frame; }
    const int CurrentLayerIndex() const { return current_layer; }
    LayerProps* LayerInfo(int layer) { return &layer_info[layer]; }
    LayerProps* CurrentLayerInfo() { return &layer_info[current_layer]; }
    const int FrameCount() const { return timeline.size(); }
    const QSize ImageSize() const { return image_size; }
    const palette_t Palette() const { return palette; }
    const bppformat_t BppFormat() const { return image_bpp; }
    const QString FileName() const { return filename; }
    const QString SharedPalette() const { return shared_palette_filename; }
    bool IsSaved() { return is_saved; }
    ImageCanvas* Canvas() { return canvas; }
    MainWindow* ParentWindow() { return main_window; }
    const QPoint PaltableAPosition() const { return paltable_Apos; }
    const QPoint PaltableBPosition() const { return paltable_Bpos; }
    const int PaltableAIndex() const { return paltable_Apos.x()+paltable_Apos.y()*PALETTE_W; }
    const int PaltableBIndex() const { return paltable_Bpos.x()+paltable_Bpos.y()*PALETTE_W; }
    Tool CurrentTool() { if (!UiToolPanel()) return Tool(); return UiToolPanel()->GetCurrentTool(); }

    void SetCurrentFrameIndex(int frame);
    void SetCurrentLayerIndex(int layer);
    void SetPaltableAPosition(QPoint pos);
    void SetPaltableBPosition(QPoint pos);
    void SetImageSize(QSize size);
    void SetPalette(palette_t palette= palette_t(), bool record= false); //Call with no arguments to just update the pal. on all frames
    void SetBppFormat(bppformat_t fmt);
    void SetFileName(QString filename);
    bool SetSharedPalette(QString filename); //Returns true on success | set to "" to disable
    void SetSaved(bool saved);
    void InsertFrameAt(int pos);
    void InsertFrame() { InsertFrameAt(timeline.size()); }
    void RemoveFrame(int frame);
    void InsertLayerAt(int pos, bool record= true);
    void InsertLayer(bool record= true) { InsertLayerAt(current_layer, record); }
    void RemoveLayer(int pos, bool record= true);
    void SwapColorIndex(int index_a, int index_b);
    void FillPaletteLinear(int index_a, int index_b, QRgb color);
    void FillPaletteRect(QPoint pos_a, QPoint pos_b, QRgb color);
    void UpdateLayerPanel() { UiLayerPanel()->Update(); }
    void SwapAllLayers(int index_a, int index_b) { _PRJ_FOREACH_FRAME FrameAt(ifr)->SwapLayers(index_a, index_b); SetPalette(); }
    void SetCurrentToolType(tooltype_t type) { UiToolPanel()->SetCurrentToolType(type); }
    void FixLayerDB();
};

#endif // PROJECT_H
