#ifndef RME_XML_BRUSH_IO_H
#define RME_XML_BRUSH_IO_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace RME {
namespace ui {
namespace dialogs {

// Forward declarations
enum class BorderPosition;
struct BorderItem;
struct GroundItem;
struct WallItem;
struct DoodadItem;

// Border data structure
struct BorderData {
    int id = 0;
    QString name;
    int groupId = 0;
    bool optional = false;
    bool isGroundBorder = false;
    QMap<BorderPosition, uint16_t> items;
    
    bool isValid() const { return id > 0 && !name.isEmpty(); }
};

// Ground brush data structure
struct GroundBrushData {
    QString name;
    QString type = "ground";
    uint16_t serverLookId = 0;
    int zOrder = 0;
    QVector<QPair<uint16_t, int>> items; // itemId, chance
    
    // Border association
    int borderAssocId = 0;
    QString borderAlignment = "outer";
    bool includeToNone = false;
    bool includeInner = false;
    
    bool isValid() const { return !name.isEmpty() && !items.isEmpty(); }
};

// Wall brush data structure
struct WallBrushData {
    QString name;
    QString type = "wall";
    uint16_t serverLookId = 0;
    
    // Basic wall items
    uint16_t horizontalWall = 0;
    uint16_t verticalWall = 0;
    uint16_t wallPole = 0;
    
    // TODO: Extend for full wall system
    QMap<QString, QVector<uint16_t>> wallTypes; // "horizontal", "vertical", etc.
    
    bool isValid() const { return !name.isEmpty(); }
};

// Doodad brush data structure
struct DoodadBrushData {
    QString name;
    QString type = "doodad";
    uint16_t serverLookId = 0;
    bool draggable = false;
    bool blocking = false;
    
    struct DoodadItem {
        uint16_t itemId = 0;
        int xOffset = 0;
        int yOffset = 0;
        int zOffset = 0;
    };
    
    QVector<DoodadItem> items;
    
    bool isValid() const { return !name.isEmpty(); }
};

// Tileset data structure
struct TilesetData {
    QString name;
    QVector<uint16_t> items;
    QVector<QString> brushes;
    QString category; // "terrain", "doodad", "items", etc.
};

/**
 * @brief XML I/O utility class for brush and material data
 * 
 * This class handles reading and writing of XML files for borders, ground brushes,
 * wall brushes, doodad brushes, and tilesets.
 */
class XmlBrushIO {
public:
    XmlBrushIO() = default;
    ~XmlBrushIO() = default;
    
    // Border operations
    static QVector<BorderData> loadBorders(const QString& filePath);
    static bool saveBorders(const QString& filePath, const QVector<BorderData>& borders);
    static BorderData loadBorderById(const QString& filePath, int borderId);
    static bool saveBorder(const QString& filePath, const BorderData& border);
    
    // Ground brush operations
    static QVector<GroundBrushData> loadGroundBrushes(const QString& filePath);
    static bool saveGroundBrushes(const QString& filePath, const QVector<GroundBrushData>& brushes);
    static GroundBrushData loadGroundBrushByName(const QString& filePath, const QString& name);
    static bool saveGroundBrush(const QString& filePath, const GroundBrushData& brush);
    
    // Wall brush operations
    static QVector<WallBrushData> loadWallBrushes(const QString& filePath);
    static bool saveWallBrushes(const QString& filePath, const QVector<WallBrushData>& brushes);
    static WallBrushData loadWallBrushByName(const QString& filePath, const QString& name);
    static bool saveWallBrush(const QString& filePath, const WallBrushData& brush);
    
    // Doodad brush operations
    static QVector<DoodadBrushData> loadDoodadBrushes(const QString& filePath);
    static bool saveDoodadBrushes(const QString& filePath, const QVector<DoodadBrushData>& brushes);
    static DoodadBrushData loadDoodadBrushByName(const QString& filePath, const QString& name);
    static bool saveDoodadBrush(const QString& filePath, const DoodadBrushData& brush);
    
    // Tileset operations
    static QVector<TilesetData> loadTilesets(const QString& filePath);
    static bool saveTilesets(const QString& filePath, const QVector<TilesetData>& tilesets);
    static bool addBrushToTileset(const QString& filePath, const QString& tilesetName, const QString& brushName);
    static bool addItemToTileset(const QString& filePath, const QString& tilesetName, uint16_t itemId);
    
    // Utility methods
    static QString getDefaultXmlPath(const QString& filename);
    static bool backupFile(const QString& filePath);
    static QString borderPositionToString(BorderPosition pos);
    static BorderPosition borderPositionFromString(const QString& str);

private:
    // Internal parsing methods
    static BorderData parseBorderElement(QXmlStreamReader& xml);
    static GroundBrushData parseGroundBrushElement(QXmlStreamReader& xml);
    static WallBrushData parseWallBrushElement(QXmlStreamReader& xml);
    static DoodadBrushData parseDoodadBrushElement(QXmlStreamReader& xml);
    static TilesetData parseTilesetElement(QXmlStreamReader& xml);
    
    // Internal writing methods
    static void writeBorderElement(QXmlStreamWriter& xml, const BorderData& border);
    static void writeGroundBrushElement(QXmlStreamWriter& xml, const GroundBrushData& brush);
    static void writeWallBrushElement(QXmlStreamWriter& xml, const WallBrushData& brush);
    static void writeDoodadBrushElement(QXmlStreamWriter& xml, const DoodadBrushData& brush);
    static void writeTilesetElement(QXmlStreamWriter& xml, const TilesetData& tileset);
    
    // Error handling
    static QString m_lastError;
    
public:
    static QString getLastError() { return m_lastError; }
    static void setLastError(const QString& error) { m_lastError = error; }
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_XML_BRUSH_IO_H