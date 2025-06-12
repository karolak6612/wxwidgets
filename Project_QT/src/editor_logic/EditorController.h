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

    class EditorController : public QObject {
        Q_OBJECT
    public:
        explicit EditorController(
            Map* map, // RME::Map
            QUndoStack* undoStack,
            SelectionManager* selectionManager, // RME::SelectionManager
            BrushManagerService* brushManagerService, // RME::BrushManagerService
            QObject* parent = nullptr
        );
        ~EditorController() override;

    public slots:
        void applyBrushStroke(const QList<Position>& positions, const BrushSettings& settings, bool isEraseOperation);
        void deleteSelection();

    private:
        Map* m_map = nullptr;
        QUndoStack* m_undoStack = nullptr;
        SelectionManager* m_selectionManager = nullptr;
        BrushManagerService* m_brushManagerService = nullptr;
    };
} // namespace RME
#endif // RME_EDITORCONTROLLER_H
