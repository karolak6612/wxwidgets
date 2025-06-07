import yaml
import hashlib

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

spawn_cpp_content = """\
//////////////////////////////////////////////////////////////////////
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

#include "tile.h"
#include "spawn.h"

Spawns::Spawns() {
	////
}

Spawns::~Spawns() {
	////
}

void Spawns::addSpawn(Tile* tile) {
	ASSERT(tile->spawn);

	auto it = spawns.insert(tile->getPosition());
	ASSERT(it.second);
}

void Spawns::removeSpawn(Tile* tile) {
	ASSERT(tile->spawn);
	spawns.erase(tile->getPosition());
#if 0
	SpawnPositionList::iterator iter = begin();
	while(iter != end()) {
		if(*iter == tile->getPosition()) {
			spawns.erase(iter);
			return;
		}
		++iter;
	}
	ASSERT(false);
#endif
}

std::ostream& operator<<(std::ostream& os, const Spawn& spawn) {
	os << &spawn << ":: -> " << spawn.getSize() << std::endl;
	return os;
}
"""

spawn_h_content = """\
//////////////////////////////////////////////////////////////////////
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

#ifndef RME_SPAWN_H_
#define RME_SPAWN_H_

class Tile;

class Spawn {
public:
	Spawn(int size = 3) :
		size(0), selected(false) {
		setSize(size);
	}
	~Spawn() { }

	Spawn* deepCopy() {
		Spawn* copy = newd Spawn(size);
		copy->selected = selected;
		return copy;
	}

	bool isSelected() const {
		return selected;
	}
	void select() {
		selected = true;
	}
	void deselect() {
		selected = false;
	}

	// Does not compare selection!
	bool operator==(const Spawn& other) {
		return size == other.size;
	}
	bool operator!=(const Spawn& other) {
		return size != other.size;
	}

	void setSize(int newsize) {
		ASSERT(size < 100);
		size = newsize;
	}
	int getSize() const {
		return size;
	}

protected:
	int size;
	bool selected;
};

typedef std::set<Position> SpawnPositionList;
typedef std::list<Spawn*> SpawnList;

class Spawns {
public:
	Spawns();
	~Spawns();

	void addSpawn(Tile* tile);
	void removeSpawn(Tile* tile);

	SpawnPositionList::iterator begin() {
		return spawns.begin();
	}
	SpawnPositionList::const_iterator begin() const {
		return spawns.begin();
	}
	SpawnPositionList::iterator end() {
		return spawns.end();
	}
	SpawnPositionList::const_iterator end() const {
		return spawns.end();
	}
	void erase(SpawnPositionList::iterator iter) {
		spawns.erase(iter);
	}
	SpawnPositionList::iterator find(Position& pos) {
		return spawns.find(pos);
	}

private:
	SpawnPositionList spawns;
};

#endif
"""

md5_spawn_cpp = hashlib.md5(spawn_cpp_content.encode('utf-8')).hexdigest()
content_lite_spawn_cpp = "\\n".join(spawn_cpp_content.splitlines()[:200])

md5_spawn_h = hashlib.md5(spawn_h_content.encode('utf-8')).hexdigest()
content_lite_spawn_h = "\\n".join(spawn_h_content.splitlines()[:200])

yaml_data = {
    "wbs_item_id": "LOGIC-07", # Corrected ID as per prompt
    "name": "Port Spawn & Group Management",
    "description": "Migrate the logic for creating, managing, and storing creature/item spawns and group definitions. This includes spawn areas, spawn times, creature/item lists within spawns, and how groups of spawns are defined and managed.",
    "dependencies": [
        "CORE-03",   # For saving/loading spawn data (creature list, radius, time) with tiles
        "RENDER-02", # For visually indicating spawn areas/radii on the map
        "UI-08"      # For UI elements to create/edit spawns and their creature lists
    ],
    "input_files": [
        "wxwidgets/spawn.cpp",
        "wxwidgets/spawn.h"
    ],
    "analyzed_input_files": [
        {
            "file_path": "wxwidgets/spawn.cpp",
            "description": "Implements the `Spawns` class, which manages a set of tile positions that are designated as spawn points.",
            "md5_hash": md5_spawn_cpp,
            "content_lite": content_lite_spawn_cpp
        },
        {
            "file_path": "wxwidgets/spawn.h",
            "description": "Defines the `Spawn` class (primarily for spawn radius/size on a tile) and the `Spawns` class (a manager for `SpawnPositionList`). The actual list of creatures and spawn time are typically stored directly on the Tile object in RME.",
            "md5_hash": md5_spawn_h,
            "content_lite": content_lite_spawn_h
        }
    ],
    "documentation_references": [
        "Qt Data Structures: https://doc.qt.io/qt-6/qtcore-containers.html (e.g., QSet for positions)"
    ],
    "current_functionality_summary": """\
The provided `spawn.h` defines a `Spawn` class that mainly stores a `size` (radius) and selection state. It does not store the list of creatures or spawn timings.
The `Spawns` class manages a `std::set<Position>` (`SpawnPositionList`), effectively keeping track of which tile positions are considered spawn points.
- `Spawns::addSpawn(Tile* tile)` adds a tile's position to this set, assuming `tile->spawn` (the Spawn object with radius) is already set on the tile.
- `Spawns::removeSpawn(Tile* tile)` removes a tile's position from the set.
The actual list of creatures to spawn and the spawn interval are typically attributes of the `Tile` object itself in the OTBM map format, rather than being managed by these specific `Spawn` or `Spawns` classes.
"Group Management" in this context likely refers to managing the group of creatures within a single spawn definition on a tile.\
""",
    "definition_of_done": [
        "A Qt6 `SpawnProperties` class/struct is defined (e.g., in `spawn.h` or `tile.h`) to store the spawn radius (integer) associated with a tile. (The original `Spawn` class mostly served this purpose).",
        "The main `Tile` class (or its Qt6 equivalent) is augmented to store:",
        "  - An optional `SpawnProperties` object (or `std::optional<SpawnProperties>`).",
        "  - A `QList<QString>` to hold the names of creatures in this tile's spawn list.",
        "  - An `int` for the spawn time/interval in seconds.",
        "A Qt6 `SpawnManager` class is implemented to maintain a registry of all tile positions that function as spawn points (e.g., using `QSet<Position>`).",
        "The `SpawnManager` provides methods `void registerSpawnTile(const Position& pos);` and `void unregisterSpawnTile(const Position& pos);` to update its registry.",
        "The logic for creating, reading, updating, and deleting the spawn radius, creature list, and spawn time is primarily handled by direct modifications to the `Tile` object's properties. The `SpawnManager` serves as a central point to quickly identify all spawn locations.",
        "The system is prepared for integration with OTBM loading/saving (CORE-03), where the tile attributes (radius, creature list, spawn time) will be read/written."
    ],
    "boilerplate_coder_ai_prompt": """\
Your task is to port the conceptual system for managing spawn points to Qt6, based on the provided `wxwidgets/spawn.cpp` and `wxwidgets/spawn.h`. The original system had a `Spawn` class for radius and a `Spawns` class to track positions of spawn tiles. In OTBM, detailed spawn data (creatures, interval) is typically stored directly on the tile.

**1. `SpawnProperties` Data Structure (e.g., in `spawn.h` or `tile.h`):**
   - Define a simple C++ class or struct, possibly named `SpawnProperties`.
   - It should store: `int radius;` (the spawn area size).
   - (The `selected` member from the original `Spawn` class is more of a UI concern and might be handled differently, e.g., by the selection system).

**2. Augmenting the `Tile` Class (Conceptual - actual changes in Tile's own task):**
   - The main `Tile` class (or its Qt6 equivalent) will be the primary owner of spawn data. Ensure it can store:
     - `std::optional<SpawnProperties> spawn_props;` (or a pointer `SpawnProperties* spawn_props = nullptr;`). This will hold the radius if the tile is a spawn point.
     - `QList<QString> creature_list;` // List of creature names for this spawn.
     - `int spawn_time_seconds;` // Respawn interval.
   - The `Tile` class should have methods like `bool isSpawn() const { return spawn_props.has_value(); }`, `void setSpawn(int radius, const QList<QString>& creatures, int time_secs)`, `void clearSpawn()`.

**3. `SpawnManager` Class (Registry of Spawn Locations):**
   - Create a `SpawnManager` class.
   - Its main role is to keep a fast lookup of all tiles that are spawn points.
   - Internally, use `QSet<Position> m_spawn_locations;` (assuming `Position` is defined and hashable).
   - **`void registerSpawnLocation(const Position& pos);`**: Adds `pos` to `m_spawn_locations`. This would be called when a tile at `pos` is configured as a spawn (e.g., its `spawn_props` are set).
   - **`void unregisterSpawnLocation(const Position& pos);`**: Removes `pos` from `m_spawn_locations`. This is called when a tile at `pos` is no longer a spawn (e.g., `tile->clearSpawn()` is called).
   - **`bool isSpawnLocation(const Position& pos) const;`**: Checks if `pos` is in `m_spawn_locations`.
   - Provide methods to iterate over all registered spawn locations (`const QSet<Position>& getSpawnLocations() const;`).

**4. Core Logic for Modifying Spawns:**
   - Creating a spawn on a tile: Set the `spawn_props` (radius), `creature_list`, and `spawn_time_seconds` on the `Tile` object, then call `spawnManager.registerSpawnLocation(tile->getPosition())`.
   - Modifying a spawn: Directly modify the properties on the `Tile` object.
   - Removing a spawn from a tile: Clear the spawn properties on the `Tile` object (e.g., `tile->clearSpawn()`) and then call `spawnManager.unregisterSpawnLocation(tile->getPosition())`.

**5. Group Management:**
   - "Group Management" in this context refers to managing the `QList<QString> creature_list` on each `Tile` that is a spawn point. There's no indication of a system for grouping multiple distinct spawn areas together under a common name in `spawn.cpp`/`.h`.

**6. File I/O:**
   - The `SpawnManager` itself doesn't handle file I/O. The OTBM loader/saver (task `CORE-03`) will read/write the spawn radius, creature list, and spawn time directly from/to each tile's attributes. When a map is loaded, and tiles with spawn data are created, they should register their positions with the `SpawnManager`.

Populate the `analyzed_input_files` section by reading the first 200 lines of `wxwidgets/spawn.cpp` and `wxwidgets/spawn.h` and calculating their MD5 hashes.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/LOGIC-07.yaml" # Corrected ID

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

# Clean up the temporary file contents from variables (optional, good practice)
del spawn_cpp_content
del spawn_h_content
del yaml_data
