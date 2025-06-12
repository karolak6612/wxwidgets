#ifndef RME_CARPET_BRUSH_H
#define RME_CARPET_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h"
#include "core/brush/BrushEnums.h" // For BorderType (used by s_carpet_types conceptually)

#include <QString>
#include <cstdint> // For uint16_t, uint32_t

// Forward declarations
namespace RME { namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace assets { class MaterialData; struct MaterialCarpetSpecifics; struct MaterialItemEntry; }
    namespace editor { class EditorControllerInterface; }
}}

namespace RME {
namespace core {

class ::TestCarpetBrush; // Forward declaration

class CarpetBrush : public RME::core::Brush {
    friend class ::TestCarpetBrush;
public:
    CarpetBrush();
    ~CarpetBrush() override = default;

    void setMaterial(const RME::core::assets::MaterialData* materialData);
    const RME::core::assets::MaterialData* getMaterial() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    int getLookID(const RME::core::BrushSettings& settings) const override; // Based on material's lookid or a center piece
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    // Specific static initialization for CarpetBrush (e.g., for s_carpet_types table)
    static void initializeStaticData();

private:
    // Main helper for determining carpet appearance and triggering changes
    void updateCarpetAppearance(RME::core::editor::EditorControllerInterface* controller,
                                const RME::core::Position& pos,
                                const RME::core::map::Map* map, // Map needed for neighbor analysis
                                const RME::core::assets::MaterialData* currentBrushMaterial); // Pass current material to avoid race conditions

    // Gets a specific item ID for a given alignment string and material specifics
    uint16_t getRandomItemIdForAlignment(const QString& alignStr,
                                         const RME::core::assets::MaterialCarpetSpecifics* carpetSpecifics) const;

    // Translates a BorderType enum (from s_carpet_types) to an alignment string (e.g., "n", "ne", "center")
    QString borderTypeToAlignmentString(RME::BorderType borderType) const;


    const RME::core::assets::MaterialData* m_materialData = nullptr;

    // The carpet_types lookup table, similar to wxwidgets and GroundBrush.
    // It maps an 8-bit neighbor configuration to a BorderType enum value,
    // which then needs to be translated to an 'align' string for MaterialData.
    static uint32_t s_carpet_types[256]; // Stores one BorderType enum (0-13 conceptually)
    static bool s_staticDataInitialized;
};

} // namespace core
} // namespace RME

#endif // RME_CARPET_BRUSH_H
