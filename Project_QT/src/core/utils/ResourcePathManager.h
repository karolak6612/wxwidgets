#pragma once

#include <QString>
#include <QStringList>
#include <QMap>

namespace RME {
namespace core {
namespace utils {

/**
 * @brief Centralized resource path management system
 * 
 * This class handles path resolution for various resource types
 * across the application, providing consistent access to resources
 * whether they're in the Qt resource system or external files.
 */
class ResourcePathManager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static ResourcePathManager& instance();

    /**
     * @brief Initialize the resource manager with application paths
     * @param appDirPath The application directory path
     */
    void initialize(const QString& appDirPath);

    /**
     * @brief Resolve a resource path
     * @param resourceName Name of the resource
     * @param resourceType Type of resource (xml, image, etc.)
     * @return Full path to the resource
     */
    QString resolvePath(const QString& resourceName, const QString& resourceType);

private:
    ResourcePathManager();
    ~ResourcePathManager();

    // Prevent copying
    ResourcePathManager(const ResourcePathManager&) = delete;
    ResourcePathManager& operator=(const ResourcePathManager&) = delete;

    QString m_appDirPath;
    QStringList m_searchPaths;
    QMap<QString, QStringList> m_resourceTypePaths;
};

} // namespace utils
} // namespace core
} // namespace RME