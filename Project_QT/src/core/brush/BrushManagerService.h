#ifndef RME_BRUSH_MANAGER_SERVICE_H
#define RME_BRUSH_MANAGER_SERVICE_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QList> // For getRegisteredBrushNames
#include <memory> // For std::unique_ptr

#include "Brush.h" // For RME::Brush pointer and std::unique_ptr<Brush>
#include "BrushSettings.h" // For RME::BrushSettings

namespace RME {

class BrushManagerService : public QObject {
    Q_OBJECT

public:
    explicit BrushManagerService(QObject *parent = nullptr);
    ~BrushManagerService() override = default;

    // Brush registration and retrieval
    // Takes ownership of the brush. Returns false if a brush with the same name already exists.
    bool registerBrush(std::unique_ptr<Brush> brush);
    Brush* getBrush(const QString& name) const; // Returns nullptr if not found
    QList<QString> getRegisteredBrushNames() const;

    // Active brush management
    Brush* getActiveBrush() const;
    QString getActiveBrushName() const; // Same as m_currentSettings.activeBrushName
    void setActiveBrushName(const QString& name); // Sets m_currentSettings.activeBrushName and emits activeBrushChanged

    // Current brush settings management
    const BrushSettings& getCurrentSettings() const;

    void setCurrentShape(BrushShape shape);
    BrushShape getCurrentShape() const;

    void setCurrentSize(int size);
    int getCurrentSize() const;

    void setCurrentVariation(int variation);
    int getCurrentVariation() const;

    void setEraseMode(bool enabled);
    bool isEraseMode() const;

    // Allows setting all brush parameters at once
    void updateBrushSettings(const BrushSettings& newSettings);


signals:
    // Emitted when the active brush changes (e.g., user selects a new brush from palette)
    void activeBrushChanged(RME::Brush* activeBrush);

    // Emitted when any part of the BrushSettings (shape, size, variation, erase mode) changes
    // This signal is also emitted if activeBrushName changes via setActiveBrushName or updateBrushSettings
    void brushSettingsChanged(const RME::BrushSettings& newSettings);

private:
    QHash<QString, std::unique_ptr<Brush>> m_brushes;
    BrushSettings m_currentSettings;
    Brush* m_activeBrush = nullptr; // Cached pointer to the active brush for quick access
};

} // namespace RME

#endif // RME_BRUSH_MANAGER_SERVICE_H
