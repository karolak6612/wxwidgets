#include "BrushManagerService.h"
#include <QDebug> // For warnings

namespace RME {

BrushManagerService::BrushManagerService(QObject *parent)
    : QObject(parent), m_activeBrush(nullptr) {
    // m_currentSettings is default constructed
}

bool BrushManagerService::registerBrush(std::unique_ptr<Brush> brush) {
    if (!brush) {
        qWarning() << "BrushManagerService::registerBrush - Attempted to register a null brush.";
        return false;
    }
    QString name = brush->getName();
    if (name.isEmpty()) {
        qWarning() << "BrushManagerService::registerBrush - Brush has no name, cannot register.";
        return false;
    }
    if (m_brushes.contains(name)) {
        qWarning() << "BrushManagerService::registerBrush - Brush with name" << name << "already exists.";
        return false;
    }

    Brush* rawBrushPtr = brush.get(); // Get raw pointer before move
    m_brushes.insert(name, std::move(brush));
    qInfo() << "BrushManagerService::registerBrush - Registered brush:" << name;

    if (!m_activeBrush && m_brushes.size() == 1) { // If no active brush and this is the first one
        setActiveBrushName(name); // This will set m_activeBrush and emit signals
    }
    return true;
}

Brush* BrushManagerService::getBrush(const QString& name) const {
    return m_brushes.value(name, nullptr).get();
}

QList<QString> BrushManagerService::getRegisteredBrushNames() const {
    return m_brushes.keys();
}

Brush* BrushManagerService::getActiveBrush() const {
    return m_activeBrush;
}

QString BrushManagerService::getActiveBrushName() const {
    return m_currentSettings.activeBrushName;
}

void BrushManagerService::setActiveBrushName(const QString& name) {
    // Check if there's actually a change
    if (m_currentSettings.activeBrushName == name) {
        // If name is same, ensure m_activeBrush pointer is also consistent.
        // It could be inconsistent if current name is valid but m_activeBrush is somehow null.
        if (name.isEmpty() && m_activeBrush == nullptr) return; // No change: still no brush
        if (!name.isEmpty() && m_activeBrush && m_activeBrush->getName() == name) return; // No change: still same valid brush
    }

    Brush* newActiveBrush = nullptr;
    if (!name.isEmpty()) {
        newActiveBrush = getBrush(name);
        if (!newActiveBrush) {
            qWarning() << "BrushManagerService::setActiveBrushName - Brush with name" << name << "not found. Active brush not changed.";
            // Optionally, if current active brush name is now invalid, clear it
            if(m_currentSettings.activeBrushName != "" && getBrush(m_currentSettings.activeBrushName) == nullptr) {
                 m_activeBrush = nullptr;
                 m_currentSettings.activeBrushName = ""; // Clear to no brush state
                 emit activeBrushChanged(nullptr);
                 emit brushSettingsChanged(m_currentSettings);
            }
            return; // Do not proceed if new name is invalid
        }
    }

    // Proceed if new brush is different or if name is cleared
    if (m_activeBrush != newActiveBrush || m_currentSettings.activeBrushName != name) {
        m_activeBrush = newActiveBrush;
        m_currentSettings.activeBrushName = name;
        qInfo() << "BrushManagerService::setActiveBrushName - Active brush set to:" << (m_activeBrush ? name : "None");
        emit activeBrushChanged(m_activeBrush);
        emit brushSettingsChanged(m_currentSettings); // activeBrushName is part of settings
    }
}

const BrushSettings& BrushManagerService::getCurrentSettings() const {
    return m_currentSettings;
}

void BrushManagerService::setCurrentShape(BrushShape shape) {
    if (m_currentSettings.shape != shape) {
        m_currentSettings.shape = shape;
        emit brushSettingsChanged(m_currentSettings);
    }
}
BrushShape BrushManagerService::getCurrentShape() const { return m_currentSettings.shape; }

void BrushManagerService::setCurrentSize(int size) {
    if (size < 1) size = 1;
    // Max size could be defined by AppSettings or a constant
    // static const int MAX_BRUSH_SIZE = 100; // Example
    // if (size > MAX_BRUSH_SIZE) size = MAX_BRUSH_SIZE;

    if (m_currentSettings.size != size) {
        m_currentSettings.size = size;
        emit brushSettingsChanged(m_currentSettings);
    }
}
int BrushManagerService::getCurrentSize() const { return m_currentSettings.size; }

void BrushManagerService::setCurrentVariation(int variation) {
    if (variation < 0) variation = 0;
    if (m_currentSettings.variation != variation) {
        m_currentSettings.variation = variation;
        emit brushSettingsChanged(m_currentSettings);
    }
}
int BrushManagerService::getCurrentVariation() const { return m_currentSettings.variation; }

void BrushManagerService::setEraseMode(bool enabled) {
    if (m_currentSettings.isEraseMode != enabled) {
        m_currentSettings.isEraseMode = enabled;
        emit brushSettingsChanged(m_currentSettings);
    }
}
bool BrushManagerService::isEraseMode() const { return m_currentSettings.isEraseMode; }

void BrushManagerService::updateBrushSettings(const BrushSettings& newSettings) {
    // Check if significant changes occurred that require signals
    bool activeBrushPointerChanged = false;
    if (m_currentSettings.activeBrushName != newSettings.activeBrushName || m_activeBrush == nullptr || (m_activeBrush && m_activeBrush->getName() != newSettings.activeBrushName) ) {
        Brush* newActiveBrushPtr = getBrush(newSettings.activeBrushName);
        // Allow empty name to clear active brush
        if (newSettings.activeBrushName.isEmpty() || newActiveBrushPtr) {
            if (m_activeBrush != newActiveBrushPtr) {
                m_activeBrush = newActiveBrushPtr;
                activeBrushPointerChanged = true;
            }
        } else {
            qWarning() << "BrushManagerService::updateBrushSettings - Invalid brush name in new settings:" << newSettings.activeBrushName << ". Active brush not changed from:" << m_currentSettings.activeBrushName;
            // Do not update m_currentSettings.activeBrushName if the new one is invalid
            // and retain the old m_activeBrush pointer.
            // Only update the parts of settings that are not activeBrushName.
            BrushSettings tempSettings = newSettings;
            tempSettings.activeBrushName = m_currentSettings.activeBrushName; // Keep old valid name

            if (m_currentSettings != tempSettings) { // Check if other fields changed
                 m_currentSettings = tempSettings;
                 emit brushSettingsChanged(m_currentSettings);
            }
            // activeBrushPointerChanged remains false
            if (activeBrushPointerChanged) { // This will be false due to logic above.
                 emit activeBrushChanged(m_activeBrush); // Emit with potentially old brush if new name was bad
            }
            return; // Exit without fully applying newSettings if name was bad
        }
    }

    // If we are here, newSettings.activeBrushName is valid or empty.
    if (m_currentSettings != newSettings) {
        m_currentSettings = newSettings; // Apply all new settings
        // m_activeBrush has been updated above if name changed and was valid.
        emit brushSettingsChanged(m_currentSettings);
    }

    if (activeBrushPointerChanged) { // Emit activeBrushChanged only if the pointer actually changed
        emit activeBrushChanged(m_activeBrush);
    }
}

} // namespace RME
