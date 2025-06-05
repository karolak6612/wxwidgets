import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "CORE-06",
    "section": "Data I/O & Management",
    "title": "Port Creature XML & OTB Integration",
    "original_input_files": "creatures.h/cpp, creature_brush.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/creatures.h",
        "wxwidgets/creatures.cpp",
        "wxwidgets/creature_brush.h",
        "wxwidgets/creature_brush.cpp"
    ],
    "dependencies": [
        "CORE-02",
        "CORE-04"
    ],
    "current_functionality_summary": """\
- `CreatureDatabase` (in `creatures.h/cpp`): Loads creature and NPC definitions from various XML sources (custom editor `creatures.xml`, and OT server `monsters/` and `npcs/` directories) using pugixml. It stores these as `CreatureType` objects (name, outfit, NPC status). It also supports saving custom creature data back to its own `creatures.xml`.
- `CreatureBrush` (in `creature_brush.h/cpp`): Uses these `CreatureType` definitions to allow placement of creatures on the map.
The "OTB Integration" aspect likely refers to resolving creature names from loaded OTB maps against this database to get full type information.\
""",
    "qt6_migration_steps": """\
1. Within the `mapcore` library, enhance the `CreatureDatabase` (ported in `CORE-02`).
2. Implement recursive directory scanning logic in `CreatureDatabase` using `QDirIterator` to locate all monster and NPC XML files within specified game data directory structures (e.g., `data/Tibia/monsters/`, `data/Tibia/npcs/`).
3. Ensure all XML parsing for these files (and the custom `creatures.xml`) uses `QXmlStreamReader` to populate the `CreatureDatabase` with `CreatureType` objects.
4. Implement the functionality for `CreatureDatabase` to save its current state (particularly custom or editor-modified creature types) to a `creatures.xml` file using `QXmlStreamWriter`. This file should also be loadable by `CreatureDatabase` at startup.
5. Verify that `CreatureBrush` (ported in `CORE-04`) correctly queries the `mapcore`-internal `CreatureDatabase` to obtain `CreatureType` information for placing creatures.
6. Ensure that creature instances loaded from OTB maps (which typically store only the creature's name) can have their full `CreatureType` details (e.g., outfit data for rendering) resolved by looking up their name in the `mapcore`'s `CreatureDatabase`.\
""",
    "definition_of_done": """\
The `CreatureDatabase` within `mapcore` is fully functional for managing creature and NPC definitions from various XML sources and its own persisted custom data.
Key requirements:
- `CreatureDatabase` can recursively scan specified directories to find and parse all standard monster and NPC XML files using `QXmlStreamReader`.
- `CreatureDatabase` can save its current state (custom/modified creatures) to a `creatures.xml` file using `QXmlStreamWriter`.
- `CreatureDatabase` can load data from this custom `creatures.xml` file upon initialization.
- `CreatureBrush` correctly utilizes the `mapcore`-internal `CreatureDatabase`.
- Creature instances loaded from OTB maps can have their types successfully resolved against the `CreatureDatabase` to retrieve properties like outfits.
- All functionality is contained within `mapcore` and exclusively uses Qt classes for XML and file I/O operations.\
""",
    "boilerplate_coder_ai_prompt": """\
Your task is to finalize the `CreatureDatabase` system within the `mapcore` static library (this builds on `CORE-02` and `CORE-04`). The classes should remain as modular files within `mapcore`.
1. Implement recursive directory scanning in `CreatureDatabase` using `QDirIterator` to find and parse all monster and NPC XML files from typical OT server directory structures. All XML parsing must use `QXmlStreamReader`.
2. Implement methods in `CreatureDatabase` to save its current data (especially any custom or editor-modified creatures) to a `creatures.xml` file. Use `QXmlStreamWriter` for this.
3. Implement logic in `CreatureDatabase` to load data from this `creatures.xml` file during its initialization.
4. Confirm that `CreatureBrush` (already ported) correctly interacts with this enhanced `CreatureDatabase`.
5. Ensure that creature names stored in loaded OTB maps can be effectively used to query the `CreatureDatabase` for complete `CreatureType` information (e.g., for rendering outfits). This primarily involves ensuring the database is robustly populated and queryable.
6. All code must reside within `mapcore` and use Qt types for file/XML I/O. No wxWidgets or pugixml dependencies.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/CORE-06.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
