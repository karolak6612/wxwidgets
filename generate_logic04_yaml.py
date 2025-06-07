import yaml
import hashlib

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

waypoints_cpp_content = """\
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

#include "waypoints.h"
#include "map.h"

void Waypoints::addWaypoint(Waypoint* wp) {
	removeWaypoint(wp->name);
	if (wp->pos != Position()) {
		Tile* t = map.getTile(wp->pos);
		if (!t) {
			map.setTile(wp->pos, t = map.allocator(map.createTileL(wp->pos)));
		}
		t->getLocation()->increaseWaypointCount();
	}
	waypoints.insert(std::make_pair(as_lower_str(wp->name), wp));
}

Waypoint* Waypoints::getWaypoint(std::string name) {
	to_lower_str(name);
	WaypointMap::iterator iter = waypoints.find(name);
	if (iter == waypoints.end()) {
		return nullptr;
	}
	return iter->second;
}

Waypoint* Waypoints::getWaypoint(TileLocation* location) {
	if (!location) {
		return nullptr;
	}
	// TODO find waypoint by position hash.
	for (WaypointMap::iterator it = waypoints.begin(); it != waypoints.end(); it++) {
		Waypoint* waypoint = it->second;
		if (waypoint && waypoint->pos == location->position) {
			return waypoint;
		}
	}
	return nullptr;
}

void Waypoints::removeWaypoint(std::string name) {
	to_lower_str(name);
	WaypointMap::iterator iter = waypoints.find(name);
	if (iter == waypoints.end()) {
		return;
	}
	delete iter->second;
	waypoints.erase(iter);
}
"""

waypoints_h_content = """\
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

#ifndef RME_WAYPOINTS_H_
#define RME_WAYPOINTS_H_

#include "position.h"

class Waypoint {
public:
	std::string name;
	Position pos;
};

typedef std::map<std::string, Waypoint*> WaypointMap;

class Waypoints {
	Map& map;

public:
	Waypoints(Map& map) :
		map(map) { }
	~Waypoints() {
		for (WaypointMap::iterator iter = waypoints.begin(); iter != waypoints.end(); ++iter) {
			delete iter->second;
		}
	}

	void addWaypoint(Waypoint* wp);
	Waypoint* getWaypoint(std::string name);
	Waypoint* getWaypoint(TileLocation* location);
	void removeWaypoint(std::string name);

	WaypointMap waypoints;

	WaypointMap::iterator begin() {
		return waypoints.begin();
	}
	WaypointMap::const_iterator begin() const {
		return waypoints.begin();
	}
	WaypointMap::iterator end() {
		return waypoints.end();
	}
	WaypointMap::const_iterator end() const {
		return waypoints.end();
	}
};

#endif
"""

md5_waypoints_cpp = hashlib.md5(waypoints_cpp_content.encode('utf-8')).hexdigest()
content_lite_waypoints_cpp = "\\n".join(waypoints_cpp_content.splitlines()[:200])

md5_waypoints_h = hashlib.md5(waypoints_h_content.encode('utf-8')).hexdigest()
content_lite_waypoints_h = "\\n".join(waypoints_h_content.splitlines()[:200])

yaml_data = {
    "wbs_item_id": "LOGIC-04",
    "name": "Port Waypoint System",
    "description": "Migrate the waypoint data structures, storage, and editing logic from wxWidgets to Qt6. This includes how waypoints are created, modified, displayed on the map, and saved/loaded.",
    "dependencies": [
        "CORE-03", # For saving/loading waypoint data with the map
        "RENDER-02", # For displaying waypoints on the map
        "UI-07"      # For UI elements to create/edit waypoints
    ],
    "input_files": [
        "wxwidgets/waypoints.cpp",
        "wxwidgets/waypoints.h"
    ],
    "analyzed_input_files": [
        {
            "file_path": "wxwidgets/waypoints.cpp",
            "description": "Contains the implementation of the `Waypoints` class, which manages a collection of waypoints. Includes logic for adding, getting, and removing waypoints, and interacting with the map tiles to update waypoint counts.",
            "md5_hash": md5_waypoints_cpp,
            "content_lite": content_lite_waypoints_cpp
        },
        {
            "file_path": "wxwidgets/waypoints.h",
            "description": "Defines the `Waypoint` class (name, position) and the `Waypoints` manager class (using `std::map` to store waypoints by name).",
            "md5_hash": md5_waypoints_h,
            "content_lite": content_lite_waypoints_h
        }
    ],
    "documentation_references": [
        "Qt Data Structures: https://doc.qt.io/qt-6/qtcore-containers.html (e.g., QMap, QHash, QList)",
        "QVector3D for positions: https://doc.qt.io/qt-6/qvector3d.html"
    ],
    "current_functionality_summary": """\
The waypoint system is managed by the `Waypoints` class, which holds `Waypoint` objects (containing a name and `Position`).
- Waypoints are stored in a `std::map` keyed by their lowercase name.
- Adding a waypoint (`addWaypoint`):
  - Removes any existing waypoint with the same name.
  - If the waypoint has a valid position, it ensures the tile exists at that position.
  - It calls `tile->getLocation()->increaseWaypointCount()` to mark the tile.
- Retrieving waypoints:
  - `getWaypoint(std::string name)`: Case-insensitive lookup by name.
  - `getWaypoint(TileLocation* location)`: Iterates all waypoints to find one at the given position (has a TODO for optimization).
- Removing a waypoint (`removeWaypoint`): Deletes the waypoint object and removes it from the map. (Does not appear to decrease tile waypoint count in the provided snippet).
- The `Waypoint` class itself is a simple structure defined in `waypoints.h`.
- Saving/loading of waypoints is not detailed in these files but is expected to be part of the map's OTBM serialization.\
""",
    "definition_of_done": [
        "A Qt6 `Waypoint` class (or struct) is defined, storing a `QString name` and a 3D position (e.g., using `QVector3D` or a custom `Position` class).",
        "A Qt6 `WaypointManager` class is implemented to manage a collection of `Waypoint` objects.",
        "The `WaypointManager` uses an efficient Qt container for storing waypoints, keyed by name (e.g., `QHash<QString, Waypoint*>` or `QMap<QString, Waypoint*>`), ensuring case-insensitive name lookups.",
        "Methods for adding, retrieving (by name and by 3D position), and removing waypoints are implemented in `WaypointManager`.",
        "The logic for associating waypoints with map tiles is ported. When a waypoint is added or removed, the corresponding `Tile` object (or its equivalent in the Qt6 map structure) must have its waypoint count (or similar flag) updated (both incremented on add and decremented on remove).",
        "Memory management for `Waypoint` objects is handled correctly by the `WaypointManager` (e.g., deletion on removal or when the manager is destroyed).",
        "The system ensures waypoint names are unique (case-insensitively) within the map.",
        "The `WaypointManager` is designed to integrate with the OTBM map saving/loading mechanism (CORE-03), allowing waypoint data to be serialized and deserialized with the map.",
        "Consideration is given to efficient position-based lookup of waypoints (e.g., if the original TODO for a position hash is to be addressed, perhaps using `QHash<Position, Waypoint*>` if `Position` is hashable, or other spatial indexing if needed for large numbers of waypoints)."
    ],
    "boilerplate_coder_ai_prompt": """\
Your task is to port the core logic for managing waypoints from the wxWidgets-based Remere's Map Editor to Qt6. The reference C++ files are `wxwidgets/waypoints.cpp` and `wxwidgets/waypoints.h` (which also contains the `Waypoint` class definition).

**1. Waypoint Data Structure:**
   - Define a `Waypoint` class or struct in Qt6.
   - It should store:
     - `QString name;`
     - `QVector3D position;` (or a custom `Position` class if a more complex one is used throughout the application, ensure it holds x, y, z coordinates).

**2. WaypointManager Class:**
   - Create a `WaypointManager` class responsible for all waypoint operations.
   - It should hold a collection of `Waypoint` objects, for instance, using `QHash<QString, Waypoint*> m_waypoints;` for efficient name-based lookup. Remember to handle waypoint name case-insensitivity for lookups and insertions (e.g., by always storing/querying lowercase names).
   - **Constructor:** Should take a reference to the main `Map` object (or its Qt6 equivalent) to interact with tiles.
   - **Destructor:** Ensure all dynamically allocated `Waypoint` objects are deleted.
   - **`void addWaypoint(Waypoint* waypoint);`**
     - Convert waypoint name to lowercase for consistent storage and lookup.
     - Remove/delete any existing waypoint with the same name before adding the new one.
     - If `waypoint->position` is valid:
       - Get the `Tile` (or Qt6 equivalent) at `waypoint->position`.
       - If the tile doesn't exist, it should be created (this might be a responsibility of the `Map` class).
       - Call a method on the tile or its location data to signify a waypoint is present (e.g., `tile->increaseWaypointCount();`).
     - Add the waypoint to `m_waypoints`.
   - **`Waypoint* getWaypoint(const QString& name);`**
     - Perform a case-insensitive lookup in `m_waypoints`. Return `nullptr` if not found.
   - **`Waypoint* getWaypoint(const Position& pos);` (or `const QVector3D& pos`)**
     - Implement lookup by position. The original code iterated through all waypoints. If performance is a concern for many waypoints, consider optimizing this (e.g., using a `QHash<PositionKey, Waypoint*>` if a suitable `PositionKey` can be derived and made hashable, or a simpler iteration if waypoint counts are typically low).
     - Return `nullptr` if not found.
   - **`void removeWaypoint(const QString& name);`**
     - Find the waypoint by its (case-insensitive) name.
     - If found:
       - If the waypoint has a valid position, get the `Tile` at its position and call a method to decrease its waypoint count (e.g., `tile->decreaseWaypointCount();`). This part was missing in the original `removeWaypoint` and is important.
       - Delete the `Waypoint` object.
       - Remove the waypoint from `m_waypoints`.
   - Provide `begin()` and `end()` iterators if direct iteration over waypoints is needed externally.

**3. Integration with Map and Tile System:**
   - The `WaypointManager` will need a reference to the main map object to access and modify tile data related to waypoints.
   - Ensure that the `Tile` class (or its Qt6 equivalent) has a mechanism to track associated waypoints (e.g., a counter or a flag). This is crucial for rendering waypoints on the map and for other tools that might interact with waypoints on tiles.

**4. File I/O (Conceptual):**
   - This task focuses on in-memory management. The actual saving and loading of waypoints with the map file (likely OTBM format) will be handled by the `CORE-03` (Port OTBM/OTMM File I/O) task.
   - Your `WaypointManager` should be designed to easily expose its data for serialization (e.g., providing access to the waypoint list) and to be populated from deserialized data.

**Note:** The original `waypoints.cpp` did not show `waypoint.cpp` and `waypoint.h` as separate files, as the `Waypoint` struct was defined within `waypoints.h`.
Populate the `analyzed_input_files` section by reading the first 200 lines of `wxwidgets/waypoints.cpp` and `wxwidgets/waypoints.h` and calculating their MD5 hashes.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/LOGIC-04.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

# Clean up the temporary file contents from variables (optional, good practice)
del waypoints_cpp_content
del waypoints_h_content
del yaml_data
