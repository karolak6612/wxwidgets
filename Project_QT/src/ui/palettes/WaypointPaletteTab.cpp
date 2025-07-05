#include "WaypointPaletteTab.h"
#include "core/waypoints/WaypointManager.h"
#include "core/waypoints/Waypoint.h"
#include "core/brush/BrushStateManager.h"
#include "core/brush/WaypointBrush.h"
#include "core/Position.h"
#include <QApplication>

namespace RME {
namespace ui {
namespace palettes {

WaypointPaletteTab::WaypointPaletteTab(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
}

void WaypointPaletteTab::setWaypointManager(RME::core::waypoints::WaypointManager* waypointManager)
{
    m_waypointManager = waypointManager;
    refreshContent();
}

void WaypointPaletteTab::setBrushStateManager(RME::core::brush::BrushStateManager* brushManager)
{
    m_brushStateManager = brushManager;
}

void WaypointPaletteTab::setEditorController(RME::core::editor::EditorControllerInterface* controller)
{
    m_editorController = controller;
}

void WaypointPaletteTab::refreshContent()
{
    reloadWaypoints();
}

void WaypointPaletteTab::reloadWaypoints()
{
    updateWaypointList();
}

QString WaypointPaletteTab::getSelectedWaypointName() const
{
    QListWidgetItem* currentItem = m_waypointList->currentItem();
    if (!currentItem) {
        return QString();
    }
    
    return currentItem->data(Qt::UserRole).toString();
}

void WaypointPaletteTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Waypoint list
    m_waypointList = new QListWidget(this);
    m_waypointList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_mainLayout->addWidget(m_waypointList);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_addWaypointButton = new QPushButton("Add Waypoint", this);
    m_removeWaypointButton = new QPushButton("Remove Waypoint", this);
    
    m_buttonLayout->addWidget(m_addWaypointButton);
    m_buttonLayout->addWidget(m_removeWaypointButton);
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Initial state
    m_removeWaypointButton->setEnabled(false);
}

void WaypointPaletteTab::connectSignals()
{
    connect(m_waypointList, &QListWidget::itemSelectionChanged,
            this, &WaypointPaletteTab::onWaypointSelectionChanged);
    
    connect(m_waypointList, &QListWidget::itemChanged,
            this, &WaypointPaletteTab::onWaypointItemChanged);
    
    connect(m_addWaypointButton, &QPushButton::clicked,
            this, &WaypointPaletteTab::onAddWaypoint);
    
    connect(m_removeWaypointButton, &QPushButton::clicked,
            this, &WaypointPaletteTab::onRemoveWaypoint);
}

void WaypointPaletteTab::updateWaypointList()
{
    if (!m_waypointManager) return;
    
    m_updatingUI = true;
    m_waypointList->clear();
    
    auto waypoints = m_waypointManager->getAllWaypoints();
    for (const auto* waypoint : waypoints) {
        if (!waypoint) continue;
        
        QListWidgetItem* item = new QListWidgetItem(waypoint->getName());
        item->setData(Qt::UserRole, waypoint->getName());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        m_waypointList->addItem(item);
    }
    
    // Sort the list
    m_waypointList->sortItems();
    
    m_updatingUI = false;
    onWaypointSelectionChanged(); // Update button states
}

void WaypointPaletteTab::updateBrushState()
{
    if (!m_brushStateManager) return;
    
    QString selectedWaypointName = getSelectedWaypointName();
    if (selectedWaypointName.isEmpty()) return;
    
    // Set waypoint brush as active with selected waypoint
    auto* waypointBrush = dynamic_cast<RME::core::brush::WaypointBrush*>(
        m_brushStateManager->getBrush("WaypointBrush"));
    
    if (waypointBrush) {
        waypointBrush->setCurrentWaypoint(selectedWaypointName);
        m_brushStateManager->setActiveBrush("WaypointBrush");
    }
}

void WaypointPaletteTab::selectWaypointInList(const QString& waypointName)
{
    for (int i = 0; i < m_waypointList->count(); ++i) {
        QListWidgetItem* item = m_waypointList->item(i);
        if (item && item->data(Qt::UserRole).toString() == waypointName) {
            m_waypointList->setCurrentItem(item);
            break;
        }
    }
}

QString WaypointPaletteTab::generateUniqueWaypointName() const
{
    if (!m_waypointManager) return "Waypoint";
    
    QString baseName = "Waypoint";
    QString name = baseName;
    int counter = 1;
    
    while (m_waypointManager->getWaypointByName(name) != nullptr) {
        name = QString("%1 %2").arg(baseName).arg(counter);
        counter++;
    }
    
    return name;
}

bool WaypointPaletteTab::validateWaypointName(const QString& name, const QString& originalName) const
{
    if (!m_waypointManager) return false;
    
    // Check if name is empty
    if (name.trimmed().isEmpty()) {
        return false;
    }
    
    // Check if name is unique (unless it's the same as original)
    if (name != originalName) {
        auto* existingWaypoint = m_waypointManager->getWaypointByName(name);
        if (existingWaypoint != nullptr) {
            return false;
        }
    }
    
    return true;
}

QList<QString> WaypointPaletteTab::getSelectedWaypointNames() const
{
    QList<QString> waypointNames;
    auto selectedItems = m_waypointList->selectedItems();
    
    for (const auto* item : selectedItems) {
        waypointNames.append(item->data(Qt::UserRole).toString());
    }
    
    return waypointNames;
}

void WaypointPaletteTab::onWaypointSelectionChanged()
{
    auto selectedItems = m_waypointList->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();
    bool singleSelection = selectedItems.size() == 1;
    
    m_removeWaypointButton->setEnabled(hasSelection);
    
    if (singleSelection) {
        QString waypointName = getSelectedWaypointName();
        emit waypointSelected(waypointName);
        updateBrushState();
        
        // Navigate to waypoint position
        if (m_waypointManager) {
            auto* waypoint = m_waypointManager->getWaypointByName(waypointName);
            if (waypoint) {
                emit navigateToWaypoint(waypoint->getPosition());
            }
        }
    }
}

void WaypointPaletteTab::onWaypointItemChanged(QListWidgetItem* item)
{
    if (m_updatingUI || !item || !m_waypointManager) return;
    
    QString newName = item->text().trimmed();
    QString originalName = item->data(Qt::UserRole).toString();
    
    // Validate new name
    if (!validateWaypointName(newName, originalName)) {
        // Revert to original name
        m_updatingUI = true;
        item->setText(originalName);
        m_updatingUI = false;
        
        if (newName.isEmpty()) {
            QMessageBox::warning(this, "Invalid Name", "Waypoint name cannot be empty.");
        } else {
            QMessageBox::warning(this, "Invalid Name", 
                                 QString("A waypoint named '%1' already exists.").arg(newName));
        }
        return;
    }
    
    // Update waypoint name in manager
    auto* waypoint = m_waypointManager->getWaypointByName(originalName);
    if (waypoint) {
        // Remove old waypoint and add with new name
        RME::core::Position pos = waypoint->getPosition();
        m_waypointManager->removeWaypoint(originalName);
        m_waypointManager->addWaypoint(newName, pos);
        
        // Update item data
        item->setData(Qt::UserRole, newName);
        
        // Update brush state if this waypoint is selected
        updateBrushState();
    }
}

void WaypointPaletteTab::onAddWaypoint()
{
    if (!m_waypointManager) return;
    
    // Generate unique name
    QString waypointName = generateUniqueWaypointName();
    
    // Create waypoint at default position (0, 0, 7)
    RME::core::Position defaultPos(0, 0, 7);
    if (m_waypointManager->addWaypoint(waypointName, defaultPos)) {
        // Refresh list and select new waypoint
        reloadWaypoints();
        selectWaypointInList(waypointName);
        
        // Start editing the name
        QListWidgetItem* item = m_waypointList->currentItem();
        if (item) {
            m_waypointList->editItem(item);
        }
    } else {
        QMessageBox::warning(this, "Error", "Failed to create waypoint.");
    }
}

void WaypointPaletteTab::onRemoveWaypoint()
{
    auto selectedWaypointNames = getSelectedWaypointNames();
    if (selectedWaypointNames.isEmpty() || !m_waypointManager) return;
    
    QString message;
    if (selectedWaypointNames.size() == 1) {
        message = QString("Are you sure you want to remove waypoint '%1'?")
                  .arg(selectedWaypointNames.first());
    } else {
        message = QString("Are you sure you want to remove %1 waypoints?")
                  .arg(selectedWaypointNames.size());
    }
    
    int result = QMessageBox::question(this, "Confirm Removal", message,
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        for (const QString& waypointName : selectedWaypointNames) {
            m_waypointManager->removeWaypoint(waypointName);
        }
        reloadWaypoints();
    }
}

} // namespace palettes
} // namespace ui
} // namespace RME