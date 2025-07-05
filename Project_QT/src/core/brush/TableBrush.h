#ifndef RME_TABLE_BRUSH_H
#define RME_TABLE_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h"
#include "core/brush/BrushEnums.h" // For RME::BorderType

#include <QString>
#include <cstdint> // For uint16_t, uint32_t

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace assets { class MaterialData; struct MaterialTableSpecifics; }
    namespace editor { class EditorControllerInterface; }
} // namespace core
} // namespace RME

// Forward declaration for the test class (global namespace)
class TestTableBrush;

namespace RME {
namespace core {

class TableBrush : public RME::core::Brush {
    friend class ::TestTableBrush; // Friend class for testing

public:
    TableBrush();
    ~TableBrush() override = default;

    void setMaterial(const RME::core::assets::MaterialData* materialData);
    const RME::core::assets::MaterialData* getMaterial() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    int getLookID(const RME::core::BrushSettings& settings) const override;
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    static void initializeStaticData();

private:
    void updateTableAppearance(RME::core::editor::EditorControllerInterface* controller, const RME::core::Position& pos);
    uint16_t getRandomItemIdForAlignString(const QString& alignStr, const RME::core::assets::MaterialTableSpecifics* specifics) const;
    QString tableSegmentTypeToAlignString(RME::BorderType segmentType) const;
    const RME::core::assets::MaterialTableSpecifics* getCurrentTableSpecifics() const;

    const RME::core::assets::MaterialData* m_materialData = nullptr;

    static uint32_t s_table_types[256]; // Stores RME::BorderType for table segments
    static bool s_staticDataInitialized;
};

} // namespace core
} // namespace RME

#endif // RME_TABLE_BRUSH_H
