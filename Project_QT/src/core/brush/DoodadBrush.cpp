#include "core/brush/DoodadBrush.h"
#include "core/assets/MaterialData.h" // Includes MaterialDoodadSpecifics, MaterialAlternate, MaterialCompositeTile
#include "core/map/Map.h"
#include "core/Tile.h"
#include "core/Item.h"
#include "core/assets/ItemData.h"     // For getLookID
#include "core/assets/ItemDatabase.h" // For getLookID
#include "core/editor/EditorControllerInterface.h"
#include "core/settings/BrushSettings.h"
#include "core/assets/AssetManager.h" // For ItemDatabase in getLookID

#include <QRandomGenerator>
#include <QDebug>
#include <QSet> // For collecting affected positions
#include <array> // Not strictly needed by this cpp, but often included with brush files
#include <algorithm> // For std::sort, if used (not directly in this provided code)

namespace RME {
namespace core {

DoodadBrush::DoodadBrush() : m_materialData(nullptr) {
    // Static data initialization if any (not typical for DoodadBrush unless it has lookup tables)
}

void DoodadBrush::setMaterial(const RME::core::assets::MaterialData* materialData) {
    if (materialData && materialData->isDoodad()) {
        m_materialData = materialData;
    } else {
        m_materialData = nullptr;
        qWarning() << "DoodadBrush::setMaterial: Material is null or not a doodad type.";
    }
}

const RME::core::assets::MaterialData* DoodadBrush::getMaterial() const {
    return m_materialData;
}

const RME::core::assets::MaterialDoodadSpecifics* DoodadBrush::getCurrentDoodadSpecifics() const {
    if (m_materialData && m_materialData->isDoodad()) {
        return std::get_if<RME::core::assets::MaterialDoodadSpecifics>(&m_materialData->specificData);
    }
    return nullptr;
}

QString DoodadBrush::getName() const {
    if (m_materialData) {
        return m_materialData->id;
    }
    return QStringLiteral("Doodad Brush");
}

int DoodadBrush::getLookID(const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData) return 0;

    if (m_materialData->lookId != 0) {
        return m_materialData->lookId;
    }

    const auto* specifics = getCurrentDoodadSpecifics();
    if (specifics) {
        const auto* defaultAlternate = selectAlternate(specifics, 0);
        if (defaultAlternate) {
            uint16_t serverItemId = 0;
            if (!defaultAlternate->singleItemIds.isEmpty()) {
                serverItemId = defaultAlternate->singleItemIds.first();
            } else if (!defaultAlternate->compositeTiles.isEmpty() && !defaultAlternate->compositeTiles.first().itemIds.isEmpty()) {
                serverItemId = defaultAlternate->compositeTiles.first().itemIds.first();
            }

            if (serverItemId != 0) {
                qWarning("DoodadBrush 'getLookID': Material %s has no client lookId. Attempting to use server ID %u from default alternate. THIS REQUIRES CONVERSION by MaterialManager or caller.",
                         qUtf8Printable(m_materialData->id), serverItemId);
                return 0;
            }
        }
    }

    if (m_materialData->serverLookId != 0) {
        qWarning("DoodadBrush 'getLookID': Material %s has serverLookId %u but no client lookId. THIS REQUIRES CONVERSION by MaterialManager or caller.",
                 qUtf8Printable(m_materialData->id), m_materialData->serverLookId);
        return 0;
    }

    qWarning("DoodadBrush 'getLookID': Material %s has no usable lookId information.", qUtf8Printable(m_materialData->id));
    return 0;
}

bool DoodadBrush::canApply(const RME::core::map::Map* map,
                             const RME::core::Position& pos,
                             const RME::core::BrushSettings& /*settings*/) const {
    if (!m_materialData) return false;
    const auto* specifics = getCurrentDoodadSpecifics();
    if (!specifics || specifics->alternates.isEmpty()) {
        qWarning("DoodadBrush::canApply: No alternates defined for doodad material %s", qUtf8Printable(m_materialData->id));
        return false;
    }
    if (!map || !map->isPositionValid(pos)) return false;
    return true;
}

const RME::core::assets::MaterialAlternate* DoodadBrush::selectAlternate(
    const RME::core::assets::MaterialDoodadSpecifics* specifics,
    int variationIndex) const {
    if (!specifics || specifics->alternates.isEmpty()) {
        return nullptr;
    }
    if (variationIndex < 0) variationIndex = 0;
    int index = variationIndex % specifics->alternates.size();
    return &specifics->alternates[index];
}

void DoodadBrush::apply(RME::core::editor::EditorControllerInterface* controller,
                          const RME::core::Position& pos,
                          const RME::core::BrushSettings& settings) {
    if (!controller || !canApply(controller->getMap(), pos, settings)) {
        return;
    }

    const auto* doodadSpecifics = getCurrentDoodadSpecifics();
    if (!doodadSpecifics) return;

    int variation = settings.getGenericBrushParameter("variationIndex", 0).toInt();
    const RME::core::assets::MaterialAlternate* chosenAlternate = selectAlternate(doodadSpecifics, variation);

    if (!chosenAlternate) {
        qWarning("DoodadBrush::apply: No alternate found for material %s with variation %d.",
                 qUtf8Printable(m_materialData->id), variation);
        return;
    }

    QSet<RME::core::Position> affectedTiles;
    affectedTiles.insert(pos);

    if (settings.isEraseMode) {
        if (!chosenAlternate->singleItemIds.isEmpty()) {
            for (uint16_t itemId : chosenAlternate->singleItemIds) {
                controller->recordRemoveItem(pos, itemId);
                qDebug("DoodadBrush::apply (erase): Removing single item %u at %s", itemId, qUtf8Printable(pos.toString()));
            }
        } else if (!chosenAlternate->compositeTiles.isEmpty()) {
            for (const auto& compTile : chosenAlternate->compositeTiles) {
                RME::core::Position actualPos = pos.translated(compTile.x, compTile.y, compTile.z);
                for (uint16_t itemId : compTile.itemIds) {
                    controller->recordRemoveItem(actualPos, itemId);
                    qDebug("DoodadBrush::apply (erase): Removing composite item %u at %s (offset %d,%d,%d)",
                           itemId, qUtf8Printable(actualPos.toString()), compTile.x, compTile.y, compTile.z);
                }
                affectedTiles.insert(actualPos);
            }
        }
    } else { // Drawing mode
        if (!chosenAlternate->singleItemIds.isEmpty()) {
            for (uint16_t itemId : chosenAlternate->singleItemIds) {
                controller->recordAddItem(pos, itemId);
                qDebug("DoodadBrush::apply (draw): Adding single item %u at %s", itemId, qUtf8Printable(pos.toString()));
            }
        } else if (!chosenAlternate->compositeTiles.isEmpty()) {
            for (const auto& compTile : chosenAlternate->compositeTiles) {
                RME::core::Position actualPos = pos.translated(compTile.x, compTile.y, compTile.z);
                for (uint16_t itemId : compTile.itemIds) {
                    controller->recordAddItem(actualPos, itemId);
                    qDebug("DoodadBrush::apply (draw): Adding composite item %u at %s (offset %d,%d,%d)",
                           itemId, qUtf8Printable(actualPos.toString()), compTile.x, compTile.y, compTile.z);
                }
                affectedTiles.insert(actualPos);
            }
        } else {
            qWarning("DoodadBrush::apply: Chosen alternate for material %s has no single items or composite tiles.",
                     qUtf8Printable(m_materialData->id));
        }
    }

    for (const RME::core::Position& affectedPos : affectedTiles) {
        if(controller->getMap() && controller->getMap()->isPositionValid(affectedPos)){
             controller->notifyTileChanged(affectedPos);
        }
    }
}

} // namespace core
} // namespace RME
