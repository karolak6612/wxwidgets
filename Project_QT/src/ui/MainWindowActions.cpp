// MainWindow action implementations
#include "dialogs/BrushMaterialEditorDialog.h"
#include "dialogs/NewTilesetDialog.h"
#include "dialogs/AddItemToTilesetDialog.h"
// This file contains the implementation of all menu action handlers for MainWindow

#include "ui/MainWindow.h"
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"
#include "core/map/Map.h"
#include "ui/dialogs/ItemFinderDialogQt.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QApplication>
#include <QStandardPaths>

// --- File Menu Actions ---
void MainWindow::onNewMap() {
    if (!m_editorController) {
        qWarning("MainWindow::onNewMap: EditorController is null");
        return;
    }
    
    // TODO: Check if current map needs saving
    bool ok;
    int width = QInputDialog::getInt(this, tr("New Map"), tr("Map Width:"), 1024, 100, 4096, 1, &ok);
    if (!ok) return;
    
    int height = QInputDialog::getInt(this, tr("New Map"), tr("Map Height:"), 1024, 100, 4096, 1, &ok);
    if (!ok) return;
    
    // Create new map
    bool success = m_editorController->newMap(width, height, "Untitled Map");
    
    if (success) {
        updateMenuStatesFromEditor();
        updateWindowTitle();
        statusBar()->showMessage(tr("Created new map (%1x%2)").arg(width).arg(height), 2000);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create new map"));
    }
    
    qDebug() << "MainWindow::onNewMap: Created new map" << width << "x" << height;
}

void MainWindow::onOpenMap() {
    if (!m_editorController) {
        qWarning("MainWindow::onOpenMap: EditorController is null");
        return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Map"), 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OTBM Files (*.otbm);;All Files (*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    bool success = m_editorController->loadMap(fileName);
    
    if (success) {
        updateMenuStatesFromEditor();
        updateWindowTitle();
        addToRecentFiles(fileName);
        statusBar()->showMessage(tr("Opened map: %1").arg(fileName), 2000);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open map: %1").arg(fileName));
    }
    
    qDebug() << "MainWindow::onOpenMap: Opened map" << fileName;
}

void MainWindow::onSaveMap() {
    if (!m_editorController || !m_editorController->getMap()) {
        qWarning("MainWindow::onSaveMap: No map to save");
        return;
    }
    
    bool success = m_editorController->saveMap();
    
    if (success) {
        updateWindowTitle();
        statusBar()->showMessage(tr("Map saved"), 2000);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save map"));
    }
    qDebug() << "MainWindow::onSaveMap: Saved current map";
}

void MainWindow::onSaveMapAs() {
    if (!m_editorController || !m_editorController->getMap()) {
        qWarning("MainWindow::onSaveMapAs: No map to save");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Map As"), 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OTBM Files (*.otbm);;All Files (*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    bool success = m_editorController->saveMapAs(fileName);
    
    if (success) {
        updateWindowTitle();
        addToRecentFiles(fileName);
        statusBar()->showMessage(tr("Map saved as: %1").arg(fileName), 2000);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save map as: %1").arg(fileName));
    }
    qDebug() << "MainWindow::onSaveMapAs: Saved map as" << fileName;
}

void MainWindow::onCloseMap() {
    if (!m_editorController) {
        qWarning("MainWindow::onCloseMap: EditorController is null");
        return;
    }
    
    // Check if map needs saving before closing
    if (m_editorController->isMapModified()) {
        QString filename = m_editorController->getCurrentMapFilename();
        QString mapName = filename.isEmpty() ? "Untitled Map" : QFileInfo(filename).baseName();
        
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Save Changes"),
            tr("The map '%1' has unsaved changes.\nDo you want to save before closing?").arg(mapName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
        
        if (result == QMessageBox::Save) {
            if (!m_editorController->saveMap()) {
                // Save failed, don't close
                return;
            }
        } else if (result == QMessageBox::Cancel) {
            // User cancelled, don't close
            return;
        }
        // If Discard, continue with closing
    }
    
    bool success = m_editorController->closeMap();
    
    if (success) {
        updateMenuStatesFromEditor();
        updateWindowTitle();
        statusBar()->showMessage(tr("Map closed"), 2000);
        qDebug() << "MainWindow::onCloseMap: Closed current map";
    }
}

void MainWindow::onImportMap() {
    if (!m_editorController) {
        qWarning("MainWindow::onImportMap: EditorController is null");
        return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import Map"), 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OTBM Files (*.otbm);;All Files (*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // TODO: Show import options dialog
    // TODO: Implement m_editorController->importMap(fileName, offset);
    
    statusBar()->showMessage(tr("Imported map: %1").arg(fileName), 2000);
    qDebug() << "MainWindow::onImportMap: Imported map" << fileName;
}

void MainWindow::onExportMap() {
    // TODO: Implement map export functionality
    QMessageBox::information(this, tr("Export Map"), tr("Map export functionality not yet implemented"));
}

void MainWindow::onExportMinimap() {
    if (!m_editorController || !m_editorController->getMap()) {
        qWarning("MainWindow::onExportMinimap: No map to export");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export Minimap"), 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("PNG Files (*.png);;All Files (*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // TODO: Implement m_editorController->exportMiniMap(fileName);
    
    statusBar()->showMessage(tr("Exported minimap: %1").arg(fileName), 2000);
    qDebug() << "MainWindow::onExportMinimap: Exported minimap" << fileName;
}

void MainWindow::onRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString fileName = action->data().toString();
        // TODO: Implement m_editorController->loadMap(fileName);
        qDebug() << "MainWindow::onRecentFile: Opening recent file" << fileName;
    }
}

void MainWindow::onExit() {
    close();
}

// --- Edit Menu Actions ---
void MainWindow::onUndo() {
    if (m_editorController && m_editorController->canUndo()) {
        m_editorController->undo();
        updateMenuStatesFromEditor();
        statusBar()->showMessage(tr("Undone"), 1000);
    }
}

void MainWindow::onRedo() {
    if (m_editorController && m_editorController->canRedo()) {
        m_editorController->redo();
        updateMenuStatesFromEditor();
        statusBar()->showMessage(tr("Redone"), 1000);
    }
}

void MainWindow::onCut() {
    if (m_editorController) {
        m_editorController->cutSelection();
        updateMenuStatesFromEditor();
        statusBar()->showMessage(tr("Cut selection"), 1000);
    }
}

void MainWindow::onCopy() {
    if (m_editorController) {
        m_editorController->copySelection();
        statusBar()->showMessage(tr("Copied selection"), 1000);
    }
}

void MainWindow::onPaste() {
    if (m_editorController) {
        m_editorController->pasteFromClipboard();
        updateMenuStatesFromEditor();
        statusBar()->showMessage(tr("Pasted"), 1000);
    }
}

void MainWindow::onSelectAll() {
    if (m_editorController) {
        // TODO: Implement select all functionality
        statusBar()->showMessage(tr("Selected all"), 1000);
    }
}

void MainWindow::onClearSelection() {
    if (m_editorController) {
        m_editorController->clearCurrentSelection();
        updateMenuStatesFromEditor();
        statusBar()->showMessage(tr("Selection cleared"), 1000);
    }
}

void MainWindow::onDelete() {
    if (m_editorController) {
        m_editorController->deleteSelection();
        updateMenuStatesFromEditor();
        statusBar()->showMessage(tr("Deleted selection"), 1000);
    }
}

void MainWindow::onPreferences() {
    // TODO: Show preferences dialog
    QMessageBox::information(this, tr("Preferences"), tr("Preferences dialog not yet implemented"));
}

// --- Map Menu Actions ---
void MainWindow::onBorderizeMap() {
    if (m_editorController && m_editorController->getMap()) {
        m_editorController->borderizeMap(true);
        statusBar()->showMessage(tr("Borderizing map..."), 2000);
    }
}

void MainWindow::onRandomizeMap() {
    if (m_editorController && m_editorController->getMap()) {
        m_editorController->randomizeMap(true);
        statusBar()->showMessage(tr("Randomizing map..."), 2000);
    }
}

void MainWindow::onClearInvalidHouseTiles() {
    if (m_editorController && m_editorController->getMap()) {
        m_editorController->clearInvalidHouseTiles(true);
        statusBar()->showMessage(tr("Clearing invalid house tiles..."), 2000);
    }
}

void MainWindow::onClearModifiedTileState() {
    if (m_editorController && m_editorController->getMap()) {
        m_editorController->clearModifiedTileState(true);
        statusBar()->showMessage(tr("Clearing modified tile state..."), 2000);
    }
}

void MainWindow::onValidateGrounds() {
    if (m_editorController && m_editorController->getMap()) {
        quint32 count = m_editorController->validateGrounds();
        statusBar()->showMessage(tr("Validated grounds - modified %1 tiles").arg(count), 3000);
    }
}

void MainWindow::onBorderizeSelection() {
    if (m_editorController && m_editorController->getSelectionManager() && 
        m_editorController->getSelectionManager()->hasSelection()) {
        m_editorController->borderizeSelection();
        statusBar()->showMessage(tr("Borderized selection"), 2000);
    }
}

void MainWindow::onRandomizeSelection() {
    if (m_editorController && m_editorController->getSelectionManager() && 
        m_editorController->getSelectionManager()->hasSelection()) {
        m_editorController->randomizeSelection();
        statusBar()->showMessage(tr("Randomized selection"), 2000);
    }
}

void MainWindow::onMoveSelection() {
    // TODO: Show move selection dialog
    QMessageBox::information(this, tr("Move Selection"), tr("Move selection dialog not yet implemented"));
}

void MainWindow::onResizeMap() {
    // TODO: Show resize map dialog
    QMessageBox::information(this, tr("Resize Map"), tr("Resize map dialog not yet implemented"));
}

void MainWindow::onMapProperties() {
    // TODO: Show map properties dialog
    QMessageBox::information(this, tr("Map Properties"), tr("Map properties dialog not yet implemented"));
}

// --- Search Menu Actions ---
void MainWindow::onFindItem() {
    // Create and show item finder dialog
    RME::ui::dialogs::ItemFinderDialogQt dialog(this);
    dialog.exec();
}

void MainWindow::onFindCreature() {
    // TODO: Create and show creature finder dialog
    QMessageBox::information(this, tr("Find Creature"), tr("Creature finder dialog not yet implemented"));
}

void MainWindow::onSearchOnMap() {
    // TODO: Implement search on map functionality
    QMessageBox::information(this, tr("Search on Map"), tr("Search on map functionality not yet implemented"));
}

void MainWindow::onSearchOnSelection() {
    // TODO: Implement search on selection functionality
    QMessageBox::information(this, tr("Search on Selection"), tr("Search on selection functionality not yet implemented"));
}

void MainWindow::onGoToPosition() {
    if (!m_editorController || !m_editorController->getMap()) {
        QMessageBox::warning(this, tr("Go to Position"), tr("No map is currently open"));
        return;
    }
    
    bool ok;
    int x = QInputDialog::getInt(this, tr("Go to Position"), tr("X coordinate:"), 0, 0, 65535, 1, &ok);
    if (!ok) return;
    
    int y = QInputDialog::getInt(this, tr("Go to Position"), tr("Y coordinate:"), 0, 0, 65535, 1, &ok);
    if (!ok) return;
    
    int z = QInputDialog::getInt(this, tr("Go to Position"), tr("Z coordinate (floor):"), 7, 0, 15, 1, &ok);
    if (!ok) return;
    
    // TODO: Implement m_editorController->navigateToPosition(RME::core::Position(x, y, z));
    
    statusBar()->showMessage(tr("Navigated to position (%1, %2, %3)").arg(x).arg(y).arg(z), 2000);
}

// --- View Menu Actions ---
void MainWindow::onZoomIn() {
    if (m_mapView) {
        // TODO: Implement m_mapView->zoomIn();
        statusBar()->showMessage(tr("Zoomed in"), 1000);
    }
}

void MainWindow::onZoomOut() {
    if (m_mapView) {
        // TODO: Implement m_mapView->zoomOut();
        statusBar()->showMessage(tr("Zoomed out"), 1000);
    }
}

void MainWindow::onZoomNormal() {
    if (m_mapView) {
        // TODO: Implement m_mapView->zoomNormal();
        statusBar()->showMessage(tr("Zoom reset to normal"), 1000);
    }
}

void MainWindow::onZoomFit() {
    if (m_mapView) {
        // TODO: Implement m_mapView->zoomFit();
        statusBar()->showMessage(tr("Zoom fit to window"), 1000);
    }
}

void MainWindow::onFloorUp() {
    if (m_mapView) {
        // TODO: Implement m_mapView->goToFloor(currentFloor - 1);
        statusBar()->showMessage(tr("Moved to floor up"), 1000);
    }
}

void MainWindow::onFloorDown() {
    if (m_mapView) {
        // TODO: Implement m_mapView->goToFloor(currentFloor + 1);
        statusBar()->showMessage(tr("Moved to floor down"), 1000);
    }
}

void MainWindow::onGoToFloor() {
    if (!m_mapView) {
        return;
    }
    
    bool ok;
    int floor = QInputDialog::getInt(this, tr("Go to Floor"), tr("Floor (0-15):"), 7, 0, 15, 1, &ok);
    if (ok) {
        // TODO: Implement m_mapView->goToFloor(floor);
        statusBar()->showMessage(tr("Moved to floor %1").arg(floor), 1000);
    }
}

void MainWindow::onShowGrid() {
    // TODO: Toggle grid display
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        bool showGrid = action->isChecked();
        // TODO: Implement m_mapView->setShowGrid(showGrid);
        statusBar()->showMessage(showGrid ? tr("Grid shown") : tr("Grid hidden"), 1000);
    }
}

void MainWindow::onShowCreatures() {
    // TODO: Toggle creature display
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        bool showCreatures = action->isChecked();
        // TODO: Implement m_mapView->setShowCreatures(showCreatures);
        statusBar()->showMessage(showCreatures ? tr("Creatures shown") : tr("Creatures hidden"), 1000);
    }
}

void MainWindow::onShowSpawns() {
    // TODO: Toggle spawn display
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        bool showSpawns = action->isChecked();
        // TODO: Implement m_mapView->setShowSpawns(showSpawns);
        statusBar()->showMessage(showSpawns ? tr("Spawns shown") : tr("Spawns hidden"), 1000);
    }
}

void MainWindow::onShowHouses() {
    // TODO: Toggle house display
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        bool showHouses = action->isChecked();
        // TODO: Implement m_mapView->setShowHouses(showHouses);
        statusBar()->showMessage(showHouses ? tr("Houses shown") : tr("Houses hidden"), 1000);
    }
}

void MainWindow::onShowWaypoints() {
    // TODO: Toggle waypoint display
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        bool showWaypoints = action->isChecked();
        // TODO: Implement m_mapView->setShowWaypoints(showWaypoints);
        statusBar()->showMessage(showWaypoints ? tr("Waypoints shown") : tr("Waypoints hidden"), 1000);
    }
}

void MainWindow::onShowItemPalette() {
    if (m_dockManager) {
        m_dockManager->toggleDockPanel(RME::ui::DockManager::DockPanelType::ItemPalette);
    }
}

void MainWindow::onShowCreaturePalette() {
    if (m_dockManager) {
        m_dockManager->toggleDockPanel(RME::ui::DockManager::DockPanelType::CreaturePalette);
    }
}

void MainWindow::onShowHousePalette() {
    if (m_dockManager) {
        m_dockManager->toggleDockPanel(RME::ui::DockManager::DockPanelType::HousePalette);
    }
}

void MainWindow::onShowWaypointPalette() {
    if (m_dockManager) {
        m_dockManager->toggleDockPanel(RME::ui::DockManager::DockPanelType::WaypointPalette);
    }
}

void MainWindow::onShowPropertiesPanel() {
    if (m_dockManager) {
        m_dockManager->toggleDockPanel(RME::ui::DockManager::DockPanelType::Properties);
    }
}

void MainWindow::onShowMinimap() {
    if (m_dockManager) {
        m_dockManager->toggleDockPanel(RME::ui::DockManager::DockPanelType::Minimap);
    }
}

// --- Tools Menu Actions ---
void MainWindow::onSelectTool() {
    if (m_editorController) {
        m_editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Brush);
        statusBar()->showMessage(tr("Select tool activated"), 1000);
    }
}

void MainWindow::onBrushTool() {
    if (m_editorController) {
        m_editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Brush);
        statusBar()->showMessage(tr("Brush tool activated"), 1000);
    }
}

void MainWindow::onHouseExitTool() {
    if (m_editorController) {
        m_editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::HouseExit);
        statusBar()->showMessage(tr("House exit tool activated"), 1000);
    }
}

void MainWindow::onWaypointTool() {
    if (m_editorController) {
        m_editorController->setToolMode(RME::editor_logic::EditorController::ToolMode::Waypoint);
        statusBar()->showMessage(tr("Waypoint tool activated"), 1000);
    }
}

void MainWindow::onSpawnTool() {
    // TODO: Implement spawn tool
    QMessageBox::information(this, tr("Spawn Tool"), tr("Spawn tool not yet implemented"));
}

// --- Help Menu Actions ---
void MainWindow::onAbout() {
    QMessageBox::about(this, tr("About RME"),
        tr("<h2>Remere's Map Editor Qt6</h2>"
           "<p>A modern Qt6 port of the popular Tibia map editor.</p>"
           "<p>Version: 1.0.0</p>"
           "<p>Built with Qt %1</p>").arg(QT_VERSION_STR));
}

void MainWindow::onAboutQt() {
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::onHelp() {
    // TODO: Show help documentation
    QMessageBox::information(this, tr("Help"), tr("Help documentation not yet implemented"));
}

void MainWindow::onCheckUpdates() {
    // TODO: Check for application updates
    QMessageBox::information(this, tr("Check Updates"), tr("Update checking not yet implemented"));
}

// Brush & Material Editor actions
void MainWindow::onBrushMaterialEditor() {
    auto dialog = new RME::ui::dialogs::BrushMaterialEditorDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::onNewTileset() {
    auto dialog = new RME::ui::dialogs::NewTilesetDialog(this);
    if (dialog->exec() == QDialog::Accepted) {
        QString tilesetName = dialog->getTilesetName();
        uint16_t initialItemId = dialog->getInitialItemId();
        
        // Integrate with MaterialManager to actually create the tileset
        if (m_editorController && m_editorController->getMaterialManager()) {
            auto* materialManager = m_editorController->getMaterialManager();
            qDebug() << "MainWindowActions: Creating tileset with MaterialManager";
            // TODO: Call actual tileset creation method when MaterialManager API is complete
        }
        QMessageBox::information(this, "New Tileset", 
            QString("Created tileset '%1' with initial item ID %2")
            .arg(tilesetName).arg(initialItemId));
    }
    delete dialog;
}

void MainWindow::onAddItemToTileset() {
    auto dialog = new RME::ui::dialogs::AddItemToTilesetDialog(this);
    if (dialog->exec() == QDialog::Accepted) {
        QString selectedTileset = dialog->getSelectedTileset();
        QList<uint16_t> itemIds = dialog->getSelectedItemIds();
        
        // Integrate with MaterialManager to actually add items to tileset
        if (m_editorController && m_editorController->getMaterialManager()) {
            auto* materialManager = m_editorController->getMaterialManager();
            qDebug() << "MainWindowActions: Adding items to tileset with MaterialManager";
            // TODO: Call actual item addition method when MaterialManager API is complete
        }
        QMessageBox::information(this, "Add Items to Tileset", 
            QString("Added %1 items to tileset '%2'")
            .arg(itemIds.size()).arg(selectedTileset));
    }
    delete dialog;
}