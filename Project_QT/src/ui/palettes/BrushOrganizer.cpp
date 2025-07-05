#include "BrushOrganizer.h"
#include "core/brush/Brush.h"
#include <QDebug>
#include <QJsonDocument>
#include <QFile>
#include <QDateTime>
#include <algorithm>

namespace RME {
namespace ui {
namespace palettes {

BrushOrganizer::BrushOrganizer(QObject* parent)
    : QObject(parent)
{
    qDebug() << "BrushOrganizer: Created";
}

BrushOrganizer::~BrushOrganizer()
{
    qDebug() << "BrushOrganizer: Destroyed";
}

QStringList BrushOrganizer::getCustomCategories() const
{
    return m_customCategories;
}

void BrushOrganizer::addCustomCategory(const QString& categoryName)
{
    if (!categoryName.isEmpty() && !m_customCategories.contains(categoryName)) {
        m_customCategories.append(categoryName);
        emit customCategoriesChanged();
        
        qDebug() << "BrushOrganizer: Added custom category" << categoryName;
    }
}

void BrushOrganizer::removeCustomCategory(const QString& categoryName)
{
    if (m_customCategories.removeAll(categoryName) > 0) {
        // Remove all brushes from this category
        m_categoryBrushes.remove(categoryName);
        m_customOrders.remove(categoryName);
        
        // Update brush categories mapping
        for (auto it = m_brushCategories.begin(); it != m_brushCategories.end(); ++it) {
            it.value().removeAll(categoryName);
        }
        
        emit customCategoriesChanged();
        
        qDebug() << "BrushOrganizer: Removed custom category" << categoryName;
    }
}

void BrushOrganizer::renameCustomCategory(const QString& oldName, const QString& newName)
{
    if (oldName != newName && m_customCategories.contains(oldName) && !m_customCategories.contains(newName)) {
        // Update categories list
        int index = m_customCategories.indexOf(oldName);
        m_customCategories[index] = newName;
        
        // Update category brushes mapping
        if (m_categoryBrushes.contains(oldName)) {
            m_categoryBrushes[newName] = m_categoryBrushes.take(oldName);
        }
        
        // Update custom orders
        if (m_customOrders.contains(oldName)) {
            m_customOrders[newName] = m_customOrders.take(oldName);
        }
        
        // Update brush categories mapping
        for (auto it = m_brushCategories.begin(); it != m_brushCategories.end(); ++it) {
            QStringList& categories = it.value();
            int categoryIndex = categories.indexOf(oldName);
            if (categoryIndex >= 0) {
                categories[categoryIndex] = newName;
            }
        }
        
        emit customCategoriesChanged();
        
        qDebug() << "BrushOrganizer: Renamed category" << oldName << "to" << newName;
    }
}

QStringList BrushOrganizer::getBrushesInCategory(const QString& categoryName) const
{
    return m_categoryBrushes.value(categoryName);
}

void BrushOrganizer::addBrushToCategory(RME::core::Brush* brush, const QString& categoryName)
{
    if (!brush || categoryName.isEmpty()) {
        return;
    }
    
    QString brushId = generateUniqueId(brush);
    
    // Add to category brushes
    if (!m_categoryBrushes[categoryName].contains(brushId)) {
        m_categoryBrushes[categoryName].append(brushId);
    }
    
    // Add to brush categories
    if (!m_brushCategories[brushId].contains(categoryName)) {
        m_brushCategories[brushId].append(categoryName);
    }
    
    // Update ID mappings
    m_brushToId[brush] = brushId;
    m_idToBrush[brushId] = brush;
    
    qDebug() << "BrushOrganizer: Added brush" << brush->getName() << "to category" << categoryName;
}

void BrushOrganizer::removeBrushFromCategory(RME::core::Brush* brush, const QString& categoryName)
{
    if (!brush) {
        return;
    }
    
    QString brushId = generateUniqueId(brush);
    
    // Remove from category brushes
    m_categoryBrushes[categoryName].removeAll(brushId);
    
    // Remove from brush categories
    m_brushCategories[brushId].removeAll(categoryName);
    
    qDebug() << "BrushOrganizer: Removed brush" << brush->getName() << "from category" << categoryName;
}

QStringList BrushOrganizer::getCategoriesForBrush(RME::core::Brush* brush) const
{
    if (!brush) {
        return QStringList();
    }
    
    QString brushId = generateUniqueId(brush);
    return m_brushCategories.value(brushId);
}

void BrushOrganizer::addToFavorites(RME::core::Brush* brush)
{
    if (!brush) {
        return;
    }
    
    QString brushId = generateUniqueId(brush);
    if (!m_favoriteBrushes.contains(brushId)) {
        m_favoriteBrushes.insert(brushId);
        
        // Update ID mappings
        m_brushToId[brush] = brushId;
        m_idToBrush[brushId] = brush;
        
        emit favoritesChanged();
        
        qDebug() << "BrushOrganizer: Added brush" << brush->getName() << "to favorites";
    }
}

void BrushOrganizer::removeFromFavorites(RME::core::Brush* brush)
{
    if (!brush) {
        return;
    }
    
    QString brushId = generateUniqueId(brush);
    if (m_favoriteBrushes.remove(brushId)) {
        emit favoritesChanged();
        
        qDebug() << "BrushOrganizer: Removed brush" << brush->getName() << "from favorites";
    }
}

bool BrushOrganizer::isFavorite(RME::core::Brush* brush) const
{
    if (!brush) {
        return false;
    }
    
    QString brushId = generateUniqueId(brush);
    return m_favoriteBrushes.contains(brushId);
}

QList<RME::core::Brush*> BrushOrganizer::getFavorites() const
{
    QList<RME::core::Brush*> favorites;
    
    for (const QString& brushId : m_favoriteBrushes) {
        if (m_idToBrush.contains(brushId)) {
            favorites.append(m_idToBrush[brushId]);
        }
    }
    
    return favorites;
}

void BrushOrganizer::clearFavorites()
{
    if (!m_favoriteBrushes.isEmpty()) {
        m_favoriteBrushes.clear();
        emit favoritesChanged();
        
        qDebug() << "BrushOrganizer: Cleared all favorites";
    }
}

void BrushOrganizer::recordBrushUsage(RME::core::Brush* brush)
{
    if (!brush) {
        return;
    }
    
    QString brushId = generateUniqueId(brush);
    
    // Update usage count
    m_usageCount[brushId]++;
    
    // Update last usage time
    m_lastUsage[brushId] = QDateTime::currentDateTime();
    
    // Update recently used list
    m_recentlyUsed.removeAll(brushId);
    m_recentlyUsed.prepend(brushId);
    
    // Limit recently used list size
    while (m_recentlyUsed.size() > m_maxRecentBrushes) {
        m_recentlyUsed.removeLast();
    }
    
    // Update ID mappings
    m_brushToId[brush] = brushId;
    m_idToBrush[brushId] = brush;
    
    emit usageStatisticsChanged();
}

int BrushOrganizer::getBrushUsageCount(RME::core::Brush* brush) const
{
    if (!brush) {
        return 0;
    }
    
    QString brushId = generateUniqueId(brush);
    return m_usageCount.value(brushId, 0);
}

QDateTime BrushOrganizer::getLastUsageTime(RME::core::Brush* brush) const
{
    if (!brush) {
        return QDateTime();
    }
    
    QString brushId = generateUniqueId(brush);
    return m_lastUsage.value(brushId);
}

QList<RME::core::Brush*> BrushOrganizer::getRecentlyUsedBrushes(int maxCount) const
{
    QList<RME::core::Brush*> recentBrushes;
    
    int count = qMin(maxCount, m_recentlyUsed.size());
    for (int i = 0; i < count; ++i) {
        const QString& brushId = m_recentlyUsed[i];
        if (m_idToBrush.contains(brushId)) {
            recentBrushes.append(m_idToBrush[brushId]);
        }
    }
    
    return recentBrushes;
}

QList<RME::core::Brush*> BrushOrganizer::getMostUsedBrushes(int maxCount) const
{
    // Create list of brush-usage pairs
    QList<QPair<RME::core::Brush*, int>> usagePairs;
    
    for (auto it = m_usageCount.begin(); it != m_usageCount.end(); ++it) {
        const QString& brushId = it.key();
        int usage = it.value();
        
        if (m_idToBrush.contains(brushId)) {
            usagePairs.append(qMakePair(m_idToBrush[brushId], usage));
        }
    }
    
    // Sort by usage count (descending)
    std::sort(usagePairs.begin(), usagePairs.end(),
              [](const QPair<RME::core::Brush*, int>& a, const QPair<RME::core::Brush*, int>& b) {
                  return a.second > b.second;
              });
    
    // Extract brushes
    QList<RME::core::Brush*> mostUsedBrushes;
    int count = qMin(maxCount, usagePairs.size());
    for (int i = 0; i < count; ++i) {
        mostUsedBrushes.append(usagePairs[i].first);
    }
    
    return mostUsedBrushes;
}

QList<RME::core::Brush*> BrushOrganizer::sortBrushes(const QList<RME::core::Brush*>& brushes, SortOrder order) const
{
    QList<RME::core::Brush*> sortedBrushes = brushes;
    
    switch (order) {
        case NameAscending:
            std::sort(sortedBrushes.begin(), sortedBrushes.end(),
                      [](RME::core::Brush* a, RME::core::Brush* b) {
                          return a->getName().compare(b->getName(), Qt::CaseInsensitive) < 0;
                      });
            break;
            
        case NameDescending:
            std::sort(sortedBrushes.begin(), sortedBrushes.end(),
                      [](RME::core::Brush* a, RME::core::Brush* b) {
                          return a->getName().compare(b->getName(), Qt::CaseInsensitive) > 0;
                      });
            break;
            
        case TypeAscending:
            std::sort(sortedBrushes.begin(), sortedBrushes.end(),
                      [](RME::core::Brush* a, RME::core::Brush* b) {
                          return a->getType().compare(b->getType(), Qt::CaseInsensitive) < 0;
                      });
            break;
            
        case TypeDescending:
            std::sort(sortedBrushes.begin(), sortedBrushes.end(),
                      [](RME::core::Brush* a, RME::core::Brush* b) {
                          return a->getType().compare(b->getType(), Qt::CaseInsensitive) > 0;
                      });
            break;
            
        case RecentlyUsed:
            std::sort(sortedBrushes.begin(), sortedBrushes.end(),
                      [this](RME::core::Brush* a, RME::core::Brush* b) {
                          QDateTime timeA = getLastUsageTime(a);
                          QDateTime timeB = getLastUsageTime(b);
                          return timeA > timeB; // More recent first
                      });
            break;
            
        case MostUsed:
            std::sort(sortedBrushes.begin(), sortedBrushes.end(),
                      [this](RME::core::Brush* a, RME::core::Brush* b) {
                          return getBrushUsageCount(a) > getBrushUsageCount(b);
                      });
            break;
            
        case Custom:
            // Custom sorting is handled by getCustomOrder
            break;
    }
    
    return sortedBrushes;
}

void BrushOrganizer::setSortOrder(SortOrder order)
{
    if (m_sortOrder != order) {
        m_sortOrder = order;
        emit sortOrderChanged(order);
        
        qDebug() << "BrushOrganizer: Sort order changed to" << static_cast<int>(order);
    }
}

void BrushOrganizer::setCustomOrder(const QString& categoryName, const QList<RME::core::Brush*>& brushes)
{
    QStringList brushIds;
    for (RME::core::Brush* brush : brushes) {
        if (brush) {
            QString brushId = generateUniqueId(brush);
            brushIds.append(brushId);
            
            // Update ID mappings
            m_brushToId[brush] = brushId;
            m_idToBrush[brushId] = brush;
        }
    }
    
    m_customOrders[categoryName] = brushIds;
    
    qDebug() << "BrushOrganizer: Set custom order for category" << categoryName;
}

QList<RME::core::Brush*> BrushOrganizer::getCustomOrder(const QString& categoryName) const
{
    QList<RME::core::Brush*> orderedBrushes;
    
    const QStringList& brushIds = m_customOrders.value(categoryName);
    for (const QString& brushId : brushIds) {
        if (m_idToBrush.contains(brushId)) {
            orderedBrushes.append(m_idToBrush[brushId]);
        }
    }
    
    return orderedBrushes;
}

void BrushOrganizer::moveInCustomOrder(const QString& categoryName, RME::core::Brush* brush, int newIndex)
{
    if (!brush || !m_customOrders.contains(categoryName)) {
        return;
    }
    
    QString brushId = generateUniqueId(brush);
    QStringList& order = m_customOrders[categoryName];
    
    int currentIndex = order.indexOf(brushId);
    if (currentIndex >= 0 && currentIndex != newIndex) {
        order.move(currentIndex, newIndex);
        
        qDebug() << "BrushOrganizer: Moved brush" << brush->getName() 
                 << "from index" << currentIndex << "to" << newIndex 
                 << "in category" << categoryName;
    }
}

void BrushOrganizer::moveBrushToCategory(RME::core::Brush* brush, const QString& fromCategory, const QString& toCategory)
{
    if (!brush || fromCategory == toCategory) {
        return;
    }
    
    // Remove from old category
    if (!fromCategory.isEmpty()) {
        removeBrushFromCategory(brush, fromCategory);
    }
    
    // Add to new category
    if (!toCategory.isEmpty()) {
        addBrushToCategory(brush, toCategory);
    }
    
    emit brushMovedToCategory(brush, toCategory);
    
    qDebug() << "BrushOrganizer: Moved brush" << brush->getName() 
             << "from category" << fromCategory << "to" << toCategory;
}

bool BrushOrganizer::canMoveBrushToCategory(RME::core::Brush* brush, const QString& categoryName) const
{
    // For now, allow moving any brush to any category
    // This could be extended with validation logic
    return brush != nullptr && !categoryName.isEmpty();
}

void BrushOrganizer::onBrushUsed(RME::core::Brush* brush)
{
    recordBrushUsage(brush);
}

QString BrushOrganizer::generateUniqueId(RME::core::Brush* brush) const
{
    if (!brush) {
        return QString();
    }
    
    // Use brush pointer address and type as unique identifier
    return QString("%1_%2").arg(reinterpret_cast<quintptr>(brush)).arg(brush->getType());
}

// Persistence methods (simplified implementations)

QJsonObject BrushOrganizer::saveToJson() const
{
    QJsonObject json;
    
    // Save custom categories
    QJsonArray categoriesArray;
    for (const QString& category : m_customCategories) {
        categoriesArray.append(category);
    }
    json["customCategories"] = categoriesArray;
    
    // Save favorites
    QJsonArray favoritesArray;
    for (const QString& brushId : m_favoriteBrushes) {
        favoritesArray.append(brushId);
    }
    json["favorites"] = favoritesArray;
    
    // Save usage statistics
    QJsonObject usageObject;
    for (auto it = m_usageCount.begin(); it != m_usageCount.end(); ++it) {
        usageObject[it.key()] = it.value();
    }
    json["usageCount"] = usageObject;
    
    // Save sort order
    json["sortOrder"] = static_cast<int>(m_sortOrder);
    
    return json;
}

void BrushOrganizer::loadFromJson(const QJsonObject& json)
{
    // Load custom categories
    QJsonArray categoriesArray = json["customCategories"].toArray();
    m_customCategories.clear();
    for (const QJsonValue& value : categoriesArray) {
        m_customCategories.append(value.toString());
    }
    
    // Load favorites
    QJsonArray favoritesArray = json["favorites"].toArray();
    m_favoriteBrushes.clear();
    for (const QJsonValue& value : favoritesArray) {
        m_favoriteBrushes.insert(value.toString());
    }
    
    // Load usage statistics
    QJsonObject usageObject = json["usageCount"].toObject();
    m_usageCount.clear();
    for (auto it = usageObject.begin(); it != usageObject.end(); ++it) {
        m_usageCount[it.key()] = it.value().toInt();
    }
    
    // Load sort order
    m_sortOrder = static_cast<SortOrder>(json["sortOrder"].toInt(NameAscending));
    
    qDebug() << "BrushOrganizer: Loaded configuration from JSON";
}

void BrushOrganizer::saveToFile(const QString& filename) const
{
    QJsonDocument doc(saveToJson());
    
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qDebug() << "BrushOrganizer: Saved to file" << filename;
    } else {
        qWarning() << "BrushOrganizer: Failed to save to file" << filename;
    }
}

void BrushOrganizer::loadFromFile(const QString& filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        loadFromJson(doc.object());
        qDebug() << "BrushOrganizer: Loaded from file" << filename;
    } else {
        qWarning() << "BrushOrganizer: Failed to load from file" << filename;
    }
}

// Statistics methods

int BrushOrganizer::getTotalBrushCount() const
{
    return m_idToBrush.size();
}

int BrushOrganizer::getCategoryBrushCount(const QString& categoryName) const
{
    return m_categoryBrushes.value(categoryName).size();
}

QMap<QString, int> BrushOrganizer::getCategoryStatistics() const
{
    QMap<QString, int> stats;
    
    for (auto it = m_categoryBrushes.begin(); it != m_categoryBrushes.end(); ++it) {
        stats[it.key()] = it.value().size();
    }
    
    return stats;
}

QMap<RME::core::Brush*, int> BrushOrganizer::getUsageStatistics() const
{
    QMap<RME::core::Brush*, int> stats;
    
    for (auto it = m_usageCount.begin(); it != m_usageCount.end(); ++it) {
        const QString& brushId = it.key();
        int usage = it.value();
        
        if (m_idToBrush.contains(brushId)) {
            stats[m_idToBrush[brushId]] = usage;
        }
    }
    
    return stats;
}

} // namespace palettes
} // namespace ui
} // namespace RME