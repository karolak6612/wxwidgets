#include "core/brush/GroundBrush.h"
#include "core/assets/MaterialData.h"
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/editor/EditorControllerInterface.h"
#include "core/assets/AssetManager.h"
#include "core/brush/BrushEnums.h"
// #include "core/assets/ItemData.h" // For ItemData::materialId // Removed
// #include "core/assets/ItemDatabase.h" // For AssetManager::getItemDatabase() // Removed

#include <QRandomGenerator>
#include <QDebug>
// #include <array> // For std::array // Removed
// #include <algorithm> // For std::sort and std::find // Removed

// Static member definitions (as before)
// bool RME::core::GroundBrush::s_staticDataInitialized = false; // This will be removed from the header later

namespace RME {
namespace core {

// Constructor and other methods (as before)
GroundBrush::GroundBrush() : m_materialData(nullptr) {
    // initializeStaticData(); // Removed
}

void GroundBrush::setMaterial(const RME::core::assets::MaterialData* materialData) {
    if (materialData && materialData->isGround()) {
        m_materialData = materialData;
    } else {
        m_materialData = nullptr;
        qWarning() << "GroundBrush::setMaterial: Material is null or not a ground type.";
    }
}

const RME::core::assets::MaterialData* GroundBrush::getMaterial() const {
    return m_materialData;
}

const RME::core::assets::MaterialGroundSpecifics* GroundBrush::getCurrentGroundSpecifics() const {
    if (m_materialData && m_materialData->isGround()) {
        return std::get_if<RME::core::assets::MaterialGroundSpecifics>(&m_materialData->specificData);
    }
    return nullptr;
}

QString GroundBrush::getName() const {
    if (m_materialData) {
        return m_materialData->id;
    }
    return "Ground Brush";
}

int GroundBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    if (m_materialData) {
        const auto* specifics = getCurrentGroundSpecifics();
        if (specifics && !specifics->items.empty()) {
            if (m_materialData->lookId != 0) return m_materialData->lookId;
            if (m_materialData->serverLookId != 0) {
                 return m_materialData->serverLookId;
            }
        }
    }
    return 0;
}

bool GroundBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData || !getCurrentGroundSpecifics()) {
        return false;
    }
    if (!map) {
        return false;
    }
    return map->isPositionValid(pos);
}

void GroundBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) {
    if (!controller) {
        qWarning() << "GroundBrush::apply: Null controller.";
        return;
    }
    RME::core::map::Map* map = controller->getMap();
    if (!map) {
        qWarning() << "GroundBrush::apply: Null map from controller.";
        return;
    }
    if (!m_materialData) {
        qWarning() << "GroundBrush::apply: No material set.";
        return;
    }
    const auto* materialSpecifics = getCurrentGroundSpecifics();
    if (!materialSpecifics || (!settings.isEraseMode && materialSpecifics->items.empty())) {
        qWarning() << "GroundBrush::apply: Material has no ground items defined for drawing.";
        if (!settings.isEraseMode) return;
    }

    RME::core::Tile* tile = controller->getTileForEditing(pos);
    if (!tile) {
        qWarning() << "GroundBrush::apply: Failed to get tile for editing at" << pos.x << pos.y << pos.z;
        return;
    }

    uint16_t oldGroundItemId = 0;
    const Item* currentGround = tile->getGround();
    if (currentGround) {
        oldGroundItemId = currentGround->getID();
    }

    if (settings.isEraseMode) {
        if (oldGroundItemId != 0) {
            controller->recordSetGroundItem(pos, 0, oldGroundItemId); // Erase: new ID is 0
            qDebug() << "GroundBrush: Called recordSetGroundItem to erase ground at" << pos.x << pos.y << pos.z << "(was " << oldGroundItemId << ")";
        } else {
            qDebug() << "GroundBrush: Erase mode, but no ground to erase at" << pos.x << pos.y << pos.z;
            // No ground change, but borders might still need update if neighbors changed.
        }
    } else { // Drawing mode
        // This check should be done before `materialSpecifics->items.empty()` if materialSpecifics can be null.
        // It was done at the start of the function.
        // if (!materialSpecifics || materialSpecifics->items.empty()) { /* already handled */ }

        int totalChance = 0;
        if (materialSpecifics){ // materialSpecifics is already checked for null at the start of the function along with items.empty() for non-erase mode
            for (const auto& itemEntry : materialSpecifics->items) {
                totalChance += itemEntry.chance;
            }
            if (totalChance == 0 && !materialSpecifics->items.empty()) {
                 totalChance = materialSpecifics->items.first().chance > 0 ? materialSpecifics->items.first().chance : 1; // Ensure some chance if items exist
            }
        }

        if (totalChance == 0) { // Still 0 means no items or all items have 0 chance and items list was empty.
             qWarning() << "GroundBrush::apply: Total chance for ground items is 0.";
            return;
        }

        uint16_t selectedItemId = materialSpecifics->items.first().itemId;
        int randomValue = QRandomGenerator::global()->bounded(totalChance);
        int currentChanceSum = 0;
        for (const auto& itemEntry : materialSpecifics->items) {
            currentChanceSum += itemEntry.chance;
            if (randomValue < currentChanceSum) {
                selectedItemId = itemEntry.itemId;
                break;
            }
        }

        // Record action even if item ID is the same, as this ensures undo stack consistency
        // and allows the controller's action to handle no-op if necessary.
        // Or, add a check: if (oldGroundItemId != selectedItemId)
        controller->recordSetGroundItem(pos, selectedItemId, oldGroundItemId);
        qDebug() << "GroundBrush: Called recordSetGroundItem to draw ground item" << selectedItemId << "at" << pos.x << pos.y << pos.z << "(old was " << oldGroundItemId << ")";
    }

    // Auto-Bordering Phase (remains the same)
    // doAutoBorders(controller, pos, settings); // Removed

    // static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1}; // Removed
    // static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1}; // Removed

    // for (int i = 0; i < 8; ++i) { // Removed
    //     RMEPosition neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
    //     if (map->isPositionValid(neighborPos)) {
    //         doAutoBorders(controller, neighborPos, settings);
    //     }
    // }

    controller->notifyTileChanged(pos);
    // for (int i = 0; i < 8; ++i) { // Removed
    //     RMEPosition neighborPos(pos.x + dx[i], pos.y + dy[i], pos.z);
    //     if (map->isPositionValid(neighborPos)) {
    //         controller->notifyTileChanged(neighborPos);
    //     }
    // }
}

// ... (apply method and other GroundBrush methods as before) ...
} // namespace core
} // namespace RME
