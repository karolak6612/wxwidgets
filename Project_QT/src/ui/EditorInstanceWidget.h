#ifndef RME_EDITOR_INSTANCE_WIDGET_H
#define RME_EDITOR_INSTANCE_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QUndoStack>
#include <QString>
#include <QFileInfo>

// Forward declarations
namespace RME {
namespace core {
    class Map;
    namespace settings { class AppSettings; }
    namespace assets { class AssetManager; }
    namespace sprites { class TextureManager; }
}
namespace editor_logic {
    class EditorController;
}
namespace ui {
namespace widgets {
    class MapView;
}
}
}

namespace RME {
namespace ui {

/**
 * @brief Widget representing a single map editor instance
 * 
 * This widget contains all the components needed for editing a single map:
 * - MapView for rendering and interaction
 * - EditorController for handling editor operations
 * - QUndoStack for undo/redo functionality
 * - Map data and associated metadata
 * 
 * Each tab in the main editor window contains one EditorInstanceWidget.
 */
class EditorInstanceWidget : public QWidget {
    Q_OBJECT

public:
    explicit EditorInstanceWidget(RME::core::Map* map, 
                                  const QString& filePath,
                                  QWidget* parent = nullptr);
    ~EditorInstanceWidget() override;

    // Prevent copying
    EditorInstanceWidget(const EditorInstanceWidget&) = delete;
    EditorInstanceWidget& operator=(const EditorInstanceWidget&) = delete;

    // Accessors
    RME::ui::widgets::MapView* getMapView() const { return m_mapView; }
    RME::editor_logic::EditorController* getEditorController() const { return m_editorController; }
    RME::core::Map* getMap() const { return m_map; }
    QUndoStack* getUndoStack() const { return m_undoStack; }
    
    // File management
    QString getFilePath() const { return m_filePath; }
    void setFilePath(const QString& filePath);
    QString getDisplayName() const;
    bool isModified() const;
    bool isUntitled() const;
    
    // Integration
    void setAppSettings(RME::core::settings::AppSettings* settings);
    void setAssetManager(RME::core::assets::AssetManager* assetManager);
    void setTextureManager(RME::core::sprites::TextureManager* textureManager);

public slots:
    void onMapModified();
    void onUndoStackCleanChanged(bool clean);

signals:
    void modificationChanged(bool modified);
    void displayNameChanged(const QString& name);
    void requestClose();

private:
    void setupUI();
    void connectSignals();
    void updateDisplayName();

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    RME::ui::widgets::MapView* m_mapView = nullptr;

    // Core components
    RME::core::Map* m_map = nullptr;
    RME::editor_logic::EditorController* m_editorController = nullptr;
    QUndoStack* m_undoStack = nullptr;

    // File information
    QString m_filePath;
    bool m_isUntitled = false;
    bool m_isModified = false;

    // External dependencies
    RME::core::settings::AppSettings* m_appSettings = nullptr;
    RME::core::assets::AssetManager* m_assetManager = nullptr;
    RME::core::sprites::TextureManager* m_textureManager = nullptr;
};

} // namespace ui
} // namespace RME

#endif // RME_EDITOR_INSTANCE_WIDGET_H