#include "ui/MainToolBar.h"
#include "editor_logic/EditorController.h"
#include "core/brush/BrushIntegrationManager.h"

#include <QAction>
#include <QActionGroup>
#include <QSpinBox>
#include <QLabel>
#include <QStyle>
#include <QApplication>

namespace RME {
namespace ui {

MainToolBar::MainToolBar(QWidget* parent)
    : QToolBar(tr("Main Toolbar"), parent)
{
    setObjectName("MainToolBar");
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    createActions();
    createToolGroups();
    setupLayout();
    connectSignals();
    
    // Initial state - all actions disabled until map is loaded
    updateToolStates();
}

void MainToolBar::setEditorController(RME::editor_logic::EditorController* controller) {
    m_editorController = controller;
    updateToolStates();
}

void MainToolBar::setBrushIntegrationManager(RME::core::brush::BrushIntegrationManager* manager) {
    m_brushManager = manager;
    
    if (m_brushManager) {
        // Connect brush manager signals
        connect(m_brushManager, &RME::core::brush::BrushIntegrationManager::toolModeChanged,
                this, &MainToolBar::onToolModeChanged);
    }
}

void MainToolBar::createActions() {
    // File operations
    m_newAction = createAction(tr("New"), "document-new", tr("Create a new map"));
    m_openAction = createAction(tr("Open"), "document-open", tr("Open an existing map"));
    m_saveAction = createAction(tr("Save"), "document-save", tr("Save the current map"));
    
    // Edit operations
    m_undoAction = createAction(tr("Undo"), "edit-undo", tr("Undo the last action"));
    m_redoAction = createAction(tr("Redo"), "edit-redo", tr("Redo the last undone action"));
    m_cutAction = createAction(tr("Cut"), "edit-cut", tr("Cut selection to clipboard"));
    m_copyAction = createAction(tr("Copy"), "edit-copy", tr("Copy selection to clipboard"));
    m_pasteAction = createAction(tr("Paste"), "edit-paste", tr("Paste from clipboard"));
    
    // Tool selection
    m_selectToolAction = createAction(tr("Select"), "edit-select", tr("Selection tool"));
    m_brushToolAction = createAction(tr("Brush"), "draw-brush", tr("Brush tool"));
    m_houseExitToolAction = createAction(tr("House Exit"), "go-home", tr("House exit tool"));
    m_waypointToolAction = createAction(tr("Waypoint"), "flag", tr("Waypoint tool"));
    
    // Make tool actions checkable
    m_selectToolAction->setCheckable(true);
    m_brushToolAction->setCheckable(true);
    m_houseExitToolAction->setCheckable(true);
    m_waypointToolAction->setCheckable(true);
    
    // View controls
    m_zoomInAction = createAction(tr("Zoom In"), "zoom-in", tr("Zoom in"));
    m_zoomOutAction = createAction(tr("Zoom Out"), "zoom-out", tr("Zoom out"));
    m_zoomNormalAction = createAction(tr("Zoom Normal"), "zoom-original", tr("Reset zoom to normal"));
    
    // Map operations
    m_borderizeMapAction = createAction(tr("Borderize"), "view-grid", tr("Apply borders to map"));
    m_randomizeMapAction = createAction(tr("Randomize"), "roll", tr("Randomize map grounds"));
    m_validateGroundsAction = createAction(tr("Validate"), "dialog-ok", tr("Validate ground tiles"));
}

void MainToolBar::createToolGroups() {
    // Create tool group for exclusive selection
    m_toolGroup = new QActionGroup(this);
    m_toolGroup->addAction(m_selectToolAction);
    m_toolGroup->addAction(m_brushToolAction);
    m_toolGroup->addAction(m_houseExitToolAction);
    m_toolGroup->addAction(m_waypointToolAction);
    
    // Set default tool
    m_brushToolAction->setChecked(true);
}

void MainToolBar::setupLayout() {
    // File operations section
    addAction(m_newAction);
    addAction(m_openAction);
    addAction(m_saveAction);
    addSeparatorWithSpacing();
    
    // Edit operations section
    addAction(m_undoAction);
    addAction(m_redoAction);
    addSeparatorWithSpacing();
    addAction(m_cutAction);
    addAction(m_copyAction);
    addAction(m_pasteAction);
    addSeparatorWithSpacing();
    
    // Tool selection section
    addAction(m_selectToolAction);
    addAction(m_brushToolAction);
    addAction(m_houseExitToolAction);
    addAction(m_waypointToolAction);
    addSeparatorWithSpacing();
    
    // View controls section
    addAction(m_zoomInAction);
    addAction(m_zoomOutAction);
    addAction(m_zoomNormalAction);
    addSeparatorWithSpacing();
    
    // Floor control
    m_floorLabel = new QLabel(tr("Floor:"));
    addWidget(m_floorLabel);
    m_floorSpinBox = new QSpinBox();
    m_floorSpinBox->setRange(0, 15);
    m_floorSpinBox->setValue(7);
    m_floorSpinBox->setToolTip(tr("Current floor level"));
    addWidget(m_floorSpinBox);
    addSeparatorWithSpacing();
    
    // Map operations section
    addAction(m_borderizeMapAction);
    addAction(m_randomizeMapAction);
    addAction(m_validateGroundsAction);
    addSeparatorWithSpacing();
    
    // Status indicator
    addWidget(new QLabel()); // Spacer
    m_statusLabel = new QLabel(tr("Ready"));
    m_statusLabel->setStyleSheet("QLabel { color: gray; }");
    addWidget(m_statusLabel);
}

void MainToolBar::connectSignals() {
    // File operations
    connect(m_newAction, &QAction::triggered, this, &MainToolBar::onNewMapClicked);
    connect(m_openAction, &QAction::triggered, this, &MainToolBar::onOpenMapClicked);
    connect(m_saveAction, &QAction::triggered, this, &MainToolBar::onSaveMapClicked);
    
    // Edit operations
    connect(m_undoAction, &QAction::triggered, this, &MainToolBar::onUndoClicked);
    connect(m_redoAction, &QAction::triggered, this, &MainToolBar::onRedoClicked);
    connect(m_cutAction, &QAction::triggered, this, &MainToolBar::onCutClicked);
    connect(m_copyAction, &QAction::triggered, this, &MainToolBar::onCopyClicked);
    connect(m_pasteAction, &QAction::triggered, this, &MainToolBar::onPasteClicked);
    
    // Tool selection
    connect(m_selectToolAction, &QAction::triggered, this, &MainToolBar::onSelectToolClicked);
    connect(m_brushToolAction, &QAction::triggered, this, &MainToolBar::onBrushToolClicked);
    connect(m_houseExitToolAction, &QAction::triggered, this, &MainToolBar::onHouseExitToolClicked);
    connect(m_waypointToolAction, &QAction::triggered, this, &MainToolBar::onWaypointToolClicked);
    
    // View controls
    connect(m_zoomInAction, &QAction::triggered, this, &MainToolBar::onZoomInClicked);
    connect(m_zoomOutAction, &QAction::triggered, this, &MainToolBar::onZoomOutClicked);
    connect(m_zoomNormalAction, &QAction::triggered, this, &MainToolBar::onZoomNormalClicked);
    connect(m_floorSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainToolBar::onFloorChanged);
    
    // Map operations
    connect(m_borderizeMapAction, &QAction::triggered, this, &MainToolBar::onBorderizeMapClicked);
    connect(m_randomizeMapAction, &QAction::triggered, this, &MainToolBar::onRandomizeMapClicked);
    connect(m_validateGroundsAction, &QAction::triggered, this, &MainToolBar::onValidateGroundsClicked);
}

QAction* MainToolBar::createAction(const QString& text, const QString& iconName, const QString& tooltip) {
    QAction* action = new QAction(text, this);
    action->setToolTip(tooltip);
    action->setStatusTip(tooltip);
    
    // Try to set icon from system theme
    if (QIcon::hasThemeIcon(iconName)) {
        action->setIcon(QIcon::fromTheme(iconName));
    } else {
        // Fallback to standard pixmap if available
        QStyle::StandardPixmap pixmap = QStyle::SP_CustomBase;
        if (iconName == "document-new") pixmap = QStyle::SP_FileIcon;
        else if (iconName == "document-open") pixmap = QStyle::SP_DirOpenIcon;
        else if (iconName == "document-save") pixmap = QStyle::SP_DialogSaveButton;
        else if (iconName == "edit-undo") pixmap = QStyle::SP_ArrowLeft;
        else if (iconName == "edit-redo") pixmap = QStyle::SP_ArrowRight;
        
        if (pixmap != QStyle::SP_CustomBase) {
            action->setIcon(style()->standardIcon(pixmap));
        }
    }
    
    return action;
}

void MainToolBar::addSeparatorWithSpacing() {
    addSeparator();
}

void MainToolBar::updateToolStates() {
    bool hasMap = m_editorController && m_editorController->getMap();
    bool hasSelection = hasMap && m_editorController->getSelectionManager() && 
                       m_editorController->getSelectionManager()->hasSelection();
    bool canUndo = hasMap && m_editorController->canUndo();
    bool canRedo = hasMap && m_editorController->canRedo();
    
    // File operations - always enabled
    m_newAction->setEnabled(true);
    m_openAction->setEnabled(true);
    m_saveAction->setEnabled(hasMap);
    
    // Edit operations
    m_undoAction->setEnabled(canUndo);
    m_redoAction->setEnabled(canRedo);
    m_cutAction->setEnabled(hasSelection);
    m_copyAction->setEnabled(hasSelection);
    m_pasteAction->setEnabled(hasMap); // TODO: Check if clipboard has content
    
    // Tool selection - enabled when map is loaded
    m_selectToolAction->setEnabled(hasMap);
    m_brushToolAction->setEnabled(hasMap);
    m_houseExitToolAction->setEnabled(hasMap);
    m_waypointToolAction->setEnabled(hasMap);
    
    // View controls
    m_zoomInAction->setEnabled(hasMap);
    m_zoomOutAction->setEnabled(hasMap);
    m_zoomNormalAction->setEnabled(hasMap);
    m_floorSpinBox->setEnabled(hasMap);
    
    // Map operations
    m_borderizeMapAction->setEnabled(hasMap);
    m_randomizeMapAction->setEnabled(hasMap);
    m_validateGroundsAction->setEnabled(hasMap);
    
    // Update status
    if (hasMap) {
        m_statusLabel->setText(tr("Map loaded"));
        m_statusLabel->setStyleSheet("QLabel { color: green; }");
    } else {
        m_statusLabel->setText(tr("No map"));
        m_statusLabel->setStyleSheet("QLabel { color: gray; }");
    }
}

void MainToolBar::updateZoomLevel(int zoomLevel) {
    // TODO: Update zoom display if needed
    Q_UNUSED(zoomLevel)
}

void MainToolBar::updateFloorLevel(int floor) {
    if (m_floorSpinBox->value() != floor) {
        m_floorSpinBox->blockSignals(true);
        m_floorSpinBox->setValue(floor);
        m_floorSpinBox->blockSignals(false);
    }
}

// Slots
void MainToolBar::onMapStateChanged() {
    updateToolStates();
}

void MainToolBar::onSelectionChanged() {
    updateToolStates();
}

void MainToolBar::onToolModeChanged(int toolMode) {
    // Update tool button states based on tool mode
    switch (toolMode) {
        case 0: // Brush mode
            m_brushToolAction->setChecked(true);
            break;
        case 1: // HouseExit mode
            m_houseExitToolAction->setChecked(true);
            break;
        case 2: // Waypoint mode
            m_waypointToolAction->setChecked(true);
            break;
        default:
            m_selectToolAction->setChecked(true);
            break;
    }
}

// Action handlers - emit signals for MainWindow to handle
void MainToolBar::onNewMapClicked() { emit newMapRequested(); }
void MainToolBar::onOpenMapClicked() { emit openMapRequested(); }
void MainToolBar::onSaveMapClicked() { emit saveMapRequested(); }

void MainToolBar::onUndoClicked() {
    if (m_editorController) m_editorController->undo();
}

void MainToolBar::onRedoClicked() {
    if (m_editorController) m_editorController->redo();
}

void MainToolBar::onCutClicked() {
    if (m_editorController) m_editorController->cutSelection();
}

void MainToolBar::onCopyClicked() {
    if (m_editorController) m_editorController->copySelection();
}

void MainToolBar::onPasteClicked() {
    if (m_editorController) m_editorController->pasteFromClipboard();
}

void MainToolBar::onSelectToolClicked() {
    emit toolModeChangeRequested(0); // Brush mode
}

void MainToolBar::onBrushToolClicked() {
    emit toolModeChangeRequested(0); // Brush mode
}

void MainToolBar::onHouseExitToolClicked() {
    emit toolModeChangeRequested(1); // HouseExit mode
}

void MainToolBar::onWaypointToolClicked() {
    emit toolModeChangeRequested(2); // Waypoint mode
}

void MainToolBar::onZoomInClicked() {
    emit zoomChangeRequested(1); // Zoom in
}

void MainToolBar::onZoomOutClicked() {
    emit zoomChangeRequested(-1); // Zoom out
}

void MainToolBar::onZoomNormalClicked() {
    emit zoomChangeRequested(0); // Reset zoom
}

void MainToolBar::onFloorUpClicked() {
    int currentFloor = m_floorSpinBox->value();
    if (currentFloor > 0) {
        m_floorSpinBox->setValue(currentFloor - 1);
    }
}

void MainToolBar::onFloorDownClicked() {
    int currentFloor = m_floorSpinBox->value();
    if (currentFloor < 15) {
        m_floorSpinBox->setValue(currentFloor + 1);
    }
}

void MainToolBar::onFloorChanged(int floor) {
    emit floorChangeRequested(floor);
}

void MainToolBar::onBorderizeMapClicked() {
    if (m_editorController) m_editorController->borderizeMap();
}

void MainToolBar::onRandomizeMapClicked() {
    if (m_editorController) m_editorController->randomizeMap();
}

void MainToolBar::onValidateGroundsClicked() {
    if (m_editorController) m_editorController->validateGrounds();
}

} // namespace ui
} // namespace RME

// #include "MainToolBar.moc" // Removed - Q_OBJECT is in header