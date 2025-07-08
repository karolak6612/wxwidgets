#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>

namespace RME {
namespace core {
    class Brush;
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Individual brush icon widget for grid display
 * 
 * This widget represents a single brush with an icon and optional label.
 * Handles selection state and mouse interactions.
 */
class BrushIconWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BrushIconWidget(RME::core::Brush* brush, QWidget* parent = nullptr);
    ~BrushIconWidget();

    // Brush management
    RME::core::Brush* getBrush() const { return m_brush; }
    void setBrush(RME::core::Brush* brush);

    // Selection state
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected);

    // Icon configuration
    void setIconSize(const QSize& size);
    QSize getIconSize() const;
    
    void setShowLabel(bool show);
    bool getShowLabel() const;

Q_SIGNALS: // Changed
    void selected(RME::core::Brush* brush);
    void activated(RME::core::Brush* brush);

protected:
    void setupUI();
    void updateIcon();
    void updateLabel();
    void updateSelectionState();
    
    // Icon drawing helpers
    void drawBrushTypeShape(QPainter& painter, const QString& brushType);
    void drawTypeIndicator(QPainter& painter, const QString& brushType);
    QString getTypeIndicator(const QString& brushType) const;
    QColor getBrushTypeColor(const QString& brushType) const;
    
    // Event handling
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    // Members moved earlier
    RME::core::Brush* m_brush = nullptr;
    bool m_selected = false;
    QSize m_iconSize = QSize(48, 48);
    bool m_showLabel = true;

    // Brush data
    // RME::core::Brush* m_brush = nullptr; // Moved
    
    // UI Components
    QVBoxLayout* m_layout = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_textLabel = nullptr;
    
    // State
    // bool m_selected = false; // Moved
    bool m_hovered = false;
    // bool m_showLabel = true; // Moved
    // QSize m_iconSize = QSize(48, 48); // Moved
    
    // Styling
    static const QString SELECTED_STYLE;
    static const QString HOVERED_STYLE;
    static const QString NORMAL_STYLE;
};

} // namespace palettes
} // namespace ui
} // namespace RME