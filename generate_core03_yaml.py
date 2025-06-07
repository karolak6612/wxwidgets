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
    "wxwidgets/action.cpp": """//////////////////////////////////////////////////////////////////////
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

#include "action.h"
#include "settings.h"
#include "map.h"
#include "editor.h"
#include "gui.h"

// Add necessary includes for exception handling and file operations
#include <exception>
#include <fstream>
#include <wx/filename.h>
#include <wx/datetime.h>
#include <wx/stdpaths.h>

Change::Change() :
	type(CHANGE_NONE), data(nullptr) {
	////
}

Change::Change(Tile* t) :
	type(CHANGE_TILE) {
	ASSERT(t);
	data = t;
}

Change* Change::Create(House* house, const Position& where) {
	Change* c = newd Change();
	c->type = CHANGE_MOVE_HOUSE_EXIT;
	std::pair<uint32_t, Position>* p = newd std::pair<uint32_t, Position>;
	p->first = house->getID();
	p->second = where;
	c->data = p;
	return c;
}

Change* Change::Create(Waypoint* wp, const Position& where) {
	Change* c = newd Change();
	c->type = CHANGE_MOVE_WAYPOINT;
	std::pair<std::string, Position>* p = newd std::pair<std::string, Position>;
	p->first = wp->name;
	p->second = where;
	c->data = p;
	return c;
}

Change::~Change() {
	clear();
}

void Change::clear() {
	switch (type) {
		case CHANGE_TILE:
			ASSERT(data);
			delete reinterpret_cast<Tile*>(data);
			break;
		case CHANGE_MOVE_HOUSE_EXIT:
			ASSERT(data);
			delete reinterpret_cast<std::pair<uint32_t, Position>*>(data);
			break;
		case CHANGE_MOVE_WAYPOINT:
			ASSERT(data);
			delete reinterpret_cast<std::pair<std::string, Position>*>(data);
			break;
		case CHANGE_NONE:
			break;
		default:
#ifdef __DEBUG_MODE__
			if (data) {
				printf("UNHANDLED CHANGE TYPE! Leak!");
			}
#endif
			break;
	}
	type = CHANGE_NONE;
	data = nullptr;
}

uint32_t Change::memsize() const {
	uint32_t mem = sizeof(*this);
	switch (type) {
		case CHANGE_TILE:
			ASSERT(data);
			mem += reinterpret_cast<Tile*>(data)->memsize();
			break;
		default:
			break;
	}
	return mem;
}

Action::Action(Editor& editor, ActionIdentifier ident) :
	commited(false),
	editor(editor),
	type(ident) {
}

Action::~Action() {
	ChangeList::const_reverse_iterator it = changes.rbegin();
	while (it != changes.rend()) {
		delete *it;
		++it;
	}
}

size_t Action::approx_memsize() const {
	uint32_t mem = sizeof(*this);
	mem += changes.size() * (sizeof(Change) + sizeof(Tile) + sizeof(Item) + 6 /* approx overhead*/);
	return mem;
}

size_t Action::memsize() const {
	uint32_t mem = sizeof(*this);
	mem += sizeof(Change*) * 3 * changes.size();
	ChangeList::const_iterator it = changes.begin();
	while (it != changes.end()) {
		Change* c = *it;
		switch (c->type) {
			case CHANGE_TILE: {
				ASSERT(c->data);
				mem += reinterpret_cast<Tile*>(c->data)->memsize();
				break;
			}

			default:
				break;
		}
		++it;
	}
	return mem;
}

void Action::commit(DirtyList* dirty_list) {
	editor.selection.start(Selection::INTERNAL);
	ChangeList::const_iterator it = changes.begin();
	while (it != changes.end()) {
		Change* c = *it;
		switch (c->type) {
			case CHANGE_TILE: {
				void** data = &c->data;
				Tile* newtile = reinterpret_cast<Tile*>(*data);
				ASSERT(newtile);
				Position pos = newtile->getPosition();

				if (editor.IsLiveClient()) {
					QTreeNode* nd = editor.map.getLeaf(pos.x, pos.y);
					if (!nd || !nd->isVisible(pos.z > GROUND_LAYER)) {
						// Delete all changes that affect tiles outside our view
						c->clear();
						++it;
						continue;
					}
				}

				Tile* oldtile = editor.map.swapTile(pos, newtile);
				TileLocation* location = newtile->getLocation();

				// Update other nodes in the network
				if (editor.IsLiveServer() && dirty_list) {
					dirty_list->AddPosition(pos.x, pos.y, pos.z);
				}

				newtile->update();

				// std::cout << "\\tSwitched tile at " << pos.x << ";" << pos.y << ";" << pos.z << " from " << (void*)oldtile << " to " << *data <<  std::endl;
				if (newtile->isSelected()) {
					editor.selection.addInternal(newtile);
				}

				if (oldtile) {
					if (newtile->getHouseID() != oldtile->getHouseID()) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = editor.map.houses.getHouse(oldtile->getHouseID());
						if (house) {
							house->removeTile(oldtile);
						}

						house = editor.map.houses.getHouse(newtile->getHouseID());
						if (house) {
							house->addTile(newtile);
						}
					}
					if (oldtile->spawn) {
						if (newtile->spawn) {
							if (*oldtile->spawn != *newtile->spawn) {
								editor.map.removeSpawn(oldtile);
								editor.map.addSpawn(newtile);
							}
						} else {
							// Spawn has been removed
							editor.map.removeSpawn(oldtile);
						}
					} else if (newtile->spawn) {
						editor.map.addSpawn(newtile);
					}

					// oldtile->update();
					if (oldtile->isSelected()) {
						editor.selection.removeInternal(oldtile);
					}

					*data = oldtile;
				} else {
					*data = editor.map.allocator(location);
					if (newtile->getHouseID() != 0) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = editor.map.houses.getHouse(newtile->getHouseID());
						if (house) {
							house->addTile(newtile);
						}
					}

					if (newtile->spawn) {
						editor.map.addSpawn(newtile);
					}
				}
				// Mark the tile as modified
				newtile->modify();

				// Update client dirty list
				if (editor.IsLiveClient() && dirty_list && type != ACTION_REMOTE) {
					// Local action, assemble changes
					dirty_list->AddChange(c);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				std::pair<uint32_t, Position>* p = reinterpret_cast<std::pair<uint32_t, Position>*>(c->data);
				ASSERT(p);
				House* whathouse = editor.map.houses.getHouse(p->first);

				if (whathouse) {
					Position oldpos = whathouse->getExit();
					whathouse->setExit(p->second);
					p->second = oldpos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				std::pair<std::string, Position>* p = reinterpret_cast<std::pair<std::string, Position>*>(c->data);
				ASSERT(p);
				Waypoint* wp = editor.map.waypoints.getWaypoint(p->first);

				if (wp) {
					// Change the tiles
					TileLocation* oldtile = editor.map.getTileL(wp->pos);
					TileLocation* newtile = editor.map.getTileL(p->second);

					// Only need to remove from old if it actually exists
					if (p->second != Position()) {
						if (oldtile && oldtile->getWaypointCount() > 0) {
							oldtile->decreaseWaypointCount();
						}
					}

					newtile->increaseWaypointCount();

					// Update shit
					Position oldpos = wp->pos;
					wp->pos = p->second;
					p->second = oldpos;
				}
				break;
			}

			default:
				break;
		}
		++it;
	}
	editor.selection.finish(Selection::INTERNAL);
	commited = true;
}

void Action::undo(DirtyList* dirty_list) {
	if (changes.empty()) {
		return;
	}

	editor.selection.start(Selection::INTERNAL);
	ChangeList::reverse_iterator it = changes.rbegin();

	while (it != changes.rend()) {
		Change* c = *it;
		switch (c->type) {
			case CHANGE_TILE: {
				void** data = &c->data;
				Tile* oldtile = reinterpret_cast<Tile*>(*data);
				ASSERT(oldtile);
				Position pos = oldtile->getPosition();

				if (editor.IsLiveClient()) {
					QTreeNode* nd = editor.map.getLeaf(pos.x, pos.y);
					if (!nd || !nd->isVisible(pos.z > GROUND_LAYER)) {
						// Delete all changes that affect tiles outside our view
						c->clear();
						++it;
						continue;
					}
				}

				Tile* newtile = editor.map.swapTile(pos, oldtile);

				// Update server side change list (for broadcast)
				if (editor.IsLiveServer() && dirty_list) {
					dirty_list->AddPosition(pos.x, pos.y, pos.z);
				}

				if (oldtile->isSelected()) {
					editor.selection.addInternal(oldtile);
				}
				if (newtile->isSelected()) {
					editor.selection.removeInternal(newtile);
				}

				if (newtile->getHouseID() != oldtile->getHouseID()) {
					// oooooomggzzz we need to remove it from the appropriate house!
					House* house = editor.map.houses.getHouse(newtile->getHouseID());
					if (house) {
						house->removeTile(newtile);
					} else {
						// Set tile house to 0, house has been removed
						newtile->setHouse(nullptr);
					}

					house = editor.map.houses.getHouse(oldtile->getHouseID());
					if (house) {
						house->addTile(oldtile);
					}
				}

				if (oldtile->spawn) {
					if (newtile->spawn) {
						if (*oldtile->spawn != *newtile->spawn) {
							editor.map.removeSpawn(newtile);
							editor.map.addSpawn(oldtile);
						}
					} else {
						editor.map.addSpawn(oldtile);
					}
				} else if (newtile->spawn) {
					editor.map.removeSpawn(newtile);
				}
				*data = newtile;

				// Update client dirty list
				if (editor.IsLiveClient() && dirty_list && type != ACTION_REMOTE) {
					// Local action, assemble changes
					dirty_list->AddChange(c);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				std::pair<uint32_t, Position>* p = reinterpret_cast<std::pair<uint32_t, Position>*>(c->data);
				ASSERT(p);
				House* whathouse = editor.map.houses.getHouse(p->first);
				if (whathouse) {
					Position oldpos = whathouse->getExit();
					whathouse->setExit(p->second);
					p->second = oldpos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				std::pair<std::string, Position>* p = reinterpret_cast<std::pair<std::string, Position>*>(c->data);
				ASSERT(p);
				Waypoint* wp = editor.map.waypoints.getWaypoint(p->first);

				if (wp) {
					// Change the tiles
					TileLocation* oldtile = editor.map.getTileL(wp->pos);
					TileLocation* newtile = editor.map.getTileL(p->second);

					// Only need to remove from old if it actually exists
					if (p->second != Position()) {
						if (oldtile && oldtile->getWaypointCount() > 0) {
							oldtile->decreaseWaypointCount();
						}
					}

					if (newtile) {
						newtile->increaseWaypointCount();
					}

					// Update shit
					Position oldpos = wp->pos;
					wp->pos = p->second;
					p->second = oldpos;
				}
				break;
			}

			default:
				break;
		}
		++it;
	}
	editor.selection.finish(Selection::INTERNAL);
	commited = false;
}

BatchAction::BatchAction(Editor& editor, ActionIdentifier ident) :
	editor(editor),
	timestamp(0),
	memory_size(0),
	type(ident) {
	////
}

BatchAction::~BatchAction() {
	for (Action* action : batch) {
		delete action;
	}
	batch.clear();
}

size_t BatchAction::memsize(bool recalc) const {
	// Expensive operation, only evaluate once (won't change anyways)
	if (!recalc && memory_size > 0) {
		return memory_size;
	}

	uint32_t mem = sizeof(*this);
	mem += sizeof(Action*) * 3 * batch.size();

	for (Action* action : batch) {
#ifdef __USE_EXACT_MEMSIZE__
		mem += action->memsize();
#else
		// Less exact but MUCH faster
		mem += action->approx_memsize();
#endif
	}

	const_cast<BatchAction*>(this)->memory_size = mem;
	return mem;
}

void BatchAction::addAction(Action* action) {
	// If empty, do nothing.
	if (action->size() == 0) {
		delete action;
		return;
	}

	ASSERT(action->getType() == type);

	if (!editor.CanEdit()) {
		delete action;
		return;
	}

	// Add it!
	batch.push_back(action);
	timestamp = time(nullptr);
}

void BatchAction::addAndCommitAction(Action* action) {
	// If empty, do nothing.
	if (action->size() == 0) {
		delete action;
		return;
	}

	if (!editor.CanEdit()) {
		delete action;
		return;
	}

	// Add it!
	action->commit(nullptr);
	batch.push_back(action);
	timestamp = time(nullptr);
}

void BatchAction::commit() {
	for (Action* action : batch) {
		if (!action->isCommited()) {
			action->commit(nullptr);
		}
	}
}

void BatchAction::undo() {
	for (Action* action : boost::adaptors::reverse(batch)) {
		action->undo(nullptr);
	}
}

void BatchAction::redo() {
	for (Action* action : batch) {
		action->redo(nullptr);
	}
}

void BatchAction::merge(BatchAction* other) {
	batch.insert(batch.end(), other->batch.begin(), other->batch.end());
	other->batch.clear();
}

ActionQueue::ActionQueue(Editor& editor) :
	current(0), memory_size(0), editor(editor) {
	////
}

ActionQueue::~ActionQueue() {
	for (auto it = actions.begin(); it != actions.end(); it = actions.erase(it)) {
		delete *it;
	}
}

Action* ActionQueue::createAction(ActionIdentifier ident) {
	return newd Action(editor, ident);
}

Action* ActionQueue::createAction(BatchAction* batch) {
	return newd Action(editor, batch->getType());
}

BatchAction* ActionQueue::createBatch(ActionIdentifier ident) {
	return newd BatchAction(editor, ident);
}

void ActionQueue::resetTimer() {
	if (!actions.empty()) {
		actions.back()->resetTimer();
	}
}

void ActionQueue::addAction(Action* action, int stacking_delay) {
	// Ensure we're on the main thread
	if (!wxThread::IsMain()) {
		wxTheApp->CallAfter([=]() {
			this->addAction(action, stacking_delay);
		});
		return;
	}

	BatchAction* batch = createBatch(action->getType());
	batch->addAndCommitAction(action);
	if (batch->size() == 0) {
		delete batch;
		return;
	}

	addBatch(batch, stacking_delay);
}

void ActionQueue::addBatch(BatchAction* batch, int stacking_delay) {
	// Ensure we're on the main thread
	if (!wxThread::IsMain()) {
		// Use CallAfter but with a 'copy' of needed parameters
		// to prevent race conditions
		ActionQueue* self = this;
		int delay = stacking_delay;
		wxTheApp->CallAfter([self, batch, delay]() {
			self->addBatch(batch, delay);
		});
		return;
	}

	// Safety check - if batch is null, just return
	if (!batch) {
		return;
	}

	// Additional safety check to catch any potential crashes
	try {
		ASSERT(current <= actions.size());

		if (batch->size() == 0) {
			delete batch;
			return;
		}

		// Commit any uncommited actions...
		batch->commit();

		// Update title
		if (editor.map.doChange()) {
			// Use a safer version that doesn't trigger UI updates
			// during the first drawing operation
			static bool isFirstOperation = true;
			if (!isFirstOperation) {
				g_gui.UpdateTitle();
			} else {
				isFirstOperation = false;
			}
		}

		if (batch->type == ACTION_REMOTE) {
			delete batch;
			return;
		}

		// Protect against memory corruption
		while (current < actions.size() && !actions.empty()) {
			try {
				memory_size -= actions.back()->memsize();
				BatchAction* todelete = actions.back();
				actions.pop_back();
				delete todelete;
			} catch (const std::exception& e) {
				// Log error but continue processing
				std::ofstream logFile((wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + "action_error.log").ToStdString(), std::ios::app);
				if (logFile.is_open()) {
					wxDateTime now = wxDateTime::Now();
					logFile << now.FormatISOCombined() << ": Error clearing action: " << e.what() << "\\n";
					logFile.close();
				}
				break;
			}
		}

		// Safely manage memory
		try {
			while (memory_size > size_t(1024 * 1024 * g_settings.getInteger(Config::UNDO_MEM_SIZE)) && !actions.empty()) {
				memory_size -= actions.front()->memsize();
				delete actions.front();
				actions.pop_front();
				if (current > 0) {
					current--;
				}
			}

			if (actions.size() > size_t(g_settings.getInteger(Config::UNDO_SIZE)) && !actions.empty()) {
				memory_size -= actions.front()->memsize();
				BatchAction* todelete = actions.front();
				actions.pop_front();
				delete todelete;
				if (current > 0) {
					current--;
				}
			}

			// Process action with additional safety
			do {
				if (!actions.empty()) {
					BatchAction* lastAction = actions.back();
					if (lastAction->type == batch->type && g_settings.getInteger(Config::GROUP_ACTIONS) && time(nullptr) - stacking_delay < lastAction->timestamp) {
						lastAction->merge(batch);
						lastAction->timestamp = time(nullptr);
						memory_size -= lastAction->memsize();
						memory_size += lastAction->memsize(true);
						delete batch;
						break;
					}
				}
				memory_size += batch->memsize();
				actions.push_back(batch);
				batch->timestamp = time(nullptr);
				current++;
			} while (false);
		} catch (const std::exception& e) {
			// Log error but don't crash
			std::ofstream logFile((wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + "action_error.log").ToStdString(), std::ios::app);
			if (logFile.is_open()) {
				wxDateTime now = wxDateTime::Now();
				logFile << now.FormatISOCombined() << ": Error processing batch: " << e.what() << "\\n";
				logFile.close();
			}
		}
	} catch (const std::exception& e) {
		// Last resort error handler for entire function
		std::ofstream logFile((wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + "action_error.log").ToStdString(), std::ios::app);
		if (logFile.is_open()) {
			wxDateTime now = wxDateTime::Now();
			logFile << now.FormatISOCombined() << ": Critical error in addBatch: " << e.what() << "\\n";
			logFile.close();
		}
	}
}

void ActionQueue::undo() {
	if (current > 0) {
		current--;
		BatchAction* batch = actions[current];
		batch->undo();
	}
}

void ActionQueue::redo() {
	if (current < actions.size()) {
		BatchAction* batch = actions[current];
		batch->redo();
		current++;
	}
}

void ActionQueue::clear() {
	for (ActionList::iterator it = actions.begin(); it != actions.end();) {
		delete *it;
		it = actions.erase(it);
	}
	current = 0;
}

DirtyList::DirtyList() :
	owner(0) {
	;
}

DirtyList::~DirtyList() {
	;
}

void DirtyList::AddPosition(int x, int y, int z) {
	uint32_t m = ((x >> 2) << 18) | ((y >> 2) << 4);
	ValueType fi = { m, 0 };
	SetType::iterator s = iset.find(fi);
	if (s != iset.end()) {
		ValueType v = *s;
		iset.erase(s);
		v.floors = (1 << z) | v.floors;
		iset.insert(v);
	} else {
		ValueType v = { m, (uint32_t)(1 << z) };
		iset.insert(v);
	}
}

void DirtyList::AddChange(Change* c) {
	ichanges.push_back(c);
}

DirtyList::SetType& DirtyList::GetPosList() {
	return iset;
}

ChangeList& DirtyList::GetChanges() {
	return ichanges;
}
""",
    "wxwidgets/action.h": """//////////////////////////////////////////////////////////////////////
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

#ifndef RME_ACTION_H_
#define RME_ACTION_H_

#include "position.h"

#include <deque>

class Editor;
class Tile;
class House;
class Waypoint;
class Change;
class Action;
class BatchAction;
class ActionQueue;

enum ChangeType {
	CHANGE_NONE,
	CHANGE_TILE,
	CHANGE_MOVE_HOUSE_EXIT,
	CHANGE_MOVE_WAYPOINT,
};

class Change {
private:
	ChangeType type;
	void* data;

	Change();

public:
	Change(Tile* tile);
	static Change* Create(House* house, const Position& where);
	static Change* Create(Waypoint* wp, const Position& where);
	~Change();
	void clear();

	ChangeType getType() const {
		return type;
	}
	void* getData() const {
		return data;
	}

	// Get memory footprint
	uint32_t memsize() const;

	friend class Action;
};

typedef std::vector<Change*> ChangeList;

// A dirty list represents a list of all tiles that was changed in an action
class DirtyList {
public:
	DirtyList();
	~DirtyList();

	struct ValueType {
		uint32_t pos;
		uint32_t floors;
	};

	uint32_t owner;

protected:
	struct Comparator {
		bool operator()(const ValueType& a, const ValueType& b) const {
			return a.pos < b.pos;
		}
	};

public:
	typedef std::set<ValueType, Comparator> SetType;

	void AddPosition(int x, int y, int z);
	void AddChange(Change* c);
	bool Empty() const {
		return iset.empty() && ichanges.empty();
	}
	SetType& GetPosList();
	ChangeList& GetChanges();

protected:
	SetType iset;
	ChangeList ichanges;
};

enum ActionIdentifier {
	ACTION_MOVE,
	ACTION_REMOTE,
	ACTION_SELECT,
	ACTION_DELETE_TILES,
	ACTION_CUT_TILES,
	ACTION_PASTE_TILES,
	ACTION_RANDOMIZE,
	ACTION_BORDERIZE,
	ACTION_DRAW,
	ACTION_SWITCHDOOR,
	ACTION_ROTATE_ITEM,
	ACTION_REPLACE_ITEMS,
	ACTION_CHANGE_PROPERTIES,
};

class Action {
public:
	virtual ~Action();

	void addChange(Change* t) {
		changes.push_back(t);
	}

	// Get memory footprint
	size_t approx_memsize() const;
	size_t memsize() const;
	size_t size() const {
		return changes.size();
	}
	ActionIdentifier getType() const {
		return type;
	}

	void commit(DirtyList* dirty_list);
	bool isCommited() const {
		return commited;
	}
	void undo(DirtyList* dirty_list);
	void redo(DirtyList* dirty_list) {
		commit(dirty_list);
	}

protected:
	Action(Editor& editor, ActionIdentifier ident);

	bool commited;
	ChangeList changes;
	Editor& editor;
	ActionIdentifier type;

	friend class ActionQueue;
};

typedef std::vector<Action*> ActionVector;

class BatchAction {
public:
	virtual ~BatchAction();

	void resetTimer() {
		timestamp = 0;
	}

	// Get memory footprint
	size_t memsize(bool resize = false) const;
	size_t size() const {
		return batch.size();
	}
	ActionIdentifier getType() const {
		return type;
	}

	virtual void addAction(Action* action);
	virtual void addAndCommitAction(Action* action);

protected:
	BatchAction(Editor& editor, ActionIdentifier ident);

	virtual void commit();
	virtual void undo();
	virtual void redo();

	void merge(BatchAction* other);

	Editor& editor;
	int timestamp;
	uint32_t memory_size;
	ActionIdentifier type;
	ActionVector batch;

	friend class ActionQueue;
};

class ActionQueue {
public:
	ActionQueue(Editor& editor);
	virtual ~ActionQueue();

	typedef std::deque<BatchAction*> ActionList;

	void resetTimer();

	virtual Action* createAction(ActionIdentifier ident);
	virtual Action* createAction(BatchAction* parent);
	virtual BatchAction* createBatch(ActionIdentifier ident);

	void addBatch(BatchAction* action, int stacking_delay = 0);
	void addAction(Action* action, int stacking_delay = 0);

	void undo();
	void redo();
	void clear();

	bool canUndo() {
		return current > 0;
	}
	bool canRedo() {
		return current < actions.size();
	}

protected:
	size_t current;
	size_t memory_size;
	Editor& editor;
	ActionList actions;
};

#endif
"""
}

analyzed_files_data = []
file_descriptions = {
    "wxwidgets/action.cpp": "Implementation of Action, Change, BatchAction, and ActionQueue classes for managing undo/redo history and specific editor operations.",
    "wxwidgets/action.h": "Header file for the action and history system, defining interfaces for Action, Change, BatchAction, and ActionQueue."
}

for file_path, content in cpp_files_content.items():
    analyzed_files_data.append({
        "file_path": file_path, # Corrected from 'filename'
        "description": file_descriptions.get(file_path, "N/A"),
        "md5_hash": get_md5_hash(content),
        "content_lite": get_content_lite(content)
    })

yaml_output = {
    "wbs_item_id": "CORE-03",
    "name": "Port Action & History System",
    "description": "Migrate the action recording and history (undo/redo) mechanisms from `mapcore` (likely within `action.cpp`/`.h`) to a robust Qt6/C++ implementation. This should support various editor operations and provide a reliable undo/redo stack.",
    "dependencies": [ # From original WBS: CORE-02
        "CORE-02"
    ],
    "input_files": [
        "wxwidgets/action.cpp",
        "wxwidgets/action.h"
    ],
    "analyzed_input_files": analyzed_files_data,
    "documentation_references": [ # Added from similar tasks
        "Qt Command Pattern: https://doc.qt.io/qt-6/qtwidgets-mainwindows-application-example.html#command-pattern",
        "QUndoStack: https://doc.qt.io/qt-6/qundostack.html",
        "QUndoCommand: https://doc.qt.io/qt-6/qundocommand.html",
        "std::unique_ptr: https://en.cppreference.com/w/cpp/memory/unique_ptr"
    ],
    "current_functionality_summary": """The wxWidgets-based action system consists of:
- `Change`: Encapsulates data for a modification (e.g., a copy of a `Tile` before changes).
- `Action`: Represents an operation that can be undone/redone. It holds a list of `Change` objects. Key methods are `commit()` (apply changes) and `undo()`.
- `BatchAction`: Groups multiple related `Action` instances, often of the same type, if they occur close in time (stacking delay). It manages a list of sub-actions.
- `ActionQueue`: Manages a list of `BatchAction` objects, providing `undo()` and `redo()` capabilities. It also handles memory limits for the undo stack.""",
    "definition_of_done": [
        "A Qt6/C++ Command pattern is implemented, including:",
        "  - An abstract `Action` (or `Command`) base class with virtual `execute()`, `undo()`, and potentially `redo()` methods.",
        "  - A mechanism equivalent to the `Change` class to encapsulate the actual data/state modification for undo/redo purposes.",
        "  - An `ActionQueue` (or `UndoStack`) class capable of managing a list of `Action` objects.",
        "The `ActionQueue` supports:",
        "  - Adding new actions to the stack.",
        "  - Performing undo operations, which revert the last executed action.",
        "  - Performing redo operations, which re-apply the last undone action.",
        "  - Clearing actions that are undone when a new action is added.",
        "  - Limits on undo stack size (memory or count) as per original settings (e.g., `UNDO_MEM_SIZE`, `UNDO_SIZE`).",
        "`Action` objects correctly store the necessary context or `Change` objects to perform and revert their specific operations.",
        "Memory management for `Action` objects and their associated `Change` data is handled using modern C++ practices (e.g., smart pointers).",
        "The system is designed to be extensible for various editor operations (drawing, deleting, pasting, etc.) by creating new classes derived from the `Action` base class.",
        "Basic unit tests are created for the `ActionQueue` to verify:",
        "  - Adding actions.",
        "  - Correct execution of undo and redo.",
        "  - Stack limits and clearing behavior.",
        "Unit tests for a sample concrete `Action` implementation to verify its execute/undo logic and interaction with `Change` objects.",
        "The ported code compiles successfully within a Qt6 project structure.",
        "A brief report detailing the mapping from the old action system to the new Command pattern, highlighting design choices."
    ],
    "boilerplate_coder_ai_prompt": """The user wants to migrate an action and history (undo/redo) system from an old C++/wxWidgets application to Qt6/modern C++.
Analyze the provided C++ header and source file snippets for `Action`, `Change`, `BatchAction`, and `ActionQueue` (from `wxwidgets/action.cpp` and `wxwidgets/action.h`).

**1. Change Object Equivalent:**
   - Design a class or struct (e.g., `ChangeData` or use `QVariant` / `std::any` for flexibility) to encapsulate the information needed to undo/redo a specific modification. The original `Change` class stores a `type` and `void* data` (pointing to old `Tile*`, `std::pair` for moves, etc.). The new system should be more type-safe.
   - For example, a `TileChangeData` might store a copy of the `Tile` before modification, or pointers/references to the old and new states.

**2. Action (Command) Class:**
   - Create an abstract base class `Action` (or `Command`) with pure virtual methods:
     - `virtual void execute() = 0;`
     - `virtual void undo() = 0;`
     - (Optional: a `redo()` method, which might simply call `execute()`, if redo is always a re-execution).
   - Concrete action classes (e.g., `DrawTileAction`, `MoveItemAction`) will derive from this and implement these methods.
   - Each concrete action will be responsible for creating and storing the necessary `ChangeData` objects during its `execute()` method to allow `undo()` to function.
   - The constructor for a concrete action should take all necessary parameters to perform the operation (e.g., target position, item data).

**3. ActionQueue (UndoStack) Class:**
   - Implement an `ActionQueue` class to manage the undo/redo history.
   - It should use a container (e.g., `QList<std::unique_ptr<Action>>` or `std::vector<std::unique_ptr<Action>>`) to store executed actions.
   - Maintain an index to the current position in the list (top of the undo stack).
   - `push(std::unique_ptr<Action> action)`: Adds a new action. This should execute the action. If there are actions in the "redo" part of the stack (i.e., current index < list size - 1), they should be cleared.
   - `undo()`: If possible, calls `undo()` on the action at the current index and decrements the index.
   - `redo()`: If possible, calls `execute()` (or `redo()`) on the action at the next index and increments the index.
   - `canUndo()`: Returns true if actions can be undone.
   - `canRedo()`: Returns true if actions can be redone.
   - Implement logic for limiting the stack size (e.g., based on item count or estimated memory, similar to `UNDO_MEM_SIZE` and `UNDO_SIZE` from `Config`). When the limit is reached, the oldest actions are discarded.
   - The `BatchAction` concept from the original code allowed grouping multiple `Action` instances, often of the same type, if they occurred close in time (stacking delay). Consider if this is a requirement for the initial port. If so, the `ActionQueue::push` logic would need to check the last action and potentially merge the new one if it meets the criteria, or a `BatchAction` itself could be an `Action` subclass that manages a list of sub-actions. For simplicity, individual actions might be sufficient for a first pass.

**General Porting Instructions:**
   - Replace wxWidgets specific types or utilities with Qt/STL equivalents.
   - Ensure robust memory management using smart pointers (`std::unique_ptr` is highly recommended for actions in the queue).
   - The `Action` classes will need to interact with the map data. This interaction should ideally be through well-defined interfaces or references to map manipulation services, passed into the actions or accessible globally (though direct global access is less ideal).
   - The `ActionIdentifier` enum can be reused or adapted.
   - The `DirtyList` from the original code seems related to notifying other parts of the system about changes. This can be handled by having actions emit signals (if using Qt's signal/slot mechanism) or by calling a notification service after execution/undo.
   - Provide header (.h) and source (.cpp) files for the `Action` base class, the `ActionQueue`, and any example concrete action(s).
   - Include basic unit tests (e.g., using Qt Test framework) for `ActionQueue` functionality and a sample action.
"""
}

output_filename = "enhanced_wbs_yaml_files/CORE-03.yaml"
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
