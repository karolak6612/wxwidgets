#ifndef RME_TERRAIN_BRUSH_PALETTE_TAB_H
#define RME_TERRAIN_BRUSH_PALETTE_TAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QXmlStreamReader>
#include <QStringList>
#include <QHash>
#include <QIcon>

// Forward declarations
namespace RME {
namespace core {
    namespace assets { class MaterialManager; }
    namespace brush { class BrushStateManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Data structure for a terrain brush entry
 */
struct TerrainBrushEntry {
    QString name;
    QString type; // "ground", "wall", "doodad"
    quint16 serverId = 0;
    quint16 zOrder = 0;
    QString xmlFile;
    QIcon icon;
    
    TerrainBrushEntry() = default;
    TerrainBrushEntry(const QString& brushName, const QString& brushType, quint16 serverLookId = 0)
        : name(brushName), type(brushType), serverId(serverLookId) {}
};

/**
 * @brief Terrain Brush palette tab for the main palette system
 * 
 * Provides UI for browsing and selecting terrain brushes (grounds, walls, doodads)
 * from XML definitions. Terrain brushes provide specialized painting behavior
 * with auto-bordering and material properties.
 */
class TerrainBrushPaletteTab : public QWidget {
    Q_OBJECT

public:
    explicit TerrainBrushPaletteTab(QWidget* parent = nullptr);
    ~TerrainBrushPaletteTab() override = default;

    // Integration with core systems
    void setMaterialManager(RME::core::assets::MaterialManager* materialManager);
    void setBrushStateManager(RME::core::brush::BrushStateManager* brushManager);
    void setEditorController(RME::core::editor::EditorControllerInterface* controller);

    // Public interface
    void refreshContent();
    void loadTerrainBrushesFromXml();
    QString getSelectedBrushName() const;
    QString getSelectedBrushType() const;

public slots:
    void onBrushTypeChanged();
    void onBrushSelectionChanged();
    void onBrushDoubleClicked(QListWidgetItem* item);
    void onSearchTextChanged(const QString& text);
    void onClearSearch();

signals:
    void brushSelected(const QString& brushName, const QString& brushType);
    void terrainBrushActivated(const QString& brushName, const QString& brushType);

private:
    void setupUI();
    void connectSignals();
    void parseGroundsXml();
    void parseWallsXml();
    void parseDoodadsFromTilesets();
    void populateBrushTypeCombo();
    void updateBrushList();
    void updateBrushList(const QString& typeFilter);
    void applySearchFilter(const QString& searchText);
    void activateTerrainBrush(const QString& brushName, const QString& brushType);
    
    // Helper methods
    QIcon getBrushIcon(const TerrainBrushEntry& entry) const;
    QString formatBrushListEntry(const TerrainBrushEntry& entry) const;
    void parseBrushFromXml(QXmlStreamReader& xml, const QString& brushType, const QString& xmlFile);

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QGroupBox* m_filterGroup = nullptr;
    QHBoxLayout* m_filterLayout = nullptr;
    QComboBox* m_brushTypeCombo = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_clearSearchButton = nullptr;
    
    QGroupBox* m_brushesGroup = nullptr;
    QVBoxLayout* m_brushesLayout = nullptr;
    QListWidget* m_brushList = nullptr;
    QLabel* m_brushCountLabel = nullptr;
    
    QGroupBox* m_infoGroup = nullptr;
    QVBoxLayout* m_infoLayout = nullptr;
    QLabel* m_selectedBrushLabel = nullptr;
    QLabel* m_brushDetailsLabel = nullptr;

    // Core system integration
    RME::core::assets::MaterialManager* m_materialManager = nullptr;
    RME::core::brush::BrushStateManager* m_brushStateManager = nullptr;
    RME::core::editor::EditorControllerInterface* m_editorController = nullptr;

    // Data
    QList<TerrainBrushEntry> m_terrainBrushes;
    QStringList m_brushTypes;
    QHash<QString, QList<TerrainBrushEntry>> m_brushesByType;
    QList<TerrainBrushEntry> m_filteredBrushes;
    
    // State
    QString m_currentBrushType;
    QString m_currentSearchText;
    bool m_updatingUI = false;
    
    // Constants
    static const QString ALL_TYPES_TEXT;
    static const QString GROUNDS_XML_PATH;
    static const QString WALLS_XML_PATH;
    static const QString TILESETS_XML_PATH;
    static const QString DOODADS_XML_PATH;
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_TERRAIN_BRUSH_PALETTE_TAB_H