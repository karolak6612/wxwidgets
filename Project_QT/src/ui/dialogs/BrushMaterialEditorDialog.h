#ifndef RME_BRUSH_MATERIAL_EDITOR_DIALOG_H
#define RME_BRUSH_MATERIAL_EDITOR_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QTextStream>

// Forward declarations
namespace RME {
namespace core {
    class Map;
}
}

namespace RME {
namespace ui {
namespace dialogs {

// Enums for border positions (from wxWidgets implementation)
enum class BorderPosition {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3,
    CORNER_NW = 4,
    CORNER_NE = 5,
    CORNER_SE = 6,
    CORNER_SW = 7,
    DIAGONAL_NW = 8,
    DIAGONAL_NE = 9,
    DIAGONAL_SE = 10,
    DIAGONAL_SW = 11
};

// Border item structure
struct BorderItem {
    BorderPosition position;
    uint16_t itemId = 0;
    
    BorderItem() = default;
    BorderItem(BorderPosition pos, uint16_t id) : position(pos), itemId(id) {}
};

// Ground item structure (for ground brushes)
struct GroundItem {
    uint16_t itemId = 0;
    int chance = 100;
    
    GroundItem() = default;
    GroundItem(uint16_t id, int ch) : itemId(id), chance(ch) {}
};

/**
 * @brief Custom widget for visual border grid editing
 * 
 * Displays a 3x3 grid representing the 12 border positions and allows
 * clicking to select positions for item assignment.
 */
class BorderGridEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit BorderGridEditorWidget(QWidget* parent = nullptr);
    ~BorderGridEditorWidget() override = default;

    void setItemForPosition(BorderPosition pos, uint16_t itemId);
    uint16_t getItemForPosition(BorderPosition pos) const;
    void clearAllItems();
    BorderPosition getSelectedPosition() const { return m_selectedPosition; }
    void setSelectedPosition(BorderPosition pos);

signals:
    void positionSelected(BorderPosition pos);
    void itemChanged(BorderPosition pos, uint16_t itemId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QVector<BorderItem> m_borderItems;
    BorderPosition m_selectedPosition = BorderPosition::NORTH;
    QSize m_cellSize;
    QRect m_gridRect;
    
    void updateLayout();
    QRect getCellRect(BorderPosition pos) const;
    BorderPosition getPositionFromPoint(const QPoint& point) const;
    QString getPositionName(BorderPosition pos) const;
    void drawCell(QPainter& painter, BorderPosition pos, const QRect& rect, bool selected) const;
};

/**
 * @brief Custom widget for border preview
 * 
 * Shows a 5x5 preview of how the border will look when applied.
 */
class BorderPreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit BorderPreviewWidget(QWidget* parent = nullptr);
    ~BorderPreviewWidget() override = default;

    void updatePreview(const QVector<BorderItem>& items);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<BorderItem> m_previewItems;
    QSize m_cellSize;
    
    void drawPreviewGrid(QPainter& painter) const;
    uint16_t getItemForPreviewPosition(int x, int y) const;
};

/**
 * @brief Main Brush & Material Editor Dialog
 * 
 * Comprehensive editor for borders, ground brushes, wall brushes, and doodad brushes.
 * Uses a tabbed interface to organize different editing modes.
 */
class BrushMaterialEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit BrushMaterialEditorDialog(QWidget* parent = nullptr,
                                 RME::core::assets::MaterialManager* materialManager = nullptr,
                                 RME::core::assets::ItemDatabase* itemDatabase = nullptr);
    ~BrushMaterialEditorDialog() override = default;

public slots:
    void accept() override;
    void reject() override;

private slots:
    // Borders tab slots
    void onBorderPositionSelected(BorderPosition pos);
    void onBorderItemIdChanged();
    void onBrowseBorderItem();
    void onApplyBorderItem();
    void onLoadBorder();
    void onSaveBorder();
    void onClearBorderGrid();
    void onBorderPropertyChanged();

    // Ground brushes tab slots
    void onAddGroundItem();
    void onRemoveGroundItem();
    void onEditGroundItem();
    void onBrowseGroundItem();
    void onLoadGroundBrush();
    void onSaveGroundBrush();
    void onGroundPropertyChanged();

    // Wall brushes tab slots
    void onLoadWallBrush();
    void onSaveWallBrush();
    void onWallPropertyChanged();

    // Doodad brushes tab slots
    void onAddDoodadItem();
    void onRemoveDoodadItem();
    void onEditDoodadItem();
    void onLoadDoodadBrush();
    void onSaveDoodadBrush();
    void onDoodadPropertyChanged();

    // Helper slots
    void onTabChanged(int index);

signals:
    void borderSaved(int borderId);
    void groundBrushSaved(const QString& brushName);
    void wallBrushSaved(const QString& brushName);
    void doodadBrushSaved(const QString& brushName);

private:
    // Core data
    const RME::core::Map* m_map = nullptr;
    bool m_wasModified = false;

    // UI components
    QTabWidget* m_tabWidget = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;

    // Borders tab
    QWidget* m_bordersTab = nullptr;
    QSplitter* m_bordersSplitter = nullptr;
    BorderGridEditorWidget* m_borderGrid = nullptr;
    BorderPreviewWidget* m_borderPreview = nullptr;
    QLineEdit* m_borderNameEdit = nullptr;
    QSpinBox* m_borderIdSpin = nullptr;
    QSpinBox* m_groupIdSpin = nullptr;
    QCheckBox* m_optionalCheck = nullptr;
    QCheckBox* m_groundBorderCheck = nullptr;
    QComboBox* m_borderCombo = nullptr;
    QSpinBox* m_borderItemIdSpin = nullptr;
    QPushButton* m_browseBorderItemButton = nullptr;
    QPushButton* m_applyBorderItemButton = nullptr;
    QPushButton* m_saveBorderButton = nullptr;
    QPushButton* m_clearBorderButton = nullptr;
    QLabel* m_selectedPositionLabel = nullptr;

    // Ground brushes tab
    QWidget* m_groundTab = nullptr;
    QLineEdit* m_brushNameEdit = nullptr;
    QSpinBox* m_serverLookIdSpin = nullptr;
    QSpinBox* m_zOrderSpin = nullptr;
    QTableWidget* m_groundItemsTable = nullptr;
    QComboBox* m_tilesetCombo = nullptr;
    QComboBox* m_groundBrushCombo = nullptr;
    QPushButton* m_addGroundItemButton = nullptr;
    QPushButton* m_removeGroundItemButton = nullptr;
    QPushButton* m_editGroundItemButton = nullptr;
    QPushButton* m_saveGroundBrushButton = nullptr;
    
    // Border association controls
    QSpinBox* m_borderAssocIdSpin = nullptr;
    QComboBox* m_borderAlignmentCombo = nullptr;
    QCheckBox* m_includeToNoneCheck = nullptr;
    QCheckBox* m_includeInnerCheck = nullptr;

    // Wall brushes tab
    QWidget* m_wallTab = nullptr;
    QLineEdit* m_wallBrushNameEdit = nullptr;
    QSpinBox* m_wallServerLookIdSpin = nullptr;
    QComboBox* m_wallTilesetCombo = nullptr;
    QComboBox* m_wallBrushCombo = nullptr;
    QSpinBox* m_horizontalWallSpin = nullptr;
    QSpinBox* m_verticalWallSpin = nullptr;
    QSpinBox* m_wallPoleSpin = nullptr;
    QPushButton* m_saveWallBrushButton = nullptr;

    // Doodad brushes tab
    QWidget* m_doodadTab = nullptr;
    QLineEdit* m_doodadBrushNameEdit = nullptr;
    QSpinBox* m_doodadServerLookIdSpin = nullptr;
    QComboBox* m_doodadTilesetCombo = nullptr;
    QComboBox* m_doodadBrushCombo = nullptr;
    QTableWidget* m_doodadItemsTable = nullptr;
    QCheckBox* m_draggableCheck = nullptr;
    QCheckBox* m_blockingCheck = nullptr;
    QPushButton* m_addDoodadItemButton = nullptr;
    QPushButton* m_removeDoodadItemButton = nullptr;
    QPushButton* m_editDoodadItemButton = nullptr;
    QPushButton* m_saveDoodadBrushButton = nullptr;

    // Helper methods
    void setupUI();
    void setupBordersTab();
    void setupGroundBrushesTab();
    void setupWallBrushesTab();
    void setupDoodadBrushesTab();
    void setupButtonBox();
    
    // Data management
    RME::core::assets::MaterialManager* m_materialManager;
    RME::core::assets::ItemDatabase* m_itemDatabase;
    
    // XML file paths
    QString getXmlFilePath(const QString& filename) const;
    bool ensureXmlDirectoryExists() const;
    
    void loadData();
    void loadExistingBorders();
    void loadExistingGroundBrushes();
    void loadExistingWallBrushes();
    void loadExistingDoodadBrushes();
    void loadTilesets();
    
    void connectSignals();
    void markAsModified();
    
    // Validation
    bool validateBorderData();
    bool validateGroundBrushData();
    bool validateWallBrushData();
    bool validateDoodadBrushData();
    
    // Data management
    void clearBorderData();
    void clearGroundBrushData();
    void clearWallBrushData();
    void clearDoodadBrushData();
    
    // Helper methods for item management
    void updateGroundItemsTable();
    void updateDoodadItemsTable();
    QString getItemName(uint16_t itemId) const;
    
    // XML operations
    bool saveBorderToXml();
    bool saveGroundBrushToXml();
    bool saveWallBrushToXml();
    bool saveDoodadBrushToXml();
    
    // XML loading operations
    bool loadBorderFromXml(const QString& borderName);
    bool loadGroundBrushFromXml(const QString& brushName);
    bool loadWallBrushFromXml(const QString& brushName);
    bool loadDoodadBrushFromXml(const QString& brushName);
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_BRUSH_MATERIAL_EDITOR_DIALOG_H