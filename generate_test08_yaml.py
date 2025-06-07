import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Using f-string for the C++ code block, ensuring {{ and }} are used for literal braces
# No C++ code block in this specific YAML's boilerplate_coder_ai_prompt, so no special f-string needed here.

yaml_data = {
    "wbs_item_id": "TEST-08",
    "name": "Integration Test Live Server/Client",
    "description": "Create integration tests for the live editing server and client. This involves starting a server instance, connecting one or more clients, performing map editing actions, and verifying that changes are correctly propagated and applied across all instances.",
    "dependencies": [
        "NET-01",  # Defines Network Protocol
        "NET-02",  # Implements Server Logic
        "NET-03",  # Implements Client Logic
        "TEST-01", # Core data structures (Map, Tile) must be stable
        "TEST-06", # Network protocol message serialization/deserialization should be stable
    ],
    "input_files": [], # No direct wxWidgets C++ files are ported for this task; tests are new.
    "analyzed_input_files": [], # N/A
    "documentation_references": [
        "Qt Test Framework: https://doc.qt.io/qt-6/qttest-index.html",
        "Qt Network Module (QTcpServer, QTcpSocket): https://doc.qt.io/qt-6/qtnetwork-index.html",
        "QSignalSpy: https://doc.qt.io/qt-6/qtest.html#QSignalSpy",
        "Asynchronous Testing in Qt: https://doc.qt.io/qt-6/qttest-overview.html#testing-asynchronous-operations"
    ],
    "current_functionality_summary": """\
This task is for creating new integration tests for the Qt6 live editing (multi-user) functionality. The system (developed under NET-01, NET-02, NET-03) allows multiple clients to connect to a server, edit a map collaboratively, and see each other's changes in real-time. These tests will simulate this environment to validate data synchronization and core interactions.\
""",
    "definition_of_done": [
        "Integration tests using the Qt Test framework are implemented to cover key live server/client interactions.",
        "Tests can programmatically start a local `LiveServer` instance on a specific test port and connect one or more `LiveClient` instances to it.",
        "Client Connection & Disconnection: Tests verify that clients can successfully connect to and disconnect from the server, and that the server correctly manages its list of connected clients.",
        "Map Edit Propagation: Tests simulate one client performing various map editing actions (e.g., changing a ground tile, placing/modifying an item, creating a waypoint, defining a spawn area with creatures). Assertions confirm:",
        "  - The server correctly receives and processes these actions, updating its master map state.",
        "  - The server correctly broadcasts these changes to all other connected clients.",
        "  - Other clients correctly receive these changes and update their local map states to match the server's state.",
        "Data Consistency: After actions and propagation, tests verify that the relevant parts of the map data are identical across the server and all client instances.",
        "Initial Map Synchronization: Tests verify that a newly connecting client receives the complete and current map state from the server.",
        "Basic User Presence: If implemented, tests for broadcasting/receiving user cursor positions or active selections are included.",
        "Chat Message Propagation: Tests ensure chat messages sent by one client are relayed through the server to other clients.",
        "Tests correctly handle the asynchronous nature of network communication, using mechanisms like `QSignalSpy` or event loops with timeouts to wait for operations to complete before making assertions.",
        "All created integration tests pass successfully.",
        "Tests are integrated into the CMake build system for automated execution."
    ],
    "boilerplate_coder_ai_prompt": """\
Your task is to create integration tests for the live editing server and client system of the Qt6 Remere's Map Editor. This involves testing the network interactions, data synchronization, and basic collaborative editing features. Use the Qt Test framework. The server, client, and protocol logic are primarily from `NET-01`, `NET-02`, and `NET-03`.

**Core Test Setup (`initTestCase` or per-test setup):**
1.  **Server Initialization:** Instantiate your `LiveServer` class. Start it listening on `QHostAddress::LocalHost` and a dedicated test port.
2.  **Client Initialization:** Instantiate at least two `LiveClient` objects (e.g., `clientA`, `clientB`). Each client should have its own `Map` instance.
3.  **Connections:**
    - Connect `clientA` to the server. Wait for and verify successful connection (e.g., using `QSignalSpy` on server's `clientConnected` signal and client's `connectedToServer` signal).
    - Connect `clientB` to the server. Verify successful connection.
4.  **Initial State:** Ensure server and all clients start with a known, consistent (possibly empty or simple predefined) map state. If new clients are expected to receive the full map on join, test this explicitly.

**Test Scenarios:**

1.  **Basic Client Connection/Disconnection:**
    - Test server acknowledging new connections.
    - Test server handling client disconnections (e.g., client calls `disconnectFromServer()`).
    - Verify server updates its internal list of connected clients.

2.  **Map Edit Propagation (Client A -> Server -> Client B):**
    - **Action:** `clientA` performs a map modification (e.g., `clientA->getMap()->setGroundTile(Position(10,10,7), newGroundItem); clientA->sendTileUpdate(Position(10,10,7));`).
    - **Verification:**
        - Use `QSignalSpy` or `QTest::qWait` to allow time for network transmission and processing.
        - **Server:** Assert that the server's master `Map` object reflects the change made by `clientA`.
        - **Client B:** Assert that `clientB`'s local `Map` object has been updated with the change originating from `clientA`.
        - Perform deep comparisons of the affected `Tile` objects on all instances.
    - **Cover Diverse Actions:**
        - Changing ground tiles.
        - Adding/removing items from tiles (test item attributes, stacking).
        - Placing/removing creatures.
        - Creating/modifying/deleting waypoints.
        - Defining/modifying/clearing spawn areas (radius, creature list, interval on tiles).
        - Applying brushes (if actions are brush-based, ensure the full set of resulting tile changes are propagated).

3.  **Initial Map Synchronization for Late-Joining Client:**
    - `clientA` connects and makes several map changes, which are processed by the server.
    - Then, `clientC` connects to the server.
    - **Assertion:** Verify that `clientC` receives the *complete current state* of the map from the server, including all changes made by `clientA` before `clientC` joined.

4.  **Chat Message System:**
    - `clientA` sends a chat message.
    - **Assertions:**
        - Server receives the message.
        - Server broadcasts the message to `clientB`.
        - `clientB` receives the correct chat message from `clientA`.

5.  **User Presence (If Implemented):**
    - If the system supports broadcasting cursor positions or selections:
        - `clientA` moves its cursor or makes a selection.
        - **Assertion:** `clientB` receives an update indicating `clientA`'s cursor/selection.

**Important Considerations for Asynchronous Testing:**
- Network operations are inherently asynchronous.
- Use `QSignalSpy` to wait for specific signals (e.g., `server::mapUpdated`, `client::tileChangedFromServer`) before making assertions.
- Alternatively, use `QEventLoop` with `QTimer::singleShot` or `QTest::qWait()` to introduce controlled delays, allowing network events to process. Be cautious with fixed delays, as they can make tests brittle. `QSignalSpy` is generally preferred.

**Cleanup (`cleanupTestCase`):**
- Ensure all clients are disconnected.
- Stop the server.

Integrate these tests into your CMake build system.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/TEST-08.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

del yaml_data
