#include "Brush.h"
// No specific includes needed yet beyond Brush.h for these default implementations.
// Map, Position, BrushSettings, EditorControllerInterface are forward declared in .h
// If default implementations needed to access members of those, then full headers would be required.

namespace RME {

// Default implementation for getLookID
int Brush::getLookID(const BrushSettings& /*settings*/) const {
    return 0; // No specific icon by default
}

// Default implementation for canApply
bool Brush::canApply(const Map* /*map*/,
                     const Position& /*pos*/,
                     const BrushSettings& /*settings*/) const {
    return true; // Can always apply by default
}

// Virtual destructor definition, even if defaulted in header, can be provided in .cpp
// This ensures vtable is emitted in this translation unit.
// Brush::~Brush() = default; // Already defaulted in .h, so this is optional here.

} // namespace RME
