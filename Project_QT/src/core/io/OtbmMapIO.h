#ifndef RME_OTBM_MAP_IO_H
#define RME_OTBM_MAP_IO_H

#include "core/io/IMapIO.h"
#include "core/io/otbm_constants.h" // For node type constants
#include "core/navigation/WaypointData.h" // New WaypointData location
#include "core/world/TownData.h" // Added for Town I/O
#include <QVariantMap> // For passing attributes around if needed
#include <QByteArray>  // For compress/decompress helpers

// Forward declarations
// namespace RME { namespace core { class Map; class AssetManager; class AppSettings; }} // Already in IMapIO.h
namespace RME {
namespace core {
namespace io {
    class BinaryNode;
    class NodeFileReadHandle;
    class NodeFileWriteHandle; // Added for save operations
} // namespace io
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace io {

/**
 * @brief Implements IMapIO for the Open Tibia Binary Map (OTBM) format.
 *
 * This class handles the loading and saving of map data in the OTBM format,
 * which is a common binary format used by Open Tibia servers and editors.
 * It uses NodeFileReadHandle and NodeFileWriteHandle (typically their disk-based
 * implementations) to process the underlying OTBM node structure.
 */
class OtbmMapIO : public IMapIO {
public:
    /**
     * @brief Default constructor.
     */
    OtbmMapIO();

    /**
     * @brief Destructor.
     */
    ~OtbmMapIO() override;

    // --- IMapIO interface implementation ---

    /**
     * @brief Loads map data from an OTBM file into the provided Map object.
     * @param filePath The path to the OTBM map file.
     * @param map The Map object to populate with loaded data.
     * @param assetManager Reference to the AssetManager for validating item and creature data.
     * @param settings Reference to the AppSettings for any I/O related settings (e.g., version compatibility).
     * @return True if loading was successful, false otherwise. Sets m_lastError on failure.
     */
    bool loadMap(const QString& filePath, Map& map, AssetManager& assetManager, AppSettings& settings) override;

    /**
     * @brief Saves map data from the given Map object to an OTBM file.
     * @param filePath The path to save the OTBM map file to.
     * @param map The Map object containing the data to save.
     * @param assetManager Reference to the AssetManager (e.g., for item type information during serialization).
     * @param settings Reference to the AppSettings for any I/O related settings.
     * @return True if saving was successful, false otherwise. Sets m_lastError on failure.
     */
    bool saveMap(const QString& filePath, const Map& map, AssetManager& assetManager, AppSettings& settings) override;

    /**
     * @brief Gets a list of file extensions supported by this map I/O handler.
     * @return A QStringList containing "*.otbm".
     */
    QStringList getSupportedFileExtensions() const override;

    /**
     * @brief Gets the human-readable name of the map format handled by this class.
     * @return A QString "Open Tibia Binary Map".
     */
    QString getFormatName() const override;

    /**
     * @brief Gets the last error message encountered during loading or saving.
     * @return QString The last error message.
     */
    QString getLastError() const { return m_lastError; }

private:
    // --- Helper methods for loading ---

    /**
     * @brief Parses the main map data node (OTBM_NODE_MAP_DATA).
     * Extracts map attributes and then processes child nodes like tile areas.
     * @param mapDataNode The BinaryNode representing map data.
     * @param map The Map object to populate.
     * @param assetManager For item validation.
     * @param settings Application settings.
     * @return True on success, false on failure.
     */
    bool parseMapDataNode(BinaryNode* mapDataNode, Map& map, AssetManager& assetManager, AppSettings& settings);

    /**
     * @brief Parses a tile area node (OTBM_NODE_TILE_AREA).
     * Extracts the base position for the area and processes child tile nodes.
     * @param tileAreaNode The BinaryNode for the tile area.
     * @param map The Map object to populate.
     * @param assetManager For item validation.
     * @param settings Application settings.
     * @return True on success, false on failure.
     */
    bool parseTileAreaNode(BinaryNode* tileAreaNode, Map& map, AssetManager& assetManager, AppSettings& settings);

    /**
     * @brief Parses a single tile node (OTBM_NODE_TILE or OTBM_NODE_HOUSETILE).
     * Extracts tile position, attributes, and processes child item/creature nodes.
     * @param tileNode The BinaryNode for the tile.
     * @param map The Map object.
     * @param assetManager For item validation.
     * @param areaBasePos The base position of the current tile area.
     * @param settings Application settings.
     * @return True on success, false on failure.
     */
    bool parseTileNode(BinaryNode* tileNode, Map& map, AssetManager& assetManager, const Position& areaBasePos, AppSettings& settings);

    /**
     * @brief Parses an item node (OTBM_NODE_ITEM).
     * Extracts item ID and attributes, creates the item, and adds it to the tile.
     * @param itemNode The BinaryNode for the item.
     * @param tile The Tile object to add the item to.
     * @param assetManager For item validation and creation.
     * @param settings Application settings.
     * @return True on success, false on failure.
     */
    bool parseItemNode(BinaryNode* itemNode, Tile* tile, AssetManager& assetManager, AppSettings& settings);
    // Town Data Parsing
    bool parseTownsContainerNode(BinaryNode* containerNode, Map& map, AssetManager& assetManager, AppSettings& settings);
    bool parseTownNode(BinaryNode* townNode, Map& map, AssetManager& assetManager, AppSettings& settings);
    // Waypoint Data Parsing
    bool parseWaypointsContainerNode(BinaryNode* containerNode, Map& map, AssetManager& assetManager, AppSettings& settings);
    bool parseWaypointNode(BinaryNode* waypointNode, Map& map, AssetManager& assetManager, AppSettings& settings);
    // TODO: Add parseCreatureNode, parseSpawnNode etc.

    // --- Helper methods for saving (declarations) ---
    bool serializeMapDataNode(NodeFileWriteHandle& writer, const Map& map, AssetManager& assetManager, AppSettings& settings);
    bool serializeTileAreaNode(NodeFileWriteHandle& writer, const Map& map,
                                   const Position& areaBasePos,
                                   AssetManager& assetManager, AppSettings& settings);
    bool serializeTileNode(NodeFileWriteHandle& writer, const Tile* tile, AssetManager& assetManager, AppSettings& settings);
    bool serializeItemNode(NodeFileWriteHandle& writer, const Item* item, AssetManager& assetManager, AppSettings& settings);
    // Town Data Serialization
    bool serializeTownsContainerNode(NodeFileWriteHandle& writer, const Map& map, AssetManager& assetManager, AppSettings& settings);
    bool serializeTownNode(NodeFileWriteHandle& writer, const RME::core::world::TownData& town, AssetManager& assetManager, AppSettings& settings);
    // Waypoint Data Serialization
    bool serializeWaypointsContainerNode(NodeFileWriteHandle& writer, const Map& map, AssetManager& assetManager, AppSettings& settings);
    bool serializeWaypointNode(NodeFileWriteHandle& writer, const RME::core::navigation::WaypointData& waypoint, AssetManager& assetManager, AppSettings& settings);
    // TODO: Add serializeCreatureNode, serializeSpawnNode etc.

    // --- Helper for zlib compression/decompression ---
    /**
     * @brief Decompresses data using qUncompress.
     * @param compressedData The byte vector containing compressed data.
     * @return QByteArray The decompressed data. Returns empty on error.
     */
    QByteArray decompressNodeData(const std::vector<uint8_t>& compressedData);

    /**
     * @brief Compresses data using qCompress.
     * @param uncompressedData The QByteArray containing uncompressed data.
     * @return std::vector<uint8_t> The compressed data. Returns empty on error.
     */
    std::vector<uint8_t> compressNodeData(const QByteArray& uncompressedData);

    QString m_lastError; ///< Stores the last error message encountered.
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_OTBM_MAP_IO_H
