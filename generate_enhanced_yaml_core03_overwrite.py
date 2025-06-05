import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "CORE-03",
    "section": "Core Migration Tasks",
    "title": "Port OTBM/OTMM File I/O",
    "original_input_files": "iomap_otbm.h/cpp, iomap_otmm.h/cpp, filehandle.h/cpp, iomap.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/iomap_otbm.h",
        "wxwidgets/iomap_otbm.cpp",
        "wxwidgets/iomap_otmm.h",
        "wxwidgets/iomap_otmm.cpp",
        "wxwidgets/filehandle.h",
        "wxwidgets/filehandle.cpp",
        "wxwidgets/iomap.h",
        "wxwidgets/iomap.cpp"
    ],
    "dependencies": [
        "CORE-02"
    ],
    "current_functionality_summary": """\
These files provide the framework for serializing and deserializing map data.
- `filehandle.h/cpp`: Defines low-level classes for binary file I/O, including a node-based system (e.g., `NodeFileReadHandle`, `BinaryNode`) for structured data with nesting, used by OTBM/OTMM. Uses `FILE*`.
- `iomap.h/cpp`: Defines `IOMap` as an abstract base class for map I/O, handling errors/warnings (using wxWidgets types).
- `iomap_otbm.h/cpp`: Implements OTBM (OpenTibia Binary Map) format I/O. Parses the main `.otbm` and can load auxiliary XML spawn/house files (using pugixml) and decompress `.otgz` archives (using libarchive).
- `iomap_otmm.h/cpp`: Implements OTMM (OpenTibia Map Mapped) format I/O.\
""",
    "qt6_migration_steps": """\
1. Integrate all listed I/O classes (`FileHandle`, `NodeFileReadHandle`, `IOMap`, `IOMapOTBM`, `IOMapOTMM`, etc.) into the `mapcore` library. These classes should be added as separate, modular `.h/.cpp` files within the `mapcore` library's source structure.
2. Refactor `FileHandle` and its derivatives: Replace all `FILE*` based operations with `QFile` methods (`open`, `close`, `read`, `write`, `seek`, `pos`). Use appropriate `QIODevice` open modes.
3. In `IOMapOTBM`, replace `pugixml` usage for parsing/writing external XML spawn/house files with `QXmlStreamReader` and `QXmlStreamWriter`.
4. For `.otgz` handling in `IOMapOTBM`, retain `libarchive` usage. Ensure `mapcore`'s CMake configuration correctly finds and links `libarchive`. Remove any wxWidgets types from the libarchive integration code.
5. Remove all wxWidgets dependencies:
   - Replace `wxString` with `std::string`.
   - Replace `wxArrayString` (e.g., for `IOMap::warnings`) with `std::vector<std::string>`.
   - Replace `wxFileName` with `std::filesystem::path` or `QFileInfo`/`QDir`.
   - Eliminate UI interaction calls (`IOMap::queryUser`, `g_gui.PopupDialog`, progress bar updates). Errors/warnings should be stored in `IOMap` members (e.g., `std::string errorstr`, `std::vector<std::string> warnings`).
6. Preserve the core logic of `NodeFileReadHandle`, `NodeFileWriteHandle`, and `BinaryNode` for handling the OTBM/OTMM node structure, adapting it to use the new `QFile`-based stream backend.
7. Ensure the I/O classes correctly interact with the data models from `CORE-01` and asset information from `CORE-02` as needed during serialization/deserialization.
8. Compile `mapcore` and rigorously test loading and saving for OTBM (including `.otgz` and external XMLs) and OTMM formats to ensure data integrity and format compliance.\
""",
    "definition_of_done": """\
The `mapcore` library can fully serialize and deserialize maps in OTBM and OTMM formats.
Key requirements:
- All file operations (reading/writing OTBM, OTMM, auxiliary XMLs) use Qt classes (`QFile`, `QXmlStreamReader/Writer`) instead of C-style FILE* or wxFile.
- Pugixml dependency is removed and replaced with Qt XML for spawn/house XML files.
- Libarchive usage for .otgz is maintained and correctly linked (if retained).
- All wxWidgets dependencies (wxString, wxArrayString, wxFileName, UI dialogs for errors/progress) are removed from these I/O classes. Error reporting is done via member strings/vectors.
- The node-based binary format handling (`NodeFileReadHandle`, `BinaryNode`) is preserved and functional with the new file stream backend.
- The I/O system correctly interacts with data models from CORE-01 and asset databases from CORE-02.
- Map saving and loading for both OTBM (including .otgz and external XMLs) and OTMM formats are verified to be functional and produce byte-identical or logically equivalent files to the original system (where format specifics allow).\
""",
    "boilerplate_coder_ai_prompt": """\
Your task is to port the map file I/O systems (OTBM, OTMM, and underlying `FileHandle` hierarchy) to the `mapcore` library. This builds on `CORE-01` and `CORE-02`. The ported classes should maintain a modular file structure within `mapcore`.
1.  Refactor `FileHandle` and its derivatives (`NodeFileReadHandle`, `NodeFileWriteHandle`, etc.) to use `QFile` instead of `FILE*` for all disk operations.
2.  In `IOMapOTBM.cpp`, replace `pugixml` calls for reading/writing spawn and house XML data with `QXmlStreamReader` and `QXmlStreamWriter`.
3.  For `.otgz` support in `IOMapOTBM.cpp`, continue using `libarchive`. Ensure it can be linked by `mapcore`.
4.  Remove all wxWidgets types (`wxString` to `std::string`, `wxArrayString` to `std::vector<std::string>`, `wxFileName` to `std::filesystem::path` or `QFileInfo`).
5.  Eliminate UI calls (popups, progress bars). Error messages should be stored in `IOMap` member variables.
6.  The core logic of `NodeFileReadHandle`, `BinaryNode` (node start/end markers, escaping) must be preserved over the new `QFile`-based backend.
7.  Ensure the code correctly uses data models from `CORE-01` and asset data from `CORE-02`.
8.  Verify `mapcore` compiles and can successfully load/save OTBM and OTMM maps.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/CORE-03.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Overwritten/Generated {output_file_path}")
