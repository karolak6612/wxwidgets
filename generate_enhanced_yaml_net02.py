import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "NET-02",
    "section": "Core Migration Tasks",
    "title": "Port Live Server Logic",
    "original_input_files": "live_server.h/cpp, live_peer.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/live_server.h",
        "wxwidgets/live_server.cpp",
        "wxwidgets/live_peer.h",
        "wxwidgets/live_peer.cpp"
    ],
    "dependencies": [
        "NET-01",  # For NetworkMessage and protocol definitions
        "CORE-05" # For ActionQueue and map data models
    ],
    "current_functionality_summary": """\
- `LiveServer` (in `live_server.h/cpp`): Manages multiple client connections (`LivePeer` objects) using Boost.Asio for TCP/IP listening (`tcp::acceptor`) and socket management. It broadcasts map changes, chat messages, and cursor updates to connected clients. It holds a reference to the main `Editor` object to access map data.
- `LivePeer` (in `live_peer.h/cpp`): Represents a server-side connection to a single client, managing its `tcp::socket`. It handles asynchronous reading of data, parsing `NetworkMessage`s based on `LivePacketType`, applying received changes to the map (often via the `ActionQueue`), and sending data back to its client.\
""",
    "qt6_migration_steps": """\
1. Create a new, separate Qt Console Application project (e.g., named `RMELiveServer` or similar). This application will be headless (no GUI).
2. The `RMELiveServer` project must be configured in CMake to link against the `mapcore` static library (which provides components from `NET-01`, `CORE-01`, `CORE-03`, `CORE-05`).
3. Create a `QtLiveServer` class (e.g., inheriting `QObject`) within the new server application:
   a. This class will contain a `QTcpServer* m_tcpServer` member.
   b. Implement a method to start the server, e.g., `bool listen(quint16 port);`, which calls `m_tcpServer->listen(QHostAddress::Any, port)`.
   c. Connect the `QTcpServer::newConnection()` signal to a slot in `QtLiveServer`, e.g., `handleNewConnection()`.
   d. `QtLiveServer` will own the authoritative `Map` object (loaded at server startup using `mapcore`'s I/O) and the main `ActionQueue` for this map.
4. Create a `QtLivePeer` class (e.g., inheriting `QObject`) within the new server application:
   a. It will take a `QTcpSocket*` (representing the connection to one client) in its constructor, along with references to the `QtLiveServer`'s `Map` and `ActionQueue`.
   b. Store the `QTcpSocket*` as a member.
   c. Connect the socket's `readyRead()` signal to a slot like `QtLivePeer::onReadyRead()` for processing incoming data.
   d. Connect the socket's `disconnected()` signal to a slot like `QtLivePeer::onDisconnected()` which should notify `QtLiveServer` to remove and delete this peer.
   e. The `onReadyRead()` slot must implement logic to read data from the `QTcpSocket`, handle message framing (e.g., assuming a 4-byte length prefix for packets), and populate `NetworkMessage` objects (from `NET-01`).
   f. Port the message parsing logic from the original `LivePeer::parseLoginPacket()` and `LivePeer::parseEditorPacket()` (and their sub-handlers like `parseHello`, `parseNodeRequest`, `parseReceiveChanges`) to methods within `QtLivePeer`. These methods will use the `mapcore::network` components to deserialize data from `NetworkMessage`.
   g. Implement a `QtLivePeer::send(NetworkMessage& msg)` method that writes the `NetworkMessage` content (including its 4-byte size prefix) to its `QTcpSocket` and calls `flush()` or `waitForBytesWritten()`.
5. In `QtLiveServer::handleNewConnection()`:
   a. Call `QTcpSocket* clientSocket = m_tcpServer->nextPendingConnection();`.
   b. Create `new QtLivePeer(this, clientSocket, &m_mapInstance, &m_actionQueueInstance)`.
   c. Store the new `QtLivePeer` in a list (e.g., `QList<QtLivePeer*> m_peers`).
6. Port the broadcasting logic from the original `LiveServer` (e.g., `broadcastNodes`, `broadcastChat`, `broadcastCursor`) to `QtLiveServer`. These methods will iterate over the `m_peers` list and call the `send()` method of each `QtLivePeer`.
7. In the server application's `main()` function:
   a. Create a `QCoreApplication` instance.
   b. Instantiate the `QtLiveServer`.
   c. Load the desired map file into the `QtLiveServer`'s `Map` object using `mapcore`'s I/O functionalities.
   d. Call the `QtLiveServer`'s method to start listening on a configured port.
   e. Call `app.exec()` to start the Qt event loop.\
""",
    "definition_of_done": """\
A standalone, headless server application (`RMELiveServer`) is created using Qt Console, successfully replacing the Boost.Asio based server logic with `QTcpServer` and `QTcpSocket`.
Key requirements:
- The server application uses `QCoreApplication` and does not have a GUI.
- It contains `QtLiveServer` and `QtLivePeer` classes (or equivalents) utilizing `QTcpServer` and `QTcpSocket` for all network communications.
- The server can accept and manage connections from multiple clients (`QtLivePeer` instances).
- It correctly uses the `NetworkMessage` and protocol serialization/deserialization logic from `mapcore`'s `network` module (task `NET-01`).
- The server accurately parses messages from clients, including login requests, map change data, cursor updates, and chat messages.
- It can broadcast messages (map updates, chat messages, cursor positions) to all connected clients.
- The server loads and holds the authoritative version of the game map using `mapcore` components.
- Client-initiated map changes are processed through `mapcore`'s `ActionQueue` and subsequently broadcast to other clients.
- The server application compiles cleanly and runs as a headless console application.\
""",
    "boilerplate_coder_ai_prompt": """\
Create a new Qt Console Application project for the `RMELiveServer`. This server will be headless and link against the `mapcore` library.
1.  **`QtLiveServer` class (inherits `QObject`):**
    -   Members: `QTcpServer* m_tcpServer;`, `QList<QtLivePeer*> m_peers;`, `Map m_map;` (from `mapcore`), `ActionQueue m_actionQueue;` (from `mapcore`, initialized with `m_map`).
    -   `void startServer(quint16 port)`: Initializes and starts `m_tcpServer->listen()`. Connects `newConnection()` signal.
    -   Slot `void onNewConnection()`: Accepts pending connection, creates `QtLivePeer`, adds to `m_peers`, connects peer's disconnect signal.
    -   Slot `void onPeerDisconnected(QtLivePeer* peer)`: Removes peer from `m_peers`, schedules for deletion.
    -   Broadcast methods (e.g., `broadcastMessageToPeers(const NetworkMessage& msg, QtLivePeer* excludePeer = nullptr)`).
2.  **`QtLivePeer` class (inherits `QObject`):**
    -   Members: `QTcpSocket* m_socket;`, `QtLiveServer* m_server;` (parent), `Map* m_mapRef;`, `ActionQueue* m_actionQueueRef;`, `NetworkMessage m_currentMessageBuffer;` (for assembling packets).
    -   Constructor: Takes `QTcpSocket*`, `QtLiveServer*`, `Map*`, `ActionQueue*`. Connects `m_socket->readyRead()` to `this->onReadyRead()` and `m_socket->disconnected()` to `this->onDisconnected()`.
    -   Slot `void onReadyRead()`:
        -   Appends `m_socket->readAll()` to an internal receive buffer.
        -   Loop while the buffer contains enough data for a packet header (4 bytes):
            -   Peek/read the packet size.
            -   If buffer contains full packet (header + size):
                -   Extract the full packet into a `NetworkMessage` (from `NET-01`).
                -   Call `parsePacket(NetworkMessage& msg);` (which internally calls `parseLoginPacket` or `parseEditorPacket` based on connection state).
                -   Remove processed packet from internal buffer.
            -   Else (incomplete packet), break loop and wait for more data.
    -   Slot `void onDisconnected()`: Emits a signal to `QtLiveServer` to remove this peer.
    -   `void sendPacket(NetworkMessage& msg)`: Prepends 4-byte size, then `m_socket->write(msg.buffer); m_socket->flush();`.
    -   Port parsing logic from original `LivePeer` (e.g., `parseHello`, `parseNodeRequest`, `parseReceiveChanges`) to handle `NetworkMessage` objects. Changes to the map should be done via `m_actionQueueRef`. Broadcasts are done via `m_server`.
3.  **`main.cpp` for Server:**
    -   `QCoreApplication app(argc, argv);`
    -   Set `QCoreApplication::setOrganizationName` and `QCoreApplication::setApplicationName`.
    -   Instantiate `QtLiveServer serverInstance;`.
    -   Load map into `serverInstance.m_map` (e.g., from command line argument using `mapcore`'s `IOMapOTBM`). Handle errors.
    -   `serverInstance.startServer(port);`
    -   `return app.exec();`
4.  Use `NetworkMessage` and map data serialization/deserialization from `mapcore`'s `network` module (`NET-01`).
5.  Ensure proper memory management for `QtLivePeer` objects.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/NET-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
