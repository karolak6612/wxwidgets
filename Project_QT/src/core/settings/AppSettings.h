#ifndef RME_APP_SETTINGS_H
#define RME_APP_SETTINGS_H

#include <QSettings>
#include <QString>
#include <QVariant>
#include <memory> // For std::unique_ptr

namespace RME {
namespace Config {

// Mirrored from original wxWidgets settings.h and settings.cpp for RME
enum Key {
    NONE, // Should not be used for actual settings
    VERSION_ID,

    // Version Group (Original: "Version")
    USE_CUSTOM_DATA_DIRECTORY, // int (bool)
    DATA_DIRECTORY,            // string
    EXTENSIONS_DIRECTORY,      // string
    ASSETS_DATA_DIRS,        // string (Newer, might be for multiple asset locations)
    DEFAULT_CLIENT_VERSION,  // int
    CHECK_SIGNATURES,        // int (bool)

    // Graphics Group (Original: "Graphics")
    TEXTURE_MANAGEMENT,        // int (bool)
    TEXTURE_CLEAN_PULSE,       // int (seconds)
    TEXTURE_CLEAN_THRESHOLD,   // int (MB)
    TEXTURE_LONGEVITY,         // int (seconds)
    HARD_REFRESH_RATE,         // int (ms)
    USE_MEMCACHED_SPRITES,     // int (bool) - Note: Original has IntToSave, implies separate save key
    USE_MEMCACHED_SPRITES_TO_SAVE, // Specific key for saving if different from runtime
    SOFTWARE_CLEAN_THRESHOLD,  // int (MB)
    SOFTWARE_CLEAN_SIZE,       // int (MB)
    ICON_BACKGROUND,           // int (enum or color index)
    SCREENSHOT_DIRECTORY,      // string
    SCREENSHOT_FORMAT,         // string ("png", "jpg", etc.)
    MINIMAP_UPDATE_DELAY,      // int (ms)
    MINIMAP_VIEW_BOX,          // int (bool)
    MINIMAP_EXPORT_DIR,        // string
    TILESET_EXPORT_DIR,        // string
    CURSOR_RED,                // int (0-255)
    CURSOR_GREEN,              // int (0-255)
    CURSOR_BLUE,               // int (0-255)
    CURSOR_ALPHA,              // int (0-255)
    CURSOR_ALT_RED,            // int (0-255)
    CURSOR_ALT_GREEN,          // int (0-255)
    CURSOR_ALT_BLUE,           // int (0-255)
    CURSOR_ALT_ALPHA,          // int (0-255)
    EXPERIMENTAL_FOG,          // int (bool) (Original: section("experimental"))

    // View Group (Original: "View")
    TRANSPARENT_FLOORS,        // int (bool)
    TRANSPARENT_ITEMS,         // int (bool)
    SHOW_INGAME_BOX,           // int (bool)
    SHOW_GRID,                 // int (bool)
    SHOW_EXTRA,                // int (bool)
    SHOW_ALL_FLOORS,           // int (bool)
    SHOW_CREATURES,            // int (bool)
    SHOW_SPAWNS,               // int (bool)
    SHOW_HOUSES,               // int (bool)
    SHOW_SHADE,                // int (bool)
    SHOW_SPECIAL_TILES,        // int (bool)
    SHOW_ZONE_AREAS,           // int (bool)
    HIGHLIGHT_ITEMS,           // int (bool)
    SHOW_ITEMS,                // int (bool)
    SHOW_BLOCKING,             // int (bool)
    SHOW_TOOLTIPS,             // int (bool)
    SHOW_PREVIEW,              // int (bool)
    SHOW_WALL_HOOKS,           // int (bool)
    SHOW_AS_MINIMAP,           // int (bool) - Likely specific to a view context
    SHOW_ONLY_TILEFLAGS,       // int (bool)
    SHOW_ONLY_MODIFIED_TILES,  // int (bool)
    HIDE_ITEMS_WHEN_ZOOMED,    // int (bool)
    DRAW_LOCKED_DOOR,          // int (bool)
    HIGHLIGHT_LOCKED_DOORS,    // int (bool)
    SHOW_LIGHTS,               // int (bool)
    SHOW_LIGHT_STR,            // int (bool) - show light intensity string
    SHOW_TECHNICAL_ITEMS,      // int (bool)
    SHOW_WAYPOINTS,            // int (bool)
    SHOW_TOWNS,                // int (bool)
    ALWAYS_SHOW_ZONES,         // int (bool)
    EXT_HOUSE_SHADER,          // int (bool) - External house shader effect

    // Editor Group (Original: "Editor")
    GROUP_ACTIONS,             // int (bool)
    SCROLL_SPEED,              // float
    ZOOM_SPEED,                // float
    UNDO_SIZE,                 // int
    UNDO_MEM_SIZE,             // int (MB)
    MERGE_PASTE,               // int (bool)
    SELECTION_TYPE,            // int (enum)
    COMPENSATED_SELECT,        // int (bool)
    BORDER_IS_GROUND,          // int (bool)
    BORDERIZE_PASTE,           // int (bool)
    BORDERIZE_DRAG,            // int (bool)
    BORDERIZE_DRAG_THRESHOLD,  // int
    BORDERIZE_PASTE_THRESHOLD, // int
    BORDERIZE_DELETE,          // int (bool)
    ALWAYS_MAKE_BACKUP,        // int (bool)
    USE_AUTOMAGIC,             // int (bool)
    SAME_GROUND_TYPE_BORDER,   // int (bool)
    WALLS_REPEL_BORDERS,       // int (bool)
    LAYER_CARPETS,             // int (bool)
    CUSTOM_BORDER_ENABLED,     // int (bool)
    WORKER_THREADS,            // int
    MERGE_MOVE,                // int (bool)
    SHOW_TILESET_EDITOR,       // int (bool)
    CUSTOM_BORDER_ID,          // int (item id)
    HOUSE_BRUSH_REMOVE_ITEMS,  // int (bool)
    AUTO_ASSIGN_DOORID,        // int (bool)
    ERASER_LEAVE_UNIQUE,       // int (bool)
    DOODAD_BRUSH_ERASE_LIKE,   // int (bool)
    WARN_FOR_DUPLICATE_ID,     // int (bool)
    USE_UPDATER,               // int (bool) (Original: section("") but seems editor related)
    USE_OTBM_4_FOR_ALL_MAPS,   // int (bool)
    USE_OTGZ,                  // int (bool)
    SAVE_WITH_OTB_MAGIC_NUMBER,// int (bool)
    REPLACE_SIZE,              // int
    MAX_SPAWN_RADIUS,          // int
    CURRENT_SPAWN_RADIUS,      // int
    AUTO_CREATE_SPAWN,         // int (bool)
    DEFAULT_SPAWNTIME,         // int (seconds)
    SWITCH_MOUSEBUTTONS,       // int (bool)
    DOUBLECLICK_PROPERTIES,    // int (bool)
    LISTBOX_EATS_ALL_EVENTS,   // int (bool)
    RAW_LIKE_SIMONE,           // int (bool)
    COPY_POSITION_FORMAT,      // int (enum)
    AUTO_SELECT_RAW_ON_RIGHTCLICK, // int (bool)
    AUTO_SAVE_ENABLED,         // int (bool)
    AUTO_SAVE_INTERVAL,        // int (minutes)
    RECENT_FILES,              // string (list, needs special handling if not simple string)
    RECENT_EDITED_MAP_PATH,    // string (Original: section(""))
    RECENT_EDITED_MAP_POSITION,// string (Original: section(""))
    FIND_ITEM_MODE,            // int (Original: section(""))
    JUMP_TO_ITEM_MODE,         // int (Original: section(""))

    // UI Group (Original: "UI")
    USE_LARGE_CONTAINER_ICONS,   // int (bool)
    USE_LARGE_CHOOSE_ITEM_ICONS, // int (bool)
    USE_LARGE_TERRAIN_TOOLBAR,   // int (bool)
    USE_LARGE_DOODAD_SIZEBAR,    // int (bool)
    USE_LARGE_ITEM_SIZEBAR,      // int (bool)
    USE_LARGE_HOUSE_SIZEBAR,     // int (bool)
    USE_LARGE_RAW_SIZEBAR,       // int (bool)
    USE_GUI_SELECTION_SHADOW,    // int (bool)
    PALETTE_COL_COUNT,           // int
    PALETTE_TERRAIN_STYLE,       // string
    PALETTE_DOODAD_STYLE,        // string
    PALETTE_ITEM_STYLE,          // string
    PALETTE_RAW_STYLE,           // string
    PALETTE_COLLECTION_STYLE,    // string
    USE_LARGE_COLLECTION_TOOLBAR,// int (bool)

    // Window Group (Original: "Window")
    PALETTE_LAYOUT,              // string (complex layout string)
    MINIMAP_VISIBLE,             // int (bool)
    MINIMAP_LAYOUT,              // string (complex layout string)
    WINDOW_HEIGHT,               // int
    WINDOW_WIDTH,                // int
    WINDOW_MAXIMIZED,            // int (bool)
    WELCOME_DIALOG,              // int (bool)
    SHOW_TOOLBAR_STANDARD,       // int (bool)
    SHOW_TOOLBAR_BRUSHES,        // int (bool)
    SHOW_TOOLBAR_POSITION,       // int (bool)
    SHOW_TOOLBAR_SIZES,          // int (bool)
    TOOLBAR_STANDARD_LAYOUT,     // string
    TOOLBAR_BRUSHES_LAYOUT,      // string
    TOOLBAR_POSITION_LAYOUT,     // string
    TOOLBAR_SIZES_LAYOUT,        // string

    // Hotkeys Group (Original: "Hotkeys") - Only one example, real hotkeys are dynamic
    NUMERICAL_HOTKEYS,           // string (complex, might need special handling)

    // Network Group (Original: "Network")
    LIVE_HOST,                   // string
    LIVE_PORT,                   // int
    LIVE_PASSWORD,               // string
    LIVE_USERNAME,               // string

    // Interface Group (Original: "Interface" for Dark Mode)
    DARK_MODE,                   // int (bool)
    DARK_MODE_CUSTOM_COLOR,      // int (bool)
    DARK_MODE_RED,               // int
    DARK_MODE_GREEN,             // int
    DARK_MODE_BLUE,              // int

    // HouseCreation Group (Original: "HouseCreation")
    MAX_HOUSE_TILES,             // int
    HOUSE_FLOOR_SCAN,            // int (bool)
    AUTO_DETECT_HOUSE_EXIT,      // int (bool)

    // LOD Group (Original: "LOD")
    TOOLTIP_MAX_ZOOM,                // int
    GROUND_ONLY_ZOOM_THRESHOLD,      // int
    ITEM_DISPLAY_ZOOM_THRESHOLD,     // int
    SPECIAL_FEATURES_ZOOM_THRESHOLD, // int
    ANIMATION_ZOOM_THRESHOLD,        // int
    EFFECTS_ZOOM_THRESHOLD,          // int
    LIGHT_ZOOM_THRESHOLD,            // int
    SHADE_ZOOM_THRESHOLD,            // int
    TOWN_ZONE_ZOOM_THRESHOLD,        // int
    GRID_ZOOM_THRESHOLD,             // int

    // PaletteGrid Group (Original: "PaletteGrid")
    GRID_CHUNK_SIZE,                 // int
    GRID_VISIBLE_ROWS_MARGIN,        // int

    // Misc/Root Level (Original: section(""))
    GOTO_WEBSITE_ON_BOOT,        // int (bool)
    INDIRECTORY_INSTALLATION,    // int (bool) - Note: This might be determined at runtime, not stored.
    AUTOCHECK_FOR_UPDATES,       // int (bool) - Same as USE_UPDATER? (USE_UPDATER is in Editor)
    ONLY_ONE_INSTANCE,           // int (bool)
    LAST_WEBSITES_OPEN_TIME,     // int (timestamp)

    LAST_KEY // Use instead of LAST_KEY_PLACEHOLDER for typical enum end
};

} // namespace Config

class AppSettings {
public:
    AppSettings(QSettings::Format format = QSettings::IniFormat,
                QSettings::Scope scope = QSettings::UserScope,
                const QString& organization = "RMEditor_TestOrg",
                const QString& application = "RME-Qt_TestApp");

    ~AppSettings();

    QVariant getValue(Config::Key key, const QVariant& defaultValue = QVariant()) const;
    void setValue(Config::Key key, const QVariant& value);

    static QString getKeyString(Config::Key key);

    // --- Typed Getters & Setters (Full List) ---
    // Version Group
    bool isUseCustomDataDirectory() const; void setUseCustomDataDirectory(bool val);
    QString getDataDirectory() const; void setDataDirectory(const QString& val);
    QString getExtensionsDirectory() const; void setExtensionsDirectory(const QString& val);
    QString getAssetsDataDirs() const; void setAssetsDataDirs(const QString& val);
    int getDefaultClientVersion() const; void setDefaultClientVersion(int val);
    bool isCheckSignaturesEnabled() const; void setCheckSignaturesEnabled(bool val);

    // Graphics Group
    bool isTextureManagementEnabled() const; void setTextureManagementEnabled(bool val);
    int getTextureCleanPulse() const; void setTextureCleanPulse(int val);
    int getTextureCleanThreshold() const; void setTextureCleanThreshold(int val);
    int getTextureLongevity() const; void setTextureLongevity(int val);
    int getHardRefreshRate() const; void setHardRefreshRate(int val);
    bool useMemcachedSprites() const; void setUseMemcachedSprites(bool val); // For USE_MEMCACHED_SPRITES
    bool useMemcachedSpritesToSave() const; void setUseMemcachedSpritesToSave(bool val); // For USE_MEMCACHED_SPRITES_TO_SAVE
    int getSoftwareCleanThreshold() const; void setSoftwareCleanThreshold(int val);
    int getSoftwareCleanSize() const; void setSoftwareCleanSize(int val);
    int getIconBackground() const; void setIconBackground(int val);
    QString getScreenshotDirectory() const; void setScreenshotDirectory(const QString& val);
    QString getScreenshotFormat() const; void setScreenshotFormat(const QString& val);
    int getMinimapUpdateDelay() const; void setMinimapUpdateDelay(int val);
    bool isMinimapViewBoxEnabled() const; void setMinimapViewBoxEnabled(bool val);
    QString getMinimapExportDir() const; void setMinimapExportDir(const QString& val);
    QString getTilesetExportDir() const; void setTilesetExportDir(const QString& val);
    int getCursorRed() const; void setCursorRed(int val);
    int getCursorGreen() const; void setCursorGreen(int val);
    int getCursorBlue() const; void setCursorBlue(int val);
    int getCursorAlpha() const; void setCursorAlpha(int val);
    int getCursorAltRed() const; void setCursorAltRed(int val);
    int getCursorAltGreen() const; void setCursorAltGreen(int val);
    int getCursorAltBlue() const; void setCursorAltBlue(int val);
    int getCursorAltAlpha() const; void setCursorAltAlpha(int val);
    bool isExperimentalFogEnabled() const; void setExperimentalFogEnabled(bool val);

    // View Group
    bool isTransparentFloorsEnabled() const; void setTransparentFloorsEnabled(bool val);
    bool isTransparentItemsEnabled() const; void setTransparentItemsEnabled(bool val);
    bool isShowIngameBoxEnabled() const; void setShowIngameBoxEnabled(bool val);
    bool isShowGridEnabled() const; void setShowGridEnabled(bool val);
    bool isShowExtraEnabled() const; void setShowExtraEnabled(bool val);
    bool isShowAllFloorsEnabled() const; void setShowAllFloorsEnabled(bool val);
    bool isShowCreaturesEnabled() const; void setShowCreaturesEnabled(bool val);
    bool isShowSpawnsEnabled() const; void setShowSpawnsEnabled(bool val);
    bool isShowHousesEnabled() const; void setShowHousesEnabled(bool val);
    bool isShowShadeEnabled() const; void setShowShadeEnabled(bool val);
    bool isShowSpecialTilesEnabled() const; void setShowSpecialTilesEnabled(bool val);
    bool isShowZoneAreasEnabled() const; void setShowZoneAreasEnabled(bool val);
    bool isHighlightItemsEnabled() const; void setHighlightItemsEnabled(bool val);
    bool isShowItemsEnabled() const; void setShowItemsEnabled(bool val);
    bool isShowBlockingEnabled() const; void setShowBlockingEnabled(bool val);
    bool isShowTooltipsEnabled() const; void setShowTooltipsEnabled(bool val);
    bool isShowPreviewEnabled() const; void setShowPreviewEnabled(bool val);
    bool isShowWallHooksEnabled() const; void setShowWallHooksEnabled(bool val);
    bool isShowAsMinimapEnabled() const; void setShowAsMinimapEnabled(bool val);
    bool isShowOnlyTileFlagsEnabled() const; void setShowOnlyTileFlagsEnabled(bool val);
    bool isShowOnlyModifiedTilesEnabled() const; void setShowOnlyModifiedTilesEnabled(bool val);
    bool isHideItemsWhenZoomedEnabled() const; void setHideItemsWhenZoomedEnabled(bool val);
    bool isDrawLockedDoorEnabled() const; void setDrawLockedDoorEnabled(bool val);
    bool isHighlightLockedDoorsEnabled() const; void setHighlightLockedDoorsEnabled(bool val);
    bool isShowLightsEnabled() const; void setShowLightsEnabled(bool val);
    bool isShowLightStrengthEnabled() const; void setShowLightStrengthEnabled(bool val); // SHOW_LIGHT_STR
    bool isShowTechnicalItemsEnabled() const; void setShowTechnicalItemsEnabled(bool val);
    bool isShowWaypointsEnabled() const; void setShowWaypointsEnabled(bool val);
    bool isShowTownsEnabled() const; void setShowTownsEnabled(bool val);
    bool isAlwaysShowZonesEnabled() const; void setAlwaysShowZonesEnabled(bool val);
    bool isExternalHouseShaderEnabled() const; void setExternalHouseShaderEnabled(bool val); // EXT_HOUSE_SHADER

    // Editor Group
    bool isGroupActionsEnabled() const; void setGroupActionsEnabled(bool val);
    float getScrollSpeed() const; void setScrollSpeed(float val);
    float getZoomSpeed() const; void setZoomSpeed(float val);
    int getUndoSize() const; void setUndoSize(int val);
    int getUndoMemorySize() const; void setUndoMemorySize(int val); // UNDO_MEM_SIZE
    bool isMergePasteEnabled() const; void setMergePasteEnabled(bool val);
    int getSelectionType() const; void setSelectionType(int val);
    bool isCompensatedSelectEnabled() const; void setCompensatedSelectEnabled(bool val);
    bool isBorderIsGroundEnabled() const; void setBorderIsGroundEnabled(bool val);
    bool isBorderizePasteEnabled() const; void setBorderizePasteEnabled(bool val);
    bool isBorderizeDragEnabled() const; void setBorderizeDragEnabled(bool val);
    int getBorderizeDragThreshold() const; void setBorderizeDragThreshold(int val);
    int getBorderizePasteThreshold() const; void setBorderizePasteThreshold(int val);
    bool isBorderizeDeleteEnabled() const; void setBorderizeDeleteEnabled(bool val);
    bool isAlwaysMakeBackupEnabled() const; void setAlwaysMakeBackupEnabled(bool val);
    bool isUseAutomagicEnabled() const; void setUseAutomagicEnabled(bool val);
    bool isSameGroundTypeBorderEnabled() const; void setSameGroundTypeBorderEnabled(bool val);
    bool isWallsRepelBordersEnabled() const; void setWallsRepelBordersEnabled(bool val);
    bool isLayerCarpetsEnabled() const; void setLayerCarpetsEnabled(bool val);
    bool isCustomBorderEnabled() const; void setCustomBorderEnabled(bool val);
    int getCustomBorderID() const; void setCustomBorderID(int val);
    bool isHouseBrushRemoveItemsEnabled() const; void setHouseBrushRemoveItemsEnabled(bool val);
    bool isAutoAssignDoorIDEnabled() const; void setAutoAssignDoorIDEnabled(bool val);
    bool isEraserLeaveUniqueEnabled() const; void setEraserLeaveUniqueEnabled(bool val);
    bool isDoodadBrushEraseLikeEnabled() const; void setDoodadBrushEraseLikeEnabled(bool val);
    bool isWarnForDuplicateIDEnabled() const; void setWarnForDuplicateIDEnabled(bool val);
    bool isUseUpdaterEnabled() const; void setUseUpdaterEnabled(bool val); // USE_UPDATER
    bool isUseOTBM4ForAllMapsEnabled() const; void setUseOTBM4ForAllMapsEnabled(bool val);
    bool isUseOTGZEnabled() const; void setUseOTGZEnabled(bool val);
    bool isSaveWithOTBMagicNumberEnabled() const; void setSaveWithOTBMagicNumberEnabled(bool val);
    int getReplaceSize() const; void setReplaceSize(int val);
    int getMaxSpawnRadius() const; void setMaxSpawnRadius(int val);
    int getCurrentSpawnRadius() const; void setCurrentSpawnRadius(int val);
    bool isAutoCreateSpawnEnabled() const; void setAutoCreateSpawnEnabled(bool val);
    int getDefaultSpawnTime() const; void setDefaultSpawnTime(int val);
    bool areMouseButtonsSwitched() const; void setMouseButtonsSwitched(bool val); // SWITCH_MOUSEBUTTONS
    bool isDoubleClickPropertiesEnabled() const; void setDoubleClickPropertiesEnabled(bool val);
    bool isListboxEatsAllEventsEnabled() const; void setListboxEatsAllEventsEnabled(bool val);
    bool isRawLikeSimoneEnabled() const; void setRawLikeSimoneEnabled(bool val);
    int getCopyPositionFormat() const; void setCopyPositionFormat(int val);
    bool isAutoSelectRawOnRightClickEnabled() const; void setAutoSelectRawOnRightClickEnabled(bool val);
    bool isAutoSaveEnabled() const; void setAutoSaveEnabled(bool val);
    int getAutoSaveInterval() const; void setAutoSaveInterval(int val);
    QString getRecentFiles() const; void setRecentFiles(const QString& val); // Handle as string, parsing is app logic
    QString getRecentEditedMapPath() const; void setRecentEditedMapPath(const QString& val);
    QString getRecentEditedMapPosition() const; void setRecentEditedMapPosition(const QString& val);
    int getFindItemMode() const; void setFindItemMode(int val);
    int getJumpToItemMode() const; void setJumpToItemMode(int val);

    // UI Group
    bool useLargeContainerIcons() const; void setUseLargeContainerIcons(bool val);
    bool useLargeChooseItemIcons() const; void setUseLargeChooseItemIcons(bool val);
    bool useLargeTerrainToolbar() const; void setUseLargeTerrainToolbar(bool val);
    bool useLargeDoodadSizebar() const; void setUseLargeDoodadSizebar(bool val);
    bool useLargeItemSizebar() const; void setUseLargeItemSizebar(bool val);
    bool useLargeHouseSizebar() const; void setUseLargeHouseSizebar(bool val);
    bool useLargeRawSizebar() const; void setUseLargeRawSizebar(bool val);
    bool useGuiSelectionShadow() const; void setUseGuiSelectionShadow(bool val);
    int getPaletteColCount() const; void setPaletteColCount(int val);
    QString getPaletteTerrainStyle() const; void setPaletteTerrainStyle(const QString& val);
    QString getPaletteDoodadStyle() const; void setPaletteDoodadStyle(const QString& val);
    QString getPaletteItemStyle() const; void setPaletteItemStyle(const QString& val);
    QString getPaletteRawStyle() const; void setPaletteRawStyle(const QString& val);
    QString getPaletteCollectionStyle() const; void setPaletteCollectionStyle(const QString& val);
    bool useLargeCollectionToolbar() const; void setUseLargeCollectionToolbar(bool val);

    // Window Group
    QString getPaletteLayout() const; void setPaletteLayout(const QString& val);
    bool isMinimapVisible() const; void setMinimapVisible(bool val);
    QString getMinimapLayout() const; void setMinimapLayout(const QString& val);
    int getWindowHeight() const; void setWindowHeight(int val);
    int getWindowWidth() const; void setWindowWidth(int val);
    bool isWindowMaximized() const; void setWindowMaximized(bool val);
    bool isWelcomeDialogEnabled() const; void setWelcomeDialogEnabled(bool val); // WELCOME_DIALOG
    bool isShowToolbarStandardEnabled() const; void setShowToolbarStandardEnabled(bool val);
    bool isShowToolbarBrushesEnabled() const; void setShowToolbarBrushesEnabled(bool val);
    bool isShowToolbarPositionEnabled() const; void setShowToolbarPositionEnabled(bool val);
    bool isShowToolbarSizesEnabled() const; void setShowToolbarSizesEnabled(bool val);
    QString getToolbarStandardLayout() const; void setToolbarStandardLayout(const QString& val);
    QString getToolbarBrushesLayout() const; void setToolbarBrushesLayout(const QString& val);
    QString getToolbarPositionLayout() const; void setToolbarPositionLayout(const QString& val);
    QString getToolbarSizesLayout() const; void setToolbarSizesLayout(const QString& val);

    // Hotkeys Group
    QString getNumericalHotkeys() const; void setNumericalHotkeys(const QString& val);

    // Network Group
    QString getLiveHost() const; void setLiveHost(const QString& val);
    int getLivePort() const; void setLivePort(int val);
    QString getLivePassword() const; void setLivePassword(const QString& val);
    QString getLiveUsername() const; void setLiveUsername(const QString& val);

    // Interface Group (Dark Mode)
    bool isDarkModeEnabled() const; void setDarkModeEnabled(bool val);
    bool isDarkModeCustomColorEnabled() const; void setDarkModeCustomColorEnabled(bool val);
    int getDarkModeRed() const; void setDarkModeRed(int val);
    int getDarkModeGreen() const; void setDarkModeGreen(int val);
    int getDarkModeBlue() const; void setDarkModeBlue(int val);

    // HouseCreation Group
    int getMaxHouseTiles() const; void setMaxHouseTiles(int val);
    bool isHouseFloorScanEnabled() const; void setHouseFloorScanEnabled(bool val);
    bool isAutoDetectHouseExitEnabled() const; void setAutoDetectHouseExitEnabled(bool val);

    // LOD Group
    int getTooltipMaxZoom() const; void setTooltipMaxZoom(int val);
    int getGroundOnlyZoomThreshold() const; void setGroundOnlyZoomThreshold(int val);
    int getItemDisplayZoomThreshold() const; void setItemDisplayZoomThreshold(int val);
    int getSpecialFeaturesZoomThreshold() const; void setSpecialFeaturesZoomThreshold(int val);
    int getAnimationZoomThreshold() const; void setAnimationZoomThreshold(int val);
    int getEffectsZoomThreshold() const; void setEffectsZoomThreshold(int val);
    int getLightZoomThreshold() const; void setLightZoomThreshold(int val);
    int getShadeZoomThreshold() const; void setShadeZoomThreshold(int val);
    int getTownZoneZoomThreshold() const; void setTownZoneZoomThreshold(int val);
    int getGridZoomThreshold() const; void setGridZoomThreshold(int val);

    // PaletteGrid Group
    int getGridChunkSize() const; void setGridChunkSize(int val);
    int getGridVisibleRowsMargin() const; void setGridVisibleRowsMargin(int val);

    // Misc/Root Level
    bool isGoToWebsiteOnBootEnabled() const; void setGoToWebsiteOnBootEnabled(bool val);
    bool isIndirectoryInstallation() const; void setIndirectoryInstallation(bool val); // Note: RME logic for this is complex
    bool isAutoCheckForUpdatesEnabled() const; void setAutoCheckForUpdatesEnabled(bool val);
    bool isOnlyOneInstanceEnabled() const; void setOnlyOneInstanceEnabled(bool val);
    int getLastWebsitesOpenTime() const; void setLastWebsitesOpenTime(int val);


private:
    std::unique_ptr<QSettings> settings;
};

} // namespace RME

#endif // RME_APP_SETTINGS_H
