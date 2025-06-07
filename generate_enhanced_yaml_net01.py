import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Using f-string for the C++ code block, ensuring {{ and }} are used for literal braces
cpp_struct_example_in_boilerplate = """\
struct NetworkColor {{ uint8_t r, g, b, a; }};\
"""

yaml_content = {
    "id": "NET-01",
    "section": "Core Migration Tasks",
    "title": "Isolate and Port Network Protocol",
    "original_input_files": "live_packets.h, net_connection.h/cpp, live_socket.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/live_packets.h",
        "wxwidgets/net_connection.h",
        "wxwidgets/net_connection.cpp",
        "wxwidgets/live_socket.h",
        "wxwidgets/live_socket.cpp"
    ],
    "dependencies": [
        "CORE-01", # For Position, Tile, Item etc.
    ],
    "current_functionality_summary": """\
- `live_packets.h`: Defines enums for network packet types.
- `net_connection.h/cpp`: Contains `NetworkMessage` for building/parsing byte streams (with special handling for strings and Position) and `NetworkConnection` which manages a Boost.Asio `io_context` for asynchronous network operations.
- `live_socket.h/cpp`: Defines `LiveSocket`, an abstract base class containing core logic for serializing/deserializing map data (Tiles, Items, Nodes, Floors, Cursors) into/from `NetworkMessage` buffers. This serialization uses `MemoryNodeFileReadHandle/WriteHandle` and OTBM-style attribute definitions. It is intended to be independent of the actual socket send/receive mechanism.\
""",
    "qt6_migration_steps": """\
1. Create a new subdirectory `network` within the `mapcore` library's source tree (e.g., `mapcore/network/`).
2. Move the `live_packets.h` file (which defines `LivePacketType` enums) into `mapcore/network/`.
3. Port the `NetworkMessage` class from `net_connection.h/cpp` into new files (e.g., `mapcore/network/network_message.h` and `mapcore/network/network_message.cpp`).
   - Ensure `NetworkMessage` uses standard C++ types like `std::vector<uint8_t>` for its internal buffer and `std::string`.
   - Verify that its template methods `read<T>()` and `write<T>()`, along with specializations for `std::string` and `Position`, are correctly implemented and free of wxWidgets or Boost.Asio dependencies.
4. Extract the map data serialization and deserialization logic currently present in `LiveSocket`'s helper methods (e.g., `sendTile`, `readTile`, `sendNode`, `receiveNode`, `sendFloor`, `receiveFloor`, `readCursor`, `writeCursor`).
   - Place this logic into one or more new classes or sets of free functions within the `mapcore/network/` module (e.g., in `MapProtocol.h/.cpp` or `NetworkSerializer.h/.cpp`).
   - These new components must operate on `mapcore` data types (Tile, Item, QTreeNode, Floor, Position, etc., from `CORE-01`) and use the ported `NetworkMessage` class for buffer manipulation.
   - They will utilize `MemoryNodeFileReadHandle` and `MemoryNodeFileWriteHandle` (from `CORE-03`, ensuring these memory handlers operate on `std::vector<uint8_t>` or `QByteArray` for in-memory streams, not disk files).
   - The `LiveCursor` struct should be part of this module. If it uses `wxColor`, replace it with a simple color struct (e.g., `{uint8_t r,g,b,a;}`) or `QColor` if Qt basic types are permissible in `mapcore`'s network module.
5. The `NetworkConnection` class (which manages Boost.Asio `io_context` and `std::thread`) is explicitly *not* to be ported as part of this task. The `network` module in `mapcore` must be independent of specific network I/O frameworks like Boost.Asio or Qt Network at this stage.
6. Ensure all UI-related logging (e.g., `LiveLogTab* log`) is removed from the ported protocol code.
7. The `mapcore/network` module must compile cleanly and have no dependencies on Boost.Asio or wxWidgets.\
""",
    "definition_of_done": """\
A `network` module (subdirectory) is created within the `mapcore` library, containing the definitions and logic for serializing and deserializing all network messages and map data structures required for the live editing feature.
Key requirements:
- The `mapcore/network/` module includes the `LivePacketType` enum.
- A UI-agnostic `NetworkMessage` class is present in `mapcore/network/` for serializing/deserializing primitive data types, strings, and `Position` objects into/from byte buffers.
- It contains classes/functions responsible for serializing and deserializing `mapcore`'s `Tile`, `Item`, `QTreeNode`, `Floor`, and `LiveCursor` data structures into/from `NetworkMessage` buffers, replicating the logic from `LiveSocket`'s helper methods (using memory-based node handlers).
- This `network` module has no dependencies on Boost.Asio, wxWidgets, or any specific network I/O framework.
- All components compile cleanly within `mapcore`.\
""",
    "boilerplate_coder_ai_prompt": f"""\
Refactor network protocol definitions and serialization logic into a new `network` module within the `mapcore` static library. This module must be independent of Boost.Asio and wxWidgets. (Depends on `CORE-01` for data types and potentially `CORE-03` for memory-based node handlers).
1.  Create a directory `mapcore/network/`.
2.  Move `live_packets.h` (defining `LivePacketType`) into `mapcore/network/`.
3.  Port the `NetworkMessage` class from `wxwidgets/net_connection.h/cpp` to `mapcore/network/network_message.h` and `mapcore/network/network_message.cpp`.
    - Ensure its internal buffer is `std::vector<uint8_t>`.
    - Ensure methods use standard C++ types (`std::string`, `uint16_t`, etc.).
    - Verify serialization for `std::string` (length-prefixed) and `Position`.
4.  Create new files (e.g., `mapcore/network/MapProtocol.h`, `mapcore/network/MapProtocol.cpp`).
    - Implement functions/classes to serialize and deserialize map data structures. This involves porting the logic from `LiveSocket::sendTile`, `LiveSocket::readTile`, `sendNode`, `receiveNode`, `sendFloor`, `receiveFloor`, `LiveSocket::readCursor`, `LiveSocket::writeCursor`.
    - These functions will take `mapcore` objects (e.g., `Tile*`, `QTreeNode*`) and `NetworkMessage&` as parameters.
    - They should use `mapcore`'s `MemoryNodeFileWriteHandle` and `MemoryNodeFileReadHandle` (from `CORE-03`, ensure these can operate on in-memory byte vectors/arrays) for the node-based serialization within the `NetworkMessage`.
    - Define `LiveCursor` struct in `mapcore/network/`. Replace `wxColor` with a simple color struct (e.g., `{cpp_struct_example_in_boilerplate}`) or `QColor` if permitted for basic data types in `mapcore`.
5.  **Do NOT port `NetworkConnection` (the Boost.Asio part) in this task.**
6.  Remove any logging calls related to `LiveLogTab` or other UI elements.
7.  Ensure all files in `mapcore/network/` compile as part of the `mapcore` library without Boost.Asio or wxWidgets dependencies.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/NET-01.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
