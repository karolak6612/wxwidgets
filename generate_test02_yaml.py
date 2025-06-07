import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_data = {
    "wbs_item_id": "TEST-02",
    "name": "Unit Test Asset Loading & Parsing",
    "description": "Create unit tests for the asset loading and parsing logic. This involves testing the correct parsing of data files (e.g., `items.xml`, `creatures.xml`, `materials.xml` and its includes, client version files) and ensuring the data is correctly populated into the relevant data structures.",
    "dependencies": [
        "CORE-02", # Defines ItemDatabase, ItemData, CreatureData, ClientProfile and their parsers
        "CORE-04", # Defines BrushSystem/MaterialManager and parsers for materials.xml and its includes
        "TEST-01"  # Core data structures should ideally be stable before testing their population via parsing
    ],
    "input_files": [
        # XML files are conceptual inputs for test case design, not direct C++ porting inputs
        "XML/760/items.xml", # (and/or OTB format if applicable)
        "XML/760/creatures.xml",
        "XML/clients.xml",
        "XML/760/materials.xml",
        "XML/760/grounds.xml",
        "XML/760/walls.xml",
        "XML/760/doodads.xml",
        "XML/760/borders.xml",
        "XML/760/creature_palette.xml",
        "XML/760/item_palette.xml",
        "XML/760/raw_palette.xml",
        "XML/760/tilesets.xml",
        "XML/760/collections.xml"
    ],
    "analyzed_input_files": [], # No specific wxWidgets C++ files are ported for this task; tests are new.
    "documentation_references": [
        "Qt Test Framework: https://doc.qt.io/qt-6/qttest-index.html",
        "Mocking and Test Data: Best practices for creating representative test data snippets."
    ],
    "current_functionality_summary": """\
This task involves creating new unit tests for the Qt6 asset parsers. The parsers themselves are developed in other tasks (CORE-02, CORE-04) and are responsible for processing various XML files (items, creatures, materials, etc.) that define game assets and editor configurations. These tests will validate that the parsers correctly transform XML data into the application's C++ data structures.\
""",
    "definition_of_done": [
        "Unit tests using the Qt Test framework are developed for each key asset parser: Items, Creatures, Client Versions, and the comprehensive Materials system (including grounds, walls, doodads, borders, palettes, tilesets, collections).",
        "Test cases utilize small, focused mock XML data snippets (or valid small example files) to cover:",
        "  - Correct parsing of all known attributes and elements into their corresponding C++ object fields.",
        "  - Handling of optional attributes/elements (e.g., ensuring default values are applied or fields are left empty/null as expected).",
        "  - Parsing of different valid data variations and value types.",
        "  - Robust error handling for malformed XML, missing required data, incorrect data types, and invalid references (e.g., a ground brush referencing a non-existent border ID in a controlled test setup).",
        "For the materials system, tests verify that hierarchical loading (e.g., `materials.xml` including `grounds.xml`) and the linking of data between related XMLs (e.g., `grounds.xml` using border definitions from `borders.xml`) function correctly with mock data.",
        "Tests confirm that the C++ data structures (like `ItemData`, `GroundBrush`, `WallBrushDoodadBrush`, palette groups) are accurately populated based on the mock XML input.",
        "All created unit tests pass successfully.",
        "The tests are integrated into the CMake build system for automated execution."
    ],
    "boilerplate_coder_ai_prompt": """\
Your task is to write unit tests for the XML asset parsers developed for the Qt6 Remere's Map Editor. These parsers handle files like `items.xml`, `creatures.xml`, `clients.xml`, and the `materials.xml` suite (which includes `grounds.xml`, `walls.xml`, `doodads.xml`, `borders.xml`, various palette files, `tilesets.xml`, and `collections.xml`). Use the Qt Test framework.

**General Approach:**
1.  For each distinct parser (e.g., ItemParser, CreatureParser, MaterialsParser), create a separate test class inheriting from `QObject`.
2.  For each test function, prepare a small, self-contained XML string or load a small, dedicated test XML file. This mock data should be designed to test specific scenarios. Avoid using the full, large game data XMLs for unit tests.
3.  Instantiate the parser and the target data structure(s) (e.g., `ItemDatabase`, `MaterialManager`).
4.  Invoke the parsing method with the mock XML data.
5.  Use `QCOMPARE`, `QVERIFY`, and other Qt Test macros to assert that the resulting C++ objects are populated correctly and that error conditions are handled as expected.

**Specific Areas and XMLs to Test:**

*   **Item Parser (e.g., for `items.xml` or OTB):**
    - Test parsing of various item attributes: `id`, `name`, `type`, article, plural, description, looktypes, light properties, stackability, corpse ID, ground speed, slot, maxitems, weapon/armor values, charges, decay, container size.
    - Test correct interpretation of all boolean flags: `blockprojectile`, `blockpath`, `pickupable`, `hangable`, `hookSouth`, `hookEast`, `rotateable`, `haslight`, `unpassable`, `unmoveable`, `useable`, `animate`, `floorchange`, `alwaysontop`.
    - Test with items having minimal attributes vs. all attributes.
    - Test error handling for missing `id` or `name`.

*   **Creature Parser (e.g., for `creatures.xml`):**
    - Test parsing of creature `name`, `type`, `looktype`, and individual outfit components (`lookhead`, `lookbody`, `looklegs`, `lookfeet`).
    - Test creatures with and without optional outfit components.

*   **ClientVersion Parser (e.g., for `clients.xml`):**
    - Test parsing of client `id`, `name`, `version`, `description`, `signature`.
    - Test parsing of nested `<extension>` elements.

*   **Materials System Parsers (Hierarchical):**
    - **`borders.xml`:** Test parsing of border definitions with different edge types and item IDs.
    - **`grounds.xml`:**
        - Test parsing of ground brushes with item variations (IDs and chances).
        - Test parsing of `z-order`.
        - Test parsing of `<border align="..." id="..." [to="..."] [ground_equivalent="..."] [super="..."]/>` tags and ensure the `id` correctly links to a (mocked) border definition.
        - Test parsing of complex conditional border logic (`<specific>`, `<conditions>`, `<actions>`).
        - Test parsing of `<friend name="..."/>` tags.
    - **`walls.xml`:**
        - Test parsing of different wall types (horizontal, vertical, pole, T-junctions, corners, diagonals, ends).
        - Test parsing of associated `<item id="..." chance="..."/>`.
        - Test parsing of embedded `<door id="..." type="..." open="..." [locked="..."]/>` with various door types.
        - Test `<friend name="..." redirect="true"/>` logic.
    - **`doodads.xml` / `walls_extra.xml` (for Doodad Brushes):**
        - Test parsing of doodad attributes: `server_lookid`, `draggable`, `on_blocking`, `thickness`, `one_size`, `redo_borders`.
        - Test parsing of `<alternate>` and `<composite>` structures, including correct `tile x="..." y="..." [z="..."]` with item IDs.
        - Test simple item doodads vs. complex composites.
    - **Palette Files (`creature_palette.xml`, `item_palette.xml`, `raw_palette.xml`):**
        - Test parsing of `<tileset name="...">` containing `<items>`, `<doodad>`, or `<raw>` tags.
        - Verify correct parsing of individual `<item id="..."/>` and ranged `<item fromid="..." toid="..."/>`.
        - For `<doodad>` sections in palettes, test parsing of item IDs and `<brush name="..."/>` references.
    - **`tilesets.xml` (Master Aggregator):**
        - Test parsing of `<tileset name="...">` containing `<items>`, `<doodad>`, `<raw>`, and `<terrain>` sections.
        - Ensure it correctly references brushes by name (which would be loaded from other mocked files like `grounds.xml`, `walls.xml`, `doodads.xml`).
    - **`collections.xml`:**
        - Test parsing of `<tileset name="...">` containing `<collections>` with `<brush name="..."/>` and `<item id="..."/>` (and ranges).

**Error Handling Tests:**
- For each parser, include tests for:
  - Malformed XML input.
  - Missing required attributes or elements.
  - Attributes with incorrect data types (e.g., string for an integer).
  - Invalid references (e.g., a brush in `tilesets.xml` referencing a non-existent brush name).

Integrate these tests into the CMake build system.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/TEST-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

del yaml_data
