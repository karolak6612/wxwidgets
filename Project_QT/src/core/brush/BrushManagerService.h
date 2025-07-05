#ifndef RME_BRUSH_MANAGER_SERVICE_H
#define RME_BRUSH_MANAGER_SERVICE_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QDateTime>
#include <memory> // For std::unique_ptr

#include "core/brush/BrushSettings.h" // Includes BrushShape by proxy

// Forward declaration
namespace RME { namespace core { class Brush; }}

namespace RME {
namespace core {

class BrushManagerService : public QObject {
    Q_OBJECT

public:
    explicit BrushManagerService(QObject *parent = nullptr);
    ~BrushManagerService() override;

    // Basic brush management
    void registerBrush(std::unique_ptr<Brush> brush);
    Brush* getBrush(const QString& name) const;
    Brush* getActiveBrush() const;
    QList<Brush*> getAllBrushes() const;

    // Brush settings
    void setActiveBrushName(const QString& name);
    void setCurrentShape(BrushShape shape);
    void setCurrentSize(int size);
    void setCurrentVariation(int variation);
    void setIsEraseMode(bool isErase);
    const BrushSettings& getCurrentSettings() const;

    // Phase 4.1: Brush categorization methods
    QStringList getBrushCategories() const;
    QList<Brush*> getBrushesByCategory(const QString& category) const;
    QString getBrushCategory(Brush* brush) const;
    void setBrushCategory(Brush* brush, const QString& category);

    // Phase 4.1: Brush metadata (description, category, tags)
    QString getBrushDescription(Brush* brush) const;
    void setBrushDescription(Brush* brush, const QString& description);
    QStringList getBrushTags(Brush* brush) const;
    void setBrushTags(Brush* brush, const QStringList& tags);
    void addBrushTag(Brush* brush, const QString& tag);
    void removeBrushTag(Brush* brush, const QString& tag);

    // Phase 4.1: Recently used brushes tracking
    QList<Brush*> getRecentlyUsedBrushes(int maxCount = 20) const;
    void recordBrushUsage(Brush* brush);
    void clearRecentBrushes();
    int getBrushUsageCount(Brush* brush) const;
    QDateTime getLastBrushUsage(Brush* brush) const;

    // Phase 4.1: Search/filtering capabilities
    QList<Brush*> searchBrushes(const QString& searchText) const;
    QList<Brush*> filterBrushesByTags(const QStringList& tags) const;
    QList<Brush*> filterBrushesByCategory(const QString& category) const;

signals:
    void activeBrushChanged(Brush* activeBrush);
    void brushSettingsChanged(const BrushSettings& newSettings);
    
    // Phase 4.1: New signals for enhanced functionality
    void brushRegistered(Brush* brush);
    void brushMetadataChanged(Brush* brush);
    void brushCategoryChanged(Brush* brush, const QString& category);
    void brushTagsChanged(Brush* brush, const QStringList& tags);
    void recentBrushesChanged();
    void brushUsageRecorded(Brush* brush);

private:
    // Helper methods
    QString generateBrushId(Brush* brush) const;
    void updateBrushMetadata(Brush* brush);

    // Core data
    QHash<QString, std::unique_ptr<Brush>> m_brushes;
    BrushSettings m_currentSettings;

    // Phase 4.1: Metadata storage
    QMap<QString, QString> m_brushCategories;    // brush ID -> category
    QMap<QString, QString> m_brushDescriptions;  // brush ID -> description
    QMap<QString, QStringList> m_brushTags;      // brush ID -> tags list

    // Phase 4.1: Usage tracking
    QList<QString> m_recentBrushIds;             // ordered list of recently used brush IDs
    QMap<QString, int> m_brushUsageCount;        // brush ID -> usage count
    QMap<QString, QDateTime> m_lastBrushUsage;   // brush ID -> last usage time
    int m_maxRecentBrushes = 20;
};

} // namespace core
} // namespace RME

#endif // RME_BRUSH_MANAGER_SERVICE_H
