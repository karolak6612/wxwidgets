#ifndef RME_IMAP_IO_H
#define RME_IMAP_IO_H

#include <QString>
#include <QStringList>

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class AssetManager; // From core/assets
    class AppSettings;  // From core/settings
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace io {

/**
 * @brief Interface for map loading and saving operations.
 *
 * This interface defines a contract for classes that can read map data from
 * a file into a Map object, or save a Map object's data to a file in a
 * specific format.
 */
class IMapIO {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IMapIO() = default;

    /**
     * @brief Loads map data from the specified file path into the given Map object.
     * @param filePath The path to the map file.
     * @param map The Map object to populate.
     * @param assetManager Reference to the AssetManager for item/creature data validation.
     * @param settings Reference to the AppSettings for any I/O related settings.
     * @return True if loading was successful, false otherwise.
     */
    virtual bool loadMap(const QString& filePath, Map& map, AssetManager& assetManager, AppSettings& settings) = 0;

    /**
     * @brief Saves map data from the given Map object to the specified file path.
     * @param filePath The path to save the map file to.
     * @param map The Map object to read data from.
     * @param assetManager Reference to the AssetManager (e.g., for item type information).
     * @param settings Reference to the AppSettings for any I/O related settings.
     * @return True if saving was successful, false otherwise.
     */
    virtual bool saveMap(const QString& filePath, const Map& map, AssetManager& assetManager, AppSettings& settings) = 0;

    /**
     * @brief Gets a list of file extensions supported by this map I/O handler.
     * @return A QStringList of supported extensions (e.g., {"*.otbm", "*.otm"}).
     */
    virtual QStringList getSupportedFileExtensions() const = 0;

    /**
     * @brief Gets the human-readable name of the map format handled by this class.
     * @return A QString representing the format name (e.g., "Open Tibia Binary Map").
     */
    virtual QString getFormatName() const = 0;
};

} // namespace io
} // namespace core
} // namespace RME

#endif // RME_IMAP_IO_H
