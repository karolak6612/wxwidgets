#include "EditorInstanceWidget.h"
#include "widgets/MapView.h"
#include "core/map/Map.h"
#include "editor_logic/EditorController.h"
#include <QFileInfo>

namespace RME {
namespace ui {

EditorInstanceWidget::EditorInstanceWidget(RME::core::Map* map, 
                                           const QString& filePath,
                                           QWidget* parent)
    : QWidget(parent)
    , m_map(map)
    , m_filePath(filePath)
{
    // Determine if this is an untitled map
    m_isUntitled = filePath.isEmpty() || filePath == "Untitled" || !QFileInfo::exists(filePath);
    
    // Create undo stack
    m_undoStack = new QUndoStack(this);
    
    // Create editor controller
    m_editorController = new RME::editor_logic::EditorController(this);
    if (m_map) {
        m_editorController->setMap(m_map);
    }
    m_editorController->setUndoStack(m_undoStack);
    
    setupUI();
    connectSignals();
    updateDisplayName();
}

EditorInstanceWidget::~EditorInstanceWidget()
{
    // Map cleanup will be handled by the owner of this widget
    // Other components are cleaned up by Qt's parent-child system
}

void EditorInstanceWidget::setFilePath(const QString& filePath)
{
    if (m_filePath != filePath) {
        m_filePath = filePath;
        m_isUntitled = filePath.isEmpty() || filePath == "Untitled" || !QFileInfo::exists(filePath);
        updateDisplayName();
    }
}

QString EditorInstanceWidget::getDisplayName() const
{
    QString name;
    
    if (m_isUntitled) {
        name = "Untitled";
    } else {
        QFileInfo fileInfo(m_filePath);
        name = fileInfo.fileName();
    }
    
    if (m_isModified) {
        name += "*";
    }
    
    return name;
}

bool EditorInstanceWidget::isModified() const
{
    return m_isModified;
}

bool EditorInstanceWidget::isUntitled() const
{
    return m_isUntitled;
}

void EditorInstanceWidget::setAppSettings(RME::core::settings::AppSettings* settings)
{
    m_appSettings = settings;
    if (m_mapView) {
        m_mapView->setAppSettings(settings);
    }
}

void EditorInstanceWidget::setAssetManager(RME::core::assets::AssetManager* assetManager)
{
    m_assetManager = assetManager;
    if (m_mapView) {
        m_mapView->setAssetManager(assetManager);
    }
}

void EditorInstanceWidget::setTextureManager(RME::core::sprites::TextureManager* textureManager)
{
    m_textureManager = textureManager;
    if (m_mapView) {
        m_mapView->setTextureManager(textureManager);
    }
}

void EditorInstanceWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create MapView
    m_mapView = new RME::ui::widgets::MapView(this);
    m_mapView->setMap(m_map);
    m_mapView->setEditorController(m_editorController);
    
    // Set dependencies if available
    if (m_appSettings) {
        m_mapView->setAppSettings(m_appSettings);
    }
    if (m_assetManager) {
        m_mapView->setAssetManager(m_assetManager);
    }
    if (m_textureManager) {
        m_mapView->setTextureManager(m_textureManager);
    }
    
    m_mainLayout->addWidget(m_mapView);
}

void EditorInstanceWidget::connectSignals()
{
    // Connect undo stack signals
    connect(m_undoStack, &QUndoStack::cleanChanged,
            this, &EditorInstanceWidget::onUndoStackCleanChanged);
    
    // Connect map modification signals
    if (m_map) {
        // TODO: Connect to map modification signals when available
        // connect(m_map, &RME::core::Map::modified, this, &EditorInstanceWidget::onMapModified);
    }
    
    // Connect editor controller signals
    if (m_editorController) {
        // TODO: Connect to editor controller signals when available
    }
}

void EditorInstanceWidget::updateDisplayName()
{
    QString newName = getDisplayName();
    emit displayNameChanged(newName);
}

void EditorInstanceWidget::onMapModified()
{
    if (!m_isModified) {
        m_isModified = true;
        updateDisplayName();
        emit modificationChanged(true);
    }
}

void EditorInstanceWidget::onUndoStackCleanChanged(bool clean)
{
    bool wasModified = m_isModified;
    m_isModified = !clean;
    
    if (wasModified != m_isModified) {
        updateDisplayName();
        emit modificationChanged(m_isModified);
    }
}

} // namespace ui
} // namespace RME