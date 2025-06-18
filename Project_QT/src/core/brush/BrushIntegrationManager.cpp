#include "core/brush/BrushIntegrationManager.h"
#include "core/brush/HouseBrush.h"
#include "core/brush/HouseExitBrush.h"
#include "core/brush/WaypointBrush.h"
#include "core/editor/EditorControllerInterface.h"
#include "editor_logic/EditorController.h" // For ToolMode enum

#include <QDebug>

namespace RME {
namespace core {
namespace brush {

BrushIntegrationManager::BrushIntegrationManager(RME::core::editor::EditorControllerInterface* editorController,
                                               QObject* parent)
    : QObject(parent)
    , m_editorController(editorController)
{
    Q_ASSERT(m_editorController);
    initializeBrushes();
}

void BrushIntegrationManager::initializeBrushes() {
    // Create brush instances
    m_houseBrush = std::make_unique<HouseBrush>();
    m_houseExitBrush = std::make_unique<HouseExitBrush>();
    m_waypointBrush = std::make_unique<WaypointBrush>();
    
    qDebug() << "BrushIntegrationManager: Initialized all brush instances";
}

void BrushIntegrationManager::activateHouseBrush(quint32 houseId) {
    if (houseId == 0) {
        qWarning("BrushIntegrationManager::activateHouseBrush: Invalid house ID (0)");
        return;
    }
    
    m_currentHouseId = houseId;
    m_houseBrush->setCurrentHouseId(houseId);
    
    // Set editor controller to brush mode and activate house brush
    if (auto* editorController = dynamic_cast<RME::editor_logic::EditorController*>(m_editorController)) {
        editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Brush);
    }
    
    emit brushActivated(m_houseBrush.get());
    emit houseToolConfigured(houseId);
    
    qDebug() << "BrushIntegrationManager::activateHouseBrush: Activated house brush for house ID" << houseId;
}

void BrushIntegrationManager::activateHouseExitTool(quint32 houseId) {
    if (houseId == 0) {
        qWarning("BrushIntegrationManager::activateHouseExitTool: Invalid house ID (0)");
        return;
    }
    
    m_currentHouseId = houseId;
    m_houseExitBrush->setCurrentHouseId(houseId);
    
    // Set editor controller to house exit tool mode
    if (auto* editorController = dynamic_cast<RME::editor_logic::EditorController*>(m_editorController)) {
        editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::HouseExit);
        editorController->setCurrentHouseForTools(houseId);
    }
    
    emit toolModeChanged(static_cast<int>(RME::editor_logic::EditorController::ToolMode::HouseExit));
    emit houseToolConfigured(houseId);
    
    qDebug() << "BrushIntegrationManager::activateHouseExitTool: Activated house exit tool for house ID" << houseId;
}

void BrushIntegrationManager::activateWaypointTool(const QString& waypointName) {
    if (waypointName.isEmpty()) {
        qWarning("BrushIntegrationManager::activateWaypointTool: Waypoint name is empty");
        return;
    }
    
    m_currentWaypointName = waypointName;
    m_waypointBrush->setCurrentWaypoint(waypointName);
    
    // Set editor controller to waypoint tool mode
    if (auto* editorController = dynamic_cast<RME::editor_logic::EditorController*>(m_editorController)) {
        editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Waypoint);
        editorController->setCurrentWaypointForTools(waypointName);
    }
    
    emit toolModeChanged(static_cast<int>(RME::editor_logic::EditorController::ToolMode::Waypoint));
    emit waypointToolConfigured(waypointName);
    
    qDebug() << "BrushIntegrationManager::activateWaypointTool: Activated waypoint tool for waypoint" << waypointName;
}

void BrushIntegrationManager::activateRegularBrush(RME::core::brush::Brush* brush) {
    if (!brush) {
        qWarning("BrushIntegrationManager::activateRegularBrush: Brush is null");
        return;
    }
    
    // Clear current house/waypoint selections
    m_currentHouseId = 0;
    m_currentWaypointName.clear();
    
    // Set editor controller to brush mode
    if (auto* editorController = dynamic_cast<RME::editor_logic::EditorController*>(m_editorController)) {
        editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Brush);
    }
    
    emit brushActivated(brush);
    emit toolModeChanged(static_cast<int>(RME::editor_logic::EditorController::ToolMode::Brush));
    
    qDebug() << "BrushIntegrationManager::activateRegularBrush: Activated regular brush" << brush->getName();
}

// State query methods
bool BrushIntegrationManager::isHouseBrushActive() const {
    if (auto* editorController = dynamic_cast<const RME::editor_logic::EditorController*>(m_editorController)) {
        return editorController->getToolMode() == RME::editor_logic::EditorController::ToolMode::Brush && 
               m_currentHouseId != 0;
    }
    return false;
}

bool BrushIntegrationManager::isHouseExitToolActive() const {
    if (auto* editorController = dynamic_cast<const RME::editor_logic::EditorController*>(m_editorController)) {
        return editorController->getToolMode() == RME::editor_logic::EditorController::ToolMode::HouseExit;
    }
    return false;
}

bool BrushIntegrationManager::isWaypointToolActive() const {
    if (auto* editorController = dynamic_cast<const RME::editor_logic::EditorController*>(m_editorController)) {
        return editorController->getToolMode() == RME::editor_logic::EditorController::ToolMode::Waypoint;
    }
    return false;
}

quint32 BrushIntegrationManager::getCurrentHouseId() const {
    return m_currentHouseId;
}

QString BrushIntegrationManager::getCurrentWaypointName() const {
    return m_currentWaypointName;
}

// Brush instance getters
RME::core::brush::HouseBrush* BrushIntegrationManager::getHouseBrush() {
    return m_houseBrush.get();
}

RME::core::brush::HouseExitBrush* BrushIntegrationManager::getHouseExitBrush() {
    return m_houseExitBrush.get();
}

RME::core::brush::WaypointBrush* BrushIntegrationManager::getWaypointBrush() {
    return m_waypointBrush.get();
}

// UI integration slots
void BrushIntegrationManager::onHouseSelected(quint32 houseId) {
    activateHouseBrush(houseId);
}

void BrushIntegrationManager::onHouseDeselected() {
    m_currentHouseId = 0;
    
    // Switch back to regular brush mode
    if (auto* editorController = dynamic_cast<RME::editor_logic::EditorController*>(m_editorController)) {
        editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Brush);
        editorController->setCurrentHouseForTools(0);
    }
    
    emit toolModeChanged(static_cast<int>(RME::editor_logic::EditorController::ToolMode::Brush));
    
    qDebug() << "BrushIntegrationManager::onHouseDeselected: Deactivated house tools";
}

void BrushIntegrationManager::onWaypointSelected(const QString& waypointName) {
    activateWaypointTool(waypointName);
}

void BrushIntegrationManager::onWaypointDeselected() {
    m_currentWaypointName.clear();
    
    // Switch back to regular brush mode
    if (auto* editorController = dynamic_cast<RME::editor_logic::EditorController*>(m_editorController)) {
        editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Brush);
        editorController->setCurrentWaypointForTools(QString());
    }
    
    emit toolModeChanged(static_cast<int>(RME::editor_logic::EditorController::ToolMode::Brush));
    
    qDebug() << "BrushIntegrationManager::onWaypointDeselected: Deactivated waypoint tool";
}

void BrushIntegrationManager::onBrushModeRequested() {
    // Clear all tool-specific selections and switch to brush mode
    m_currentHouseId = 0;
    m_currentWaypointName.clear();
    
    if (auto* editorController = dynamic_cast<RME::editor_logic::EditorController*>(m_editorController)) {
        editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Brush);
        editorController->setCurrentHouseForTools(0);
        editorController->setCurrentWaypointForTools(QString());
    }
    
    emit toolModeChanged(static_cast<int>(RME::editor_logic::EditorController::ToolMode::Brush));
    
    qDebug() << "BrushIntegrationManager::onBrushModeRequested: Switched to regular brush mode";
}

} // namespace brush
} // namespace core
} // namespace RME

#include "BrushIntegrationManager.moc"