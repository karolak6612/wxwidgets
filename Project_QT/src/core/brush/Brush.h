#ifndef RME_BRUSH_H
#define RME_BRUSH_H

#include <QString>
// Forward declarations
namespace RME { namespace core {
class Position;
struct BrushSettings;
namespace map { class Map; }
namespace editor { class EditorControllerInterface; }
}}

namespace RME {
namespace core {

class Brush {
public:
    virtual ~Brush() = default;

    // Pure virtual methods
    virtual void apply(editor::EditorControllerInterface* controller, const Position& pos, const BrushSettings& settings) = 0;
    virtual QString getName() const = 0;

    // Virtual methods with default implementations
    virtual int getLookID(const BrushSettings& settings) const;
    virtual bool canApply(const map::Map* map, const Position& pos, const BrushSettings& settings) const;
};

} // namespace core
} // namespace RME

#endif // RME_BRUSH_H
