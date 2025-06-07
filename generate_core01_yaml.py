import yaml
import os
import hashlib

def get_md5_hash(content):
    return hashlib.md5(content.encode('utf-8')).hexdigest()

def get_content_lite(content, lines=200):
    return "\\n".join(content.splitlines()[:lines])

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if "\\n" in data or "\n" in data: # Check for newlines
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Content of the C++ files
cpp_files_content = {
    "wxwidgets/tile.cpp": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "brush.h"

#include "tile.h"
#include "creature.h"
#include "house.h"
#include "basemap.h"
#include "spawn.h"
#include "ground_brush.h"
#include "wall_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"
#include "town.h"
#include "map.h"

Tile::Tile(int x, int y, int z) :
	location(nullptr),
	ground(nullptr),
	creature(nullptr),
	spawn(nullptr),
	house_id(0),
	mapflags(0),
	statflags(0),
	minimapColor(INVALID_MINIMAP_COLOR) {
	////
}

Tile::Tile(TileLocation& loc) :
	location(&loc),
	ground(nullptr),
	creature(nullptr),
	spawn(nullptr),
	house_id(0),
	mapflags(0),
	statflags(0),
	minimapColor(INVALID_MINIMAP_COLOR) {
	////
}

Tile::~Tile() {
	bool had_items = !items.empty();
	bool had_ground = ground != nullptr;

#ifdef __WXDEBUG__
	if (had_ground) {
		// Store ground info before deleting it
		uint16_t ground_id = ground->getID();
		void* ground_ptr = ground;
		printf("DEBUG: Tile destructor for %p with ground %p (ID:%d)\\n", this, ground_ptr, ground_id);

		// Get call stack info by adding a breakpoint variable
		int debug_breakpoint_for_ground_deletion = 1;
	}
#endif

	while (!items.empty()) {
		delete items.back();
		items.pop_back();
	}
	delete creature;
	delete ground;
	delete spawn;

#ifdef __WXDEBUG__
	if (had_ground) {
		printf("DEBUG: Ground %p deleted\\n", ground);
	}
#endif
}

Tile* Tile::deepCopy(BaseMap& map) {
	Tile* copy = map.allocator.allocateTile(location);
	copy->flags = flags;
	copy->house_id = house_id;

#ifdef __WXDEBUG__
	printf("DEBUG: deepCopy - Creating copy of tile %p (with ground %p)\\n",
		this, ground);
#endif

	if (spawn) {
		copy->spawn = spawn->deepCopy();
	}
	if (creature) {
		copy->creature = creature->deepCopy();
	}
	// Spawncount & exits are not transferred on copy!
	if (ground) {
#ifdef __WXDEBUG__
		printf("DEBUG: deepCopy - Copying ground %p with ID %d\\n",
			ground, ground->getID());
#endif
		copy->ground = ground->deepCopy();
#ifdef __WXDEBUG__
		printf("DEBUG: deepCopy - Ground copied to %p with ID %d\\n",
			copy->ground, copy->ground->getID());
#endif
	}

	copy->setZoneIds(this);

	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		copy->items.push_back((*it)->deepCopy());
		++it;
	}

#ifdef __WXDEBUG__
	printf("DEBUG: deepCopy - Created tile copy %p (with ground %p)\\n",
		copy, copy->ground);
#endif

	return copy;
}

uint32_t Tile::memsize() const {
	uint32_t mem = sizeof(*this);
	if (ground) {
		mem += ground->memsize();
	}

	ItemVector::const_iterator it;

	it = items.begin();
	while (it != items.end()) {
		mem += (*it)->memsize();
		++it;
	}

	mem += sizeof(Item*) * items.capacity();

	return mem;
}

int Tile::size() const {
	int sz = 0;
	if (ground) {
		++sz;
	}
	sz += items.size();
	if (creature) {
		++sz;
	}
	if (spawn) {
		++sz;
	}
	if (location) {
		if (location->getHouseExits()) {
			++sz;
		}
		if (location->getSpawnCount()) {
			++sz;
		}
		if (location->getWaypointCount()) {
			++sz;
		}
	}
	return sz;
}

void Tile::merge(Tile* other) {
	if (other->isPZ()) {
		setPZ(true);
	}
	if (other->house_id) {
		house_id = other->house_id;
	}

	if (other->ground) {
		delete ground;
		ground = other->ground;
		other->ground = nullptr;
	}

	if (other->creature) {
		delete creature;
		creature = other->creature;
		other->creature = nullptr;
	}

	if (other->spawn) {
		delete spawn;
		spawn = other->spawn;
		other->spawn = nullptr;
	}

	if (other->creature) {
		delete creature;
		creature = other->creature;
		other->creature = nullptr;
	}

	ItemVector::iterator it;

	it = other->items.begin();
	while (it != other->items.end()) {
		addItem(*it);
		++it;
	}
	other->items.clear();
}

bool Tile::hasProperty(enum ITEMPROPERTY prop) const {
	if (prop == PROTECTIONZONE && isPZ()) {
		return true;
	}

	if (ground && ground->hasProperty(prop)) {
		return true;
	}

	ItemVector::const_iterator iit;
	for (iit = items.begin(); iit != items.end(); ++iit) {
		if ((*iit)->hasProperty(prop)) {
			return true;
		}
	}

	return false;
}

int Tile::getIndexOf(Item* item) const {
	if (!item) {
		return wxNOT_FOUND;
	}

	int index = 0;
	if (ground) {
		if (ground == item) {
			return index;
		}
		index++;
	}

	if (!items.empty()) {
		auto it = std::find(items.begin(), items.end(), item);
		if (it != items.end()) {
			index += (it - items.begin());
			return index;
		}
	}
	return wxNOT_FOUND;
}

Item* Tile::getTopItem() const {
	if (!items.empty() && !items.back()->isMetaItem()) {
		return items.back();
	}
	if (ground && !ground->isMetaItem()) {
		return ground;
	}
	return nullptr;
}

Item* Tile::getItemAt(int index) const {
	if (index < 0) {
		return nullptr;
	}
	if (ground) {
		if (index == 0) {
			return ground;
		}
		index--;
	}
	if (index >= 0 && index < items.size()) {
		return items.at(index);
	}
	return nullptr;
}

void Tile::addItem(Item* item) {
	if (!item) {
		return;
	}

	// Handle ground tiles
	if (item->isGroundTile()) {
#ifdef __WXDEBUG__
		printf("DEBUG: Adding ground tile ID %d to position %d,%d,%d\\n",
			item->getID(), getPosition().x, getPosition().y, getPosition().z);
#endif
		// Always delete the existing ground first
		delete ground;
		ground = item;

		// Also check for any ground items that might be in the items list
		// and remove them to prevent stacking issues
		ItemVector::iterator it = items.begin();
		while (it != items.end()) {
			if ((*it)->isGroundTile() || (*it)->getGroundEquivalent() != 0) {
#ifdef __WXDEBUG__
				printf("DEBUG: Removing misplaced ground item with ID %d from tile items\\n", (*it)->getID());
#endif
				delete *it;
				it = items.erase(it);
			} else {
				++it;
			}
		}
		return;
	}

	// Handle items with ground equivalents
	uint16_t gid = item->getGroundEquivalent();
	if (gid != 0) {
		// If item has a ground equivalent, replace the ground
		delete ground;
		ground = Item::Create(gid);

		// Insert at the very bottom of the stack
		items.insert(items.begin(), item);

		if (item->isSelected()) {
			statflags |= TILESTATE_SELECTED;
		}
		return;
	}

	// Handle normal items
	ItemVector::iterator it;
	if (item->isAlwaysOnBottom()) {
		it = items.begin();
		while (true) {
			if (it == items.end()) {
				break;
			} else if ((*it)->isAlwaysOnBottom()) {
				if (item->getTopOrder() < (*it)->getTopOrder()) {
					break;
				}
			} else { // Always on top
				break;
			}
			++it;
		}
	} else {
		it = items.end();
	}

	items.insert(it, item);

	if (item->isSelected()) {
		statflags |= TILESTATE_SELECTED;
	}
}

void Tile::select() {
	if (size() == 0) {
		return;
	}
	if (ground) {
		ground->select();
	}
	if (spawn) {
		spawn->select();
	}
	if (creature) {
		creature->select();
	}

	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		(*it)->select();
		++it;
	}

	statflags |= TILESTATE_SELECTED;
}

void Tile::deselect() {
	if (ground) {
		ground->deselect();
	}
	if (spawn) {
		spawn->deselect();
	}
	if (creature) {
		creature->deselect();
	}

	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		(*it)->deselect();
		++it;
	}

	statflags &= ~TILESTATE_SELECTED;
}

Item* Tile::getTopSelectedItem() {
	for (ItemVector::reverse_iterator iter = items.rbegin(); iter != items.rend(); ++iter) {
		if ((*iter)->isSelected() && !(*iter)->isMetaItem()) {
			return *iter;
		}
	}
	if (ground && ground->isSelected() && !ground->isMetaItem()) {
		return ground;
	}
	return nullptr;
}

ItemVector Tile::popSelectedItems(bool ignoreTileSelected) {
	ItemVector pop_items;

	if (!ignoreTileSelected && !isSelected()) {
		return pop_items;
	}

	if (ground && ground->isSelected()) {
		pop_items.push_back(ground);
		ground = nullptr;
	}

	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isSelected()) {
			pop_items.push_back(*it);
			it = items.erase(it);
		} else {
			++it;
		}
	}

	statflags &= ~TILESTATE_SELECTED;
	return pop_items;
}

ItemVector Tile::getSelectedItems(bool unzoomed) {
	ItemVector selected_items;

	if (!isSelected()) {
		return selected_items;
	}

	if (ground && ground->isSelected()) {
		selected_items.push_back(ground);
	}

	// save performance when zoomed out
	if (!unzoomed) {
		ItemVector::iterator it;

		it = items.begin();
		while (it != items.end()) {
			if ((*it)->isSelected()) {
				selected_items.push_back(*it);
			}
			it++;
		}
	}

	return selected_items;
}

uint8_t Tile::getMiniMapColor() const {
	if (minimapColor != INVALID_MINIMAP_COLOR) {
		return minimapColor;
	}

	for (ItemVector::const_reverse_iterator item_iter = items.rbegin(); item_iter != items.rend(); ++item_iter) {
		if ((*item_iter)->getMiniMapColor()) {
			return (*item_iter)->getMiniMapColor();
			break;
		}
	}

	// check ground too
	if (hasGround()) {
		return ground->getMiniMapColor();
	}

	return 0;
}

bool tilePositionLessThan(const Tile* a, const Tile* b) {
	return a->getPosition() < b->getPosition();
}

bool tilePositionVisualLessThan(const Tile* a, const Tile* b) {
	Position pa = a->getPosition();
	Position pb = b->getPosition();

	if (pa.z > pb.z) {
		return true;
	}
	if (pa.z < pb.z) {
		return false;
	}

	if (pa.y < pb.y) {
		return true;
	}
	if (pa.y > pb.y) {
		return false;
	}

	if (pa.x < pb.x) {
		return true;
	}

	return false;
}

void Tile::update() {
	statflags &= TILESTATE_MODIFIED;

	if (spawn && spawn->isSelected()) {
		statflags |= TILESTATE_SELECTED;
	}
	if (creature && creature->isSelected()) {
		statflags |= TILESTATE_SELECTED;
	}

	if (ground) {
		if (ground->isSelected()) {
			statflags |= TILESTATE_SELECTED;
		}
		if (ground->isBlocking()) {
			statflags |= TILESTATE_BLOCKING;
		}
		if (ground->getUniqueID() != 0) {
			statflags |= TILESTATE_UNIQUE;
		}
		if (ground->getMiniMapColor() != 0) {
			minimapColor = ground->getMiniMapColor();
		}
	}

	ItemVector::const_iterator iter = items.begin();
	while (iter != items.end()) {
		Item* i = *iter;
		if (i->isSelected()) {
			statflags |= TILESTATE_SELECTED;
		}
		if (i->getUniqueID() != 0) {
			statflags |= TILESTATE_UNIQUE;
		}
		if (i->getMiniMapColor() != 0) {
			minimapColor = i->getMiniMapColor();
		}

		ItemType& it = g_items[i->getID()];
		if (it.unpassable) {
			statflags |= TILESTATE_BLOCKING;
		}
		if (it.isOptionalBorder) {
			statflags |= TILESTATE_OP_BORDER;
		}
		if (it.isTable) {
			statflags |= TILESTATE_HAS_TABLE;
		}
		if (it.isCarpet) {
			statflags |= TILESTATE_HAS_CARPET;
		}
		++iter;
	}

	if ((statflags & TILESTATE_BLOCKING) == 0) {
		if (ground == nullptr && items.size() == 0) {
			statflags |= TILESTATE_BLOCKING;
		}
	}
}

void Tile::borderize(BaseMap* parent) {
	if (g_settings.getBoolean(Config::SAME_GROUND_TYPE_BORDER)) {
		// Use the custom reborderize method for better border placement
		GroundBrush::reborderizeTile(parent, this);
	} else {
		// Standard border handling
		GroundBrush::doBorders(parent, this);
	}
}

void Tile::addBorderItem(Item* item) {
	if (!item) {
		return;
	}
	ASSERT(item->isBorder());

	if (g_settings.getBoolean(Config::SAME_GROUND_TYPE_BORDER)) {
		// When Same Ground Type Border is enabled, add borders at the end (top) of the stack
		// This ensures borders appear on top of existing items
		items.push_back(item);
	} else {
		// Standard behavior (borders at the bottom of the stack)
		items.insert(items.begin(), item);
	}
}

GroundBrush* Tile::getGroundBrush() const {
	if (ground) {
		if (ground->getGroundBrush()) {
			return ground->getGroundBrush();
		}
	}
	return nullptr;
}

void Tile::cleanBorders() {
	// If Same Ground Type Border is enabled, we don't clean all borders
	// This will be handled in the GroundBrush::doBorders method
	if (g_settings.getBoolean(Config::SAME_GROUND_TYPE_BORDER)) {
		return;
	}

	// When Same Ground Type Border is disabled, remove all border items
	for (ItemVector::iterator it = items.begin(); it != items.end();) {
		if ((*it)->isBorder()) {
			delete *it;
			it = items.erase(it);
		} else {
			++it;
		}
	}
}

void Tile::wallize(BaseMap* parent) {
	WallBrush::doWalls(parent, this);
}

Item* Tile::getWall() const {
	ItemVector::const_iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isWall()) {
			return *it;
		}
		++it;
	}
	return nullptr;
}

Item* Tile::getCarpet() const {
	ItemVector::const_iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isCarpet()) {
			return *it;
		}
		++it;
	}
	return nullptr;
}

Item* Tile::getTable() const {
	ItemVector::const_iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isTable()) {
			return *it;
		}
		++it;
	}
	return nullptr;
}

void Tile::addWallItem(Item* item) {
	if (!item) {
		return;
	}
	ASSERT(item->isWall());

	addItem(item);
}

void Tile::cleanWalls(bool dontdelete) {
	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isWall()) {
			if (!dontdelete) {
				delete *it;
			}
			it = items.erase(it);
		} else {
			++it;
		}
	}
}

void Tile::cleanWalls(WallBrush* wb) {
	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isWall() && wb->hasWall(*it)) {
			delete *it;
			it = items.erase(it);
		} else {
			++it;
		}
	}
}

void Tile::cleanTables(bool dontdelete) {
	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isTable()) {
			if (!dontdelete) {
				delete *it;
			}
			it = items.erase(it);
		} else {
			++it;
		}
	}
}

void Tile::tableize(BaseMap* parent) {
	TableBrush::doTables(parent, this);
}

void Tile::carpetize(BaseMap* parent) {
	CarpetBrush::doCarpets(parent, this);
}

void Tile::selectGround() {
	bool selected_ = false;
	if (ground) {
		ground->select();
		selected_ = true;
	}
	ItemVector::iterator it;

	it = items.begin();
	while (it != items.end()) {
		if ((*it)->isBorder()) {
			(*it)->select();
			selected_ = true;
		} else {
			break;
		}
		++it;
	}
	if (selected_) {
		statflags |= TILESTATE_SELECTED;
	}
}

void Tile::deselectGround() {
	if (ground) {
		ground->deselect();
	}
	ItemVector::iterator it = items.begin();
	while (it != items.end()) {
		if ((*it)->isBorder()) {
			(*it)->deselect();
		} else {
			break;
		}
		++it;
	}
}

void Tile::setHouse(House* _house) {
	house_id = (_house ? _house->getID() : 0);
}

void Tile::setHouseID(uint32_t newHouseId) {
	house_id = newHouseId;
}

bool Tile::isTownExit(Map& map) const {
	return location->getTownCount() > 0;
}

void Tile::addHouseExit(House* h) {
	if (!h) {
		return;
	}
	HouseExitList* house_exits = location->createHouseExits();
	house_exits->push_back(h->getID());
}

void Tile::removeHouseExit(House* h) {
	if (!h) {
		return;
	}

	HouseExitList* house_exits = location->getHouseExits();
	if (!house_exits) {
		return;
	}

	for (std::vector<uint32_t>::iterator it = house_exits->begin(); it != house_exits->end(); ++it) {
		if (*it == h->getID()) {
			house_exits->erase(it);
			return;
		}
	}
}
""",
    "wxwidgets/tile.h": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_TILE_H
#define RME_TILE_H

#include "position.h"
#include "item.h"
#include "map_region.h"
#include <unordered_set>

enum {
	TILESTATE_NONE = 0x0000,
	TILESTATE_PROTECTIONZONE = 0x0001,
	TILESTATE_DEPRECATED = 0x0002, // Reserved
	TILESTATE_NOPVP = 0x0004,
	TILESTATE_NOLOGOUT = 0x0008,
	TILESTATE_PVPZONE = 0x0010,
	TILESTATE_REFRESH = 0x0020,
	TILESTATE_ZONE_BRUSH = 0x0040,
	// Internal
	TILESTATE_SELECTED = 0x0001,
	TILESTATE_UNIQUE = 0x0002,
	TILESTATE_BLOCKING = 0x0004,
	TILESTATE_OP_BORDER = 0x0008, // If this is true, gravel will be placed on the tile!
	TILESTATE_HAS_TABLE = 0x0010,
	TILESTATE_HAS_CARPET = 0x0020,
	TILESTATE_MODIFIED = 0x0040,
};

enum : uint8_t {
	INVALID_MINIMAP_COLOR = 0xFF
};

class Tile {
public: // Members
	TileLocation* location;
	Item* ground;
	ItemVector items;
	Creature* creature;
	Spawn* spawn;
	uint32_t house_id; // House id for this tile (pointer not safe)

public:
	// ALWAYS use this constructor if the Tile is EVER going to be placed on a map
	Tile(TileLocation& location);
	// Use this when the tile is only used internally by the editor (like in certain brushes)
	Tile(int x, int y, int z);

	~Tile();

	// Argument is a the map to allocate the tile from
	Tile* deepCopy(BaseMap& map);

	// The location of the tile
	// Stores state that remains between the tile being moved (like house exits)
	void setLocation(TileLocation* where) {
		location = where;
	}
	TileLocation* getLocation() {
		return location;
	}
	const TileLocation* getLocation() const {
		return location;
	}

	// Position of the tile
	Position getPosition() {
		return location->getPosition();
	}
	const Position getPosition() const {
		return location->getPosition();
	}
	int getX() const {
		return location->getPosition().x;
	}
	int getY() const {
		return location->getPosition().y;
	}
	int getZ() const {
		return location->getPosition().z;
	}

public: // Functions
	// Absorb the other tile into this tile
	void merge(Tile* other);

	// Has tile been modified since the map was loaded/created?
	bool isModified() const {
		return testFlags(statflags, TILESTATE_MODIFIED);
	}
	void modify() {
		statflags |= TILESTATE_MODIFIED;
	}
	void unmodify() {
		statflags &= ~TILESTATE_MODIFIED;
	}

	// Get memory footprint size
	uint32_t memsize() const;
	// Get number of items on the tile
	bool empty() const {
		return size() == 0;
	}
	int size() const;

	// Blocking?
	bool isBlocking() const {
		return testFlags(statflags, TILESTATE_BLOCKING);
	}

	// PZ
	bool isPZ() const {
		return testFlags(mapflags, TILESTATE_PROTECTIONZONE);
	}
	void setPZ(bool pz) {
		if (pz) {
			mapflags |= TILESTATE_PROTECTIONZONE;
		} else {
			mapflags &= ~TILESTATE_PROTECTIONZONE;
		}
	}

	bool hasProperty(enum ITEMPROPERTY prop) const;

	int getIndexOf(Item* item) const;
	Item* getTopItem() const; // Returns the topmost item, or nullptr if the tile is empty
	Item* getItemAt(int index) const;
	void addItem(Item* item);

	void select();
	void deselect();
	// This selects borders too
	void selectGround();
	void deselectGround();

	bool isSelected() const {
		return testFlags(statflags, TILESTATE_SELECTED);
	}
	bool hasUniqueItem() const {
		return testFlags(statflags, TILESTATE_UNIQUE);
	}

	ItemVector popSelectedItems(bool ignoreTileSelected = false);
	ItemVector getSelectedItems(bool unzoomed = false);
	Item* getTopSelectedItem();

	// Refresh internal flags (such as selected etc.)
	void update();

	uint8_t getMiniMapColor() const;

	// Does this tile have ground?
	bool hasGround() const {
		return ground != nullptr;
	}
	bool hasBorders() const {
		return items.size() && items[0]->isBorder();
	}

	// Get the border brush of this tile
	GroundBrush* getGroundBrush() const;

	// Remove all borders (for autoborder)
	void cleanBorders();

	// Add a border item (added at the bottom of all items)
	void addBorderItem(Item* item);

	// Borderize this tile
	void borderize(BaseMap* parent);

	bool hasTable() const {
		return testFlags(statflags, TILESTATE_HAS_TABLE);
	}
	Item* getTable() const;

	bool hasCarpet() const {
		return testFlags(statflags, TILESTATE_HAS_CARPET);
	}
	Item* getCarpet() const;

	bool hasOptionalBorder() const {
		return testFlags(statflags, TILESTATE_OP_BORDER);
	}
	void setOptionalBorder(bool b) {
		if (b) {
			statflags |= TILESTATE_OP_BORDER;
		} else {
			statflags &= ~TILESTATE_OP_BORDER;
		}
	}

	// Get the (first) wall of this tile
	Item* getWall() const;
	bool hasWall() const;
	// Remove all walls from the tile (for autowall) (only of those belonging to the specified brush
	void cleanWalls(WallBrush* wb);
	// Remove all walls from the tile
	void cleanWalls(bool dontdelete = false);
	// Add a wall item (same as just addItem, but an additional check to verify that it is a wall)
	void addWallItem(Item* item);
	// Wallize (name sucks, I know) this tile
	void wallize(BaseMap* parent);
	// Remove all tables from this tile
	void cleanTables(bool dontdelete = false);
	// Tableize (name sucks even worse, I know) this tile
	void tableize(BaseMap* parent);
	// Carpetize (name sucks even worse than last one, I know) this tile
	void carpetize(BaseMap* parent);

	// Has to do with houses
	bool isHouseTile() const;
	uint32_t getHouseID() const;
	void setHouseID(uint32_t newHouseId);
	void addHouseExit(House* h);
	void removeHouseExit(House* h);
	bool isHouseExit() const;
	bool isTownExit(Map& map) const;
	const HouseExitList* getHouseExits() const;
	HouseExitList* getHouseExits();
	bool hasHouseExit(uint32_t exit) const;
	void setHouse(House* house);
	House* getHouse() const;

	// Mapflags (PZ, PVPZONE etc.)
	void addZoneId(uint16_t _zoneId);
	void removeZoneId(uint16_t _zoneId);
	void clearZoneId();
	void setZoneIds(Tile* tile);
	const std::vector<uint16_t>& getZoneIds() const;
	uint16_t getZoneId() const;

	void setMapFlags(uint16_t _flags);
	void unsetMapFlags(uint16_t _flags);
	uint16_t getMapFlags() const;

	// Statflags (You really ought not to touch this)
	void setStatFlags(uint16_t _flags);
	void unsetStatFlags(uint16_t _flags);
	uint16_t getStatFlags() const;

protected:
	union {
		struct {
			uint16_t mapflags;
			uint16_t statflags;
		};
		uint32_t flags;
	};

	std::vector<uint16_t> zoneIds;

private:
	uint8_t minimapColor;

	Tile(const Tile& tile); // No copy
	Tile& operator=(const Tile& i); // Can't copy
	Tile& operator==(const Tile& i); // Can't compare
};

bool tilePositionLessThan(const Tile* a, const Tile* b);
// This sorts them by draw order
bool tilePositionVisualLessThan(const Tile* a, const Tile* b);

typedef std::vector<Tile*> TileVector;
typedef std::unordered_set<Tile*> TileSet;
typedef std::list<Tile*> TileList;

inline bool Tile::hasWall() const {
	return getWall() != nullptr;
}

inline bool Tile::isHouseTile() const {
	return house_id != 0;
}

inline uint32_t Tile::getHouseID() const {
	return house_id;
}

inline HouseExitList* Tile::getHouseExits() {
	return location->getHouseExits();
}

inline const HouseExitList* Tile::getHouseExits() const {
	return location->getHouseExits();
}

inline bool Tile::isHouseExit() const {
	const HouseExitList* house_exits = getHouseExits();
	if (house_exits) {
		return !house_exits->empty();
	}
	return false;
}

inline bool Tile::hasHouseExit(uint32_t exit) const {
	const HouseExitList* house_exits = getHouseExits();
	if (house_exits) {
		for (HouseExitList::const_iterator iter = house_exits->begin(); iter != house_exits->end(); ++iter) {
			if (*iter == exit) {
				return true;
			}
		}
	}
	return false;
}

inline void Tile::setMapFlags(uint16_t _flags) {
	mapflags = _flags | mapflags;
}

inline void Tile::unsetMapFlags(uint16_t _flags) {
	mapflags &= ~_flags;
}

inline uint16_t Tile::getMapFlags() const {
	return mapflags;
}

inline void Tile::setStatFlags(uint16_t _flags) {
	statflags = _flags | statflags;
}

inline void Tile::unsetStatFlags(uint16_t _flags) {
	statflags &= ~_flags;
}

inline uint16_t Tile::getStatFlags() const {
	return statflags;
}

inline void Tile::addZoneId(uint16_t _zoneId) {
	if (std::find(zoneIds.begin(), zoneIds.end(), _zoneId) == zoneIds.end()) {
		zoneIds.push_back(_zoneId);
	}
}

inline void Tile::clearZoneId() {
	zoneIds.clear();
}

inline void Tile::setZoneIds(Tile* tile) {
	zoneIds.clear();
	zoneIds.assign(tile->getZoneIds().begin(), tile->getZoneIds().end());
}

inline void Tile::removeZoneId(uint16_t _zoneId) {
	const auto& itZone = std::find(zoneIds.begin(), zoneIds.end(), _zoneId);
	if (itZone != zoneIds.end()) {
		zoneIds.erase(itZone);
	}
}

inline const std::vector<uint16_t>& Tile::getZoneIds() const {
	return zoneIds;
}

inline uint16_t Tile::getZoneId() const {
	if (zoneIds.empty()) {
		return 0;
	}

	return zoneIds.front();
}


#endif
""",
    "wxwidgets/item.cpp": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "brush.h"
#include "graphics.h"
#include "gui.h"
#include "tile.h"
#include "complexitem.h"
#include "iomap.h"
#include "item.h"

#include "ground_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"
#include "wall_brush.h"

Item* Item::Create(uint16_t _type, uint16_t _subtype /*= 0xFFFF*/) {
	if (_type == 0) {
		return nullptr;
	}
	Item* newItem = nullptr;

	const ItemType& it = g_items[_type];

	if (it.id != 0) {
		if (it.isDepot()) {
			newItem = newd Depot(_type);
		} else if (it.isContainer()) {
			newItem = newd Container(_type);
		} else if (it.isTeleport()) {
			newItem = newd Teleport(_type);
		} else if (it.isDoor()) {
			newItem = newd Door(_type);
		} else if (it.isPodium()) {
			newItem = newd Podium(_type);
		} else if (_subtype == 0xFFFF) {
			if (it.isFluidContainer()) {
				newItem = newd Item(_type, LIQUID_NONE);
			} else if (it.isSplash()) {
				newItem = newd Item(_type, LIQUID_WATER);
			} else if (it.charges > 0) {
				newItem = newd Item(_type, it.charges);
			} else {
				newItem = newd Item(_type, 1);
			}
		} else {
			newItem = newd Item(_type, _subtype);
		}
	} else {
		newItem = newd Item(_type, _subtype);
	}

	return newItem;
}

Item::Item(unsigned short _type, unsigned short _count) :
	id(_type),
	subtype(1),
	selected(false),
	frame(0) {
	if (hasSubtype()) {
		subtype = _count;
	}
}

Item::~Item() {
	////
}

Item* Item::deepCopy() const {
	Item* copy = Create(id, subtype);
	if (copy) {
		copy->selected = selected;
		if (attributes) {
			copy->attributes = newd ItemAttributeMap(*attributes);
		}
	}
	return copy;
}

Item* transformItem(Item* old_item, uint16_t new_id, Tile* parent) {
	if (old_item == nullptr) {
		return nullptr;
	}

	old_item->setID(new_id);
	// Through the magic of deepCopy, this will now be a pointer to an item of the correct type.
	Item* new_item = old_item->deepCopy();
	if (parent) {
		// Find the old item and remove it from the tile, insert this one instead!
		if (old_item == parent->ground) {
			delete old_item;
			parent->ground = new_item;
			return new_item;
		}

		std::queue<Container*> containers;
		for (ItemVector::iterator item_iter = parent->items.begin(); item_iter != parent->items.end(); ++item_iter) {
			if (*item_iter == old_item) {
				delete old_item;
				item_iter = parent->items.erase(item_iter);
				parent->items.insert(item_iter, new_item);
				return new_item;
			}

			Container* c = dynamic_cast<Container*>(*item_iter);
			if (c) {
				containers.push(c);
			}
		}

		while (containers.size() != 0) {
			Container* container = containers.front();
			ItemVector& v = container->getVector();
			for (ItemVector::iterator item_iter = v.begin(); item_iter != v.end(); ++item_iter) {
				Item* i = *item_iter;
				Container* c = dynamic_cast<Container*>(i);
				if (c) {
					containers.push(c);
				}

				if (i == old_item) {
					// Found it!
					item_iter = v.erase(item_iter);
					v.insert(item_iter, new_item);
					return new_item;
				}
			}
			containers.pop();
		}
	}

	delete new_item;
	return nullptr;
}

uint32_t Item::memsize() const {
	uint32_t mem = sizeof(*this);
	return mem;
}

void Item::setID(uint16_t newid) {
	id = newid;
}

void Item::setSubtype(uint16_t n) {
	subtype = n;
}

bool Item::hasSubtype() const {
	const ItemType& it = g_items[id];
	return (it.isFluidContainer() || it.isSplash() || isCharged() || it.stackable || it.charges != 0);
}

uint16_t Item::getSubtype() const {
	if (hasSubtype()) {
		return subtype;
	}
	return 0;
}

bool Item::hasProperty(enum ITEMPROPERTY prop) const {
	const ItemType& it = g_items[id];
	switch (prop) {
		case BLOCKSOLID:
			if (it.unpassable) {
				return true;
			}
			break;

		case MOVEABLE:
			if (it.moveable && getUniqueID() == 0) {
				return true;
			}
			break;
			/*
					case HASHEIGHT:
						if(it.height != 0 )
							return true;
						break;
			*/
		case BLOCKPROJECTILE:
			if (it.blockMissiles) {
				return true;
			}
			break;

		case BLOCKPATHFIND:
			if (it.blockPathfinder) {
				return true;
			}
			break;

		case HOOK_SOUTH:
			if (it.hookSouth) {
				return true;
			}
			break;

		case HOOK_EAST:
			if (it.hookEast) {
				return true;
			}
			break;

		case BLOCKINGANDNOTMOVEABLE:
			if (it.unpassable && (!it.moveable || getUniqueID() != 0)) {
				return true;
			}
			break;

		case HASLIGHT:
			if (hasLight()) {
				return true;
			}
			break;

		default:
			return false;
	}
	return false;
}

std::pair<int, int> Item::getDrawOffset() const {
	ItemType& it = g_items[id];
	if (it.sprite != nullptr) {
		return it.sprite->getDrawOffset();
	}
	return std::make_pair(0, 0);
}

bool Item::hasLight() const {
	const ItemType& type = g_items.getItemType(id);
	if (type.sprite) {
		return type.sprite->hasLight();
	}
	return false;
}

SpriteLight Item::getLight() const {
	const ItemType& type = g_items.getItemType(id);
	if (type.sprite) {
		return type.sprite->getLight();
	}
	return SpriteLight { 0, 0 };
}

double Item::getWeight() const {
	ItemType& it = g_items[id];
	if (it.stackable) {
		return it.weight * std::max(1, (int)subtype);
	}

	return it.weight;
}

void Item::setUniqueID(unsigned short n) {
	setAttribute("uid", n);
}

void Item::setActionID(unsigned short n) {
	setAttribute("aid", n);
}

void Item::setText(const std::string& str) {
	setAttribute("text", str);
}

void Item::setDescription(const std::string& str) {
	setAttribute("desc", str);
}

void Item::setTier(unsigned short n) {
	setAttribute("tier", n);
}

double Item::getWeight() {
	ItemType& it = g_items[id];
	if (it.isStackable()) {
		return it.weight * subtype;
	}
	return it.weight;
}

bool Item::canHoldText() const {
	return isReadable() || canWriteText();
}

bool Item::canHoldDescription() const {
	return g_items[id].allowDistRead;
}

uint8_t Item::getMiniMapColor() const {
	GameSprite* spr = g_items[id].sprite;
	if (spr) {
		return spr->getMiniMapColor();
	}
	return 0;
}

GroundBrush* Item::getGroundBrush() const {
	ItemType& item_type = g_items.getItemType(id);
	if (item_type.isGroundTile() && item_type.brush && item_type.brush->isGround()) {
		return item_type.brush->asGround();
	}
	return nullptr;
}

TableBrush* Item::getTableBrush() const {
	ItemType& item_type = g_items.getItemType(id);
	if (item_type.isTable && item_type.brush && item_type.brush->isTable()) {
		return item_type.brush->asTable();
	}
	return nullptr;
}

CarpetBrush* Item::getCarpetBrush() const {
	ItemType& item_type = g_items.getItemType(id);
	if (item_type.isCarpet && item_type.brush && item_type.brush->isCarpet()) {
		return item_type.brush->asCarpet();
	}
	return nullptr;
}

DoorBrush* Item::getDoorBrush() const {
	ItemType& item_type = g_items.getItemType(id);
	if (!item_type.isWall || !item_type.isBrushDoor || !item_type.brush || !item_type.brush->isWall()) {
		return nullptr;
	}

	DoorType door_type = item_type.brush->asWall()->getDoorTypeFromID(id);
	DoorBrush* door_brush = nullptr;
	// Quite a horrible dependency on a global here, meh.
	switch (door_type) {
		case WALL_DOOR_NORMAL: {
			door_brush = g_gui.normal_door_brush;
			break;
		}
		case WALL_DOOR_LOCKED: {
			door_brush = g_gui.locked_door_brush;
			break;
		}
		case WALL_DOOR_QUEST: {
			door_brush = g_gui.quest_door_brush;
			break;
		}
		case WALL_DOOR_MAGIC: {
			door_brush = g_gui.magic_door_brush;
			break;
		}
		case WALL_DOOR_NORMAL_ALT: {
			door_brush = g_gui.normal_door_alt_brush;
			break;
		}
		case WALL_ARCHWAY: {
			door_brush = g_gui.archway_door_brush;
			break;
		}
		case WALL_WINDOW: {
			door_brush = g_gui.window_door_brush;
			break;
		}
		case WALL_HATCH_WINDOW: {
			door_brush = g_gui.hatch_door_brush;
			break;
		}
		default: {
			break;
		}
	}
	return door_brush;
}

WallBrush* Item::getWallBrush() const {
	ItemType& item_type = g_items.getItemType(id);
	if (item_type.isWall && item_type.brush && item_type.brush->isWall()) {
		return item_type.brush->asWall();
	}
	return nullptr;
}

BorderType Item::getWallAlignment() const {
	ItemType& it = g_items[id];
	if (!it.isWall) {
		return BORDER_NONE;
	}
	return it.border_alignment;
}

BorderType Item::getBorderAlignment() const {
	ItemType& it = g_items[id];
	return it.border_alignment;
}

void Item::animate() {
	ItemType& type = g_items[id];
	GameSprite* sprite = type.sprite;
	if (!sprite || !sprite->animator) {
		return;
	}

	frame = sprite->animator->getFrame();
}

// ============================================================================
// Static conversions

std::string Item::LiquidID2Name(uint16_t id) {
	switch (id) {
		case LIQUID_NONE:
			return "None";
		case LIQUID_WATER:
			return "Water";
		case LIQUID_BLOOD:
			return "Blood";
		case LIQUID_BEER:
			return "Beer";
		case LIQUID_SLIME:
			return "Slime";
		case LIQUID_LEMONADE:
			return "Lemonade";
		case LIQUID_MILK:
			return "Milk";
		case LIQUID_MANAFLUID:
			return "Manafluid";
		case LIQUID_INK:
			return "Ink";
		case LIQUID_WATER2:
			return "Water";
		case LIQUID_LIFEFLUID:
			return "Lifefluid";
		case LIQUID_OIL:
			return "Oil";
		case LIQUID_SLIME2:
			return "Slime";
		case LIQUID_URINE:
			return "Urine";
		case LIQUID_COCONUT_MILK:
			return "Coconut Milk";
		case LIQUID_WINE:
			return "Wine";
		case LIQUID_MUD:
			return "Mud";
		case LIQUID_FRUIT_JUICE:
			return "Fruit Juice";
		case LIQUID_LAVA:
			return "Lava";
		case LIQUID_RUM:
			return "Rum";
		case LIQUID_SWAMP:
			return "Swamp";
		case LIQUID_TEA:
			return "Tea";
		case LIQUID_MEAD:
			return "Mead";
		default:
			return "Unknown";
	}
}

uint16_t Item::LiquidName2ID(std::string liquid) {
	to_lower_str(liquid);
	if (liquid == "none") {
		return LIQUID_NONE;
	}
	if (liquid == "water") {
		return LIQUID_WATER;
	}
	if (liquid == "blood") {
		return LIQUID_BLOOD;
	}
	if (liquid == "beer") {
		return LIQUID_BEER;
	}
	if (liquid == "slime") {
		return LIQUID_SLIME;
	}
	if (liquid == "lemonade") {
		return LIQUID_LEMONADE;
	}
	if (liquid == "milk") {
		return LIQUID_MILK;
	}
	if (liquid == "manafluid") {
		return LIQUID_MANAFLUID;
	}
	if (liquid == "lifefluid") {
		return LIQUID_LIFEFLUID;
	}
	if (liquid == "oil") {
		return LIQUID_OIL;
	}
	if (liquid == "urine") {
		return LIQUID_URINE;
	}
	if (liquid == "coconut milk") {
		return LIQUID_COCONUT_MILK;
	}
	if (liquid == "wine") {
		return LIQUID_WINE;
	}
	if (liquid == "mud") {
		return LIQUID_MUD;
	}
	if (liquid == "fruit juice") {
		return LIQUID_FRUIT_JUICE;
	}
	if (liquid == "lava") {
		return LIQUID_LAVA;
	}
	if (liquid == "rum") {
		return LIQUID_RUM;
	}
	if (liquid == "swamp") {
		return LIQUID_SWAMP;
	}
	if (liquid == "ink") {
		return LIQUID_INK;
	}
	if (liquid == "tea") {
		return LIQUID_TEA;
	}
	if (liquid == "mead") {
		return LIQUID_MEAD;
	}
	return LIQUID_NONE;
}

// ============================================================================
// XML Saving & loading

Item* Item::Create(pugi::xml_node xml) {
	pugi::xml_attribute attribute;

	int16_t id = 0;
	if ((attribute = xml.attribute("id"))) {
		id = attribute.as_ushort();
	}

	int16_t count = 1;
	if ((attribute = xml.attribute("count")) || (attribute = xml.attribute("subtype"))) {
		count = attribute.as_ushort();
	}

	return Create(id, count);
}
""",
    "wxwidgets/item.h": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_ITEM_H_
#define RME_ITEM_H_

#include "items.h"
#include "iomap_otbm.h"
// #include "iomap_otmm.h"
#include "item_attributes.h"
#include "doodad_brush.h"
#include "raw_brush.h"

class Creature;
class Border;
class Tile;

struct SpriteLight;

enum ITEMPROPERTY {
	BLOCKSOLID,
	HASHEIGHT,
	BLOCKPROJECTILE,
	BLOCKPATHFIND,
	PROTECTIONZONE,
	HOOK_SOUTH,
	HOOK_EAST,
	MOVEABLE,
	BLOCKINGANDNOTMOVEABLE,
	HASLIGHT
};

enum SplashType {
	LIQUID_NONE = 0,
	LIQUID_WATER = 1,
	LIQUID_BLOOD = 2,
	LIQUID_BEER = 3,
	LIQUID_SLIME = 4,
	LIQUID_LEMONADE = 5,
	LIQUID_MILK = 6,
	LIQUID_MANAFLUID = 7,
	LIQUID_INK = 8,
	LIQUID_WATER2 = 9,
	LIQUID_LIFEFLUID = 10,
	LIQUID_OIL = 11,
	LIQUID_SLIME2 = 12,
	LIQUID_URINE = 13,
	LIQUID_COCONUT_MILK = 14,
	LIQUID_WINE = 15,
	LIQUID_MUD = 19,
	LIQUID_FRUIT_JUICE = 21,
	LIQUID_LAVA = 26,
	LIQUID_RUM = 27,
	LIQUID_SWAMP = 28,
	LIQUID_TEA = 35,
	LIQUID_MEAD = 43,

	LIQUID_FIRST = LIQUID_WATER,
	LIQUID_LAST = LIQUID_MEAD
};

IMPLEMENT_INCREMENT_OP(SplashType)

class Item : public ItemAttributes {
public:
	// Factory member to create item of right type based on type
	static Item* Create(uint16_t _type, uint16_t _subtype = 0xFFFF);
	static Item* Create(pugi::xml_node);
	static Item* Create_OTBM(const IOMap& maphandle, BinaryNode* stream);
	// static Item* Create_OTMM(const IOMap& maphandle, BinaryNode* stream);

protected:
	// Constructor for items
	Item(unsigned short _type, unsigned short _count);

public:
	virtual ~Item();

	// Deep copy thingy
	virtual Item* deepCopy() const;

	// Get memory footprint size
	uint32_t memsize() const;
	/*
	virtual Container* getContainer() {return nullptr;}
	virtual const Container* getContainer() const {return nullptr;}
	virtual Teleport* getTeleport() {return nullptr;}
	virtual const Teleport* getTeleport() const {return nullptr;}
	virtual TrashHolder* getTrashHolder() {return nullptr;}
	virtual const TrashHolder* getTrashHolder() const {return nullptr;}
	virtual Mailbox* getMailbox() {return nullptr;}
	virtual const Mailbox* getMailbox() const {return nullptr;}
	virtual Door* getDoor() {return nullptr;}
	virtual const Door* getDoor() const {return nullptr;}
	virtual MagicField* getMagicField() {return nullptr;}
	virtual const MagicField* getMagicField() const {return nullptr;}
	*/

	// OTBM map interface
	// Serialize and unserialize (for save/load)
	// Used internally
	virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* stream);
	virtual bool unserializeAttributes_OTBM(const IOMap& maphandle, BinaryNode* stream);
	virtual bool unserializeItemNode_OTBM(const IOMap& maphandle, BinaryNode* node);

	// Will return a node containing this item
	virtual bool serializeItemNode_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	// Will write this item to the stream supplied in the argument
	virtual void serializeItemCompact_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;

	// OTMM map interface
	/*
	// Serialize and unserialize (for save/load)
	// Used internally
	virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* stream);
	virtual bool unserializeAttributes_OTMM(const IOMap& maphandle, BinaryNode* stream);
	virtual bool unserializeItemNode_OTMM(const IOMap& maphandle, BinaryNode* node);

	// Will return a node containing this item
	virtual bool serializeItemNode_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	// Will write this item to the stream supplied in the argument
	virtual void serializeItemCompact_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	*/

	// Static conversions
	static std::string LiquidID2Name(uint16_t id);
	static uint16_t LiquidName2ID(std::string id);

	// IDs
	uint16_t getID() const {
		return id;
	}
	uint16_t getClientID() const {
		return g_items[id].clientID;
	}
	// NOTE: This is very volatile, do NOT use this unless you know exactly what you're doing
	// which you probably don't so avoid it like the plague!
	void setID(uint16_t id);

	bool typeExists() const {
		return g_items.typeExists(id);
	}

	// Usual attributes
	virtual double getWeight() const;
	int getAttack() const {
		return g_items[id].attack;
	}
	int getArmor() const {
		return g_items[id].armor;
	}
	int getDefense() const {
		return g_items[id].defense;
	}
	uint16_t getSlotPosition() const {
		return g_items[id].slot_position;
	}
	uint8_t getWeaponType() const {
		return g_items[id].weapon_type;
	}
	uint8_t getClassification() const {
		return g_items[id].classification;
	} // 12.81

	// Item g_settings
	bool canHoldText() const;
	bool canHoldDescription() const;
	bool isReadable() const {
		return g_items[id].canReadText;
	}
	bool canWriteText() const {
		return g_items[id].canWriteText;
	}
	uint32_t getMaxWriteLength() const {
		return g_items[id].maxTextLen;
	}
	Brush* getBrush() const {
		return g_items[id].brush;
	}
	GroundBrush* getGroundBrush() const;
	WallBrush* getWallBrush() const;
	DoorBrush* getDoorBrush() const;
	TableBrush* getTableBrush() const;
	CarpetBrush* getCarpetBrush() const;
	Brush* getDoodadBrush() const {
		return g_items[id].doodad_brush;
	} // This is not necessarily a doodad brush
	RAWBrush* getRAWBrush() const {
		return g_items[id].raw_brush;
	}
	bool hasCollectionBrush() const {
		return g_items[id].collection_brush;
	}
	uint16_t getGroundEquivalent() const {
		return g_items[id].ground_equivalent;
	}
	uint16_t hasBorderEquivalent() const {
		return g_items[id].has_equivalent;
	}
	uint32_t getBorderGroup() const {
		return g_items[id].border_group;
	}

	// Drawing related
	uint8_t getMiniMapColor() const;
	int getHeight() const;
	std::pair<int, int> getDrawOffset() const;

	bool hasLight() const;
	SpriteLight getLight() const;

	// Item types
	bool hasProperty(enum ITEMPROPERTY prop) const;
	bool isBlocking() const {
		return g_items[id].unpassable;
	}
	bool isStackable() const {
		return g_items[id].stackable;
	}
	bool isClientCharged() const {
		return g_items[id].isClientCharged();
	}
	bool isExtraCharged() const {
		return g_items[id].isExtraCharged();
	}
	bool isCharged() const {
		return isClientCharged() || isExtraCharged();
	}
	bool isFluidContainer() const {
		return (g_items[id].isFluidContainer());
	}
	bool isAlwaysOnBottom() const {
		return g_items[id].alwaysOnBottom;
	}
	int getTopOrder() const {
		return g_items[id].alwaysOnTopOrder;
	}
	bool isGroundTile() const {
		return g_items[id].isGroundTile();
	}
	bool isSplash() const {
		return g_items[id].isSplash();
	}
	bool isMagicField() const {
		return g_items[id].isMagicField();
	}
	bool isNotMoveable() const {
		return !g_items[id].moveable;
	}
	bool isMoveable() const {
		return g_items[id].moveable;
	}
	bool isPickupable() const {
		return g_items[id].pickupable;
	}
	// bool isWeapon() const {return (g_items[id].weaponType != WEAPON_NONE && g_items[id].weaponType != WEAPON_AMMO);}
	// bool isUseable() const {return g_items[id].useable;}
	bool isHangable() const {
		return g_items[id].isHangable;
	}
	bool isRoteable() const {
		const ItemType& it = g_items[id];
		return it.rotable && it.rotateTo;
	}
	void doRotate() {
		if (isRoteable()) {
			id = g_items[id].rotateTo;
		}
	}
	bool hasCharges() const {
		return g_items[id].charges != 0;
	}
	bool isBorder() const {
		return g_items[id].isBorder;
	}
	bool isOptionalBorder() const {
		return g_items[id].isOptionalBorder;
	}
	bool isWall() const {
		return g_items[id].isWall;
	}
	bool isDoor() const {
		return g_items[id].isDoor();
	}
	bool isOpen() const {
		return g_items[id].isOpen;
	}
	bool isBrushDoor() const {
		return g_items[id].isBrushDoor;
	}
	bool isTable() const {
		return g_items[id].isTable;
	}
	bool isCarpet() const {
		return g_items[id].isCarpet;
	}
	bool isMetaItem() const {
		return g_items[id].isMetaItem();
	}

	// Slot-based Item Types
	bool isWeapon() const {
		uint8_t weaponType = g_items[id].weapon_type;
		return weaponType != WEAPON_NONE && weaponType != WEAPON_AMMO;
	}
	bool isAmmunition() const {
		return g_items[id].weapon_type == WEAPON_AMMO;
	}
	bool isWearableEquipment() const { // Determine if the item is wearable piece of armor
		uint16_t slotPosition = g_items[id].slot_position;
		return slotPosition & SLOTP_HEAD || slotPosition & SLOTP_NECKLACE ||
			// slotPosition & SLOTP_BACKPACK || // handled as container in properties window
			slotPosition & SLOTP_ARMOR || slotPosition & SLOTP_LEGS || slotPosition & SLOTP_FEET || slotPosition & SLOTP_RING || (slotPosition & SLOTP_AMMO && !isAmmunition()); // light sources that give stats
	}

	// Wall alignment (vertical, horizontal, pole, corner)
	BorderType getWallAlignment() const;
	// Border aligment (south, west etc.)
	BorderType getBorderAlignment() const;

	// Get the border type for z-ordering
	BorderType getBorderType() const;

	// Set the z-ordering location value for the item
	void setLocation(int z_level);
	int getLocation() const;

	// Get the name!
	const std::string getName() const {
		return g_items[id].name;
	}
	const std::string getFullName() const {
		return g_items[id].name + g_items[id].editorsuffix;
	}

	// Selection
	bool isSelected() const {
		return selected;
	}
	void select() {
		selected = true;
	}
	void deselect() {
		selected = false;
	}
	void toggleSelection() {
		selected = !selected;
	}

	// Item properties!
	virtual bool isComplex() const {
		return attributes && attributes->size();
	} // If this item requires full save (not compact)

	// Weight
	bool hasWeight() {
		return isPickupable();
	}
	virtual double getWeight();

	// Subtype (count, fluid, charges)
	int getCount() const;
	uint16_t getSubtype() const;
	void setSubtype(uint16_t n);
	bool hasSubtype() const;

	// Unique ID
	void setUniqueID(uint16_t n);
	uint16_t getUniqueID() const;

	// Action ID
	void setActionID(uint16_t n);
	uint16_t getActionID() const;

	// Tier (12.81)
	void setTier(uint16_t n);
	uint16_t getTier() const;

	// Text
	void setText(const std::string& str);
	std::string getText() const;

	// Description
	void setDescription(const std::string& str);
	std::string getDescription() const;

	void animate();
	int getFrame() const {
		return frame;
	}

	// Item locking
	bool isLocked() const {return locked;}
	void setLocked(bool b) {locked = b;}



	// Check if item blocks creatures (for border generation)
	bool isBlockingCreature() const {
		return g_items[id].unpassable;
	}

	// Check if item is a stair based on name
	bool isStairs() const {
		// Check by ID first - specific known floor transition tiles
		if (id == 459 || // Classic floor change
		    id == 924 || // Ramp-style stair
		    id == 1364 || // Northern wooden stairs
		    id == 1369 || // Southern wooden stairs
		    id == 1386) { // Wooden ladder/stair
			return true;
		}

		// Then check by ItemType property (if available)
		if (g_items[id].floorChangeDown ||
		    g_items[id].floorChangeNorth ||
		    g_items[id].floorChangeSouth ||
		    g_items[id].floorChangeEast ||
		    g_items[id].floorChangeWest) {
			return true;
		}

		// Finally check by name
		const std::string& name = getName();
		std::string lowerName = name;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
		return lowerName.find("stair") != std::string::npos ||
		       lowerName.find("spiral") != std::string::npos ||
		       lowerName.find("ramp") != std::string::npos ||
		       lowerName.find("floor change") != std::string::npos ||
		       lowerName.find("level change") != std::string::npos;
	}

	// Check if item is a ladder based on name
	bool isLadder() const {
		// Check for known ladder IDs
		if (id == 1386 || // Wooden ladder/stair
		    id == 3687 || // Wooden ladder
		    id == 5543) { // Ship ladder
			return true;
		}

		// Check by floor change properties
		if (g_items[id].floorChangeDown || g_items[id].floorChange) {
			return true;
		}

		// Check by name
		const std::string& name = getName();
		std::string lowerName = name;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
		return lowerName.find("ladder") != std::string::npos;
	}

protected:
	uint16_t id; // the same id as in ItemType
	// Subtype is either fluid type, count, subtype or charges
	uint16_t subtype;
	bool selected;
	int frame;
	bool locked;

private:
	Item& operator=(const Item& i); // Can't copy
	Item(const Item& i); // Can't copy-construct
	Item& operator==(const Item& i); // Can't compare
};

typedef std::vector<Item*> ItemVector;
typedef std::list<Item*> ItemList;

Item* transformItem(Item* old_item, uint16_t new_id, Tile* parent = nullptr);

inline int Item::getCount() const {
	if (isStackable() || isExtraCharged() || isClientCharged()) {
		return subtype;
	}
	return 1;
}

inline uint16_t Item::getUniqueID() const {
	const int32_t* a = getIntegerAttribute("uid");
	if (a) {
		return *a;
	}
	return 0;
}

inline uint16_t Item::getActionID() const {
	const int32_t* a = getIntegerAttribute("aid");
	if (a) {
		return *a;
	}
	return 0;
}

inline uint16_t Item::getTier() const {
	const int32_t* a = getIntegerAttribute("tier");
	if (a) {
		return *a;
	}
	return 0;
}

inline std::string Item::getText() const {
	const std::string* a = getStringAttribute("text");
	if (a) {
		return *a;
	}
	return "";
}

inline std::string Item::getDescription() const {
	const std::string* a = getStringAttribute("desc");
	if (a) {
		return *a;
	}
	return "";
}

#endif
""",
    "wxwidgets/position.h": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef __POSITION_HPP__
#define __POSITION_HPP__

#include <ostream>
#include <cstdint>
#include <vector>
#include <list>

class SmallPosition;

class Position {
public:
	// We use int since it's the native machine type and can be several times faster than
	// the other integer types in most cases, also, the position may be negative in some
	// cases
	int x, y, z;

	Position() :
		x(0), y(0), z(0) { }
	Position(int _x, int _y, int _z) :
		x(_x), y(_y), z(_z) { }

	bool operator<(const Position& p) const {
		if (z < p.z) {
			return true;
		}
		if (z > p.z) {
			return false;
		}

		if (y < p.y) {
			return true;
		}
		if (y > p.y) {
			return false;
		}

		if (x < p.x) {
			return true;
		}
		// if(x > p.x)
		//	return false;

		return false;
	}

	bool operator>(const Position& p) const {
		return !(*this < p);
	}

	Position operator-(const Position& p) const {
		Position newpos;
		newpos.x = x - p.x;
		newpos.y = y - p.y;
		newpos.z = z - p.z;
		return newpos;
	}

	Position operator+(const Position& p) const {
		Position newpos;
		newpos.x = x + p.x;
		newpos.y = y + p.y;
		newpos.z = z + p.z;
		return newpos;
	}

	Position& operator+=(const Position& p) {
		*this = *this + p;
		return *this;
	}

	bool operator==(const Position& p) const {
		return p.x == x && p.y == y && p.z == z;
	}

	bool operator!=(const Position& p) const {
		return !(*this == p);
	}

	bool isValid() const;
};

inline std::ostream& operator<<(std::ostream& os, const Position& pos) {
	os << pos.x << ':' << pos.y << ':' << pos.z;
	return os;
}

inline std::istream& operator>>(std::istream& is, Position& pos) {
	char a, b;
	int x, y, z;
	is >> x;
	if (!is) {
		return is;
	}
	is >> a;
	if (!is || a != ':') {
		return is;
	}
	is >> y;
	if (!is) {
		return is;
	}
	is >> b;
	if (!is || b != ':') {
		return is;
	}
	is >> z;
	if (!is) {
		return is;
	}

	pos.x = x;
	pos.y = y;
	pos.z = z;

	return is;
}

inline bool Position::isValid() const {
	return x >= 0 && x <= MAP_MAX_WIDTH && y >= 0 && y <= MAP_MAX_HEIGHT && z >= 0 && z <= MAP_MAX_LAYER;
}

inline Position abs(const Position& position) {
	return Position(
		std::abs(position.x),
		std::abs(position.y),
		std::abs(position.z)
	);
}

typedef std::vector<Position> PositionVector;
typedef std::list<Position> PositionList;

#endif
"""
}

analyzed_files_data = []
# Descriptions for each file, matching the prompt's intent
file_descriptions = {
    "wxwidgets/tile.cpp": "Implementation of the Tile class, managing items, ground, creatures, spawns, and house information for a map location.",
    "wxwidgets/tile.h": "Header file for the Tile class, defining its structure and interface for managing map tile data.",
    "wxwidgets/item.cpp": "Implementation of the Item class and its derived classes (Container, Teleport, Door, Podium), including attribute management and factory methods.",
    "wxwidgets/item.h": "Header file for the Item class and related enums/structs, defining the base item properties and interface.",
    "wxwidgets/position.h": "Header file for the Position struct, defining 3D coordinates and related operators."
}

for file_path, content in cpp_files_content.items():
    analyzed_files_data.append({
        "file_path": file_path, # Corrected from 'filename' to 'file_path'
        "description": file_descriptions.get(file_path, "N/A"),
        "md5_hash": get_md5_hash(content),
        "content_lite": get_content_lite(content)
    })

yaml_output = {
    "wbs_item_id": "CORE-01",
    "name": "Port Core Data Structures",
    "description": "Migrate fundamental data structures like Tile, Item, Position, and related container classes from `mapcore` to Qt6 equivalents or modern C++. This involves ensuring data integrity, compatibility, and performance.",
    "dependencies": [], # This task was originally 'None', so empty list or null
    "input_files": [
        "wxwidgets/tile.cpp",
        "wxwidgets/tile.h",
        "wxwidgets/item.cpp",
        "wxwidgets/item.h",
        "wxwidgets/position.h"
    ],
    "analyzed_input_files": analyzed_files_data,
    "documentation_references": [ # Added from similar tasks, as it's relevant
        "Qt Core Data Types: https://doc.qt.io/qt-6/qtcore-module.html",
        "QList: https://doc.qt.io/qt-6/qlist.html",
        "QVector: https://doc.qt.io/qt-6/qvector.html",
        "QMap: https://doc.qt.io/qt-6/qmap.html",
        "QString: https://doc.qt.io/qt-6/qstring.html",
        "QVariant: https://doc.qt.io/qt-6/qvariant.html",
        "Qt Test Framework: https://doc.qt.io/qt-6/qttest-module.html"
    ],
    "current_functionality_summary": """The wxWidgets-based `Tile` class manages a ground item, a list of other items, an optional creature, an optional spawn object, and house ID, along with various flags. It handles item addition logic, selection, and property queries (like blocking status).
The `Item` class, along with `ItemAttributes`, stores item ID, subtype, and custom attributes (UID, AID, text, description). It includes a factory `Item::Create` for different item types (Container, Teleport, Door, Podium derive from Item).
The `Position` struct is a simple 3D coordinate (x, y, z) with comparison and arithmetic operators.""",
    "definition_of_done": [
        "Qt6/C++ equivalent classes for `Tile`, `Item`, and `Position` are implemented.",
        "All significant methods and properties from the original `Tile`, `Item`, and `Position` classes (as seen in the provided snippets) are ported or have clear equivalents in the new Qt6/C++ classes.",
        "Data integrity for core attributes is maintained:",
        "  - `Tile` correctly manages its list of `Item` objects, `ground` item, `Creature*`, `Spawn*`, and `house_id`.",
        "  - `Item` correctly manages its `id`, `subtype`, and attributes (like UID, AID, text, description) based on `ItemAttributes`.",
        "  - `Position` accurately stores x, y, z coordinates.",
        "Memory management for objects created and managed by these classes (especially items within tiles) is handled using modern C++ practices (e.g., smart pointers where appropriate).",
        "The new classes are designed to be largely self-contained or rely on well-defined interfaces for external dependencies (e.g., `g_items` for item properties, `TileLocation`).",
        "Basic unit tests are created for the new `Tile`, `Item`, and `Position` classes, covering:",
        "  - Object construction and initialization.",
        "  - Correct handling of attributes and data members.",
        "  - Key functionalities like adding/removing items from a tile, item creation, position manipulation.",
        "The ported code compiles successfully within a Qt6 project structure.",
        "A brief report is provided detailing the mapping from the old class structures to the new ones, highlighting any significant design changes or assumptions made."
    ],
    "boilerplate_coder_ai_prompt": """The user wants to migrate core data structures from an old C++/wxWidgets application to Qt6/modern C++.
Analyze the provided C++ header and source file snippets for `Tile` (from `wxwidgets/tile.cpp` and `wxwidgets/tile.h`), `Item` (from `wxwidgets/item.cpp` and `wxwidgets/item.h`), and `Position` (from `wxwidgets/position.h`).

**1. Position Class/Struct:**
   - Create a simple Qt-idiomatic or modern C++ class/struct for `Position` based on `position.h`. It should store `x`, `y`, and `z` coordinates.
   - Implement comparison operators (`<`, `==`, `!=`) and arithmetic operators (`+`, `-`, `+=`) as seen in `position.h`.
   - Ensure it's lightweight and efficient.

**2. Item Class:**
   - Design a Qt-idiomatic or modern C++ class for `Item` based on `item.h` and `item.cpp`.
   - The class should store the item's `id` and `subtype`.
   - It should inherit from or incorporate a mechanism similar to `ItemAttributes` to manage custom attributes (e.g., `uid`, `aid`, `text`, `desc`, `tier`). Consider using a `QVariantMap` or a similar structure for attributes if a direct port of `ItemAttributeMap` is too complex for this task.
   - Implement a static factory method `Item::Create(uint16_t id, uint16_t subtype)` that constructs an Item. For complex item types (like `Container`, `Teleport`, `Door`, `Podium` which are mentioned in `Item::Create` but whose definitions are not provided), the factory can return a base `Item` object for now, or you can define placeholder derived classes if simple.
   - Implement a `deepCopy()` method.
   - Port methods like `getID()`, `getSubtype()`, `setSubtype()`, `hasSubtype()`, `getWeight()`, `getName()`, `getFullName()`, selection status (`isSelected`, `select`, `deselect`).
   - For item properties (e.g., `isBlocking()`, `isStackable()`, `isGroundTile()`), these often depend on a global item metadata store (`g_items`). For the ported `Item` class, assume such a store will be available (e.g., via a singleton or a passed-in service). You can define an interface for this (e.g., `ItemMetadataService::getItemType(uint16_t id)`).
   - Address how item attributes like UID, AID, text, description, and tier are set and retrieved (e.g., `setUniqueID`, `getActionID`, `setText`).

**3. Tile Class:**
   - Design a Qt-idiomatic or modern C++ class for `Tile` based on `tile.h` and `tile.cpp`.
   - A `Tile` should be associated with a `Position` (either by composition or by reference/pointer if `TileLocation` is complex).
   - It must manage:
     - A `ground` item (pointer to `Item`).
     - A collection of `Item` objects (e.g., `QList<Item*>`).
     - A pointer to a `Creature` object (`Creature* creature`).
     - A pointer to a `Spawn` object (`Spawn* spawn`).
     - A `house_id` (uint32_t).
     - Tile flags (`mapflags`, `statflags`).
   - Implement constructors, destructor (ensure proper cleanup of items, creature, spawn), and a `deepCopy()` method.
   - Port methods like `addItem(Item* item)`, `getTopItem()`, `getItemAt(int index)`, `popSelectedItems()`, `getSelectedItems()`. Pay close attention to the logic for adding items, especially how ground items and items with ground equivalents are handled.
   - Port methods related to selection (`select()`, `deselect()`, `isSelected()`).
   - Port methods for accessing `ground`, `creature`, `spawn`, `house_id`.
   - Port methods related to tile properties (`isPZ()`, `isBlocking()`, `hasProperty()`).
   - The various `*ize` methods (`borderize`, `wallize`, `tableize`, `carpetize`) and `clean*` methods (`cleanBorders`, `cleanWalls`) involve complex logic tied to specific brush types and global settings (`g_settings`). For this task, you can provide stubs for these methods, noting that their full implementation will depend on the porting of brushes and settings systems. Focus on the core data management aspects of the `Tile` class.

**General Porting Instructions:**
   - Replace wxWidgets types (like `wxNOT_FOUND`) with Qt or C++ STL equivalents (e.g., `-1` or a suitable constant, `std::vector` or `QList` for `ItemVector`).
   - Handle memory management carefully. Use smart pointers (e.g., `std::unique_ptr`, `std::shared_ptr` or Qt's equivalents like `QScopedPointer`, `QSharedPointer`) where appropriate, especially for objects owned by `Tile` (like items, creature, spawn).
   - Dependencies like `g_items`, `g_settings`, `BaseMap`, `TileLocation`, `Creature`, `Spawn`, `Brush` subclasses are external. For the purpose of these data structure classes, you can assume interfaces or stubs for them. The focus is on the structure and internal logic of `Tile`, `Item`, and `Position`.
   - Ensure the ported classes are well-encapsulated.
   - Provide header (.h) and source (.cpp) files for each class.
   - Include basic unit tests (e.g., using Qt Test framework) to verify the functionality of the ported classes.
"""
}

output_filename = "enhanced_wbs_yaml_files/CORE-01.yaml"
output_dir = os.path.dirname(output_filename)

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# Ensure the file_path key is used for analyzed_input_files
for entry in yaml_output["analyzed_input_files"]:
    if "filename" in entry: # Should not happen with current script logic but good to check
        entry["file_path"] = entry.pop("filename")


with open(output_filename, 'w') as f:
    yaml.dump(yaml_output, f, sort_keys=False, width=1000, allow_unicode=True)

print(f"Successfully generated {output_filename}")
