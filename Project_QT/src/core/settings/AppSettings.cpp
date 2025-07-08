#include "AppSettings.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMap>

// Define CLIENT_VERSION_NONE if not available globally (original RME had it in client_version.h)
// This is just a placeholder value, actual value might differ or come from another header.
#ifndef CLIENT_VERSION_NONE
#define CLIENT_VERSION_NONE 0
#endif

// Define SELECT_CURRENT_FLOOR if not available (original RME had it in common_windows.h or similar)
#ifndef SELECT_CURRENT_FLOOR
#define SELECT_CURRENT_FLOOR 0
#endif


namespace RME {

struct SettingDetail {
    QString qKey;
    QVariant defaultValue;
};

// Helper to convert original RME int-bools to bool for QVariant
inline QVariant rmeBool(int val) { return QVariant(static_cast<bool>(val)); }
// Helper for wxString defaults -> QString
inline QVariant rmeString(const char* val) { return QVariant(QString::fromUtf8(val)); }


static const QMap<Config::Key, SettingDetail>& getKeyDetailsMap() {
    static const QMap<Config::Key, SettingDetail> keyDetailsMapInstance = {
        // Version Group
        { Config::VERSION_ID,                    {"Version/VERSION_ID", QVariant(0)} }, // Not typically user-set
        { Config::USE_CUSTOM_DATA_DIRECTORY,     {"Version/USE_CUSTOM_DATA_DIRECTORY", rmeBool(0)} },
        { Config::DATA_DIRECTORY,                {"Version/DATA_DIRECTORY", rmeString("")} },
        { Config::EXTENSIONS_DIRECTORY,          {"Version/EXTENSIONS_DIRECTORY", rmeString("")} },
        { Config::ASSETS_DATA_DIRS,              {"Version/ASSETS_DATA_DIRS", rmeString("")} },
        { Config::DEFAULT_CLIENT_VERSION,        {"Editor/DEFAULT_CLIENT_VERSION", QVariant(CLIENT_VERSION_NONE)} }, // Moved to Editor in original
        { Config::CHECK_SIGNATURES,              {"Version/CHECK_SIGNATURES", rmeBool(0)} },

        // Graphics Group
        { Config::TEXTURE_MANAGEMENT,            {"Graphics/TEXTURE_MANAGEMENT", rmeBool(1)} },
        { Config::TEXTURE_CLEAN_PULSE,           {"Graphics/TEXTURE_CLEAN_PULSE", QVariant(15)} },
        { Config::TEXTURE_CLEAN_THRESHOLD,       {"Graphics/TEXTURE_CLEAN_THRESHOLD", QVariant(2500)} },
        { Config::TEXTURE_LONGEVITY,             {"Graphics/TEXTURE_LONGEVITY", QVariant(20)} },
        { Config::HARD_REFRESH_RATE,             {"Graphics/HARD_REFRESH_RATE", QVariant(200)} },
        { Config::USE_MEMCACHED_SPRITES,         {"Graphics/USE_MEMCACHED_SPRITES", rmeBool(0)} }, // Runtime
        { Config::USE_MEMCACHED_SPRITES_TO_SAVE, {"Graphics/USE_MEMCACHED_SPRITES_TO_SAVE", rmeBool(0)} }, // Saved value
        { Config::SOFTWARE_CLEAN_THRESHOLD,      {"Graphics/SOFTWARE_CLEAN_THRESHOLD", QVariant(1800)} },
        { Config::SOFTWARE_CLEAN_SIZE,           {"Graphics/SOFTWARE_CLEAN_SIZE", QVariant(500)} },
        { Config::ICON_BACKGROUND,               {"Graphics/ICON_BACKGROUND", QVariant(0)} }, // Enum/color index
        { Config::SCREENSHOT_DIRECTORY,          {"Graphics/SCREENSHOT_DIRECTORY", rmeString("")} },
        { Config::SCREENSHOT_FORMAT,             {"Graphics/SCREENSHOT_FORMAT", rmeString("png")} },
        { Config::MINIMAP_UPDATE_DELAY,          {"Graphics/MINIMAP_UPDATE_DELAY", QVariant(333)} },
        { Config::MINIMAP_VIEW_BOX,              {"Graphics/MINIMAP_VIEW_BOX", rmeBool(1)} },
        { Config::MINIMAP_EXPORT_DIR,            {"Graphics/MINIMAP_EXPORT_DIR", rmeString("")} },
        { Config::TILESET_EXPORT_DIR,            {"Graphics/TILESET_EXPORT_DIR", rmeString("")} },
        { Config::CURSOR_RED,                    {"Graphics/CURSOR_RED", QVariant(0)} },
        { Config::CURSOR_GREEN,                  {"Graphics/CURSOR_GREEN", QVariant(166)} },
        { Config::CURSOR_BLUE,                   {"Graphics/CURSOR_BLUE", QVariant(0)} },
        { Config::CURSOR_ALPHA,                  {"Graphics/CURSOR_ALPHA", QVariant(128)} },
        { Config::CURSOR_ALT_RED,                {"Graphics/CURSOR_ALT_RED", QVariant(0)} },
        { Config::CURSOR_ALT_GREEN,              {"Graphics/CURSOR_ALT_GREEN", QVariant(166)} },
        { Config::CURSOR_ALT_BLUE,               {"Graphics/CURSOR_ALT_BLUE", QVariant(0)} },
        { Config::CURSOR_ALT_ALPHA,              {"Graphics/CURSOR_ALT_ALPHA", QVariant(128)} },
        { Config::EXPERIMENTAL_FOG,              {"experimental/EXPERIMENTAL_FOG", rmeBool(0)} }, // section("experimental")

        // View Group
        { Config::TRANSPARENT_FLOORS,            {"View/TRANSPARENT_FLOORS", rmeBool(0)} },
        { Config::TRANSPARENT_ITEMS,             {"View/TRANSPARENT_ITEMS", rmeBool(0)} },
        { Config::SHOW_INGAME_BOX,               {"View/SHOW_INGAME_BOX", rmeBool(0)} },
        { Config::SHOW_GRID,                     {"View/SHOW_GRID", rmeBool(0)} },
        { Config::SHOW_EXTRA,                    {"View/SHOW_EXTRA", rmeBool(1)} },
        { Config::SHOW_ALL_FLOORS,               {"View/SHOW_ALL_FLOORS", rmeBool(1)} },
        { Config::SHOW_CREATURES,                {"View/SHOW_CREATURES", rmeBool(1)} },
        { Config::SHOW_SPAWNS,                   {"View/SHOW_SPAWNS", rmeBool(1)} },
        { Config::SHOW_HOUSES,                   {"View/SHOW_HOUSES", rmeBool(1)} },
        { Config::SHOW_SHADE,                    {"View/SHOW_SHADE", rmeBool(1)} },
        { Config::SHOW_SPECIAL_TILES,            {"View/SHOW_SPECIAL_TILES", rmeBool(1)} },
        { Config::SHOW_ZONE_AREAS,               {"View/SHOW_ZONE_AREAS", rmeBool(1)} },
        { Config::HIGHLIGHT_ITEMS,               {"View/HIGHLIGHT_ITEMS", rmeBool(0)} },
        { Config::SHOW_ITEMS,                    {"View/SHOW_ITEMS", rmeBool(1)} },
        { Config::SHOW_BLOCKING,                 {"View/SHOW_BLOCKING", rmeBool(0)} },
        { Config::SHOW_TOOLTIPS,                 {"View/SHOW_TOOLTIPS", rmeBool(1)} },
        { Config::SHOW_ONLY_TILEFLAGS,           {"View/SHOW_ONLY_TILEFLAGS", rmeBool(0)} },
        { Config::SHOW_ONLY_MODIFIED_TILES,      {"View/SHOW_ONLY_MODIFIED_TILES", rmeBool(0)} },
        { Config::SHOW_PREVIEW,                  {"View/SHOW_PREVIEW", rmeBool(1)} },
        { Config::SHOW_WALL_HOOKS,               {"View/SHOW_WALL_HOOKS", rmeBool(0)} },
        { Config::SHOW_AS_MINIMAP,               {"View/SHOW_AS_MINIMAP", rmeBool(0)} },
        { Config::DRAW_LOCKED_DOOR,              {"View/DRAW_LOCKED_DOOR", rmeBool(0)} },
        { Config::HIGHLIGHT_LOCKED_DOORS,        {"View/HIGHLIGHT_LOCKED_DOORS", rmeBool(1)} },
        { Config::SHOW_LIGHTS,                   {"View/SHOW_LIGHTS", rmeBool(0)} },
        { Config::SHOW_LIGHT_STR,                {"View/SHOW_LIGHT_STR", rmeBool(0)} },
        { Config::SHOW_TECHNICAL_ITEMS,          {"View/SHOW_TECHNICAL_ITEMS", rmeBool(1)} },
        { Config::SHOW_WAYPOINTS,                {"View/SHOW_WAYPOINTS", rmeBool(1)} },
        { Config::SHOW_TOWNS,                    {"View/SHOW_TOWNS", rmeBool(0)} },
        { Config::ALWAYS_SHOW_ZONES,             {"View/ALWAYS_SHOW_ZONES", rmeBool(1)} },
        { Config::EXT_HOUSE_SHADER,              {"View/EXT_HOUSE_SHADER", rmeBool(1)} },

        // Editor Group
        { Config::MERGE_MOVE,                    {"Editor/MERGE_MOVE", rmeBool(0)} }, // From Editor in settings.cpp
        { Config::RECENT_FILES,                  {"Editor/RECENT_FILES", rmeString("")} },
        { Config::WORKER_THREADS,                {"Editor/WORKER_THREADS", QVariant(1)} },
        { Config::MERGE_PASTE,                   {"Editor/MERGE_PASTE", rmeBool(0)} },
        { Config::UNDO_SIZE,                     {"Editor/UNDO_SIZE", QVariant(40)} },
        { Config::UNDO_MEM_SIZE,                 {"Editor/UNDO_MEM_SIZE", QVariant(64)} },
        { Config::GROUP_ACTIONS,                 {"Editor/GROUP_ACTIONS", rmeBool(1)} },
        { Config::SELECTION_TYPE,                {"Editor/SELECTION_TYPE", QVariant(SELECT_CURRENT_FLOOR)} },
        { Config::COMPENSATED_SELECT,            {"Editor/COMPENSATED_SELECT", rmeBool(1)} },
        { Config::SCROLL_SPEED,                  {"Editor/SCROLL_SPEED", QVariant(3.5f)} },
        { Config::ZOOM_SPEED,                    {"Editor/ZOOM_SPEED", QVariant(1.4f)} },
        { Config::SWITCH_MOUSEBUTTONS,           {"Editor/SWITCH_MOUSEBUTTONS", rmeBool(0)} },
        { Config::DOUBLECLICK_PROPERTIES,        {"Editor/DOUBLECLICK_PROPERTIES", rmeBool(1)} },
        { Config::LISTBOX_EATS_ALL_EVENTS,       {"Editor/LISTBOX_EATS_ALL_EVENTS", rmeBool(1)} },
        { Config::BORDER_IS_GROUND,              {"Editor/BORDER_IS_GROUND", rmeBool(0)} },
        { Config::BORDERIZE_PASTE,               {"Editor/BORDERIZE_PASTE", rmeBool(1)} },
        { Config::BORDERIZE_DRAG,                {"Editor/BORDERIZE_DRAG", rmeBool(1)} },
        { Config::BORDERIZE_DRAG_THRESHOLD,      {"Editor/BORDERIZE_DRAG_THRESHOLD", QVariant(6000)} },
        { Config::BORDERIZE_PASTE_THRESHOLD,     {"Editor/BORDERIZE_PASTE_THRESHOLD", QVariant(10000)} },
        { Config::BORDERIZE_DELETE,              {"Editor/BORDERIZE_DELETE", rmeBool(0)} },
        { Config::ALWAYS_MAKE_BACKUP,            {"Editor/ALWAYS_MAKE_BACKUP", rmeBool(0)} },
        { Config::USE_AUTOMAGIC,                 {"Editor/USE_AUTOMAGIC", rmeBool(1)} },
        { Config::SAME_GROUND_TYPE_BORDER,       {"Editor/SAME_GROUND_TYPE_BORDER", rmeBool(0)} },
        { Config::WALLS_REPEL_BORDERS,           {"Editor/WALLS_REPEL_BORDERS", rmeBool(0)} },
        { Config::LAYER_CARPETS,                 {"Editor/LAYER_CARPETS", rmeBool(0)} },
        { Config::CUSTOM_BORDER_ENABLED,         {"Editor/CUSTOM_BORDER_ENABLED", rmeBool(0)} },
        { Config::CUSTOM_BORDER_ID,              {"Editor/CUSTOM_BORDER_ID", QVariant(1)} },
        { Config::HOUSE_BRUSH_REMOVE_ITEMS,      {"Editor/HOUSE_BRUSH_REMOVE_ITEMS", rmeBool(0)} },
        { Config::AUTO_ASSIGN_DOORID,            {"Editor/AUTO_ASSIGN_DOORID", rmeBool(1)} },
        { Config::ERASER_LEAVE_UNIQUE,           {"Editor/ERASER_LEAVE_UNIQUE", rmeBool(1)} },
        { Config::DOODAD_BRUSH_ERASE_LIKE,       {"Editor/DOODAD_BRUSH_ERASE_LIKE", rmeBool(0)} },
        { Config::WARN_FOR_DUPLICATE_ID,         {"Editor/WARN_FOR_DUPLICATE_ID", rmeBool(1)} },
        { Config::AUTO_CREATE_SPAWN,             {"Editor/AUTO_CREATE_SPAWN", rmeBool(1)} },
        { Config::DEFAULT_SPAWNTIME,             {"Editor/DEFAULT_SPAWNTIME", QVariant(60)} },
        { Config::MAX_SPAWN_RADIUS,              {"Editor/MAX_SPAWN_RADIUS", QVariant(30)} },
        { Config::CURRENT_SPAWN_RADIUS,          {"Editor/CURRENT_SPAWN_RADIUS", QVariant(5)} },
        { Config::RAW_LIKE_SIMONE,               {"Editor/RAW_LIKE_SIMONE", rmeBool(1)} },
        { Config::ONLY_ONE_INSTANCE,             {"Editor/ONLY_ONE_INSTANCE", rmeBool(1)} }, // Moved from root
        { Config::SHOW_TILESET_EDITOR,           {"Editor/SHOW_TILESET_EDITOR", rmeBool(0)} },
        { Config::USE_OTBM_4_FOR_ALL_MAPS,       {"Editor/USE_OTBM_4_FOR_ALL_MAPS", rmeBool(0)} },
        { Config::USE_OTGZ,                      {"Editor/USE_OTGZ", rmeBool(1)} },
        { Config::SAVE_WITH_OTB_MAGIC_NUMBER,    {"Editor/SAVE_WITH_OTB_MAGIC_NUMBER", rmeBool(0)} },
        { Config::REPLACE_SIZE,                  {"Editor/REPLACE_SIZE", QVariant(500)} },
        { Config::COPY_POSITION_FORMAT,          {"Editor/COPY_POSITION_FORMAT", QVariant(0)} }, // Enum
        { Config::AUTO_SELECT_RAW_ON_RIGHTCLICK, {"Editor/AUTO_SELECT_RAW_ON_RIGHTCLICK", rmeBool(0)} }, // Not in original settings.cpp, assume default 0
        { Config::AUTO_SAVE_ENABLED,             {"Editor/AUTO_SAVE_ENABLED", rmeBool(0)} },
        { Config::AUTO_SAVE_INTERVAL,            {"Editor/AUTO_SAVE_INTERVAL", QVariant(5)} },

        // UI Group
        { Config::USE_LARGE_CONTAINER_ICONS,     {"UI/USE_LARGE_CONTAINER_ICONS", rmeBool(1)} },
        { Config::USE_LARGE_CHOOSE_ITEM_ICONS,   {"UI/USE_LARGE_CHOOSE_ITEM_ICONS", rmeBool(1)} },
        { Config::USE_LARGE_TERRAIN_TOOLBAR,     {"UI/USE_LARGE_TERRAIN_TOOLBAR", rmeBool(1)} },
        { Config::USE_LARGE_DOODAD_SIZEBAR,      {"UI/USE_LARGE_DOODAD_SIZEBAR", rmeBool(1)} },
        { Config::USE_LARGE_ITEM_SIZEBAR,        {"UI/USE_LARGE_ITEM_SIZEBAR", rmeBool(1)} },
        { Config::USE_LARGE_HOUSE_SIZEBAR,       {"UI/USE_LARGE_HOUSE_SIZEBAR", rmeBool(1)} },
        { Config::USE_LARGE_RAW_SIZEBAR,         {"UI/USE_LARGE_RAW_SIZEBAR", rmeBool(1)} },
        { Config::USE_GUI_SELECTION_SHADOW,      {"UI/USE_GUI_SELECTION_SHADOW", rmeBool(0)} },
        { Config::PALETTE_COL_COUNT,             {"UI/PALETTE_COL_COUNT", QVariant(8)} },
        { Config::PALETTE_TERRAIN_STYLE,         {"UI/PALETTE_TERRAIN_STYLE", rmeString("large icons")} },
        { Config::PALETTE_DOODAD_STYLE,          {"UI/PALETTE_DOODAD_STYLE", rmeString("large icons")} },
        { Config::PALETTE_ITEM_STYLE,            {"UI/PALETTE_ITEM_STYLE", rmeString("listbox")} },
        { Config::PALETTE_RAW_STYLE,             {"UI/PALETTE_RAW_STYLE", rmeString("listbox")} },
        { Config::PALETTE_COLLECTION_STYLE,      {"UI/PALETTE_COLLECTION_STYLE", rmeString("large icons")} },
        { Config::USE_LARGE_COLLECTION_TOOLBAR,  {"UI/USE_LARGE_COLLECTION_TOOLBAR", rmeBool(1)} },

        // Window Group
        { Config::PALETTE_LAYOUT,                {"Window/PALETTE_LAYOUT", rmeString("name=02c30f6048629894000011bc00000002;caption=Palette;state=2099148;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=245;besth=100;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1")} },
        { Config::MINIMAP_VISIBLE,               {"Window/MINIMAP_VISIBLE", rmeBool(0)} },
        { Config::MINIMAP_LAYOUT,                {"Window/MINIMAP_LAYOUT", rmeString("name=066e2bc8486298990000259a00000003;caption=Minimap;state=2099151;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=170;besth=130;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=221;floath=164")} },
        { Config::WINDOW_HEIGHT,                 {"Window/WINDOW_HEIGHT", QVariant(500)} },
        { Config::WINDOW_WIDTH,                  {"Window/WINDOW_WIDTH", QVariant(700)} },
        { Config::WINDOW_MAXIMIZED,              {"Window/WINDOW_MAXIMIZED", rmeBool(0)} },
        { Config::WELCOME_DIALOG,                {"Window/WELCOME_DIALOG", rmeBool(1)} },
        { Config::SHOW_TOOLBAR_STANDARD,         {"Window/SHOW_TOOLBAR_STANDARD", rmeBool(1)} },
        { Config::SHOW_TOOLBAR_BRUSHES,          {"Window/SHOW_TOOLBAR_BRUSHES", rmeBool(0)} },
        { Config::SHOW_TOOLBAR_POSITION,         {"Window/SHOW_TOOLBAR_POSITION", rmeBool(0)} },
        { Config::SHOW_TOOLBAR_SIZES,            {"Window/SHOW_TOOLBAR_SIZES", rmeBool(0)} },
        { Config::TOOLBAR_STANDARD_LAYOUT,       {"Window/TOOLBAR_STANDARD_LAYOUT", rmeString("")} },
        { Config::TOOLBAR_BRUSHES_LAYOUT,        {"Window/TOOLBAR_BRUSHES_LAYOUT", rmeString("")} },
        { Config::TOOLBAR_POSITION_LAYOUT,       {"Window/TOOLBAR_POSITION_LAYOUT", rmeString("")} },
        { Config::TOOLBAR_SIZES_LAYOUT,          {"Window/TOOLBAR_SIZES_LAYOUT", rmeString("")} },

        // Hotkeys Group
        { Config::NUMERICAL_HOTKEYS,             {"Hotkeys/NUMERICAL_HOTKEYS", rmeString("none:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\n")} },

        // Network Group
        { Config::LIVE_HOST,                     {"Network/LIVE_HOST", rmeString("localhost")} },
        { Config::LIVE_PORT,                     {"Network/LIVE_PORT", QVariant(12356)} },
        { Config::LIVE_PASSWORD,                 {"Network/LIVE_PASSWORD", rmeString("")} },
        { Config::LIVE_USERNAME,                 {"Network/LIVE_USERNAME", rmeString("")} }, // wxGetUserId().ToStdString() needs platform specific or generic user name

        // Interface Group (Dark Mode)
        { Config::DARK_MODE,                     {"Interface/DARK_MODE", rmeBool(0)} },
        { Config::DARK_MODE_CUSTOM_COLOR,        {"Interface/DARK_MODE_CUSTOM_COLOR", rmeBool(0)} },
        { Config::DARK_MODE_RED,                 {"Interface/DARK_MODE_RED", QVariant(45)} },
        { Config::DARK_MODE_GREEN,               {"Interface/DARK_MODE_GREEN", QVariant(45)} },
        { Config::DARK_MODE_BLUE,                {"Interface/DARK_MODE_BLUE", QVariant(48)} },

        // HouseCreation Group
        { Config::MAX_HOUSE_TILES,               {"HouseCreation/MAX_HOUSE_TILES", QVariant(5000)} },
        { Config::HOUSE_FLOOR_SCAN,              {"HouseCreation/HOUSE_FLOOR_SCAN", rmeBool(1)} },
        { Config::AUTO_DETECT_HOUSE_EXIT,        {"HouseCreation/AUTO_DETECT_HOUSE_EXIT", rmeBool(1)} },

        // LOD Group
        { Config::TOOLTIP_MAX_ZOOM,                {"LOD/TOOLTIP_MAX_ZOOM", QVariant(10)} },
        { Config::GROUND_ONLY_ZOOM_THRESHOLD,      {"LOD/GROUND_ONLY_ZOOM_THRESHOLD", QVariant(8)} },
        { Config::ITEM_DISPLAY_ZOOM_THRESHOLD,     {"LOD/ITEM_DISPLAY_ZOOM_THRESHOLD", QVariant(10)} },
        { Config::SPECIAL_FEATURES_ZOOM_THRESHOLD, {"LOD/SPECIAL_FEATURES_ZOOM_THRESHOLD", QVariant(10)} },
        { Config::ANIMATION_ZOOM_THRESHOLD,        {"LOD/ANIMATION_ZOOM_THRESHOLD", QVariant(2)} },
        { Config::EFFECTS_ZOOM_THRESHOLD,          {"LOD/EFFECTS_ZOOM_THRESHOLD", QVariant(6)} },
        { Config::LIGHT_ZOOM_THRESHOLD,            {"LOD/LIGHT_ZOOM_THRESHOLD", QVariant(4)} },
        { Config::SHADE_ZOOM_THRESHOLD,            {"LOD/SHADE_ZOOM_THRESHOLD", QVariant(8)} },
        { Config::TOWN_ZONE_ZOOM_THRESHOLD,        {"LOD/TOWN_ZONE_ZOOM_THRESHOLD", QVariant(6)} },
        { Config::GRID_ZOOM_THRESHOLD,             {"LOD/GRID_ZOOM_THRESHOLD", QVariant(12)} },

        // PaletteGrid Group
        { Config::GRID_CHUNK_SIZE,                 {"PaletteGrid/GRID_CHUNK_SIZE", QVariant(3000)} },
        { Config::GRID_VISIBLE_ROWS_MARGIN,        {"PaletteGrid/GRID_VISIBLE_ROWS_MARGIN", QVariant(30)} },

        // Misc/Root Level (section(""))
        { Config::GOTO_WEBSITE_ON_BOOT,          {"/GOTO_WEBSITE_ON_BOOT", rmeBool(0)} }, // Root path for QSettings
        { Config::INDIRECTORY_INSTALLATION,      {"/INDIRECTORY_INSTALLATION", rmeBool(0)} }, // This is often runtime determined
        { Config::AUTOCHECK_FOR_UPDATES,         {"Editor/USE_UPDATER", rmeBool(1)} }, // Mapped to USE_UPDATER in Editor section
        { Config::RECENT_EDITED_MAP_PATH,        {"/RECENT_EDITED_MAP_PATH", rmeString("")} },
        { Config::RECENT_EDITED_MAP_POSITION,    {"/RECENT_EDITED_MAP_POSITION", rmeString("")} },
        { Config::LAST_WEBSITES_OPEN_TIME,       {"/LAST_WEBSITES_OPEN_TIME", QVariant(0)} },
        { Config::FIND_ITEM_MODE,                {"/FIND_ITEM_MODE", QVariant(0)} },
        { Config::JUMP_TO_ITEM_MODE,             {"/JUMP_TO_ITEM_MODE", QVariant(0)} },
    };
    return keyDetailsMapInstance;
}


QString AppSettings::getKeyString(Config::Key key) {
    const auto& map = getKeyDetailsMap();
    if (map.contains(key)) {
        return map[key].qKey;
    }
    qWarning() << "AppSettings::getKeyString - Unknown Config::Key:" << static_cast<int>(key);
    return QString("Unknown/KEY_%1").arg(static_cast<int>(key));
}

QVariant AppSettings::getValue(Config::Key key, const QVariant& defaultValue) const {
    if (!settings) {
        qWarning() << "AppSettings::getValue - QSettings object is null.";
        return defaultValue.isValid() ? defaultValue : QVariant();
    }
    const auto& map = getKeyDetailsMap();
    if (map.contains(key)) {
        return settings->value(map[key].qKey, defaultValue.isValid() ? defaultValue : map[key].defaultValue);
    } else {
         qWarning() << "AppSettings::getValue - Unknown key:" << static_cast<int>(key);
         return defaultValue.isValid() ? defaultValue : QVariant();
    }
}

void AppSettings::setValue(Config::Key key, const QVariant& value) {
    if (!settings) {
        qWarning() << "AppSettings::setValue - QSettings object is null.";
        return;
    }
    const auto& map = getKeyDetailsMap();
    if (map.contains(key)) {
         settings->setValue(map[key].qKey, value);
    } else {
        qWarning() << "AppSettings::setValue - Attempted to set unknown key:" << static_cast<int>(key);
    }
}

AppSettings::AppSettings(QSettings::Format format, QSettings::Scope scope,
                         const QString& organization, const QString& application) {
    if (!organization.isEmpty() && !application.isEmpty()) {
        settings = std::make_unique<QSettings>(format, scope, organization, application);
    } else {
        if (QCoreApplication::organizationName().isEmpty() || QCoreApplication::applicationName().isEmpty()) {
            qWarning("AppSettings: QCoreApplication organizationName or applicationName is not set. "
                     "Using provided test defaults or potentially unstable default QSettings path. "
                     "Provide explicit organization and application names to constructor for stable testing.");
            QString orgToUse = organization.isEmpty() ? "RMEditor_DefaultOrg" : organization;
            QString appToUse = application.isEmpty() ? "RME-Qt_DefaultApp" : application;
            settings = std::make_unique<QSettings>(format, scope, orgToUse, appToUse);
        } else {
             settings = std::make_unique<QSettings>();
        }
    }
    if(settings) {
        qInfo() << "AppSettings: Initialized. Using settings file:" << settings->fileName();
    } else {
        qCritical("AppSettings: Failed to initialize QSettings object!");
    }
}

AppSettings::~AppSettings() {
    if (settings) {
        settings->sync();
    }
}

// --- Typed Getters & Setters (Full List) ---
// Version Group
bool AppSettings::isUseCustomDataDirectory() const { return getValue(Config::USE_CUSTOM_DATA_DIRECTORY).value<bool>(); }
void AppSettings::setUseCustomDataDirectory(bool val) { setValue(Config::USE_CUSTOM_DATA_DIRECTORY, val); }
QString AppSettings::getDataDirectory() const { return getValue(Config::DATA_DIRECTORY).value<QString>(); }
void AppSettings::setDataDirectory(const QString& val) { setValue(Config::DATA_DIRECTORY, val); }
QString AppSettings::getExtensionsDirectory() const { return getValue(Config::EXTENSIONS_DIRECTORY).value<QString>(); }
void AppSettings::setExtensionsDirectory(const QString& val) { setValue(Config::EXTENSIONS_DIRECTORY, val); }
QString AppSettings::getAssetsDataDirs() const { return getValue(Config::ASSETS_DATA_DIRS).value<QString>(); }
void AppSettings::setAssetsDataDirs(const QString& val) { setValue(Config::ASSETS_DATA_DIRS, val); }
int AppSettings::getDefaultClientVersion() const { return getValue(Config::DEFAULT_CLIENT_VERSION).value<int>(); }
void AppSettings::setDefaultClientVersion(int val) { setValue(Config::DEFAULT_CLIENT_VERSION, val); }
bool AppSettings::isCheckSignaturesEnabled() const { return getValue(Config::CHECK_SIGNATURES).value<bool>(); }
void AppSettings::setCheckSignaturesEnabled(bool val) { setValue(Config::CHECK_SIGNATURES, val); }

// Graphics Group
bool AppSettings::isTextureManagementEnabled() const { return getValue(Config::TEXTURE_MANAGEMENT).toBool(); }
void AppSettings::setTextureManagementEnabled(bool val) { setValue(Config::TEXTURE_MANAGEMENT, val); }
int AppSettings::getTextureCleanPulse() const { return getValue(Config::TEXTURE_CLEAN_PULSE).toInt(); }
void AppSettings::setTextureCleanPulse(int val) { setValue(Config::TEXTURE_CLEAN_PULSE, val); }
int AppSettings::getTextureCleanThreshold() const { return getValue(Config::TEXTURE_CLEAN_THRESHOLD).toInt(); }
void AppSettings::setTextureCleanThreshold(int val) { setValue(Config::TEXTURE_CLEAN_THRESHOLD, val); }
int AppSettings::getTextureLongevity() const { return getValue(Config::TEXTURE_LONGEVITY).toInt(); }
void AppSettings::setTextureLongevity(int val) { setValue(Config::TEXTURE_LONGEVITY, val); }
int AppSettings::getHardRefreshRate() const { return getValue(Config::HARD_REFRESH_RATE).toInt(); }
void AppSettings::setHardRefreshRate(int val) { setValue(Config::HARD_REFRESH_RATE, val); }
bool AppSettings::useMemcachedSprites() const { return getValue(Config::USE_MEMCACHED_SPRITES).toBool(); }
void AppSettings::setUseMemcachedSprites(bool val) { setValue(Config::USE_MEMCACHED_SPRITES, val); }
bool AppSettings::useMemcachedSpritesToSave() const { return getValue(Config::USE_MEMCACHED_SPRITES_TO_SAVE).toBool(); }
void AppSettings::setUseMemcachedSpritesToSave(bool val) { setValue(Config::USE_MEMCACHED_SPRITES_TO_SAVE, val); }
int AppSettings::getSoftwareCleanThreshold() const { return getValue(Config::SOFTWARE_CLEAN_THRESHOLD).toInt(); }
void AppSettings::setSoftwareCleanThreshold(int val) { setValue(Config::SOFTWARE_CLEAN_THRESHOLD, val); }
int AppSettings::getSoftwareCleanSize() const { return getValue(Config::SOFTWARE_CLEAN_SIZE).toInt(); }
void AppSettings::setSoftwareCleanSize(int val) { setValue(Config::SOFTWARE_CLEAN_SIZE, val); }
int AppSettings::getIconBackground() const { return getValue(Config::ICON_BACKGROUND).toInt(); }
void AppSettings::setIconBackground(int val) { setValue(Config::ICON_BACKGROUND, val); }
QString AppSettings::getScreenshotDirectory() const { return getValue(Config::SCREENSHOT_DIRECTORY).toString(); }
void AppSettings::setScreenshotDirectory(const QString& val) { setValue(Config::SCREENSHOT_DIRECTORY, val); }
QString AppSettings::getScreenshotFormat() const { return getValue(Config::SCREENSHOT_FORMAT).toString(); }
void AppSettings::setScreenshotFormat(const QString& val) { setValue(Config::SCREENSHOT_FORMAT, val); }
int AppSettings::getMinimapUpdateDelay() const { return getValue(Config::MINIMAP_UPDATE_DELAY).toInt(); }
void AppSettings::setMinimapUpdateDelay(int val) { setValue(Config::MINIMAP_UPDATE_DELAY, val); }
bool AppSettings::isMinimapViewBoxEnabled() const { return getValue(Config::MINIMAP_VIEW_BOX).toBool(); }
void AppSettings::setMinimapViewBoxEnabled(bool val) { setValue(Config::MINIMAP_VIEW_BOX, val); }
QString AppSettings::getMinimapExportDir() const { return getValue(Config::MINIMAP_EXPORT_DIR).toString(); }
void AppSettings::setMinimapExportDir(const QString& val) { setValue(Config::MINIMAP_EXPORT_DIR, val); }
QString AppSettings::getTilesetExportDir() const { return getValue(Config::TILESET_EXPORT_DIR).toString(); }
void AppSettings::setTilesetExportDir(const QString& val) { setValue(Config::TILESET_EXPORT_DIR, val); }
int AppSettings::getCursorRed() const { return getValue(Config::CURSOR_RED).toInt(); }
void AppSettings::setCursorRed(int val) { setValue(Config::CURSOR_RED, val); }
int AppSettings::getCursorGreen() const { return getValue(Config::CURSOR_GREEN).toInt(); }
void AppSettings::setCursorGreen(int val) { setValue(Config::CURSOR_GREEN, val); }
int AppSettings::getCursorBlue() const { return getValue(Config::CURSOR_BLUE).toInt(); }
void AppSettings::setCursorBlue(int val) { setValue(Config::CURSOR_BLUE, val); }
int AppSettings::getCursorAlpha() const { return getValue(Config::CURSOR_ALPHA).toInt(); }
void AppSettings::setCursorAlpha(int val) { setValue(Config::CURSOR_ALPHA, val); }
int AppSettings::getCursorAltRed() const { return getValue(Config::CURSOR_ALT_RED).toInt(); }
void AppSettings::setCursorAltRed(int val) { setValue(Config::CURSOR_ALT_RED, val); }
int AppSettings::getCursorAltGreen() const { return getValue(Config::CURSOR_ALT_GREEN).toInt(); }
void AppSettings::setCursorAltGreen(int val) { setValue(Config::CURSOR_ALT_GREEN, val); }
int AppSettings::getCursorAltBlue() const { return getValue(Config::CURSOR_ALT_BLUE).toInt(); }
void AppSettings::setCursorAltBlue(int val) { setValue(Config::CURSOR_ALT_BLUE, val); }
int AppSettings::getCursorAltAlpha() const { return getValue(Config::CURSOR_ALT_ALPHA).toInt(); }
void AppSettings::setCursorAltAlpha(int val) { setValue(Config::CURSOR_ALT_ALPHA, val); }
bool AppSettings::isExperimentalFogEnabled() const { return getValue(Config::EXPERIMENTAL_FOG).toBool(); }
void AppSettings::setExperimentalFogEnabled(bool val) { setValue(Config::EXPERIMENTAL_FOG, val); }

// View Group (Most were in subset, adding remaining)
bool AppSettings::isTransparentItemsEnabled() const { return getValue(Config::TRANSPARENT_ITEMS).toBool(); }
void AppSettings::setTransparentItemsEnabled(bool val) { setValue(Config::TRANSPARENT_ITEMS, val); }
bool AppSettings::isShowIngameBoxEnabled() const { return getValue(Config::SHOW_INGAME_BOX).toBool(); }
void AppSettings::setShowIngameBoxEnabled(bool val) { setValue(Config::SHOW_INGAME_BOX, val); }
bool AppSettings::isShowExtraEnabled() const { return getValue(Config::SHOW_EXTRA).toBool(); }
void AppSettings::setShowExtraEnabled(bool val) { setValue(Config::SHOW_EXTRA, val); }
bool AppSettings::isShowAllFloorsEnabled() const { return getValue(Config::SHOW_ALL_FLOORS).toBool(); }
void AppSettings::setShowAllFloorsEnabled(bool val) { setValue(Config::SHOW_ALL_FLOORS, val); }
bool AppSettings::isShowCreaturesEnabled() const { return getValue(Config::SHOW_CREATURES).toBool(); }
void AppSettings::setShowCreaturesEnabled(bool val) { setValue(Config::SHOW_CREATURES, val); }
bool AppSettings::isShowSpawnsEnabled() const { return getValue(Config::SHOW_SPAWNS).toBool(); }
void AppSettings::setShowSpawnsEnabled(bool val) { setValue(Config::SHOW_SPAWNS, val); }
bool AppSettings::isShowHousesEnabled() const { return getValue(Config::SHOW_HOUSES).toBool(); }
void AppSettings::setShowHousesEnabled(bool val) { setValue(Config::SHOW_HOUSES, val); }
bool AppSettings::isShowShadeEnabled() const { return getValue(Config::SHOW_SHADE).toBool(); }
void AppSettings::setShowShadeEnabled(bool val) { setValue(Config::SHOW_SHADE, val); }
bool AppSettings::isShowSpecialTilesEnabled() const { return getValue(Config::SHOW_SPECIAL_TILES).toBool(); }
void AppSettings::setShowSpecialTilesEnabled(bool val) { setValue(Config::SHOW_SPECIAL_TILES, val); }
bool AppSettings::isShowZoneAreasEnabled() const { return getValue(Config::SHOW_ZONE_AREAS).toBool(); }
void AppSettings::setShowZoneAreasEnabled(bool val) { setValue(Config::SHOW_ZONE_AREAS, val); }
bool AppSettings::isHighlightItemsEnabled() const { return getValue(Config::HIGHLIGHT_ITEMS).toBool(); }
void AppSettings::setHighlightItemsEnabled(bool val) { setValue(Config::HIGHLIGHT_ITEMS, val); }
bool AppSettings::isShowItemsEnabled() const { return getValue(Config::SHOW_ITEMS).toBool(); }
void AppSettings::setShowItemsEnabled(bool val) { setValue(Config::SHOW_ITEMS, val); }
bool AppSettings::isShowBlockingEnabled() const { return getValue(Config::SHOW_BLOCKING).toBool(); }
void AppSettings::setShowBlockingEnabled(bool val) { setValue(Config::SHOW_BLOCKING, val); }
bool AppSettings::isShowTooltipsEnabled() const { return getValue(Config::SHOW_TOOLTIPS).toBool(); }
void AppSettings::setShowTooltipsEnabled(bool val) { setValue(Config::SHOW_TOOLTIPS, val); }
bool AppSettings::isShowPreviewEnabled() const { return getValue(Config::SHOW_PREVIEW).toBool(); }
void AppSettings::setShowPreviewEnabled(bool val) { setValue(Config::SHOW_PREVIEW, val); }
bool AppSettings::isShowWallHooksEnabled() const { return getValue(Config::SHOW_WALL_HOOKS).toBool(); }
void AppSettings::setShowWallHooksEnabled(bool val) { setValue(Config::SHOW_WALL_HOOKS, val); }
bool AppSettings::isShowAsMinimapEnabled() const { return getValue(Config::SHOW_AS_MINIMAP).toBool(); }
void AppSettings::setShowAsMinimapEnabled(bool val) { setValue(Config::SHOW_AS_MINIMAP, val); }
bool AppSettings::isShowOnlyTileFlagsEnabled() const { return getValue(Config::SHOW_ONLY_TILEFLAGS).toBool(); }
void AppSettings::setShowOnlyTileFlagsEnabled(bool val) { setValue(Config::SHOW_ONLY_TILEFLAGS, val); }
bool AppSettings::isShowOnlyModifiedTilesEnabled() const { return getValue(Config::SHOW_ONLY_MODIFIED_TILES).toBool(); }
void AppSettings::setShowOnlyModifiedTilesEnabled(bool val) { setValue(Config::SHOW_ONLY_MODIFIED_TILES, val); }
bool AppSettings::isHideItemsWhenZoomedEnabled() const { return getValue(Config::HIDE_ITEMS_WHEN_ZOOMED).toBool(); }
void AppSettings::setHideItemsWhenZoomedEnabled(bool val) { setValue(Config::HIDE_ITEMS_WHEN_ZOOMED, val); }
bool AppSettings::isDrawLockedDoorEnabled() const { return getValue(Config::DRAW_LOCKED_DOOR).toBool(); }
void AppSettings::setDrawLockedDoorEnabled(bool val) { setValue(Config::DRAW_LOCKED_DOOR, val); }
bool AppSettings::isHighlightLockedDoorsEnabled() const { return getValue(Config::HIGHLIGHT_LOCKED_DOORS).toBool(); }
void AppSettings::setHighlightLockedDoorsEnabled(bool val) { setValue(Config::HIGHLIGHT_LOCKED_DOORS, val); }
bool AppSettings::isShowLightsEnabled() const { return getValue(Config::SHOW_LIGHTS).toBool(); }
void AppSettings::setShowLightsEnabled(bool val) { setValue(Config::SHOW_LIGHTS, val); }
bool AppSettings::isShowLightStrengthEnabled() const { return getValue(Config::SHOW_LIGHT_STR).toBool(); }
void AppSettings::setShowLightStrengthEnabled(bool val) { setValue(Config::SHOW_LIGHT_STR, val); }
bool AppSettings::isShowTechnicalItemsEnabled() const { return getValue(Config::SHOW_TECHNICAL_ITEMS).toBool(); }
void AppSettings::setShowTechnicalItemsEnabled(bool val) { setValue(Config::SHOW_TECHNICAL_ITEMS, val); }
bool AppSettings::isShowWaypointsEnabled() const { return getValue(Config::SHOW_WAYPOINTS).toBool(); }
void AppSettings::setShowWaypointsEnabled(bool val) { setValue(Config::SHOW_WAYPOINTS, val); }
bool AppSettings::isShowTownsEnabled() const { return getValue(Config::SHOW_TOWNS).toBool(); }
void AppSettings::setShowTownsEnabled(bool val) { setValue(Config::SHOW_TOWNS, val); }
bool AppSettings::isAlwaysShowZonesEnabled() const { return getValue(Config::ALWAYS_SHOW_ZONES).toBool(); }
void AppSettings::setAlwaysShowZonesEnabled(bool val) { setValue(Config::ALWAYS_SHOW_ZONES, val); }
bool AppSettings::isExternalHouseShaderEnabled() const { return getValue(Config::EXT_HOUSE_SHADER).toBool(); }
void AppSettings::setExternalHouseShaderEnabled(bool val) { setValue(Config::EXT_HOUSE_SHADER, val); }

// Editor Group (Most were in subset, adding remaining)
bool AppSettings::isGroupActionsEnabled() const { return getValue(Config::GROUP_ACTIONS).toBool(); }
void AppSettings::setGroupActionsEnabled(bool val) { setValue(Config::GROUP_ACTIONS, val); }
float AppSettings::getZoomSpeed() const { return getValue(Config::ZOOM_SPEED).toFloat(); }
void AppSettings::setZoomSpeed(float val) { setValue(Config::ZOOM_SPEED, val); }
int AppSettings::getUndoMemorySize() const { return getValue(Config::UNDO_MEM_SIZE).toInt(); }
void AppSettings::setUndoMemorySize(int val) { setValue(Config::UNDO_MEM_SIZE, val); }
bool AppSettings::isMergePasteEnabled() const { return getValue(Config::MERGE_PASTE).toBool(); }
void AppSettings::setMergePasteEnabled(bool val) { setValue(Config::MERGE_PASTE, val); }
int AppSettings::getSelectionType() const { return getValue(Config::SELECTION_TYPE).toInt(); }
void AppSettings::setSelectionType(int val) { setValue(Config::SELECTION_TYPE, val); }
bool AppSettings::isCompensatedSelectEnabled() const { return getValue(Config::COMPENSATED_SELECT).toBool(); }
void AppSettings::setCompensatedSelectEnabled(bool val) { setValue(Config::COMPENSATED_SELECT, val); }
bool AppSettings::isBorderIsGroundEnabled() const { return getValue(Config::BORDER_IS_GROUND).toBool(); }
void AppSettings::setBorderIsGroundEnabled(bool val) { setValue(Config::BORDER_IS_GROUND, val); }
bool AppSettings::isBorderizePasteEnabled() const { return getValue(Config::BORDERIZE_PASTE).toBool(); }
void AppSettings::setBorderizePasteEnabled(bool val) { setValue(Config::BORDERIZE_PASTE, val); }
bool AppSettings::isBorderizeDragEnabled() const { return getValue(Config::BORDERIZE_DRAG).toBool(); }
void AppSettings::setBorderizeDragEnabled(bool val) { setValue(Config::BORDERIZE_DRAG, val); }
int AppSettings::getBorderizeDragThreshold() const { return getValue(Config::BORDERIZE_DRAG_THRESHOLD).toInt(); }
void AppSettings::setBorderizeDragThreshold(int val) { setValue(Config::BORDERIZE_DRAG_THRESHOLD, val); }
int AppSettings::getBorderizePasteThreshold() const { return getValue(Config::BORDERIZE_PASTE_THRESHOLD).toInt(); }
void AppSettings::setBorderizePasteThreshold(int val) { setValue(Config::BORDERIZE_PASTE_THRESHOLD, val); }
bool AppSettings::isBorderizeDeleteEnabled() const { return getValue(Config::BORDERIZE_DELETE).toBool(); }
void AppSettings::setBorderizeDeleteEnabled(bool val) { setValue(Config::BORDERIZE_DELETE, val); }
bool AppSettings::isAlwaysMakeBackupEnabled() const { return getValue(Config::ALWAYS_MAKE_BACKUP).toBool(); }
void AppSettings::setAlwaysMakeBackupEnabled(bool val) { setValue(Config::ALWAYS_MAKE_BACKUP, val); }
bool AppSettings::isUseAutomagicEnabled() const { return getValue(Config::USE_AUTOMAGIC).toBool(); }
void AppSettings::setUseAutomagicEnabled(bool val) { setValue(Config::USE_AUTOMAGIC, val); }
bool AppSettings::isSameGroundTypeBorderEnabled() const { return getValue(Config::SAME_GROUND_TYPE_BORDER).toBool(); }
void AppSettings::setSameGroundTypeBorderEnabled(bool val) { setValue(Config::SAME_GROUND_TYPE_BORDER, val); }
bool AppSettings::isWallsRepelBordersEnabled() const { return getValue(Config::WALLS_REPEL_BORDERS).toBool(); }
void AppSettings::setWallsRepelBordersEnabled(bool val) { setValue(Config::WALLS_REPEL_BORDERS, val); }
bool AppSettings::isLayerCarpetsEnabled() const { return getValue(Config::LAYER_CARPETS).toBool(); }
void AppSettings::setLayerCarpetsEnabled(bool val) { setValue(Config::LAYER_CARPETS, val); }
bool AppSettings::isCustomBorderEnabled() const { return getValue(Config::CUSTOM_BORDER_ENABLED).toBool(); }
void AppSettings::setCustomBorderEnabled(bool val) { setValue(Config::CUSTOM_BORDER_ENABLED, val); }
int AppSettings::getCustomBorderID() const { return getValue(Config::CUSTOM_BORDER_ID).toInt(); }
void AppSettings::setCustomBorderID(int val) { setValue(Config::CUSTOM_BORDER_ID, val); }
bool AppSettings::isHouseBrushRemoveItemsEnabled() const { return getValue(Config::HOUSE_BRUSH_REMOVE_ITEMS).toBool(); }
void AppSettings::setHouseBrushRemoveItemsEnabled(bool val) { setValue(Config::HOUSE_BRUSH_REMOVE_ITEMS, val); }
bool AppSettings::isAutoAssignDoorIDEnabled() const { return getValue(Config::AUTO_ASSIGN_DOORID).toBool(); }
void AppSettings::setAutoAssignDoorIDEnabled(bool val) { setValue(Config::AUTO_ASSIGN_DOORID, val); }
bool AppSettings::isEraserLeaveUniqueEnabled() const { return getValue(Config::ERASER_LEAVE_UNIQUE).toBool(); }
void AppSettings::setEraserLeaveUniqueEnabled(bool val) { setValue(Config::ERASER_LEAVE_UNIQUE, val); }
bool AppSettings::isDoodadBrushEraseLikeEnabled() const { return getValue(Config::DOODAD_BRUSH_ERASE_LIKE).toBool(); }
void AppSettings::setDoodadBrushEraseLikeEnabled(bool val) { setValue(Config::DOODAD_BRUSH_ERASE_LIKE, val); }
bool AppSettings::isWarnForDuplicateIDEnabled() const { return getValue(Config::WARN_FOR_DUPLICATE_ID).toBool(); }
void AppSettings::setWarnForDuplicateIDEnabled(bool val) { setValue(Config::WARN_FOR_DUPLICATE_ID, val); }
bool AppSettings::isUseUpdaterEnabled() const { return getValue(Config::USE_UPDATER).toBool(); }
void AppSettings::setUseUpdaterEnabled(bool val) { setValue(Config::USE_UPDATER, val); }
bool AppSettings::isUseOTBM4ForAllMapsEnabled() const { return getValue(Config::USE_OTBM_4_FOR_ALL_MAPS).toBool(); }
void AppSettings::setUseOTBM4ForAllMapsEnabled(bool val) { setValue(Config::USE_OTBM_4_FOR_ALL_MAPS, val); }
bool AppSettings::isUseOTGZEnabled() const { return getValue(Config::USE_OTGZ).toBool(); }
void AppSettings::setUseOTGZEnabled(bool val) { setValue(Config::USE_OTGZ, val); }
bool AppSettings::isSaveWithOTBMagicNumberEnabled() const { return getValue(Config::SAVE_WITH_OTB_MAGIC_NUMBER).toBool(); }
void AppSettings::setSaveWithOTBMagicNumberEnabled(bool val) { setValue(Config::SAVE_WITH_OTB_MAGIC_NUMBER, val); }
int AppSettings::getReplaceSize() const { return getValue(Config::REPLACE_SIZE).toInt(); }
void AppSettings::setReplaceSize(int val) { setValue(Config::REPLACE_SIZE, val); }
int AppSettings::getMaxSpawnRadius() const { return getValue(Config::MAX_SPAWN_RADIUS).toInt(); }
void AppSettings::setMaxSpawnRadius(int val) { setValue(Config::MAX_SPAWN_RADIUS, val); }
int AppSettings::getCurrentSpawnRadius() const { return getValue(Config::CURRENT_SPAWN_RADIUS).toInt(); }
void AppSettings::setCurrentSpawnRadius(int val) { setValue(Config::CURRENT_SPAWN_RADIUS, val); }
bool AppSettings::isAutoCreateSpawnEnabled() const { return getValue(Config::AUTO_CREATE_SPAWN).toBool(); }
void AppSettings::setAutoCreateSpawnEnabled(bool val) { setValue(Config::AUTO_CREATE_SPAWN, val); }
int AppSettings::getDefaultSpawnTime() const { return getValue(Config::DEFAULT_SPAWNTIME).toInt(); }
void AppSettings::setDefaultSpawnTime(int val) { setValue(Config::DEFAULT_SPAWNTIME, val); }
bool AppSettings::areMouseButtonsSwitched() const { return getValue(Config::SWITCH_MOUSEBUTTONS).toBool(); }
void AppSettings::setMouseButtonsSwitched(bool val) { setValue(Config::SWITCH_MOUSEBUTTONS, val); }
bool AppSettings::isDoubleClickPropertiesEnabled() const { return getValue(Config::DOUBLECLICK_PROPERTIES).toBool(); }
void AppSettings::setDoubleClickPropertiesEnabled(bool val) { setValue(Config::DOUBLECLICK_PROPERTIES, val); }
bool AppSettings::isListboxEatsAllEventsEnabled() const { return getValue(Config::LISTBOX_EATS_ALL_EVENTS).toBool(); }
void AppSettings::setListboxEatsAllEventsEnabled(bool val) { setValue(Config::LISTBOX_EATS_ALL_EVENTS, val); }
bool AppSettings::isRawLikeSimoneEnabled() const { return getValue(Config::RAW_LIKE_SIMONE).toBool(); }
void AppSettings::setRawLikeSimoneEnabled(bool val) { setValue(Config::RAW_LIKE_SIMONE, val); }
int AppSettings::getCopyPositionFormat() const { return getValue(Config::COPY_POSITION_FORMAT).toInt(); }
void AppSettings::setCopyPositionFormat(int val) { setValue(Config::COPY_POSITION_FORMAT, val); }
bool AppSettings::isAutoSelectRawOnRightClickEnabled() const { return getValue(Config::AUTO_SELECT_RAW_ON_RIGHTCLICK).toBool(); }
void AppSettings::setAutoSelectRawOnRightClickEnabled(bool val) { setValue(Config::AUTO_SELECT_RAW_ON_RIGHTCLICK, val); }
bool AppSettings::isAutoSaveEnabled() const { return getValue(Config::AUTO_SAVE_ENABLED).toBool(); }
void AppSettings::setAutoSaveEnabled(bool val) { setValue(Config::AUTO_SAVE_ENABLED, val); }
int AppSettings::getAutoSaveInterval() const { return getValue(Config::AUTO_SAVE_INTERVAL).toInt(); }
void AppSettings::setAutoSaveInterval(int val) { setValue(Config::AUTO_SAVE_INTERVAL, val); }
QString AppSettings::getRecentFiles() const { return getValue(Config::RECENT_FILES).toString(); }
void AppSettings::setRecentFiles(const QString& val) { setValue(Config::RECENT_FILES, val); }
QString AppSettings::getRecentEditedMapPath() const { return getValue(Config::RECENT_EDITED_MAP_PATH).toString(); }
void AppSettings::setRecentEditedMapPath(const QString& val) { setValue(Config::RECENT_EDITED_MAP_PATH, val); }
QString AppSettings::getRecentEditedMapPosition() const { return getValue(Config::RECENT_EDITED_MAP_POSITION).toString(); }
void AppSettings::setRecentEditedMapPosition(const QString& val) { setValue(Config::RECENT_EDITED_MAP_POSITION, val); }
int AppSettings::getFindItemMode() const { return getValue(Config::FIND_ITEM_MODE).toInt(); }
void AppSettings::setFindItemMode(int val) { setValue(Config::FIND_ITEM_MODE, val); }
int AppSettings::getJumpToItemMode() const { return getValue(Config::JUMP_TO_ITEM_MODE).toInt(); }
void AppSettings::setJumpToItemMode(int val) { setValue(Config::JUMP_TO_ITEM_MODE, val); }

// UI Group
bool AppSettings::useLargeContainerIcons() const { return getValue(Config::USE_LARGE_CONTAINER_ICONS).toBool(); }
void AppSettings::setUseLargeContainerIcons(bool val) { setValue(Config::USE_LARGE_CONTAINER_ICONS, val); }
bool AppSettings::useLargeChooseItemIcons() const { return getValue(Config::USE_LARGE_CHOOSE_ITEM_ICONS).toBool(); }
void AppSettings::setUseLargeChooseItemIcons(bool val) { setValue(Config::USE_LARGE_CHOOSE_ITEM_ICONS, val); }
bool AppSettings::useLargeTerrainToolbar() const { return getValue(Config::USE_LARGE_TERRAIN_TOOLBAR).toBool(); }
void AppSettings::setUseLargeTerrainToolbar(bool val) { setValue(Config::USE_LARGE_TERRAIN_TOOLBAR, val); }
bool AppSettings::useLargeDoodadSizebar() const { return getValue(Config::USE_LARGE_DOODAD_SIZEBAR).toBool(); }
void AppSettings::setUseLargeDoodadSizebar(bool val) { setValue(Config::USE_LARGE_DOODAD_SIZEBAR, val); }
bool AppSettings::useLargeItemSizebar() const { return getValue(Config::USE_LARGE_ITEM_SIZEBAR).toBool(); }
void AppSettings::setUseLargeItemSizebar(bool val) { setValue(Config::USE_LARGE_ITEM_SIZEBAR, val); }
bool AppSettings::useLargeHouseSizebar() const { return getValue(Config::USE_LARGE_HOUSE_SIZEBAR).toBool(); }
void AppSettings::setUseLargeHouseSizebar(bool val) { setValue(Config::USE_LARGE_HOUSE_SIZEBAR, val); }
bool AppSettings::useLargeRawSizebar() const { return getValue(Config::USE_LARGE_RAW_SIZEBAR).toBool(); }
void AppSettings::setUseLargeRawSizebar(bool val) { setValue(Config::USE_LARGE_RAW_SIZEBAR, val); }
bool AppSettings::useGuiSelectionShadow() const { return getValue(Config::USE_GUI_SELECTION_SHADOW).toBool(); }
void AppSettings::setUseGuiSelectionShadow(bool val) { setValue(Config::USE_GUI_SELECTION_SHADOW, val); }
// PALETTE_COL_COUNT is in subset
QString AppSettings::getPaletteTerrainStyle() const { return getValue(Config::PALETTE_TERRAIN_STYLE).toString(); }
void AppSettings::setPaletteTerrainStyle(const QString& val) { setValue(Config::PALETTE_TERRAIN_STYLE, val); }
QString AppSettings::getPaletteDoodadStyle() const { return getValue(Config::PALETTE_DOODAD_STYLE).toString(); }
void AppSettings::setPaletteDoodadStyle(const QString& val) { setValue(Config::PALETTE_DOODAD_STYLE, val); }
QString AppSettings::getPaletteItemStyle() const { return getValue(Config::PALETTE_ITEM_STYLE).toString(); }
void AppSettings::setPaletteItemStyle(const QString& val) { setValue(Config::PALETTE_ITEM_STYLE, val); }
QString AppSettings::getPaletteRawStyle() const { return getValue(Config::PALETTE_RAW_STYLE).toString(); }
void AppSettings::setPaletteRawStyle(const QString& val) { setValue(Config::PALETTE_RAW_STYLE, val); }
QString AppSettings::getPaletteCollectionStyle() const { return getValue(Config::PALETTE_COLLECTION_STYLE).toString(); }
void AppSettings::setPaletteCollectionStyle(const QString& val) { setValue(Config::PALETTE_COLLECTION_STYLE, val); }
bool AppSettings::useLargeCollectionToolbar() const { return getValue(Config::USE_LARGE_COLLECTION_TOOLBAR).toBool(); }
void AppSettings::setUseLargeCollectionToolbar(bool val) { setValue(Config::USE_LARGE_COLLECTION_TOOLBAR, val); }

// Window Group
QString AppSettings::getPaletteLayout() const { return getValue(Config::PALETTE_LAYOUT).toString(); }
void AppSettings::setPaletteLayout(const QString& val) { setValue(Config::PALETTE_LAYOUT, val); }
bool AppSettings::isMinimapVisible() const { return getValue(Config::MINIMAP_VISIBLE).toBool(); }
void AppSettings::setMinimapVisible(bool val) { setValue(Config::MINIMAP_VISIBLE, val); }
QString AppSettings::getMinimapLayout() const { return getValue(Config::MINIMAP_LAYOUT).toString(); }
void AppSettings::setMinimapLayout(const QString& val) { setValue(Config::MINIMAP_LAYOUT, val); }
int AppSettings::getWindowHeight() const { return getValue(Config::WINDOW_HEIGHT).toInt(); }
void AppSettings::setWindowHeight(int val) { setValue(Config::WINDOW_HEIGHT, val); }
int AppSettings::getWindowWidth() const { return getValue(Config::WINDOW_WIDTH).toInt(); }
void AppSettings::setWindowWidth(int val) { setValue(Config::WINDOW_WIDTH, val); }
bool AppSettings::isWindowMaximized() const { return getValue(Config::WINDOW_MAXIMIZED).toBool(); }
void AppSettings::setWindowMaximized(bool val) { setValue(Config::WINDOW_MAXIMIZED, val); }
bool AppSettings::isWelcomeDialogEnabled() const { return getValue(Config::WELCOME_DIALOG).toBool(); }
void AppSettings::setWelcomeDialogEnabled(bool val) { setValue(Config::WELCOME_DIALOG, val); }
bool AppSettings::isShowToolbarStandardEnabled() const { return getValue(Config::SHOW_TOOLBAR_STANDARD).toBool(); }
void AppSettings::setShowToolbarStandardEnabled(bool val) { setValue(Config::SHOW_TOOLBAR_STANDARD, val); }
bool AppSettings::isShowToolbarBrushesEnabled() const { return getValue(Config::SHOW_TOOLBAR_BRUSHES).toBool(); }
void AppSettings::setShowToolbarBrushesEnabled(bool val) { setValue(Config::SHOW_TOOLBAR_BRUSHES, val); }
bool AppSettings::isShowToolbarPositionEnabled() const { return getValue(Config::SHOW_TOOLBAR_POSITION).toBool(); }
void AppSettings::setShowToolbarPositionEnabled(bool val) { setValue(Config::SHOW_TOOLBAR_POSITION, val); }
bool AppSettings::isShowToolbarSizesEnabled() const { return getValue(Config::SHOW_TOOLBAR_SIZES).toBool(); }
void AppSettings::setShowToolbarSizesEnabled(bool val) { setValue(Config::SHOW_TOOLBAR_SIZES, val); }
QString AppSettings::getToolbarStandardLayout() const { return getValue(Config::TOOLBAR_STANDARD_LAYOUT).toString(); }
void AppSettings::setToolbarStandardLayout(const QString& val) { setValue(Config::TOOLBAR_STANDARD_LAYOUT, val); }
QString AppSettings::getToolbarBrushesLayout() const { return getValue(Config::TOOLBAR_BRUSHES_LAYOUT).toString(); }
void AppSettings::setToolbarBrushesLayout(const QString& val) { setValue(Config::TOOLBAR_BRUSHES_LAYOUT, val); }
QString AppSettings::getToolbarPositionLayout() const { return getValue(Config::TOOLBAR_POSITION_LAYOUT).toString(); }
void AppSettings::setToolbarPositionLayout(const QString& val) { setValue(Config::TOOLBAR_POSITION_LAYOUT, val); }
QString AppSettings::getToolbarSizesLayout() const { return getValue(Config::TOOLBAR_SIZES_LAYOUT).toString(); }
void AppSettings::setToolbarSizesLayout(const QString& val) { setValue(Config::TOOLBAR_SIZES_LAYOUT, val); }

// Hotkeys Group
QString AppSettings::getNumericalHotkeys() const { return getValue(Config::NUMERICAL_HOTKEYS).toString(); }
void AppSettings::setNumericalHotkeys(const QString& val) { setValue(Config::NUMERICAL_HOTKEYS, val); }

// Network Group (Subset already done)
QString AppSettings::getLivePassword() const { return getValue(Config::LIVE_PASSWORD).toString(); }
void AppSettings::setLivePassword(const QString& val) { setValue(Config::LIVE_PASSWORD, val); }
QString AppSettings::getLiveUsername() const { return getValue(Config::LIVE_USERNAME).toString(); }
void AppSettings::setLiveUsername(const QString& val) { setValue(Config::LIVE_USERNAME, val); }

// Interface Group (Dark Mode)
bool AppSettings::isDarkModeEnabled() const { return getValue(Config::DARK_MODE).toBool(); }
void AppSettings::setDarkModeEnabled(bool val) { setValue(Config::DARK_MODE, val); }
bool AppSettings::isDarkModeCustomColorEnabled() const { return getValue(Config::DARK_MODE_CUSTOM_COLOR).toBool(); }
void AppSettings::setDarkModeCustomColorEnabled(bool val) { setValue(Config::DARK_MODE_CUSTOM_COLOR, val); }
int AppSettings::getDarkModeRed() const { return getValue(Config::DARK_MODE_RED).toInt(); }
void AppSettings::setDarkModeRed(int val) { setValue(Config::DARK_MODE_RED, val); }
int AppSettings::getDarkModeGreen() const { return getValue(Config::DARK_MODE_GREEN).toInt(); }
void AppSettings::setDarkModeGreen(int val) { setValue(Config::DARK_MODE_GREEN, val); }
int AppSettings::getDarkModeBlue() const { return getValue(Config::DARK_MODE_BLUE).toInt(); }
void AppSettings::setDarkModeBlue(int val) { setValue(Config::DARK_MODE_BLUE, val); }

// HouseCreation Group
int AppSettings::getMaxHouseTiles() const { return getValue(Config::MAX_HOUSE_TILES).toInt(); }
void AppSettings::setMaxHouseTiles(int val) { setValue(Config::MAX_HOUSE_TILES, val); }
bool AppSettings::isHouseFloorScanEnabled() const { return getValue(Config::HOUSE_FLOOR_SCAN).toBool(); }
void AppSettings::setHouseFloorScanEnabled(bool val) { setValue(Config::HOUSE_FLOOR_SCAN, val); }
bool AppSettings::isAutoDetectHouseExitEnabled() const { return getValue(Config::AUTO_DETECT_HOUSE_EXIT).toBool(); }
void AppSettings::setAutoDetectHouseExitEnabled(bool val) { setValue(Config::AUTO_DETECT_HOUSE_EXIT, val); }

// LOD Group
int AppSettings::getTooltipMaxZoom() const { return getValue(Config::TOOLTIP_MAX_ZOOM).toInt(); }
void AppSettings::setTooltipMaxZoom(int val) { setValue(Config::TOOLTIP_MAX_ZOOM, val); }
int AppSettings::getGroundOnlyZoomThreshold() const { return getValue(Config::GROUND_ONLY_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setGroundOnlyZoomThreshold(int val) { setValue(Config::GROUND_ONLY_ZOOM_THRESHOLD, val); }
int AppSettings::getItemDisplayZoomThreshold() const { return getValue(Config::ITEM_DISPLAY_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setItemDisplayZoomThreshold(int val) { setValue(Config::ITEM_DISPLAY_ZOOM_THRESHOLD, val); }
int AppSettings::getSpecialFeaturesZoomThreshold() const { return getValue(Config::SPECIAL_FEATURES_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setSpecialFeaturesZoomThreshold(int val) { setValue(Config::SPECIAL_FEATURES_ZOOM_THRESHOLD, val); }
int AppSettings::getAnimationZoomThreshold() const { return getValue(Config::ANIMATION_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setAnimationZoomThreshold(int val) { setValue(Config::ANIMATION_ZOOM_THRESHOLD, val); }
int AppSettings::getEffectsZoomThreshold() const { return getValue(Config::EFFECTS_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setEffectsZoomThreshold(int val) { setValue(Config::EFFECTS_ZOOM_THRESHOLD, val); }
int AppSettings::getLightZoomThreshold() const { return getValue(Config::LIGHT_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setLightZoomThreshold(int val) { setValue(Config::LIGHT_ZOOM_THRESHOLD, val); }
int AppSettings::getShadeZoomThreshold() const { return getValue(Config::SHADE_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setShadeZoomThreshold(int val) { setValue(Config::SHADE_ZOOM_THRESHOLD, val); }
int AppSettings::getTownZoneZoomThreshold() const { return getValue(Config::TOWN_ZONE_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setTownZoneZoomThreshold(int val) { setValue(Config::TOWN_ZONE_ZOOM_THRESHOLD, val); }
int AppSettings::getGridZoomThreshold() const { return getValue(Config::GRID_ZOOM_THRESHOLD).toInt(); }
void AppSettings::setGridZoomThreshold(int val) { setValue(Config::GRID_ZOOM_THRESHOLD, val); }

// PaletteGrid Group
int AppSettings::getGridChunkSize() const { return getValue(Config::GRID_CHUNK_SIZE).toInt(); }
void AppSettings::setGridChunkSize(int val) { setValue(Config::GRID_CHUNK_SIZE, val); }
int AppSettings::getGridVisibleRowsMargin() const { return getValue(Config::GRID_VISIBLE_ROWS_MARGIN).toInt(); }
void AppSettings::setGridVisibleRowsMargin(int val) { setValue(Config::GRID_VISIBLE_ROWS_MARGIN, val); }

// Misc/Root Level
bool AppSettings::isGoToWebsiteOnBootEnabled() const { return getValue(Config::GOTO_WEBSITE_ON_BOOT).toBool(); }
void AppSettings::setGoToWebsiteOnBootEnabled(bool val) { setValue(Config::GOTO_WEBSITE_ON_BOOT, val); }
bool AppSettings::isIndirectoryInstallation() const { return getValue(Config::INDIRECTORY_INSTALLATION).toBool(); }
void AppSettings::setIndirectoryInstallation(bool val) { setValue(Config::INDIRECTORY_INSTALLATION, val); }
bool AppSettings::isAutoCheckForUpdatesEnabled() const { return getValue(Config::AUTOCHECK_FOR_UPDATES).toBool(); }
void AppSettings::setAutoCheckForUpdatesEnabled(bool val) { setValue(Config::AUTOCHECK_FOR_UPDATES, val); }
bool AppSettings::isOnlyOneInstanceEnabled() const { return getValue(Config::ONLY_ONE_INSTANCE).toBool(); }
void AppSettings::setOnlyOneInstanceEnabled(bool val) { setValue(Config::ONLY_ONE_INSTANCE, val); }
int AppSettings::getLastWebsitesOpenTime() const { return getValue(Config::LAST_WEBSITES_OPEN_TIME).toInt(); }
void AppSettings::setLastWebsitesOpenTime(int val) { setValue(Config::LAST_WEBSITES_OPEN_TIME, val); }

// Already in subset: TransparentFloors, ShowGrid, DataDirectory, ScrollSpeed, UndoSize, TextureManagement, PaletteColCount, LiveHost, LivePort

} // namespace RME
