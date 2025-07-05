#include "ResourcePathManager.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

namespace RME {
namespace core {
namespace utils {

ResourcePathManager& ResourcePathManager::instance() {
    static ResourcePathManager instance;
    return instance;
}

ResourcePathManager::ResourcePathManager() {
    // Initialize with default paths
    m_searchPaths << ":/resources"  // Qt resources
                 << "."            // Current directory
                 << "../XML"       // XML directory relative to binary
                 << "../resources" // Resources directory relative to binary
                 << "../../XML";   // XML directory for development environment
}

ResourcePathManager::~ResourcePathManager() {
    // Nothing to clean up
}

void ResourcePathManager::initialize(const QString& appDirPath) {
    m_appDirPath = appDirPath;
    
    // Add application-specific paths
    m_searchPaths << appDirPath
                 << appDirPath + "/XML"
                 << appDirPath + "/resources"
                 << appDirPath + "/XML/760"; // Add client version subdirectory
                 
    // Setup resource type paths
    m_resourceTypePaths["xml"] << ":/resources" 
                              << ":" // Root of Qt resources
                              << "../XML" 
                              << "../XML/760" // Client version subdirectory
                              << appDirPath + "/XML"
                              << appDirPath + "/XML/760"; // Client version subdirectory
                              
    m_resourceTypePaths["image"] << ":/resources/images" << "../resources/images";
    m_resourceTypePaths["icon"] << ":/resources/icons" << "../resources/icons";
}

QString ResourcePathManager::resolvePath(const QString& resourceName, const QString& resourceType) {
    // First check in resource type specific paths
    if (m_resourceTypePaths.contains(resourceType)) {
        for (const QString& path : m_resourceTypePaths[resourceType]) {
            QString fullPath = path + "/" + resourceName;
            if (QFile::exists(fullPath)) {
                qDebug() << "Resource found:" << resourceName << "at" << fullPath;
                return fullPath;
            }
        }
    }
    
    // Then check in general search paths
    for (const QString& path : m_searchPaths) {
        QString fullPath = path + "/" + resourceName;
        if (QFile::exists(fullPath)) {
            qDebug() << "Resource found:" << resourceName << "at" << fullPath;
            return fullPath;
        }
    }
    
    // If not found, check if it's directly in Qt resources
    QString qtResourcePath = ":" + resourceName;
    if (QFile::exists(qtResourcePath)) {
        qDebug() << "Resource found:" << resourceName << "at" << qtResourcePath;
        return qtResourcePath;
    }
    
    // If still not found, return the Qt resource path as fallback
    QString fallbackPath = ":/resources/" + resourceName;
    qWarning() << "Resource not found:" << resourceName << "- using fallback path:" << fallbackPath;
    return fallbackPath;
}

} // namespace utils
} // namespace core
} // namespace RME