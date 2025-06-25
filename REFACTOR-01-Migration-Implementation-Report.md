# REFACTOR-01 Migration Implementation Report

## Executive Summary

The REFACTOR-01 task represents a critical architectural refactoring to eliminate the global `g_gui` singleton pattern and replace it with a modern service-oriented architecture using dependency injection. This migration will transform the Qt6 RME application from a monolithic global state approach to a modular, testable, and maintainable service-based architecture.

## Current State Analysis

### Existing Global Dependencies

Based on the codebase analysis, the original wxWidgets application heavily relied on the global `g_gui` object (947 references found) which managed:

1. **Editor Session Management**: Current editor instance, active map, canvas, floor, zoom
2. **Brush Management**: Active brush, size, shape, variation, specific brush instances
3. **Editor Mode**: Drawing, selection, pasting modes
4. **Data Management**: Client version data, sprite manager, item manager, material manager
5. **UI Element Management**: Palettes, minimap, dialogs, progress bars, main frame
6. **User Interaction**: Hotkeys, status updates, window titles
7. **Clipboard**: Copy/paste buffer management
8. **OpenGL Context**: Shared graphics context

### Current Qt6 Service Infrastructure

The Qt6 codebase already has several service-like components that can be leveraged:

- **AssetManager**: Manages client data and resources
- **MaterialManager**: Handles material system
- **ClientVersionManager**: Manages client versions
- **SpriteManager**: Handles sprite data
- **BrushManagerService**: Manages brush instances
- **BrushStateService**: Manages brush state
- **EditorStateService**: Manages editor state
- **ClipboardManager**: Handles copy/paste operations
- **UndoManager**: Manages undo/redo operations
- **SelectionManager**: Manages tile selections
- **WaypointManager**: Manages waypoints
- **SpawnManager**: Manages spawns
- **TownManager**: Manages towns

## Implementation Strategy

### Phase 1: Service Architecture Design

#### 1.1 Core Service Classes

Based on the YAML requirements and Qt6 best practices, the following service classes need to be implemented:

##### BrushStateService (Already Exists - Needs Enhancement)
```cpp
class BrushStateService : public QObject {
    Q_OBJECT
public:
    explicit BrushStateService(QObject* parent = nullptr);
    
    // Brush management
    void setActiveBrush(Brush* brush);
    Brush* getActiveBrush() const;
    
    // Brush properties
    void setBrushShape(BrushShape shape);
    BrushShape getBrushShape() const;
    
    void setBrushSize(int size);
    int getBrushSize() const;
    
    void setBrushVariation(int variation);
    int getBrushVariation() const;
    
    // Brush settings
    void setDrawLockedDoors(bool enabled);
    bool getDrawLockedDoors() const;
    
    void setUseCustomThickness(bool enabled);
    bool getUseCustomThickness() const;
    
    void setCustomThicknessMod(float mod);
    float getCustomThicknessMod() const;
    
    // Specific brush data
    void setCurrentRawItemId(uint32_t itemId);
    uint32_t getCurrentRawItemId() const;
    
    void setCurrentCreatureType(const CreatureData* creature);
    const CreatureData* getCurrentCreatureType() const;
    
    // Doodad buffer
    void setDoodadBufferMap(BaseMap* map);
    BaseMap* getDoodadBufferMap() const;

signals:
    void activeBrushChanged(Brush* brush);
    void brushShapeChanged(BrushShape shape);
    void brushSizeChanged(int size);
    void brushVariationChanged(int variation);
    void brushSettingsChanged();
    void drawLockedDoorsChanged(bool enabled);
    void customThicknessChanged(bool enabled, float mod);
    void currentRawItemIdChanged(uint32_t itemId);
    void currentCreatureTypeChanged(const CreatureData* creature);

private:
    Brush* m_activeBrush = nullptr;
    BrushShape m_currentShape = BrushShape::Square;
    int m_currentSize = 1;
    int m_brushVariation = 0;
    bool m_drawLockedDoors = false;
    bool m_useCustomThickness = false;
    float m_customThicknessMod = 1.0f;
    uint32_t m_currentRawItemId = 0;
    const CreatureData* m_currentCreatureType = nullptr;
    BaseMap* m_doodadBufferMap = nullptr;
};
```

##### EditorStateService (Already Exists - Needs Enhancement)
```cpp
class EditorStateService : public QObject {
    Q_OBJECT
public:
    enum class EditorMode {
        Drawing,
        Selection,
        Pasting,
        Filling
    };
    
    explicit EditorStateService(QObject* parent = nullptr);
    
    // Editor mode
    void setEditorMode(EditorMode mode);
    EditorMode getEditorMode() const;
    
    // Floor management
    void setCurrentFloor(int floor);
    int getCurrentFloor() const;
    
    // Editor session
    void setActiveEditorSession(EditorController* editor);
    EditorController* getActiveEditorSession() const;
    
    // Zoom level
    void setZoomLevel(float zoom);
    float getZoomLevel() const;
    
    // View position
    void setViewPosition(const QPoint& position);
    QPoint getViewPosition() const;

signals:
    void editorModeChanged(EditorMode mode);
    void currentFloorChanged(int floor);
    void activeEditorChanged(EditorController* editor);
    void zoomLevelChanged(float zoom);
    void viewPositionChanged(const QPoint& position);

private:
    EditorMode m_currentMode = EditorMode::Drawing;
    int m_currentFloor = 7; // Default ground floor
    EditorController* m_activeEditorSession = nullptr;
    float m_zoomLevel = 1.0f;
    QPoint m_viewPosition;
};
```

##### ClientDataService (New)
```cpp
class ClientDataService : public QObject {
    Q_OBJECT
public:
    explicit ClientDataService(QObject* parent = nullptr);
    
    // Client version management
    bool loadClientVersion(const QString& versionId);
    void unloadClientVersion();
    
    // Data access
    ClientVersion* getClientVersion() const;
    ItemDatabase* getItemDatabase() const;
    SpriteManager* getSpriteManager() const;
    MaterialManager* getMaterialManager() const;
    CreatureDatabase* getCreatureDatabase() const;
    AssetManager* getAssetManager() const;
    
    // Status
    bool isClientVersionLoaded() const;
    QString getCurrentVersionId() const;

signals:
    void clientVersionChanged(ClientVersion* version);
    void clientVersionLoaded(const QString& versionId);
    void clientVersionUnloaded();

private:
    ClientVersion* m_clientVersion = nullptr;
    AssetManager* m_assetManager = nullptr;
    QString m_currentVersionId;
};
```

##### WindowManagerService (New)
```cpp
class WindowManagerService : public QObject {
    Q_OBJECT
public:
    explicit WindowManagerService(QMainWindow* mainWindow, QObject* parent = nullptr);
    
    // Main window access
    QMainWindow* getMainWindow() const;
    
    // Dialog management
    void showErrorDialog(const QString& title, const QString& message);
    void showInfoDialog(const QString& title, const QString& message);
    void showWarningDialog(const QString& title, const QString& message);
    bool showConfirmDialog(const QString& title, const QString& message);
    
    // Status and title updates
    void updateStatusText(const QString& text);
    void updateWindowTitle(const QString& title);
    void updateMenuBar();
    
    // Progress management
    void showProgressDialog(const QString& title, const QString& message);
    void updateProgress(int value, int maximum);
    void hideProgressDialog();
    
    // Palette management
    void refreshPalettes();
    void showPalette(const QString& paletteName, bool visible);
    
    // Editor tabs
    EditorController* getCurrentEditor() const;
    MapView* getCurrentMapView() const;
    
    // Perspective management
    void savePerspective(const QString& name);
    void loadPerspective(const QString& name);

signals:
    void currentEditorChanged(EditorController* editor);
    void perspectiveChanged(const QString& name);

private:
    QMainWindow* m_mainWindow;
    QProgressDialog* m_progressDialog = nullptr;
    QMap<QString, QByteArray> m_perspectives;
};
```

##### ApplicationSettingsService (New)
```cpp
class ApplicationSettingsService : public QObject {
    Q_OBJECT
public:
    explicit ApplicationSettingsService(QObject* parent = nullptr);
    
    // UI-related settings
    bool isDoorLocked() const;
    void setDoorLocked(bool locked);
    
    bool isPasting() const;
    void setPasting(bool pasting);
    
    bool isAutoSaveEnabled() const;
    void setAutoSaveEnabled(bool enabled);
    
    int getAutoSaveInterval() const;
    void setAutoSaveInterval(int minutes);
    
    // View settings
    bool isGridVisible() const;
    void setGridVisible(bool visible);
    
    bool areCreaturesVisible() const;
    void setCreaturesVisible(bool visible);
    
    bool areSpawnsVisible() const;
    void setSpawnsVisible(bool visible);
    
    bool areHousesVisible() const;
    void setHousesVisible(bool visible);
    
    // Brush settings
    int getDefaultBrushSize() const;
    void setDefaultBrushSize(int size);
    
    BrushShape getDefaultBrushShape() const;
    void setDefaultBrushShape(BrushShape shape);

signals:
    void doorLockedChanged(bool locked);
    void pastingChanged(bool pasting);
    void autoSaveSettingsChanged(bool enabled, int interval);
    void viewSettingsChanged();
    void brushSettingsChanged();

private:
    QSettings* m_settings;
};
```

#### 1.2 Service Integration Points

The services will be integrated through the following mechanisms:

1. **MainWindow Ownership**: All services are owned by MainWindow
2. **Dependency Injection**: Services are passed to components that need them
3. **Signal/Slot Communication**: Services communicate via Qt's signal/slot system
4. **Interface Abstraction**: Services implement interfaces for testability

### Phase 2: MainWindow Integration

#### 2.1 Service Instantiation

```cpp
// MainWindow.h
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    // Core services
    BrushStateService* m_brushStateService;
    EditorStateService* m_editorStateService;
    ClientDataService* m_clientDataService;
    WindowManagerService* m_windowManagerService;
    ApplicationSettingsService* m_settingsService;
    ClipboardManager* m_clipboardManager;
    
    // UI components
    EditorController* m_editorController;
    MapViewWidget* m_mapViewWidget;
    // ... other UI components
};

// MainWindow.cpp constructor
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Initialize services
    m_brushStateService = new BrushStateService(this);
    m_editorStateService = new EditorStateService(this);
    m_clientDataService = new ClientDataService(this);
    m_windowManagerService = new WindowManagerService(this, this);
    m_settingsService = new ApplicationSettingsService(this);
    m_clipboardManager = new ClipboardManager(this);
    
    // Initialize UI components with dependency injection
    m_editorController = new EditorController(
        nullptr, // map will be set later
        m_brushStateService,
        m_editorStateService,
        m_clipboardManager,
        this
    );
    
    m_mapViewWidget = new MapViewWidget(
        m_editorController,
        m_brushStateService,
        m_editorStateService,
        this
    );
    
    // Connect services
    connectServices();
}
```

#### 2.2 Service Connections

```cpp
void MainWindow::connectServices() {
    // Brush state changes update UI
    connect(m_brushStateService, &BrushStateService::activeBrushChanged,
            this, &MainWindow::onActiveBrushChanged);
    
    // Editor state changes update UI
    connect(m_editorStateService, &EditorStateService::editorModeChanged,
            this, &MainWindow::onEditorModeChanged);
    
    // Client data changes update all dependent components
    connect(m_clientDataService, &ClientDataService::clientVersionChanged,
            this, &MainWindow::onClientVersionChanged);
    
    // Settings changes propagate to relevant components
    connect(m_settingsService, &ApplicationSettingsService::viewSettingsChanged,
            m_mapViewWidget, &MapViewWidget::updateViewSettings);
}
```

### Phase 3: Component Refactoring

#### 3.1 EditorController Updates

```cpp
class EditorController : public QObject {
    Q_OBJECT
public:
    explicit EditorController(
        Map* map,
        BrushStateService* brushService,
        EditorStateService* editorService,
        ClipboardManager* clipboardManager,
        QObject* parent = nullptr
    );

private:
    BrushStateService* m_brushService;
    EditorStateService* m_editorService;
    ClipboardManager* m_clipboardManager;
};
```

#### 3.2 MapViewWidget Updates

```cpp
class MapViewWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit MapViewWidget(
        EditorController* controller,
        BrushStateService* brushService,
        EditorStateService* editorService,
        QWidget* parent = nullptr
    );

private slots:
    void onActiveBrushChanged(Brush* brush);
    void onBrushSizeChanged(int size);
    void onEditorModeChanged(EditorStateService::EditorMode mode);

private:
    EditorController* m_controller;
    BrushStateService* m_brushService;
    EditorStateService* m_editorService;
};
```

#### 3.3 Palette Updates

```cpp
class ItemPalettePanel : public BasePalettePanel {
    Q_OBJECT
public:
    explicit ItemPalettePanel(
        BrushStateService* brushService,
        ClientDataService* clientDataService,
        QWidget* parent = nullptr
    );

private slots:
    void onItemSelected(uint32_t itemId);
    void onClientVersionChanged(ClientVersion* version);

private:
    BrushStateService* m_brushService;
    ClientDataService* m_clientDataService;
};
```

### Phase 4: Global Access Elimination

#### 4.1 Search and Replace Patterns

The following global access patterns need to be replaced:

```cpp
// OLD: Direct global access
g_gui.GetCurrentBrush()
g_gui.GetCurrentEditor()
g_gui.gfx
g_items
g_materials
g_creatures

// NEW: Service-based access
m_brushStateService->getActiveBrush()
m_editorStateService->getActiveEditorSession()
m_clientDataService->getSpriteManager()
m_clientDataService->getItemDatabase()
m_clientDataService->getMaterialManager()
m_clientDataService->getCreatureDatabase()
```

#### 4.2 Specific Replacement Examples

```cpp
// Brush management
// OLD: g_gui.SetCurrentBrush(brush)
// NEW: m_brushStateService->setActiveBrush(brush)

// Editor state
// OLD: g_gui.GetCurrentEditor()->getMap()
// NEW: m_editorStateService->getActiveEditorSession()->getMap()

// Client data
// OLD: g_items.getItemType(id)
// NEW: m_clientDataService->getItemDatabase()->getItemType(id)

// UI updates
// OLD: g_gui.UpdateStatusText("Ready")
// NEW: m_windowManagerService->updateStatusText("Ready")
```

### Phase 5: Signal/Slot Architecture

#### 5.1 Inter-Service Communication

```cpp
// Example: Brush selection in palette affects editor
// Palette emits signal -> BrushStateService receives -> MapView updates

// In ItemPalettePanel
void ItemPalettePanel::onItemClicked(uint32_t itemId) {
    m_brushService->setCurrentRawItemId(itemId);
    // BrushStateService emits currentRawItemIdChanged signal
}

// In MapViewWidget constructor
connect(m_brushService, &BrushStateService::currentRawItemIdChanged,
        this, &MapViewWidget::onCurrentItemChanged);
```

#### 5.2 Cross-Component Updates

```cpp
// Example: Client version change affects multiple components
void MainWindow::onClientVersionChanged(ClientVersion* version) {
    // Update all palettes
    for (auto* palette : m_palettes) {
        palette->updateClientVersion(version);
    }
    
    // Update map view
    m_mapViewWidget->updateClientVersion(version);
    
    // Update editor controller
    m_editorController->updateClientVersion(version);
}
```

## Implementation Timeline

### Week 1-2: Service Design and Implementation
- Implement core service classes
- Create service interfaces
- Set up basic dependency injection in MainWindow

### Week 3-4: Component Integration
- Update EditorController to use services
- Update MapViewWidget to use services
- Update palette components to use services

### Week 5-6: Global Access Elimination
- Systematically replace all g_gui references
- Replace direct global variable access
- Update all UI components

### Week 7-8: Signal/Slot Architecture
- Implement comprehensive signal/slot connections
- Ensure proper communication between services
- Add error handling and validation

### Week 9-10: Testing and Validation
- Comprehensive testing of all functionality
- Performance validation
- Memory leak detection
- Integration testing

## Risk Assessment

### High Risk Areas

1. **Circular Dependencies**: Services must be carefully designed to avoid circular dependencies
2. **Performance Impact**: Signal/slot overhead vs. direct access
3. **Memory Management**: Proper ownership and lifecycle management
4. **Testing Complexity**: Mocking services for unit tests

### Mitigation Strategies

1. **Interface Segregation**: Use small, focused interfaces
2. **Lazy Initialization**: Initialize services only when needed
3. **Smart Pointers**: Use appropriate smart pointer types
4. **Mock Framework**: Implement comprehensive mocking system

## Testing Strategy

### Unit Testing
- Each service class will have comprehensive unit tests
- Mock implementations for all service interfaces
- Test all signal/slot connections

### Integration Testing
- Test service interactions
- Test UI component integration
- Test complete workflows

### Performance Testing
- Compare performance before/after refactoring
- Memory usage analysis
- Signal/slot performance impact

## Benefits

### Immediate Benefits
1. **Testability**: Services can be easily mocked and tested
2. **Modularity**: Clear separation of concerns
3. **Maintainability**: Easier to modify and extend

### Long-term Benefits
1. **Scalability**: Easy to add new features
2. **Debugging**: Clear data flow and dependencies
3. **Code Quality**: Reduced coupling and improved cohesion

## Conclusion

The REFACTOR-01 task represents a fundamental architectural improvement that will transform the RME Qt6 application from a legacy global-state architecture to a modern, service-oriented design. While the implementation is complex and touches most of the codebase, the benefits in terms of maintainability, testability, and extensibility make this refactoring essential for the long-term success of the project.

The phased approach ensures that the migration can be done incrementally while maintaining application functionality throughout the process. The comprehensive service architecture will provide a solid foundation for future development and make the codebase much more approachable for new developers.