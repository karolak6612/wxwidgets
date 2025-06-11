#ifndef RME_MATERIALMANAGER_H
#define RME_MATERIALMANAGER_H

#include "core/assets/MaterialData.h"
#include <QMap>
#include <QString>
#include <QXmlStreamReader> // For private method declaration

// Forward declare AssetManager to break potential circular dependency / reduce header load
// AssetManager will include MaterialManager.h for its member.
// MaterialManager needs AssetManager& for context during loading (e.g., item ID validation).
namespace RME {
namespace core {
namespace assets {
    class AssetManager;
} // namespace assets
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace assets {

class MaterialManager {
public:
    MaterialManager();
    ~MaterialManager();

    /**
     * @brief Loads materials from a main XML file and any files it includes.
     * @param baseDir The base directory from which relative paths in include tags are resolved.
     * @param mainXmlFile The name of the main materials XML file (e.g., "materials.xml").
     * @param assetManager Reference to the asset manager for context (e.g., item validation).
     * @return True if loading was successful (or partially successful with warnings), false on critical failure.
     */
    bool loadMaterialsFromDirectory(const QString& baseDir, const QString& mainXmlFile, AssetManager& assetManager);

    /**
     * @brief Retrieves a material by its ID (brush name).
     * @param id The unique ID (name) of the material.
     * @return Const pointer to MaterialData if found, nullptr otherwise.
     */
    const MaterialData* getMaterial(const QString& id) const;

    /**
     * @brief Gets a map of all loaded materials, keyed by their ID.
     * @return Const reference to the QMap of materials.
     */
    const QMap<QString, MaterialData>& getAllMaterials() const;

    /**
     * @brief Gets the last error message from loading.
     * @return QString containing the error message, or empty if no error.
     */
    QString getLastError() const { return m_lastError; }

private:
    QMap<QString, MaterialData> m_materialsById;
    QString m_lastError;
    QSet<QString> m_parsedFiles; // To prevent circular includes and redundant parsing

    /**
     * @brief Parses a single XML file for material definitions.
     * @param filePath Full path to the XML file.
     * @param assetManager Reference to the asset manager.
     * @param currentDir The directory of the currently parsed file, for resolving relative includes.
     * @return True on success, false on critical failure for this file.
     */
    bool parseXmlFile(const QString& filePath, AssetManager& assetManager, const QString& currentDir);

    /**
     * @brief Parses a <brush> XML element and its children.
     * @param xml The QXmlStreamReader positioned at the start of a <brush> element.
     * @param assetManager Reference to the asset manager.
     * @param currentDir The directory of the currently parsed file for resolving any further relative paths if needed.
     * @return True if parsing this brush was successful, false otherwise.
     */
    bool parseBrushElement(QXmlStreamReader& xml, AssetManager& assetManager, const QString& currentDir);

    // --- Private helper methods for parsing specific parts of a <brush> element ---
    // These will populate the MaterialData object passed to them.

    void parseBrushItems(QXmlStreamReader& xml, MaterialData& materialData);
    void parseBrushBorders(QXmlStreamReader& xml, MaterialData& materialData);
    void parseBrushFriends(QXmlStreamReader& xml, MaterialData& materialData);
    void parseBrushOptionals(QXmlStreamReader& xml, MaterialData& materialData);
    void parseBrushWallParts(QXmlStreamReader& xml, MaterialData& materialData);
    void parseBrushAlternates(QXmlStreamReader& xml, MaterialData& materialData);
    void parseBrushCarpetParts(QXmlStreamReader& xml, MaterialData& materialData); // Also for tables
    // void parseBrushTableParts(QXmlStreamReader& xml, MaterialData& materialData); // Covered by CarpetParts

    // Helper to parse a <tile> element within a <composite>
    void parseCompositeTile(QXmlStreamReader& xml, MaterialCompositeTile& compositeTile);
};

} // namespace assets
} // namespace core
} // namespace RME

#endif // RME_MATERIALMANAGER_H
