#pragma once

#include <QListWidget>
#include <QList>

namespace RME {
namespace core {
    class Brush;
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief List widget for displaying brushes in list format
 * 
 * This widget displays brushes as a vertical list with icons and text.
 * Provides keyboard navigation and selection capabilities.
 */
class BrushListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit BrushListWidget(QWidget* parent = nullptr);
    ~BrushListWidget();

    // Brush management
    void setBrushes(const QList<RME::core::Brush*>& brushes);
    QList<RME::core::Brush*> getBrushes() const;

    // Selection management
    RME::core::Brush* getSelectedBrush() const;
    void setSelectedBrush(RME::core::Brush* brush);

public Q_SLOTS: // Changed
    void onItemSelectionChanged();
    void onItemActivated(QListWidgetItem* item);

Q_SIGNALS: // Changed
    void brushSelected(RME::core::Brush* brush);
    void brushActivated(RME::core::Brush* brush);

protected:
    void setupUI();
    void setupConnections();
    void populateList();
    QListWidgetItem* createBrushItem(RME::core::Brush* brush);
    
    // Helper methods for brush display
    QString getBrushTypePrefix(const QString& brushType) const;
    QString createBrushTooltip(RME::core::Brush* brush) const;
    QString getBrushTypeDescription(const QString& brushType) const;
    QIcon createBrushIcon(RME::core::Brush* brush) const;
    QColor getBrushTypeColor(const QString& brushType) const;

private:
    QList<RME::core::Brush*> m_brushes;
    RME::core::Brush* m_selectedBrush = nullptr;
};

} // namespace palettes
} // namespace ui
} // namespace RME