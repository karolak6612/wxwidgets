#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

namespace RME {
namespace core {
    class Brush;
}
}

namespace RME {
namespace ui {
namespace palettes {

class BrushPreviewGenerator;

/**
 * @brief Rich tooltip widget for displaying brush information
 * 
 * This widget provides detailed information about a brush including
 * preview image, properties, and usage instructions.
 */
class BrushTooltipWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BrushTooltipWidget(QWidget* parent = nullptr);
    ~BrushTooltipWidget();

    // Brush management
    void setBrush(RME::core::Brush* brush);
    RME::core::Brush* getBrush() const { return m_brush; }

    // Preview generator
    void setPreviewGenerator(BrushPreviewGenerator* generator);

    // Display control
    void showTooltip(const QPoint& position);
    void hideTooltip();
    bool isTooltipVisible() const;

    // Configuration
    void setShowDelay(int milliseconds);
    int getShowDelay() const;

    void setHideDelay(int milliseconds);
    int getHideDelay() const;

public slots:
    void onShowTimer();
    void onHideTimer();

signals:
    void tooltipShown();
    void tooltipHidden();

protected:
    void setupUI();
    void updateContent();
    void updatePreview();
    void updateBrushInfo();
    void updateUsageInfo();
    void positionTooltip(const QPoint& position);
    
    // Helper methods
    QString getBrushDescription(const QString& brushType) const;
    QString getBrushUsageInfo(const QString& brushType) const;
    
    // Event handling
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    // Brush data
    RME::core::Brush* m_brush = nullptr;
    BrushPreviewGenerator* m_previewGenerator = nullptr;

    // UI Components
    QVBoxLayout* m_mainLayout = nullptr;
    QHBoxLayout* m_headerLayout = nullptr;
    QVBoxLayout* m_infoLayout = nullptr;
    
    QLabel* m_previewLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_typeLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QLabel* m_usageLabel = nullptr;

    // Timers
    QTimer* m_showTimer = nullptr;
    QTimer* m_hideTimer = nullptr;

    // Configuration
    int m_showDelay = 500;  // milliseconds
    int m_hideDelay = 100;  // milliseconds
    QSize m_previewSize = QSize(64, 64);

    // State
    bool m_tooltipVisible = false;
    QPoint m_pendingPosition;
};

} // namespace palettes
} // namespace ui
} // namespace RME