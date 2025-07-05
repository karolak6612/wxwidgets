#include "core/brush/BrushManagerService.h"
#include "core/brush/Brush.h" // Required for Brush class definition
#include "core/brush/CreatureBrush.h" // Added for CreatureBrush
#include "core/brush/GroundBrush.h" // Added for GroundBrush
#include "core/brush/CarpetBrush.h" // Added for CarpetBrush
#include <utility> // For std::move
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
        qDebug() << "Registered CreatureBrush"; // Temporary debug output
    } else {
        qWarning() << "Failed to create and register CreatureBrush";
    }

    // Register GroundBrush
    auto groundBrush = std::make_unique<RME::core::GroundBrush>();
    if (groundBrush) {
        registerBrush(std::move(groundBrush));
        qDebug() << "Registered GroundBrush"; // Temporary debug output
    } else {
        qWarning() << "Failed to create and register GroundBrush";
    }

    // Register CarpetBrush
    auto carpetBrush = std::make_unique<RME::core::CarpetBrush>();
    if (carpetBrush) {
        registerBrush(std::move(carpetBrush));
        qDebug() << "Registered CarpetBrush"; // Temporary debug output
    } else {
        qWarning() << "Failed to create and register CarpetBrush";
    }
}

BrushManagerService::~BrushManagerService() = default;

void BrushManagerService::registerBrush(std::unique_ptr<Brush> brush) {
    if (brush) {
        QString name = brush->getName();
        m_brushes[name] = std::move(brush);
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
        emit brushSettingsChanged(m_currentSettings); // Emit general settings change
        emit activeBrushChanged(getActiveBrush());     // Emit specific active brush change
    }
}

void BrushManagerService::setCurrentShape(BrushShape shape) {
    if (m_currentSettings.shape != shape) {
        m_currentSettings.shape = shape;
        emit brushSettingsChanged(m_currentSettings);
    }
}

void BrushManagerService::setCurrentSize(int size) {
    if (m_currentSettings.size != size && size > 0) { // Assuming size must be positive
        m_currentSettings.size = size;
        emit brushSettingsChanged(m_currentSettings);
    }
}

void BrushManagerService::setCurrentVariation(int variation) {
    if (m_currentSettings.variation != variation) {
        m_currentSettings.variation = variation;
        emit brushSettingsChanged(m_currentSettings);
    }
}

void BrushManagerService::setIsEraseMode(bool isErase) {
    if (m_currentSettings.isEraseMode != isErase) {
        m_currentSettings.isEraseMode = isErase;
        emit brushSettingsChanged(m_currentSettings);
    }
}

const BrushSettings& BrushManagerService::getCurrentSettings() const {
    return m_currentSettings;
}

} // namespace core
} // namespace RME
