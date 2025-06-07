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
    "wxwidgets/selection.cpp": """//////////////////////////////////////////////////////////////////////
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

#include "selection.h"
#include "tile.h"
#include "creature.h"
#include "item.h"
#include "editor.h"
#include "gui.h"

Selection::Selection(Editor& editor) :
	busy(false),
	editor(editor),
	session(nullptr),
	subsession(nullptr) {
	////
}

Selection::~Selection() {
	delete subsession;
	delete session;
}

Position Selection::minPosition() const {
	Position minPos(0x10000, 0x10000, 0x10);
	for (TileSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile) {
		Position pos((*tile)->getPosition());
		if (minPos.x > pos.x) {
			minPos.x = pos.x;
		}
		if (minPos.y > pos.y) {
			minPos.y = pos.y;
		}
		if (minPos.z > pos.z) {
			minPos.z = pos.z;
		}
	}
	return minPos;
}

Position Selection::maxPosition() const {
	Position maxPos(0, 0, 0);
	for (TileSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile) {
		Position pos((*tile)->getPosition());
		if (maxPos.x < pos.x) {
			maxPos.x = pos.x;
		}
		if (maxPos.y < pos.y) {
			maxPos.y = pos.y;
		}
		if (maxPos.z < pos.z) {
			maxPos.z = pos.z;
		}
	}
	return maxPos;
}

void Selection::add(Tile* tile, Item* item) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	if (item->isSelected()) {
		return;
	}

	// Make a copy of the tile with the item selected
	item->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	item->deselect();

	if (g_settings.getInteger(Config::BORDER_IS_GROUND)) {
		if (item->isBorder()) {
			new_tile->selectGround();
		}
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile, Spawn* spawn) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(spawn);

	if (spawn->isSelected()) {
		return;
	}

	// Make a copy of the tile with the item selected
	spawn->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	spawn->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile, Creature* creature) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(creature);

	if (creature->isSelected()) {
		return;
	}

	// Make a copy of the tile with the item selected
	creature->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	creature->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile) {
	ASSERT(subsession);
	ASSERT(tile);

	Tile* new_tile = tile->deepCopy(editor.map);
	new_tile->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Item* item) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	bool tmp = item->isSelected();
	item->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if (tmp) {
		item->select();
	}
	if (item->isBorder() && g_settings.getInteger(Config::BORDER_IS_GROUND)) {
		new_tile->deselectGround();
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Spawn* spawn) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(spawn);

	bool tmp = spawn->isSelected();
	spawn->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if (tmp) {
		spawn->select();
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Creature* creature) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(creature);

	bool tmp = creature->isSelected();
	creature->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if (tmp) {
		creature->select();
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile) {
	ASSERT(subsession);

	Tile* new_tile = tile->deepCopy(editor.map);
	new_tile->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::addInternal(Tile* tile) {
	ASSERT(tile);

	tiles.insert(tile);
}

void Selection::removeInternal(Tile* tile) {
	ASSERT(tile);
	tiles.erase(tile);
}

void Selection::clear() {
	if (session) {
		for (TileSet::iterator it = tiles.begin(); it != tiles.end(); it++) {
			Tile* new_tile = (*it)->deepCopy(editor.map);
			new_tile->deselect();
			subsession->addChange(newd Change(new_tile));
		}
	} else {
		for (TileSet::iterator it = tiles.begin(); it != tiles.end(); it++) {
			(*it)->deselect();
		}
		tiles.clear();
	}
}

void Selection::start(SessionFlags flags) {
	if (!(flags & INTERNAL)) {
		if (flags & SUBTHREAD) {
			;
		} else {
			session = editor.actionQueue->createBatch(ACTION_SELECT);
		}
		subsession = editor.actionQueue->createAction(ACTION_SELECT);
	}
	busy = true;
}

void Selection::commit() {
	if (session) {
		ASSERT(subsession);
		// We need to step out of the session before we do the action, else peril awaits us!
		BatchAction* tmp = session;
		session = nullptr;

		// Do the action
		tmp->addAndCommitAction(subsession);

		// Create a newd action for subsequent selects
		subsession = editor.actionQueue->createAction(ACTION_SELECT);
		session = tmp;
	}
}

void Selection::finish(SessionFlags flags) {
	if (!(flags & INTERNAL)) {
		if (flags & SUBTHREAD) {
			ASSERT(subsession);
			subsession = nullptr;
		} else {
			ASSERT(session);
			ASSERT(subsession);
			// We need to exit the session before we do the action, else peril awaits us!
			BatchAction* tmp = session;
			session = nullptr;

			tmp->addAndCommitAction(subsession);
			editor.addBatch(tmp, 2);

			session = nullptr;
			subsession = nullptr;
		}
	}
	busy = false;
}

void Selection::updateSelectionCount() {
	if (size() > 0) {
		wxString ss;
		if (size() == 1) {
			ss << "One tile selected.";
		} else {
			ss << size() << " tiles selected.";
		}
		g_gui.SetStatusText(ss);
	}
}

void Selection::join(SelectionThread* thread) {
	thread->Wait();

	ASSERT(session);
	session->addAction(thread->result);
	thread->selection.subsession = nullptr;

	delete thread;
}

SelectionThread::SelectionThread(Editor& editor, Position start, Position end) :
	wxThread(wxTHREAD_JOINABLE),
	editor(editor),
	start(start),
	end(end),
	selection(editor),
	result(nullptr) {
	////
}

SelectionThread::~SelectionThread() {
	////
}

void SelectionThread::Execute() {
	Create();
	Run();
}

wxThread::ExitCode SelectionThread::Entry() {
	selection.start(Selection::SUBTHREAD);
	for (int z = start.z; z >= end.z; --z) {
		for (int x = start.x; x <= end.x; ++x) {
			for (int y = start.y; y <= end.y; ++y) {
				Tile* tile = editor.map.getTile(x, y, z);
				if (!tile) {
					continue;
				}

				selection.add(tile);
			}
		}
		if (z <= GROUND_LAYER && g_settings.getInteger(Config::COMPENSATED_SELECT)) {
			++start.x;
			++start.y;
			++end.x;
			++end.y;
		}
	}
	result = selection.subsession;
	selection.finish(Selection::SUBTHREAD);

	return nullptr;
}
""",
    "wxwidgets/selection.h": """//////////////////////////////////////////////////////////////////////
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

#ifndef RME_SELECTION_H
#define RME_SELECTION_H

#include "position.h"

class Action;
class Editor;
class BatchAction;

class SelectionThread;

class Selection {
public:
	Selection(Editor& editor);
	~Selection();

	// Selects the items on the tile/tiles
	// Won't work outside a selection session
	void add(Tile* tile, Item* item);
	void add(Tile* tile, Spawn* spawn);
	void add(Tile* tile, Creature* creature);
	void add(Tile* tile);
	void remove(Tile* tile, Item* item);
	void remove(Tile* tile, Spawn* spawn);
	void remove(Tile* tile, Creature* creature);
	void remove(Tile* tile);

	// The tile will be added to the list of selected tiles, however, the items on the tile won't be selected
	void addInternal(Tile* tile);
	void removeInternal(Tile* tile);

	// Clears the selection completely
	void clear();

	// Returns true when inside a session
	bool isBusy() {
		return busy;
	}

	//
	Position minPosition() const;
	Position maxPosition() const;

	// This manages a "selection session"
	// Internal session doesn't store the result (eg. no undo)
	// Subthread means the session doesn't create a complete
	// action, just part of one to be merged with the main thread
	// later.
	enum SessionFlags {
		NONE,
		INTERNAL = 1,
		SUBTHREAD = 2,
	};

	void start(SessionFlags flags = NONE);
	void commit();
	void finish(SessionFlags flags = NONE);

	// Joins the selection instance in this thread with this instance
	// This deletes the thread
	void join(SelectionThread* thread);

	size_t size() {
		return tiles.size();
	}
	size_t size() const {
		return tiles.size();
	}
	void updateSelectionCount();
	TileSet::iterator begin() {
		return tiles.begin();
	}
	TileSet::iterator end() {
		return tiles.end();
	}
	TileSet& getTiles() {
		return tiles;
	}
	Tile* getSelectedTile() {
		ASSERT(size() == 1);
		return *tiles.begin();
	}

private:
	bool busy;
	Editor& editor;
	BatchAction* session;
	Action* subsession;

	TileSet tiles;

	friend class SelectionThread;
};

class SelectionThread : public wxThread {
public:
	SelectionThread(Editor& editor, Position start, Position end);
	virtual ~SelectionThread();

	void Execute(); // Calls "Create" and then "Run"
protected:
	virtual ExitCode Entry();
	Editor& editor;
	Position start, end;
	Selection selection;
	Action* result;

	friend class Selection;
};

#endif
"""
}

analyzed_files_data = []
file_descriptions = {
    "wxwidgets/selection.cpp": "Implementation of the Selection class, managing selected tiles and objects, and integrating with an ActionQueue for undo/redo. Includes a SelectionThread for background selection operations.",
    "wxwidgets/selection.h": "Header file for the Selection and SelectionThread classes, defining interfaces for managing selections of map objects."
}

for file_path, content in cpp_files_content.items():
    analyzed_files_data.append({
        "file_path": file_path, # Corrected key
        "description": file_descriptions.get(file_path, "N/A"),
        "md5_hash": get_md5_hash(content),
        "content_lite": get_content_lite(content)
    })

yaml_output = {
    "wbs_item_id": "CORE-05",
    "name": "Port Selection & Clipboard",
    "description": "Migrate selection management (map objects, items) and clipboard operations (cut, copy, paste) from `mapcore` (likely `selection.cpp`/`.h` and potentially missing `clipboard.cpp`/`.h`) to Qt6/C++. This includes handling complex selections and serializing/deserializing data for the clipboard.",
    "dependencies": [ "CORE-04" ], # From original WBS
    "input_files": [
        "wxwidgets/selection.cpp",
        "wxwidgets/selection.h"
    ],
    "analyzed_input_files": analyzed_files_data,
    "documentation_references": [ # Added based on likely Qt features to be used
        "QClipboard: https://doc.qt.io/qt-6/qclipboard.html",
        "QMimeData: https://doc.qt.io/qt-6/qmimedata.html",
        "QDataStream: https://doc.qt.io/qt-6/qdatastream.html",
        "QSet: https://doc.qt.io/qt-6/qset.html",
        "Qt Undo Framework: https://doc.qt.io/qt-6/qundostack.html"
    ],
    "current_functionality_summary": """The wxWidgets `Selection` class manages a set of selected `Tile` objects. It uses a session-based approach (`start`, `commit`, `finish`) to group selection changes into `Action` objects for an `ActionQueue`. Methods like `add` and `remove` create deep copies of tiles with modified selection states and add these as `Change` objects to the current action. A `SelectionThread` allows performing selection operations in a separate thread. Clipboard logic (cut/copy/paste) is not explicitly detailed in these files but would typically involve serializing/deserializing the selected tiles and their contents.""",
    "definition_of_done": [
        "A `SelectionManager` class is implemented in Qt6/C++ to manage selected map objects (Tiles, Items, Creatures, Spawns).",
        "The `SelectionManager` supports:",
        "  - Adding objects (individual items, creatures, spawns, or entire tiles) to the current selection.",
        "  - Removing objects from the current selection.",
        "  - Clearing the entire selection.",
        "All selection modifications are integrated with the `ActionQueue` (from CORE-03) to ensure undo/redo functionality.",
        "Clipboard operations are implemented:",
        "  - `copySelection()`: Serializes the currently selected objects into a well-defined clipboard format. Data can be placed on `QClipboard` or an internal buffer.",
        "  - `cutSelection()`: Performs a copy operation and then creates an action to delete the selected objects from the map.",
        "  - `pasteSelection(Position target_position)`: Deserializes objects from the clipboard data format and creates an action to add them to the map at the specified target position, maintaining relative placements.",
        "A clear data format for clipboard content is defined and documented (e.g., custom binary structure, XML, or JSON representation of tiles and their contents).",
        "The system correctly handles selection of various map elements (ground, items on tile, creatures, spawns).",
        "Memory management for selected objects and clipboard data is handled using modern C++ practices.",
        "Basic unit tests are created for:",
        "  - `SelectionManager`: add, remove, clear selection operations.",
        "  - Clipboard: copy and paste of a simple selection (e.g., a few tiles with items).",
        "The ported code compiles successfully within a Qt6 project structure.",
        "A brief report outlines the design of the selection and clipboard system and the chosen clipboard data format."
    ],
    "boilerplate_coder_ai_prompt": """The user wants to migrate a selection and clipboard system from an old C++/wxWidgets application to Qt6/modern C++.
Analyze the provided C++ header and source file snippets for `Selection` (from `wxwidgets/selection.cpp` and `wxwidgets/selection.h`). Assume clipboard logic might have been in separate, currently unavailable files (`clipboard.cpp`/`.h`) or integrated/handled by UI code.

**1. SelectionManager Class:**
   - Design a `SelectionManager` class.
   - It should maintain a collection of currently selected objects. This could be a `QSet<Tile*>` if selection is tile-based, or a more complex structure if individual items/creatures within unselected tiles can be selected (the provided code suggests tile-based selection primarily, where operations on items within a tile lead to a modified copy of the tile being part of the action). The original `Selection::tiles` is a `TileSet`.
   - Implement methods like:
     - `void add(Tile* tile, Item* item = nullptr, Creature* creature = nullptr, Spawn* spawn = nullptr)`: Adds the specified object(s) to the selection. If an item/creature/spawn is specified, the containing tile is effectively part of the selection context. This should create an `Action` (see CORE-03) that, when executed, marks the objects as selected and updates the tile state.
     - `void remove(Tile* tile, Item* item = nullptr, Creature* creature = nullptr, Spawn* spawn = nullptr)`: Removes object(s) from selection, also via an `Action`.
     - `void clearSelection()`: Clears the current selection, via an `Action`.
     - `const QSet<Tile*>& getSelectedTiles() const;` (or equivalent for your chosen representation).
     - `bool isSelected(Tile* tile, Item* item = nullptr, ...) const;`
   - The original `Selection` class uses `start()`, `commit()`, `finish()` methods to batch selection changes into `Action` objects submitted to an `ActionQueue`. Adapt this pattern. An `Action` for selection might store copies of tiles in their pre-selected and post-selected states.

**2. Clipboard Functionality (to be integrated or a separate ClipboardHandler):**
   - **Data Format:** Define a data format for storing copied map data. This could be:
     - A list of `Tile` objects (deep copies).
     - A custom structure like `ClipboardContent { Position offset; QList<TileData> tiles; }`.
     - Serialized data (e.g., using `QDataStream` for binary, or XML/JSON).
     This format must capture all necessary information: items, ground, creatures, spawns, tile flags, relative positions.
   - `void copySelection()`:
     - Get the current selection from `SelectionManager`.
     - Determine the bounding box or reference point (e.g., minPosition from original code).
     - Create deep copies of the selected tiles and their contents.
     - Serialize this data into the defined clipboard format.
     - Place the serialized data onto `QApplication::clipboard()` using a custom MIME type, or store it in an internal buffer.
   - `void cutSelection()`:
     - Perform `copySelection()`.
     - Create and execute an `Action` (via `ActionQueue`) to delete the selected objects/tiles from the map.
   - `void pasteSelection(const Position& target_paste_position)`:
     - Retrieve data from `QApplication::clipboard()` or the internal buffer.
     - Deserialize it back into a list of `Tile` objects or the intermediate structure.
     - Create and execute an `Action` to add these tiles/objects to the map at `target_paste_position`, applying appropriate offsets so their relative arrangement is preserved.

**3. Integration with Actions (CORE-03):**
   - All operations that modify selection state or map data (cut, paste) must be performed through `Action` objects pushed to the `ActionQueue` to ensure undo/redo capability.
   - A `SelectAction` might store the state of selection before and after.
   - A `PasteAction` would store the data being pasted to allow its removal on undo.

**General Porting Instructions:**
   - Replace wxWidgets types (e.g., `wxThread` for `SelectionThread`) with Qt equivalents (`QThread`, `QtConcurrent::run`, or handle synchronously if `SelectionThread`'s complexity is initially too high).
   - Use smart pointers for managing dynamically allocated objects like `Tile` copies for clipboard or actions.
   - Interactions with `Editor`, `Map`, `Tile`, `Item`, `ActionQueue` are key. Define clear interfaces or pass necessary references.
   - The concept of `SessionFlags` (INTERNAL, SUBTHREAD) in the original code might be simplified or re-evaluated based on how actions are batched.
   - Provide header (.h) and source (.cpp) files for the `SelectionManager` and any clipboard-specific helper classes or data structures.
   - Include basic unit tests for selection changes and copy/paste operations.
"""
}

output_filename = "enhanced_wbs_yaml_files/CORE-05.yaml"
output_dir = os.path.dirname(output_filename)

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# Ensure the file_path key is used for analyzed_input_files
for entry in yaml_output["analyzed_input_files"]:
    if "filename" in entry:
        entry["file_path"] = entry.pop("filename")

with open(output_filename, 'w') as f:
    yaml.dump(yaml_output, f, sort_keys=False, width=1000, allow_unicode=True)

print(f"Successfully generated {output_filename}")
