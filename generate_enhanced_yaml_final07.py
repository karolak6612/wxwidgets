import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Using f-string for the C++ code block, ensuring {{ and }} are used for literal braces
cpp_example_in_boilerplate = """\
if (event->type() == QEvent::FileOpen) {{
    QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent *>(event);
    QString filePath = fileOpenEvent->file();
    // Call your global map loading function or signal MainWindow
    // Example: if (MainWindowInstance) MainWindowInstance->loadMap(filePath);
    return true;
}}
return QApplication::event(event);\
"""

yaml_content = {
    "id": "FINAL-07",
    "section": "Final Polish",
    "title": "Re-implement Drag-and-Drop and IPC",
    "original_input_files": "application.cpp (`MacOpenFiles`), process_com.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/application.cpp",
        "wxwidgets/process_com.h",
        "wxwidgets/process_com.cpp"
    ],
    "dependencies": [
        "UI-01",
        "CORE-03"
    ],
    "current_functionality_summary": """\
The application handles file opening via macOS-specific `MacOpenFiles` event (in `application.cpp`). It also implements a single-instance check using `wxSingleInstanceChecker` and a custom IPC mechanism (`RMEProcessClient`/`Server` based on `wxConnection` defined in `process_com.h/cpp`) to forward filenames from subsequent instances to the primary one. Drag-and-drop directly onto the main window is not explicitly covered by these input files but is a target feature.\
""",
    "qt6_migration_steps": """\
1. Implement drag-and-drop file opening on the `MainWindow` (from `UI-01`):
   - Call `setAcceptDrops(true)` on the `MainWindow` instance.
   - Override `MainWindow::dragEnterEvent(QDragEnterEvent *event)`: Check `event->mimeData()->hasUrls()`. If the URLs represent local files with recognized map extensions (e.g., `.otbm`), call `event->acceptProposedAction()`.
   - Override `MainWindow::dropEvent(QDropEvent *event)`: Iterate through `event->mimeData()->urls()`. For each URL that is a local file (`url.isLocalFile()`), get its path using `url.toLocalFile()` and pass it to the application's map loading logic.
2. Implement single-instance control and file argument forwarding using `QLocalServer` and `QLocalSocket`:
   - In `main.cpp` (or the application's initialization sequence):
     - Create a `QLocalSocket`. Attempt to connect to a unique, named `QLocalServer` (e.g., "RME-Qt-Instance-Server").
     - If the connection succeeds (indicating another instance is running): Send any command-line arguments (especially filenames) to the server instance through the socket. After sending, the new instance should terminate.
     - If the connection fails (indicating this is the first instance or the server setup failed): Create a `QLocalServer` instance and make it listen on the predefined unique name. Connect its `newConnection()` signal to a slot. This slot should:
       - Accept the incoming `QLocalSocket*` using `server->nextPendingConnection()`.
       - Connect this client socket's `readyRead()` signal to another slot to read the data (e.g., a filename) sent by a new instance.
       - Process the received data (e.g., open the map file).
       - Ensure the main application window is brought to the foreground (e.g., `mainWindow->raise()`, `mainWindow->activateWindow()`).
3. Handle macOS file association opening (this will replace the functionality of `MacOpenFiles`):
   - Create a custom application class by subclassing `QApplication` (e.g., `MyApplication : QApplication`).
   - In this custom class, override the `bool event(QEvent *event)` method.
   - Inside the overridden `event` method, check if `event->type() == QEvent::FileOpen`. If true, cast the event to `QFileOpenEvent*`, retrieve the file path using `fileOpenEvent->file()`, and then trigger your application's map loading logic with this path. Return `true` to indicate the event was handled.
   - If the event is not a `QFileOpenEvent`, call the base class implementation: `return QApplication::event(event);`.
   - Instantiate this custom application class in `main()`.
4. Remove the old `wxwidgets/process_com.h`, `wxwidgets/process_com.cpp` files and the `wxSingleInstanceChecker` / `RMEProcessClient` / `RMEProcessServer` related logic from `application.cpp`.\
""",
    "definition_of_done": """\
The application correctly supports opening .otbm files by dragging them onto the main window and ensures single-instance operation with file forwarding to the primary instance.
Key requirements:
- Dragging one or more .otbm files onto the main application window successfully triggers the map opening process for each valid file.
- If the application is already running, launching it again (e.g., by trying to open an associated file or via the command line with a file argument) results in the file argument(s) being passed to the existing instance. The existing instance opens the file(s) and brings itself to the foreground, while the newly launched instance terminates.
- On macOS, double-clicking an .otbm file or dropping it on the Dock icon correctly opens it in the application (or forwards to the running instance if already open).
- The old wxWidgets-based IPC mechanisms (`wxSingleInstanceChecker`, `RMEProcessClient`, `RMEProcessServer`) and the `MacOpenFiles` method are removed and replaced with Qt-native solutions (`QLocalServer`/`QLocalSocket`, `QFileOpenEvent`).\
""",
    "boilerplate_coder_ai_prompt": f"""\
Implement drag-and-drop file opening onto the MainWindow and single-instance IPC using Qt mechanisms.
1.  **Drag-and-Drop for `MainWindow`**:
    - In `MainWindow`'s constructor, call `setAcceptDrops(true)`.
    - Override `dragEnterEvent(QDragEnterEvent *event)`: Check `event->mimeData()->hasUrls()`. If URLs are local files ending with `.otbm` (or other supported map extensions), call `event->acceptProposedAction()`.
    - Override `dropEvent(QDropEvent *event)`: Iterate `event->mimeData()->urls()`. For each local file URL, get the path using `url.toLocalFile()` and call your application's map loading function.
2.  **Single Instance IPC with `QLocalServer`/`QLocalSocket`**:
    - In `main()`:
        - Create a `QLocalSocket`. Attempt `connectToServer("RME-Qt-Instance-Lock", QIODevice::WriteOnly)`.
        - If connected: Write command-line arguments (filenames) to the socket (e.g., as a newline-separated string). Wait for a brief moment for data to be written (e.g., `socket.waitForBytesWritten(100)`). Then, `return 0;` (quit the new instance).
        - If connection failed: Create `QLocalServer`. Call `server.listen("RME-Qt-Instance-Lock")`. Connect its `newConnection()` signal to a slot in your main application class (or a dedicated IPC handler class).
    - In the `newConnection()` slot:
        - Get the client socket: `QLocalSocket *clientConnection = server->nextPendingConnection();`
        - Connect `clientConnection->readyRead()` to a slot for reading data.
        - Connect `clientConnection->disconnected()` to `clientConnection->deleteLater()`.
    - In the `readyRead()` slot for the client socket:
        - Read data (e.g., `QTextStream in(clientConnection); QString filePath = in.readLine();`).
        - Process the `filePath` (load map).
        - Bring `MainWindow` to the front (e.g., `mainWindow->raise(); mainWindow->activateWindow();`).
3.  **macOS File Open Events (for file associations)**:
    - Create `MyApplication : QApplication` subclass.
    - Override `bool event(QEvent *event) override;`.
    - In `MyApplication::event()`:
      ```cpp
{cpp_example_in_boilerplate}
      ```
    - In `main()`, instantiate `MyApplication a(argc, argv);` instead of `QApplication`.
4.  Remove old IPC code (`process_com.h/cpp`, `wxSingleInstanceChecker` from `application.cpp`) and the `MacOpenFiles` method.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/FINAL-07.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
