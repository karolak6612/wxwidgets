#ifndef RME_FORWARD_DECLARATIONS_H
#define RME_FORWARD_DECLARATIONS_H

/**
 * @brief Forward declarations for RME Qt6 project
 * 
 * This file contains forward declarations for all major classes in the RME project
 * to help resolve circular dependencies and reduce compilation times.
 */

namespace RME {

// Core namespace forward declarations
namespace core {
    // Basic data structures
    class Position;
    class Item;
    class Tile;
    class BaseMap;
    class Map;
    
    // Asset management
    namespace assets {
        class AssetManager;
        class ItemDatabase;
        class CreatureDatabase;
        class MaterialManager;
        class ClientVersionManager;
        struct ItemData;
        struct CreatureData;
        struct MaterialData;
        struct ClientProfile;
    }
    
    // Brush system
    namespace brush {
        class Brush;
        class BrushStateService;
        class BrushIntegrationManager;
        class BrushManagerService;
        class BrushStateManager;
        class CarpetBrush;
        class CreatureBrush;
        class DoodadBrush;
        class EraserBrush;
        class GroundBrush;
        class HouseBrush;
        class HouseExitBrush;
        class RawBrush;
        class SpawnBrush;
        class TableBrush;
        class WallBrush;
        class WaypointBrush;
    }
    
    // Sprite management
    namespace sprites {
        class SpriteManager;
        class TextureManager;
        struct SpriteData;
    }
    
    // Map components
    namespace map {
        class Floor;
        class MapIterator;
        class QTreeNode;
    }
    
    // Actions and commands
    namespace actions {
        class UndoManager;
        class BaseCommand;
        class ChangeSetCommand;
        class DeleteCommand;
        class PasteCommand;
        class TileChangeCommand;
    }
    
    // Services
    namespace services {
        class ServiceContainer;
        class ApplicationSettingsService;
        class BrushPaletteService;
        class BrushStateService;
        class ClientDataService;
        class EditorStateService;
        class LightCalculatorService;
        class WindowManagerService;
    }
    
    // Settings
    namespace settings {
        class AppSettings;
    }
    
    // Items
    namespace items {
        class ContainerItem;
        class DepotItem;
        class DoorItem;
        class PodiumItem;
        class TeleportItem;
    }
    
    // Creatures
    namespace creatures {
        class Creature;
    }
    
    // Houses
    namespace houses {
        class HouseData;
        class Houses;
    }
    
    // Spawns
    namespace spawns {
        class Spawn;
        class SpawnManager;
    }
    
    // Waypoints
    namespace waypoints {
        class WaypointManager;
    }
    
    // World
    namespace world {
        class TownData;
        class TownManager;
    }
    
    // Lighting
    namespace lighting {
        class LightCalculatorService;
        class LightRenderer;
    }
    
    // Selection
    namespace selection {
        class SelectionManager;
        class SelectionCommand;
    }
    
    // Clipboard
    namespace clipboard {
        class ClipboardManager;
        class ClipboardData;
    }
    
    // I/O
    namespace io {
        class BinaryNode;
        class OtbmMapIO;
        class NodeFileReadHandle;
        class NodeFileWriteHandle;
    }
    
    // Network
    namespace network {
        class AuthenticationManager;
        class ChatManager;
        class ConflictResolver;
        class MapProtocolCodec;
        class NetworkMessage;
    }
    
    // Editor
    namespace editor {
        class EditorControllerInterface;
        class EditorStateService;
    }
}

// Editor logic namespace
namespace editor_logic {
    class EditorController;
    
    namespace commands {
        class BaseCommand;
        class AddCreatureCommand;
        class AddSpawnCommand;
        class AddWaypointCommand;
        class BoundingBoxSelectCommand;
        class BrushStrokeCommand;
        class ClearSelectionCommand;
        class CreateHouseCommand;
        class DeleteCommand;
        class DeleteSelectionCommand;
        class MapWideOperationCommand;
        class ModifyHousePropertiesCommand;
        class MoveWaypointCommand;
        class RecordAddRemoveItemCommand;
        class RecordModifyTileContentsCommand;
        class RecordSetGroundCommand;
        class RecordSetSpawnCommand;
        class RemoveCreatureCommand;
        class RemoveHouseCommand;
        class RemoveSpawnCommand;
        class RemoveWaypointCommand;
        class RenameWaypointCommand;
        class SetHouseExitCommand;
        class SetHouseTileCommand;
    }
}

// UI namespace
namespace ui {
    class MainWindow;
    class MainToolBar;
    class EditorInstanceWidget;
    class DockManager;
    
    namespace dialogs {
        class AboutDialog;
        class AddItemToTilesetDialog;
        class BrushMaterialEditorDialog;
        class CreaturePropertiesDialog;
        class EditHouseDialogQt;
        class EditSpawnDialogQt;
        class ItemFinderDialogQt;
        class ItemPropertiesDialog;
        class LiveConnectionDialog;
        class LiveServerControlPanelQt;
        class LiveServerDialog;
        class MapPropertiesDialog;
        class NewTilesetDialog;
        class PreferencesDialog;
        class ServerHostingDialog;
        class SpawnPropertiesDialog;
        class WelcomeDialog;
    }
    
    namespace widgets {
        class MapView;
        class MapViewWidget;
        class MinimapViewWidget;
        class LiveCollaborationPanel;
        class SpawnSettingsWidget;
    }
    
    namespace palettes {
        class BasePalettePanel;
        class BrushPalettePanel;
        class ItemPalettePanel;
        class CreaturePalettePanel;
        class HousePalettePanel;
        class WaypointPalettePanel;
        class AdvancedSearchWidget;
        class BrushCategoryTab;
        class BrushContextMenu;
        class BrushFilterManager;
        class BrushGridWidget;
        class BrushIconWidget;
        class BrushListWidget;
        class BrushOrganizer;
        class BrushPreviewGenerator;
        class BrushTooltipWidget;
        class HousePaletteTab;
        class RawItemsPaletteTab;
        class TerrainBrushPaletteTab;
        class WaypointPaletteTab;
    }
}

// Network namespace
namespace network {
    class QtLiveClient;
}

} // namespace RME

#endif // RME_FORWARD_DECLARATIONS_H