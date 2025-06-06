import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "LOGIC-05",
    "section": "House System", # As per WBS in prompt.md
    "title": "Implement House Management in Core",
    "original_input_files": "house.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/house.h",
        "wxwidgets/house.cpp"
    ],
    "dependencies": [
        "CORE-01", # For base data models (Tile, Position, Map)
    ],
    "current_functionality_summary": """\
The `house.h/cpp` files define the `House` class, representing individual houses with attributes like ID, name, rent, associated tiles, and an exit position. They also define the `Houses` class, which acts as a manager for all `House` objects within a `Map`. These classes are responsible for the core data representation and management of houses, including their relationship with map tiles. They were targeted for inclusion in `mapcore` during `CORE-01`.\
""",
    "qt6_migration_steps": """\
1. Confirm that the `House` and `Houses` classes are fully integrated into the `mapcore` library and are devoid of any wxWidgets or other UI framework dependencies (as per `CORE-01`'s objectives for these files).
2. Rigorously review and refine the `House::addTile(Tile* tile)` and `House::removeTile(Tile* tile)` methods. Ensure they robustly update the `Tile::house_id` (using `tile->setHouseID()`) and correctly manage the `House`'s internal list of tile positions.
3. Rigorously review and refine the `House::setExit(const Position& pos)` method. This method must accurately update the `HouseExitList` on the `TileLocation` objects for both the old exit tile (if any) and the new exit tile.
4. Verify the integration with the map loading/saving process (`IOMap*` classes from `CORE-03`):
   - When a map (e.g., OTBM) is loaded, ensure that the `mapInstance->houses` (a `Houses` object) is correctly populated with all `House` objects, including their lists of tiles and exit positions. This involves checking how tile attributes like `house_id` and house exit data are interpreted during parsing.
   - When a map is saved, ensure that all data for houses managed by `mapInstance->houses` (ID, name, rent, list of tiles belonging to it, exit position) is correctly serialized into the map file format (OTBM or auxiliary house XML).
5. Implement comprehensive unit tests for the `House` and `Houses` classes using the testing framework set up in `TEST-01` (e.g., Google Test). These tests should cover:
   - Creation of `House` objects and setting/getting their various attributes (name, rent, town ID, guildhall status).
   - Adding tiles to a `House` and verifying that `Tile::getHouseID()` returns the correct house ID.
   - Removing tiles from a `House` and verifying `Tile::getHouseID()` is cleared.
   - Setting and subsequently changing a `House`'s exit position, then verifying the `HouseExitList` on the affected `TileLocation`s.
   - `Houses` manager: Adding new houses, removing houses, and retrieving houses by their ID.
   - (Recommended) Test cases that involve serializing a `Map` object with houses, then deserializing it and verifying that all house data is restored with integrity.\
""",
    "definition_of_done": """\
The `mapcore` library contains fully functional `House` and `Houses` manager classes for house data management, decoupled from UI.
Key requirements:
- `House` class correctly manages its attributes and its list of associated `Tile` positions.
- `Houses` class correctly manages a collection of `House` objects within a `Map`.
- Adding/removing tiles to/from a `House` correctly updates the `Tile::house_id`.
- Setting a `House` exit correctly updates the `HouseExitList` on the relevant `TileLocation`s.
- House data is correctly serialized and deserialized during map load/save operations (via OTBM/auxiliary XMLs, as handled by `CORE-03`).
- Unit tests for `House` and `Houses` functionalities (creation, tile assignment, exit management) are implemented and pass.
- Classes are free of UI framework dependencies and reside entirely within `mapcore`.\
""",
    "boilerplate_coder_ai_prompt": """\
Your task is to ensure the `House` and `Houses` classes (from `house.h/cpp`) are fully functional data management components within the `mapcore` static library, building upon `CORE-01`.
1.  Verify and refine the `House` class within `mapcore`:
    - Ensure `House::addTile(Tile* tile)` correctly sets `tile->setHouseID(this->getID())` and adds `tile->getPosition()` to its internal list.
    - Ensure `House::removeTile(Tile* tile)` correctly calls `tile->setHouseID(0)` (or equivalent for no house) and removes the tile's position from its list.
    - Ensure `House::setExit(const Position& pos)` properly updates `TileLocation::house_exits` for both the old exit (if any) and the new exit tile, interacting with `Map` to get `TileLocation` objects.
2.  Verify and refine the `Houses` class (manager for all houses in a `Map`) within `mapcore`:
    - Confirm methods like `addHouse`, `removeHouse`, `getHouse`, `getEmptyID` are robust.
3.  Integration with Map I/O (from `CORE-03`):
    - Check the OTBM loading logic (e.g., in `IOMapOTBM.cpp`): When house tiles are encountered (e.g., `OTBM_HOUSETILE` node type or tiles with house-specific attributes), ensure `House` objects are created/retrieved via `map->houses.getHouse(house_id)` and `house->addTile(tile)` is called.
    - Ensure house exit positions are correctly read and `house->setExit()` is called.
    - Check OTBM saving logic: Ensure all `House` data (ID, name, rent, town ID, exit, and implicitly the tiles via their `house_id`) is saved.
4.  Write Unit Tests for `mapcore` (using Google Test, framework from `TEST-01`):
    - Test `House` creation, setting/getting name, rent, townid, guildhall status.
    - Test `house->addTile(tile)`: verify `tile->getHouseID()` and house's tile list.
    - Test `house->removeTile(tile)`: verify `tile->getHouseID()` is cleared and house's tile list.
    - Test `house->setExit(pos)`: verify `oldExitTileLocation->getHouseExits()` and `newExitTileLocation->getHouseExits()`.
    - Test `Houses::addHouse(house)`, `Houses::removeHouse(house)`, `Houses::getHouse(id)`, `Houses::getEmptyID()`.
5.  Ensure `house.h` and `house.cpp` are fully part of `mapcore` and have no wxWidgets or UI dependencies.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/LOGIC-05.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
