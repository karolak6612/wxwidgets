#include "core/brush/Brush.h"
#include "core/brush/BrushSettings.h" // Required for BrushSettings
#include "core/map/Map.h"           // Required for Map
#include "core/Position.h"          // Required for Position (though often included by Map or BrushSettings)

// Ensure EditorControllerInterface is declared if methods here were to use it.
// For now, it's only used by the pure virtual 'apply' in the header.
// namespace RME { namespace core { namespace editor { class EditorControllerInterface; }}}


namespace RME {
namespace core {

// Default implementation for getLookID
int Brush::getLookID(const BrushSettings& /*settings*/) const {
    // Default behavior, perhaps returning a generic icon ID or 0
    return 0;
}

// Default implementation for canApply
bool Brush::canApply(const map::Map* /*map*/, const Position& /*pos*/, const BrushSettings& /*settings*/) const {
    // Default behavior, assumes the brush can always be applied
    return true;
}

} // namespace core
} // namespace RME
