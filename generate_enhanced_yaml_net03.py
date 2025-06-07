import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "NET-03",
    "section": "Core Migration Tasks",
    "title": "Port Live Client & UI Integration",
    "original_input_files": "live_client.h/cpp, live_tab.h/cpp, live_action.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/live_client.h",
        "wxwidgets/live_client.cpp",
        "wxwidgets/live_tab.h",
        "wxwidgets/live_tab.cpp",
        "wxwidgets/live_action.h",
        "wxwidgets/live_action.cpp"
    ],
    "dependencies": [
        "NET-01",  # For NetworkMessage and protocol definitions from mapcore
        "UI-01",   # For MainWindow and basic UI structure
        "CORE-05" # For ActionQueue, NetworkedActionQueue
    ],
    "current_functionality_summary": """\
- `LiveClient` (`live_client.h/cpp`): Manages the client-side connection to a LiveServer using Boost.Asio. It handles sending local changes (from `NetworkedActionQueue`) and receiving server updates (map data, cursors, chat), applying these to a dedicated `Editor` instance.
- `LiveLogTab` (`live_tab.h/cpp`): A wxPanel providing UI for live session logs, chat, and a list of connected users.
- `NetworkedActionQueue` (`live_action.h/cpp`): Subclasses `ActionQueue` to ensure local undoable actions are also sent to the server via `LiveClient::sendChanges`.\
""",
    "qt6_migration_steps": """\
1. Create a `QtLiveClient` class (e.g., inheriting `QObject`) in the main application's codebase (not `mapcore`).
   a. It will manage a `QTcpSocket` member for all network communication with the `QtLiveServer` (from `NET-02`).
   b. Implement `connectToHost(const QString& address, quint16 port)`: This method will initialize and call `m_socket->connectToHost()`.
   c. Connect `QTcpSocket` signals: `connected()` (to trigger `sendHello()`), `disconnected()`, `errorOccurred(QAbstractSocket::SocketError)`, and `readyRead()` (to a slot for processing incoming data).
   d. Port the `sendHello()`, `sendChanges(DirtyList& changes)`, `sendNodeRequests()`, `sendChat(const QString& message)`, and `updateCursor(const Position& pos)` methods from the original `LiveClient`. These will now construct `NetworkMessage` objects (from `mapcore::network`) and use `m_socket->write()` and potentially `m_socket->flush()` or `m_socket->waitForBytesWritten()`.
   e. Implement the `readyRead()` slot: This slot will read all available data from `m_socket->readAll()` into a buffer. It must handle message framing (e.g., assuming a 4-byte length prefix per packet from `NetworkMessage`'s design) to extract complete `NetworkMessage`s.
   f. Port the packet parsing logic from the original `LiveClient::parsePacket()` and its sub-handlers (e.g., `parseHello`, `parseKick`, `parseNode`, `parseCursorUpdate`, `parseServerTalk`). These methods will operate on the received `NetworkMessage` objects, using `mapcore::network` components for deserialization. Server updates like received map node data (`parseNode`) should create `NetworkedAction` objects and add them to the local `ActionQueue` of the live editor session.
2. Port the `LiveLogTab` class to a Qt-based `QtLiveLogTab` (e.g., a `QWidget` designed to be docked or tabbed in the `MainWindow` from `UI-01`).
   a. Replicate its UI using Qt Widgets: `QTabWidget` (for "Debug" and "Chat" tabs), `QTextEdit` (for displaying logs and chat messages), `QLineEdit` (for chat input), and `QTableWidget` (for the user list).
   b. The `Message()` and `Chat()` methods will append formatted text to the respective `QTextEdit`s. Since `QTcpSocket` signals operate in the main Qt event loop, direct UI updates from these methods are generally safe.
   c. Connect the chat input `QLineEdit::returnPressed()` signal to a slot that calls `QtLiveClient::sendChat()`.
   d. `UpdateClientList()` will populate the `QTableWidget` based on data from `QtLiveClient` (e.g., a list of connected user names and their cursor colors).
3. Adapt `NetworkedActionQueue` (ported in `CORE-05`):
   a. Its `broadcast(DirtyList&)` method (or a similarly named method responsible for propagating local changes) should now call a method on the active `QtLiveClient` instance, e.g., `qtLiveClientInstance->sendChanges(dirtyList)`.
4. Integrate into the main application (`MainWindow` / `EditorController`):
   a. Provide UI elements (e.g., a menu action "Join Live Session") that allow the user to input server details and initiate a connection. This will create and manage the `QtLiveClient` instance.
   b. When a live client session is successfully established:
      i.  A new `Editor` instance should be created specifically for this session.
      ii. This `Editor` must use an instance of the (ported) `NetworkedActionQueue`, configured to communicate with the active `QtLiveClient`.
      iii. The `QtLiveLogTab` should be created and displayed within the `MainWindow`'s UI.
5. Ensure all UI updates originating from network events (e.g., new chat message, cursor update) are handled correctly in the main Qt event loop. Qt's signal/slot mechanism for `QTcpSocket` typically ensures this.
6. The original wxWidgets files (`live_client.*`, `live_tab.*`, `live_action.*`) should be removed from the build and their functionality fully replaced by the new Qt-based components.\
""",
    "definition_of_done": """\
The RME Qt application can successfully connect to the `QtLiveServer` (from `NET-02`) as a client, enabling collaborative live editing.
Key requirements:
- A `QtLiveClient` class manages the `QTcpSocket` connection and all client-server communication using the protocol defined in `NET-01`.
- The client can successfully send login information, local map changes (via `NetworkedActionQueue`), cursor updates, and chat messages to the server.
- The client can receive and correctly process server messages: map data updates (applying them to its local map via its `NetworkedActionQueue`), other clients' cursor positions (updating its local cache), and chat messages.
- A `QtLiveLogTab` (or equivalent UI component) is integrated into the `MainWindow` to display session logs, chat history, and a list of connected users with their assigned colors.
- Local editing actions performed during a live client session use the `NetworkedActionQueue` to ensure changes are correctly propagated to the server.
- All client-side network communication uses Qt Network classes (`QTcpSocket`), replacing any previous Boost.Asio implementation.
- UI updates resulting from network events are handled safely within the main Qt event loop.\
""",
    "boilerplate_coder_ai_prompt": """\
Port the live editing client functionality to Qt. This involves creating `QtLiveClient` (using `QTcpSocket`) and `QtLiveLogTab` (using Qt Widgets), and adapting `NetworkedActionQueue`. (Depends on `NET-01` for protocol, `UI-01` for MainWindow, `CORE-05` for Action system).

1.  **`QtLiveClient` class (inherits `QObject`):**
    -   Members: `QTcpSocket* m_socket;`, `NetworkMessage m_readBuffer;` (for assembling incoming packets), `Editor* m_liveEditor;` (pointer to the editor instance for this session), `QtLiveLogTab* m_logTab;`.
    -   Method: `connectToServer(const QString& host, quint16 port, const QString& userName, const QString& password)`: Initializes `m_socket`, calls `m_socket->connectToHost()`. Connects `QTcpSocket` signals (`connected`, `disconnected`, `errorOccurred`, `readyRead`) to slots within `QtLiveClient`.
    -   Slot `onConnected()`: Calls an internal `sendHello()` method which constructs and sends the `PACKET_HELLO_FROM_CLIENT` using `NetworkMessage` (from `NET-01`) and `m_socket->write()`.
    -   Slot `onReadyRead()`:
        -   Appends `m_socket->readAll()` to `m_readBuffer.buffer` (or a temporary `QByteArray`).
        -   Implement message framing: Loop while `m_readBuffer` contains enough data for a packet header (e.g., 4-byte size). Read size. If buffer contains full packet (header + size), extract it into a new `NetworkMessage` instance.
        -   Pass the complete `NetworkMessage` to `parsePacket(NetworkMessage& msg)`.
        -   Remove the processed packet from `m_readBuffer`.
    -   Method `sendPacket(NetworkMessage& msg)`: Prepends 4-byte size to `msg.buffer`, then writes `msg.buffer.data()` of total length `msg.size + 4` to `m_socket->write()`. Consider `m_socket->flush()` or `m_socket->waitForBytesWritten()`.
    -   Port the logic from original `LiveClient::parsePacket()` and its sub-handlers (`parseHello` (server response), `parseKick`, `parseNode`, `parseCursorUpdate`, `parseServerTalk`, etc.).
        -   When parsing `PACKET_NODE`, use `mapcore::network`'s `MapDataParser` to deserialize tile data from the `NetworkMessage`. Create `NetworkedAction`s and add them to `m_liveEditor->actionQueue`.
        -   Update `m_logTab` with chat messages and cursor updates.
    -   Port client sending methods: `sendChanges(DirtyList& dirtyList)`, `sendNodeRequests()`, `sendChat(const QString& chatMessage)`, `updateCursor(const Position& pos)` to construct appropriate `NetworkMessage`s and use `sendPacket()`.
2.  **`QtLiveLogTab` class (e.g., `QWidget`):**
    -   UI: `QTabWidget` with "Debug" and "Chat" tabs (`QTextEdit` each). `QLineEdit` for chat input. `QTableWidget` for user list (User ID, Name, Color indicator).
    -   Methods: `void appendLogMessage(const QString&)`, `void appendChatMessage(const QString& speaker, const QString& message)`, `void updateUserList(const QList<LiveUserData>& users)`.
    -   Connect `QLineEdit::returnPressed` to a slot that calls `QtLiveClient::sendChat()`.
    -   Handle color changes in user list (e.g., clicking a color cell to open `QColorDialog`).
3.  **`NetworkedActionQueue` (from `CORE-05`):**
    -   Ensure its `broadcast(DirtyList&)` method (or equivalent) calls `qtLiveClientInstance->sendChanges(dirtyList)`.
4.  **Integration in `MainWindow` / `EditorController`:**
    -   Add UI (e.g., menu action "Join Live Session") to get server address, port, username, password.
    -   On connect: Create `QtLiveClient`. If connection successful, create a new `Editor` instance for the live session, assign it a `NetworkedActionQueue` linked to the `QtLiveClient`. Create and display `QtLiveLogTab`.
5.  Replace all usages of old wxWidgets live client classes.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/NET-03.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
