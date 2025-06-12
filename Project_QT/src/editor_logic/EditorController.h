#ifndef RME_EDITORCONTROLLER_H
#define RME_EDITORCONTROLLER_H

#include <QObject>
#include <QList>

// Forward declarations to minimize header dependencies
class QUndoStack; // Qt class

namespace RME {
    // Forward declarations for RME types
    class Map;
    class SelectionManager;
    class BrushManagerService;
    class Brush;
    struct BrushSettings;
    struct Position;
    namespace core { class WaypointManager; } // Forward declare WaypointManager

    class EditorController : public QObject {
        Q_OBJECT
    public:
        explicit EditorController(
            Map* map, // RME::Map
            QUndoStack* undoStack,
            SelectionManager* selectionManager, // RME::SelectionManager
            BrushManagerService* brushManagerService, // RME::BrushManagerService
            RME::core::WaypointManager* waypointManager, // New parameter
            QObject* parent = nullptr
        );
        ~EditorController() override;

    public slots:
        void applyBrushStroke(const QList<core::Position>& positions, const core::BrushSettings& settings, bool isEraseOperation);
        void deleteSelection();
        void placeOrMoveWaypoint(const QString& name, const RME::core::Position& targetPos);

    private:
        Map* m_map = nullptr;
        QUndoStack* m_undoStack = nullptr;
        SelectionManager* m_selectionManager = nullptr;
        BrushManagerService* m_brushManagerService = nullptr;
        RME::core::WaypointManager* m_waypointManager = nullptr; // New member
    };
} // namespace RME
#endif // RME_EDITORCONTROLLER_H
