#ifndef RME_BRUSH_MANAGER_SERVICE_H
#define RME_BRUSH_MANAGER_SERVICE_H

#include <QObject>
#include <QHash>
#include <QString>
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

    void registerBrush(std::unique_ptr<Brush> brush);
    Brush* getBrush(const QString& name) const;
    Brush* getActiveBrush() const;

    void setActiveBrushName(const QString& name);
    void setCurrentShape(BrushShape shape);
    void setCurrentSize(int size);
    void setCurrentVariation(int variation);
    void setIsEraseMode(bool isErase);

    const BrushSettings& getCurrentSettings() const;

signals:
    void activeBrushChanged(Brush* activeBrush);
    void brushSettingsChanged(const BrushSettings& newSettings);

private:
    QHash<QString, std::unique_ptr<Brush>> m_brushes;
    BrushSettings m_currentSettings;
};

} // namespace core
} // namespace RME

#endif // RME_BRUSH_MANAGER_SERVICE_H
