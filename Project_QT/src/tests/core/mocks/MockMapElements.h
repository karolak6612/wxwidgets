#ifndef MOCKMAPELEMENTS_H
#define MOCKMAPELEMENTS_H

#include "Project_QT/src/core/Position.h"
#include "Project_QT/src/core/Tile.h"     // To inherit from, or mimic interface
#include "Project_QT/src/core/Item.h"      // To inherit from, or mimic interface
#include "Project_QT/src/core/Creature.h"  // To inherit from, or mimic interface
#include "Project_QT/src/core/Spawn.h"     // To inherit from, or mimic interface
#include "Project_QT/src/core/map/Map.h"   // To inherit from, or mimic interface
#include <QObject>
#include <QList>
#include <QSet>
#include <QMap> // For MockMap m_tiles
#include <QVariant> // For Item attributes placeholder

// --- MockItem ---
class MockItem : public RME::Item {
public:
    MockItem(uint16_t id) : m_id(id), m_selected(false) {}
    uint16_t getID() const { return m_id; }

    // RME::Item interface
    bool isSelected() const override { return m_selected; }
    void setSelected(bool selected) override { m_selected = selected; }

    // Minimal RME::Item pure virtuals (assuming these based on typical OT editor needs)
    RME::ItemType* getItemType() const override { return nullptr; } // Simplified
    void setSubType(uint16_t) override {}
    uint16_t getSubType() const override { return 0; }
    QVariant getAttribute(const QString&) const override { return QVariant(); }
    void setAttribute(const QString&, const QVariant&) override {}
    RME::Item* deepCopy() const override { auto* copy = new MockItem(m_id); copy->m_selected = m_selected; return copy; }
    bool isGroundTile() const override { return false; }
    bool isBorder() const override { return false; }
    bool isAlwaysOnTop() const override { return false; }
    uint8_t getStackOrder() const override { return 0; }


private:
    uint16_t m_id;
    bool m_selected;
};

// --- MockCreature ---
class MockCreature : public RME::Creature {
public:
    MockCreature(const QString& name) : m_name(name), m_selected(false) {}
    // RME::Creature interface
    bool isSelected() const override { return m_selected; }
    void setSelected(bool selected) override { m_selected = selected; }
    QString getName() const { return m_name; }

    // Minimal RME::Creature pure virtuals
    RME::Creature* deepCopy() const override { auto* copy = new MockCreature(m_name); copy->m_selected = m_selected; return copy; }
    void setOutfit(const RME::Outfit&) override {}
    RME::Outfit getOutfit() const override { return RME::Outfit(); } // Assuming Outfit has default constructor
    uint16_t getLookCorpse() const override { return 0;}


private:
    QString m_name;
    bool m_selected;
};

// --- MockSpawn ---
class MockSpawn : public RME::Spawn {
public:
    MockSpawn(uint16_t radius = 1) : m_radius(radius), m_selected(false) {}
    bool isSelected() const override { return m_selected; }
    void setSelected(bool selected) override { m_selected = selected; }
    uint16_t getRadius() const { return m_radius; }
    void setRadius(uint16_t r) override { m_radius = r; }
    // Minimal RME::Spawn pure virtuals
    RME::Spawn* deepCopy() const override { auto* copy = new MockSpawn(m_radius); copy->m_selected = m_selected; return copy; }
    bool addCreature(const QString&, int) override { return false; }
    void clearCreatures() override {}
    const QList<RME::SpawnCreature>& getCreatureList() const override { static QList<RME::SpawnCreature> l; return l; }
    // Added from RME::Spawn in CORE-01
    void setTile(RME::Tile*) override {} // No-op for mock
    RME::Tile* getTile() const override { return nullptr; } // No-op for mock


private:
    uint16_t m_radius;
    bool m_selected;
};


// --- MockTile ---
class MockTile : public RME::Tile {
public:
    MockTile(const RME::Position& pos) : m_position(pos), m_selected(false), m_creature(nullptr), m_spawn(nullptr), m_houseId(0), m_flags(0), m_map(nullptr) {}
    ~MockTile() override { qDeleteAll(m_items); m_items.clear(); delete m_creature; delete m_spawn; }

    // RME::Tile interface
    const RME::Position& getPosition() const override { return m_position; }
    // setPosition is not virtual in RME::Tile from CORE-01, so cannot override.
    // void setPosition(const RME::Position& pos) override { m_position = pos; }
    bool isSelected() const override { return m_selected; }
    void setSelected(bool selected) override { m_selected = selected; }

    // House related methods from RME::Tile, overridden for mock control
    uint32_t getHouseId() const override { return m_houseId_mock; }
    void setHouseId(uint32_t houseId) override { m_houseId_mock = houseId; }
    bool isHouseExit() const override { return m_isHouseExit_mock; }
    void setIsHouseExit(bool isExit) override { m_isHouseExit_mock = isExit; }

    QList<RME::Item*> getItems() const override { return m_items; }
    void addItem(RME::Item* item, bool autodelete_on_fail = false) override { if(item) m_items.append(item); else if(autodelete_on_fail && item) delete item; }
    bool removeItem(RME::Item* item, bool delete_item = true) override { bool removed = m_items.removeOne(item); if(removed && delete_item) delete item; return removed; }

    RME::Creature* getCreature() const override { return m_creature; }
    void addCreature(RME::Creature* c) override { delete m_creature; m_creature = c; if(c) c->setTile(this); }
    RME::Creature* removeCreature(bool delete_creature = true) override { auto old = m_creature; m_creature = nullptr; if(old && delete_creature) delete old; return old; }

    RME::Spawn* getSpawn() const override { return m_spawn; }
    void setSpawn(RME::Spawn* s) override { delete m_spawn; m_spawn = s; if(s) s->setTile(this); }
    RME::Spawn* removeSpawn(bool delete_spawn = true) override { auto old = m_spawn; m_spawn = nullptr; if(old && delete_spawn) delete old; return old;}


    uint32_t getHouseId() const override { return m_houseId; }
    void setHouseId(uint32_t id) override { m_houseId = id; }
    uint32_t getFlags() const override { return m_flags; }
    void setFlags(uint32_t flags) override { m_flags = flags; }

    bool hasSelectedElements() const override {
        if (m_selected) return true;
        for (RME::Item* item : m_items) { if (item && item->isSelected()) return true; }
        if (m_creature && m_creature->isSelected()) return true;
        if (m_spawn && m_spawn->isSelected()) return true;
        return false;
    }

    // Implement other pure virtuals from RME::Tile if any
    RME::Ground* getGround() const override { return nullptr; } // Simplified
    void setGround(RME::Ground*) override {}
    RME::Tile* deepCopy(RME::Map* targetMap) const override {
        MockTile* newTile = new MockTile(m_position);
        newTile->m_selected = m_selected;
        newTile->m_houseId = m_houseId;
        newTile->m_flags = m_flags;
        for(RME::Item* item : m_items) newTile->addItem(item->deepCopy(), false);
        if(m_creature) newTile->addCreature(m_creature->deepCopy());
        if(m_spawn) newTile->setSpawn(m_spawn->deepCopy());
        newTile->setMap(targetMap);
        return newTile;
    }
    void clear() override {
        qDeleteAll(m_items); m_items.clear();
        delete m_creature; m_creature = nullptr;
        delete m_spawn; m_spawn = nullptr;
        m_flags = 0; m_houseId = 0; m_selected = false;
        // ground is not handled by this mock's clear
    }
    bool isEmptyAndClean() const override { return m_items.isEmpty() && !m_creature && !m_spawn && m_flags == 0 && m_houseId == 0; }
    RME::TileLocation* getLocation() const override {return nullptr;} // Not mocked
    void setLocation(RME::TileLocation*) override {}
    uint8_t getDrawElevation() const override { return 0; }
    void setDrawElevation(uint8_t) override {}
    uint32_t getDoodadOrder() const override { return 0; }
    void setDoodadOrder(uint32_t) override {}
    RME::Map* getMap() const override { return m_map; }
    void setMap(RME::Map* map) override { m_map = map; }


public: // Mock-specific helpers
    RME::Position m_position;
    bool m_selected;
    QList<RME::Item*> m_items;
    RME::Creature* m_creature;
    RME::Spawn* m_spawn;
    uint32_t m_houseId;
    uint32_t m_flags;
    RME::Map* m_map;
    // Mock-specific members for house properties if we override getters/setters
    uint32_t m_houseId_mock = 0;
    bool m_isHouseExit_mock = false;
};

// --- MockMap ---
class MockMap : public RME::Map {
public:
    MockMap() {}
    ~MockMap() override { qDeleteAll(m_tiles); }

    RME::Tile* getTile(const RME::Position& pos) const override {
        return m_tiles.value(pos, nullptr);
    }
    RME::Tile* getOrCreateTile(const RME::Position& pos) override {
        if (!m_tiles.contains(pos)) {
            MockTile* newTile = new MockTile(pos);
            newTile->setMap(this);
            m_tiles[pos] = newTile;
        }
        return m_tiles.value(pos);
    }
    void removeTile(const RME::Position& pos, bool /*update_quadtree*/) override {
        delete m_tiles.take(pos);
    }
    void markTileDirty(const RME::Position&) override { /* usually for rendering */ }
    bool isValidPosition(const RME::Position& pos) const override { return pos.x >=0 && pos.y >=0 && pos.z >=0 && pos.x < 1000 && pos.y < 1000 && pos.z < 16; } // Keep it simple

    // Implement other pure virtuals from RME::Map if any
    int width() const override { return 1000; }
    int height() const override { return 1000; }
    int floors() const override { return 16; }
    RME::TownManager& getTowns() override { static RME::TownManager tm(this); return tm; }
    RME::HouseManager& getHouses() override { static RME::HouseManager hm(this); return hm; }
    RME::WaypointManager& getWaypoints() override { static RME::WaypointManager wm(this); return wm; }
    // RME::SpawnManager& getSpawns() override { static RME::SpawnManager sm(this); return sm; } // If SpawnManager exists
    // RME::CreatureManager& getCreatures() override { static RME::CreatureManager crm(this); return crm; } // If CreatureManager exists
    void setPath(const QString&) override {}
    QString getPath() const override { return ""; }
    void setDescription(const QString&, const QString&) override {} // Assuming two args for description and author
    QString getDescription(RME::MapDescriptionType type = RME::MapDescriptionType::DESCRIPTION) const override { Q_UNUSED(type); return ""; }


public: // Mock-specific helpers
    QMap<RME::Position, RME::Tile*> m_tiles;
};

// Ensure RME::Position has QHash and operator== for QMap/QSet
// These should ideally be part of Position.h itself.
// If not, define them here or in a common test utility.
// Assuming Position.h from CORE-01 would have included these for QSet/QMap usage.
// For example:
// inline bool operator==(const RME::Position& p1, const RME::Position& p2) {
//     return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
// }
// inline uint qHash(const RME::Position& key, uint seed = 0) {
//     return qHash(static_cast<quint16>(key.x), seed) ^ qHash(static_cast<quint16>(key.y), seed << 16) ^ qHash(static_cast<quint8>(key.z), seed << 24);
// }
// And ensure RME::Position is registered with Q_DECLARE_METATYPE if used in QVariant without prior registration.

#endif // MOCKMAPELEMENTS_H
