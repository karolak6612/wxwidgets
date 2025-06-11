#ifndef RME_MATERIAL_MANAGER_H
#define RME_MATERIAL_MANAGER_H

#include "core/assets/MaterialData.h" // Needs full definition
#include <QMap>
#include <QString>
#include <QSet> // For tracking processed includes

// Forward declaration for AssetManager to break potential include cycle
namespace RME {
namespace core {
namespace assets {
    class AssetManager; // Forward declared, full definition in AssetManager.h
} // namespace assets
} // namespace core
} // namespace RME

// Forward declaration for QXmlStreamReader
QT_BEGIN_NAMESPACE
class QXmlStreamReader;
QT_END_NAMESPACE

namespace RME {
namespace core {
namespace assets {

/**
 * @brief Manages loading, storage, and access of material definitions from XML files.
 *
 * The MaterialManager is responsible for parsing the material XML files (e.g., materials.xml)
 * including handling <include> directives to load definitions from multiple files.
 * It stores the parsed MaterialData objects and provides methods to retrieve them.
 */
class MaterialManager {
public:
    /**
     * @brief Default constructor.
     */
    MaterialManager();

    /**
     * @brief Destructor.
     */
    ~MaterialManager();

    // Prevent copying and assignment
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    /**
     * @brief Loads all materials starting from a main XML file within a specified directory.
     * This method handles <include> directives to parse multiple related XML files recursively.
     * @param directoryPath The base directory where material XML files are located.
     * @param mainFileName The name of the root material XML file (e.g., "materials.xml").
     * @param assetManager Reference to the AssetManager, which might be used for validating
     *                     item IDs referenced within materials or for other cross-asset lookups.
     * @return True if loading was successful (at least the main file was parsed without fatal errors),
     *         false otherwise (e.g., main file not found or major parsing error).
     */
    bool loadMaterialsFromDirectory(const QString& directoryPath, const QString& mainFileName, AssetManager& assetManager);

    /**
     * @brief Retrieves a material definition by its unique ID (brush name).
     * @param id The unique ID (name attribute from <brush name="...">) of the material.
     * @return A const pointer to the MaterialData if found, otherwise nullptr.
     */
    const MaterialData* getMaterial(const QString& id) const;

    /**
     * @brief Gets all loaded materials.
     * @return A const reference to the QMap storing all materials, keyed by their ID.
     */
    const QMap<QString, MaterialData>& getAllMaterials() const { return m_materialsById; }

    /**
     * @brief Gets the last error message encountered during loading.
     * @return QString The last error message.
     */
    QString getLastError() const { return m_lastError; }

private:
    /**
     * @brief Parses a single material XML file.
     * This method is called recursively to handle <include> directives.
     * @param xmlFilePath Absolute path to the XML file to parse.
     * @param directoryPath Base directory of XMLs, used for resolving relative paths in <include> tags.
     * @param assetManager Reference to AssetManager for potential validation or lookups.
     * @param processedIncludes A set of already processed absolute file paths to prevent circular includes.
     */
    void parseMaterialFile(const QString& xmlFilePath, const QString& directoryPath, AssetManager& assetManager, QSet<QString>& processedIncludes);

    // --- Helper methods to parse specific XML elements into MaterialData fields ---

    /**
     * @brief Parses a <brush> XML element and its children into a MaterialData object.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of a <brush> element.
     * @param materialData The MaterialData object to populate.
     * @param assetManager Reference to AssetManager.
     */
    void parseBrushElement(QXmlStreamReader& reader, MaterialData& materialData, AssetManager& assetManager);

    /**
     * @brief Parses an <item> child element of a <brush> or <wall> element.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of an <item> element.
     * @param materialData The MaterialData object to add the primary item to (if child of <brush>).
     * @param wallPart Optional: Pointer to a MaterialWallPart if this item is part of a wall.
     */
    void parseItemChild(QXmlStreamReader& reader, MaterialData& materialData, MaterialWallPart* wallPart = nullptr);

    /**
     * @brief Parses a <border> child element of a <brush> element.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of a <border> element.
     * @param materialData The MaterialData object to add the border definition to.
     */
    void parseBorderChild(QXmlStreamReader& reader, MaterialData& materialData);

    /**
     * @brief Parses a <friend name="..."/> child element of a <brush> element.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of a <friend> element.
     * @param materialData The MaterialData object to add the friend material name to.
     */
    void parseFriendChild(QXmlStreamReader& reader, MaterialData& materialData);

    /**
     * @brief Parses an <optional id="..."/> child element of a <brush> element.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of an <optional> element.
     * @param materialData The MaterialData object to add the optional border set ID to.
     */
    void parseOptionalChild(QXmlStreamReader& reader, MaterialData& materialData);

    /**
     * @brief Parses a <wall type="..."> child element of a <brush type="wall"> element.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of a <wall> element.
     * @param materialData The MaterialData object (must be of brushType "wall").
     */
    void parseWallChild(QXmlStreamReader& reader, MaterialData& materialData);

    /**
     * @brief Parses a <door .../> child element of a <wall> element.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of a <door> element.
     * @param wallPart The MaterialWallPart object to add the door definition to.
     */
    void parseDoorChild(QXmlStreamReader& reader, MaterialWallPart& wallPart);

    /**
     * @brief Parses a <composite> child element of a <brush type="doodad"> element.
     * @param reader Reference to the QXmlStreamReader, positioned at the start of a <composite> element.
     * @param materialData The MaterialData object (must be of brushType "doodad").
     */
    void parseCompositeChild(QXmlStreamReader& reader, MaterialData& materialData);


    QMap<QString, MaterialData> m_materialsById; ///< Stores all loaded materials, keyed by their unique ID.
    QString m_lastError;                         ///< For storing error messages from parsing.
};

} // namespace assets
} // namespace core
} // namespace RME

#endif // RME_MATERIAL_MANAGER_H
