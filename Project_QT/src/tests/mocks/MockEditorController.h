#ifndef MOCKEDITORCONTROLLER_H
#define MOCKEDITORCONTROLLER_H

#include "editor_logic/EditorController.h"
#include "map_management/Map.h" // Assuming Map class is in this header
#include "mocks/MockMap.h" // For m_mockMap
#include <QPoint>
#include <QSharedPointer> // Required for QSharedPointer

// Forward declaration if needed, or include the actual definition
// class Map; // If Map.h is heavy or causes circular dependencies

class MockEditorController : public EditorController {
    Q_OBJECT

public:
    MockEditorController(QObject *parent = nullptr)
        : EditorController(parent), m_tileChangedNotified(false) {
        // It's important that m_mockMap is initialized,
        // possibly to a default MockMap instance or null if tests always set it.
        // For now, let's assume tests will inject it.
    }
    ~MockEditorController() override = default;

    // Mocked methods from EditorController
    void notifyTileChanged(const QPoint& position) override {
        m_tileChangedNotified = true;
        m_notifiedPosition = position;
        // In a real EditorController, this would emit a signal.
        // For tests, we can check these flags.
    }

    Map* getMap() const override {
        // This is crucial for RecordSetSpawnCommand
        return m_mockMap.data();
    }

    // Test helper members - public for easy access in tests
    bool m_tileChangedNotified;
    QPoint m_notifiedPosition;
    QSharedPointer<MockMap> m_mockMap; // This will be set by the test functions

    // Implementing other pure virtual methods from EditorController
    // These are minimal implementations to make the mock concrete.
    void setTool(ToolType tool) override {
        Q_UNUSED(tool);
        // Minimal mock implementation
    }

    ToolType getCurrentTool() const override {
        return ToolType::POINTER; // Or any default/mock value
    }

    void loadMap(const QString& filePath) override {
        Q_UNUSED(filePath);
        // Minimal mock implementation
    }

    void saveMap(const QString& filePath) override {
        Q_UNUSED(filePath);
        // Minimal mock implementation
    }

    void createNewMap(int width, int height) override {
        Q_UNUSED(width);
        Q_UNUSED(height);
        // Minimal mock implementation
        // Optionally, create a new MockMap here if it makes sense for some tests
        // m_mockMap = QSharedPointer<MockMap>(new MockMap(width, height));
        // emit mapChanged(m_mockMap.data()); // If mapChanged signal is part of EditorController
    }

    bool hasUndoableActions() const override {
        return false; // Mock implementation
    }

    bool hasRedoableActions() const override {
        return false; // Mock implementation
    }

    void undo() override {
        // Minimal mock implementation
    }

    void redo() override {
        // Minimal mock implementation
    }

    void selectTileForPalette(const QSharedPointer<Tile>& tile) override {
        Q_UNUSED(tile);
        // Minimal mock implementation
    }

    QSharedPointer<Tile> getSelectedTileFromPalette() const override {
        return nullptr; // Mock implementation
    }

    void resizeMap(int newWidth, int newHeight) override {
        Q_UNUSED(newWidth);
        Q_UNUSED(newHeight);
        // Minimal mock implementation
    }

    // If EditorController has signals like mapChanged(Map*),
    // you might need to declare them here as well, though not strictly necessary
    // for this mock unless other parts of the system connect to them during tests.

signals:
    // Re-declare signals from EditorController if they are used by the code under test
    // void mapChanged(Map* newMap) override; // Example if such a signal exists

};

#endif // MOCKEDITORCONTROLLER_H
