#include "core/brush/BrushManagerService.h"
#include "core/brush/Brush.h" // Required for Brush class definition
#include "core/brush/CreatureBrush.h" // Added for CreatureBrush
#include "core/brush/GroundBrush.h" // Added for GroundBrush
#include "core/brush/CarpetBrush.h" // Added for CarpetBrush
#include <utility> // For std::move
#include <memory>  // Added for std::unique_ptr, std::make_unique
#include <QDebug> // Added for qDebug and qWarning

namespace RME {
namespace core {

BrushManagerService::BrushManagerService(QObject *parent) : QObject(parent) {
    // Initialize currentSettings with default values if not done by BrushSettings constructor
    m_currentSettings.shape = BrushShape::Square;
    m_currentSettings.size = 1;
    m_currentSettings.variation = 0;
    m_currentSettings.isEraseMode = false;
    m_currentSettings.activeBrushName = ""; // No active brush initially

    // Register standard brushes
    // Assuming other brushes like RawBrush, EraserBrush might be registered similarly.
    // For now, specifically adding CreatureBrush.
    auto creatureBrush = std::make_unique<CreatureBrush>();
    if (creatureBrush) { // Check if make_unique succeeded (it should)
        registerBrush(std::move(creatureBrush));
        qDebug() << "BrushManagerService: CreatureBrush registered successfully";
    } else {
        qWarning() << "Failed to create and register CreatureBrush";
    }

    // Register GroundBrush
    auto groundBrush = std::make_unique<RME::core::GroundBrush>();
    if (groundBrush) {
        registerBrush(std::move(groundBrush));
        qDebug() << "BrushManagerService: GroundBrush registered successfully";
    } else {
        qWarning() << "Failed to create and register GroundBrush";
    }

    // Register CarpetBrush
    auto carpetBrush = std::make_unique<RME::core::CarpetBrush>();
    if (carpetBrush) {
        registerBrush(std::move(carpetBrush));
        qDebug() << "BrushManagerService: CarpetBrush registered successfully";
    } else {
        qWarning() << "Failed to create and register CarpetBrush";
    }
}

BrushManagerService::~BrushManagerService() = default;

void BrushManagerService::registerBrush(std::unique_ptr<Brush> brush) {
    if (brush) {
        QString name = brush->getName();
        QString brushId = generateBrushId(brush.get());
        
        m_brushes[name] = std::move(brush);
        
        // Initialize default metadata
        if (!m_brushCategories.contains(brushId)) {
            m_brushCategories[brushId] = "General";
        }
        if (!m_brushDescriptions.contains(brushId)) {
            m_brushDescriptions[brushId] = QString("Brush: %1").arg(name);
        }
        if (!m_brushTags.contains(brushId)) {
            m_brushTags[brushId] = QStringList();
        }
        
        Q_EMIT brushRegistered(m_brushes[name].get()); // Changed
    }
}

Brush* BrushManagerService::getBrush(const QString& name) const {
    auto it = m_brushes.constFind(name);
    if (it != m_brushes.constEnd()) {
        return it.value().get();
    }
    return nullptr;
}

Brush* BrushManagerService::getActiveBrush() const {
    if (m_currentSettings.activeBrushName.isEmpty()) {
        return nullptr;
    }
    return getBrush(m_currentSettings.activeBrushName);
}

void BrushManagerService::setActiveBrushName(const QString& name) {
    if (m_currentSettings.activeBrushName != name) {
        m_currentSettings.activeBrushName = name;
        Q_EMIT brushSettingsChanged(m_currentSettings); // Changed // Emit general settings change
        Q_EMIT activeBrushChanged(getActiveBrush());     // Changed // Emit specific active brush change
    }
}

void BrushManagerService::setCurrentShape(BrushShape shape) {
    if (m_currentSettings.shape != shape) {
        m_currentSettings.shape = shape;
        Q_EMIT brushSettingsChanged(m_currentSettings); // Changed
    }
}

void BrushManagerService::setCurrentSize(int size) {
    if (m_currentSettings.size != size && size > 0) { // Assuming size must be positive
        m_currentSettings.size = size;
        Q_EMIT brushSettingsChanged(m_currentSettings); // Changed
    }
}

void BrushManagerService::setCurrentVariation(int variation) {
    if (m_currentSettings.variation != variation) {
        m_currentSettings.variation = variation;
        Q_EMIT brushSettingsChanged(m_currentSettings); // Changed
    }
}

void BrushManagerService::setIsEraseMode(bool isErase) {
    if (m_currentSettings.isEraseMode != isErase) {
        m_currentSettings.isEraseMode = isErase;
        Q_EMIT brushSettingsChanged(m_currentSettings); // Changed
    }
}

const BrushSettings& BrushManagerService::getCurrentSettings() const {
    return m_currentSettings;
}

QList<Brush*> BrushManagerService::getAllBrushes() const {
    QList<Brush*> brushes;
    for (const auto& brush : m_brushes) {
        brushes.append(brush.get());
    }
    return brushes;
}

// Phase 4.1: Brush categorization methods
QStringList BrushManagerService::getBrushCategories() const {
    QStringList categories = m_brushCategories.values();
    categories.removeDuplicates();
    categories.sort();
    return categories;
}

QList<Brush*> BrushManagerService::getBrushesByCategory(const QString& category) const {
    QList<Brush*> result;
    for (auto it = m_brushCategories.constBegin(); it != m_brushCategories.constEnd(); ++it) {
        if (it.value() == category) {
            QString brushId = it.key();
            // Find brush by ID
            for (const auto& brush : m_brushes) {
                if (generateBrushId(brush.get()) == brushId) {
                    result.append(brush.get());
                    break;
                }
            }
        }
    }
    return result;
}

QString BrushManagerService::getBrushCategory(Brush* brush) const {
    if (!brush) return QString();
    QString brushId = generateBrushId(brush);
    return m_brushCategories.value(brushId, "General");
}

void BrushManagerService::setBrushCategory(Brush* brush, const QString& category) {
    if (!brush) return;
    QString brushId = generateBrushId(brush);
    if (m_brushCategories.value(brushId) != category) {
        m_brushCategories[brushId] = category;
        Q_EMIT brushCategoryChanged(brush, category); // Changed
        Q_EMIT brushMetadataChanged(brush); // Changed
    }
}

// Phase 4.1: Brush metadata (description, category, tags)
QString BrushManagerService::getBrushDescription(Brush* brush) const {
    if (!brush) return QString();
    QString brushId = generateBrushId(brush);
    return m_brushDescriptions.value(brushId, QString("Brush: %1").arg(brush->getName()));
}

void BrushManagerService::setBrushDescription(Brush* brush, const QString& description) {
    if (!brush) return;
    QString brushId = generateBrushId(brush);
    if (m_brushDescriptions.value(brushId) != description) {
        m_brushDescriptions[brushId] = description;
        Q_EMIT brushMetadataChanged(brush); // Changed
    }
}

QStringList BrushManagerService::getBrushTags(Brush* brush) const {
    if (!brush) return QStringList();
    QString brushId = generateBrushId(brush);
    return m_brushTags.value(brushId, QStringList());
}

void BrushManagerService::setBrushTags(Brush* brush, const QStringList& tags) {
    if (!brush) return;
    QString brushId = generateBrushId(brush);
    if (m_brushTags.value(brushId) != tags) {
        m_brushTags[brushId] = tags;
        Q_EMIT brushTagsChanged(brush, tags); // Changed
        Q_EMIT brushMetadataChanged(brush); // Changed
    }
}

void BrushManagerService::addBrushTag(Brush* brush, const QString& tag) {
    if (!brush || tag.isEmpty()) return;
    QString brushId = generateBrushId(brush);
    QStringList currentTags = m_brushTags.value(brushId, QStringList());
    if (!currentTags.contains(tag)) {
        currentTags.append(tag);
        m_brushTags[brushId] = currentTags;
        Q_EMIT brushTagsChanged(brush, currentTags); // Changed
        Q_EMIT brushMetadataChanged(brush); // Changed
    }
}

void BrushManagerService::removeBrushTag(Brush* brush, const QString& tag) {
    if (!brush || tag.isEmpty()) return;
    QString brushId = generateBrushId(brush);
    QStringList currentTags = m_brushTags.value(brushId, QStringList());
    if (currentTags.removeOne(tag)) {
        m_brushTags[brushId] = currentTags;
        Q_EMIT brushTagsChanged(brush, currentTags); // Changed
        Q_EMIT brushMetadataChanged(brush); // Changed
    }
}

// Phase 4.1: Recently used brushes tracking
QList<Brush*> BrushManagerService::getRecentlyUsedBrushes(int maxCount) const {
    QList<Brush*> result;
    int count = 0;
    for (const QString& brushId : m_recentBrushIds) {
        if (count >= maxCount) break;
        
        // Find brush by ID
        for (const auto& brush : m_brushes) {
            if (generateBrushId(brush.get()) == brushId) {
                result.append(brush.get());
                count++;
                break;
            }
        }
    }
    return result;
}

void BrushManagerService::recordBrushUsage(Brush* brush) {
    if (!brush) return;
    
    QString brushId = generateBrushId(brush);
    QDateTime now = QDateTime::currentDateTime();
    
    // Remove from current position if exists
    m_recentBrushIds.removeAll(brushId);
    
    // Add to front
    m_recentBrushIds.prepend(brushId);
    
    // Limit size
    while (m_recentBrushIds.size() > m_maxRecentBrushes) {
        m_recentBrushIds.removeLast();
    }
    
    // Update usage statistics
    m_brushUsageCount[brushId] = m_brushUsageCount.value(brushId, 0) + 1;
    m_lastBrushUsage[brushId] = now;
    
    Q_EMIT brushUsageRecorded(brush); // Changed
    Q_EMIT recentBrushesChanged(); // Changed
}

void BrushManagerService::clearRecentBrushes() {
    if (!m_recentBrushIds.isEmpty()) {
        m_recentBrushIds.clear();
        Q_EMIT recentBrushesChanged(); // Changed
    }
}

int BrushManagerService::getBrushUsageCount(Brush* brush) const {
    if (!brush) return 0;
    QString brushId = generateBrushId(brush);
    return m_brushUsageCount.value(brushId, 0);
}

QDateTime BrushManagerService::getLastBrushUsage(Brush* brush) const {
    if (!brush) return QDateTime();
    QString brushId = generateBrushId(brush);
    return m_lastBrushUsage.value(brushId, QDateTime());
}

// Phase 4.1: Search/filtering capabilities
QList<Brush*> BrushManagerService::searchBrushes(const QString& searchText) const {
    QList<Brush*> result;
    QString lowerSearchText = searchText.toLower();
    
    for (const auto& brush : m_brushes) {
        Brush* brushPtr = brush.get();
        QString brushId = generateBrushId(brushPtr);
        
        // Search in name
        if (brushPtr->getName().toLower().contains(lowerSearchText)) {
            result.append(brushPtr);
            continue;
        }
        
        // Search in description
        QString description = m_brushDescriptions.value(brushId, "");
        if (description.toLower().contains(lowerSearchText)) {
            result.append(brushPtr);
            continue;
        }
        
        // Search in tags
        QStringList tags = m_brushTags.value(brushId, QStringList());
        for (const QString& tag : tags) {
            if (tag.toLower().contains(lowerSearchText)) {
                result.append(brushPtr);
                break;
            }
        }
        
        // Search in category
        QString category = m_brushCategories.value(brushId, "");
        if (category.toLower().contains(lowerSearchText)) {
            result.append(brushPtr);
        }
    }
    
    return result;
}

QList<Brush*> BrushManagerService::filterBrushesByTags(const QStringList& tags) const {
    QList<Brush*> result;
    
    for (const auto& brush : m_brushes) {
        Brush* brushPtr = brush.get();
        QString brushId = generateBrushId(brushPtr);
        QStringList brushTags = m_brushTags.value(brushId, QStringList());
        
        // Check if brush has all required tags
        bool hasAllTags = true;
        for (const QString& requiredTag : tags) {
            if (!brushTags.contains(requiredTag, Qt::CaseInsensitive)) {
                hasAllTags = false;
                break;
            }
        }
        
        if (hasAllTags) {
            result.append(brushPtr);
        }
    }
    
    return result;
}

QList<Brush*> BrushManagerService::filterBrushesByCategory(const QString& category) const {
    return getBrushesByCategory(category);
}

// Helper methods
QString BrushManagerService::generateBrushId(Brush* brush) const {
    if (!brush) return QString();
    // Use brush name as ID for now - could be enhanced with type info
    return brush->getName();
}

void BrushManagerService::updateBrushMetadata(Brush* brush) {
    if (!brush) return;
    Q_EMIT brushMetadataChanged(brush); // Changed
}

} // namespace core
} // namespace RME
