#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QList>

namespace RME {
namespace core {
    class Brush;
}
}

namespace RME {
namespace ui {
namespace palettes {

class BrushIconWidget;

/**
 * @brief Grid widget for displaying brushes in grid format
 * 
 * This widget displays brushes as a grid of icons with optional labels.
 * Supports different icon sizes and grid configurations.
 */
class BrushGridWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BrushGridWidget(QWidget* parent = nullptr);
    ~BrushGridWidget();

    // Brush management
    void setBrushes(const QList<RME::core::Brush*>& brushes);
    QList<RME::core::Brush*> getBrushes() const;

    // Selection management
    RME::core::Brush* getSelectedBrush() const;
    void setSelectedBrush(RME::core::Brush* brush);

    // Grid configuration
    void setIconSize(const QSize& size);
    QSize getIconSize() const;
    
    void setColumnsPerRow(int columns);
    int getColumnsPerRow() const;

public slots:
    void onBrushIconSelected(RME::core::Brush* brush);
    void onBrushIconActivated(RME::core::Brush* brush);

signals:
    void brushSelected(RME::core::Brush* brush);
    void brushActivated(RME::core::Brush* brush);

protected:
    void setupUI();
    void setupConnections();
    void populateGrid();
    void clearGrid();
    BrushIconWidget* createBrushIcon(RME::core::Brush* brush);
    void updateGridLayout();
    
    // Event handling
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    
    // Navigation helpers
    void selectNextBrush();
    void selectPreviousBrush();
    void selectBrushAt(int row, int col);
    int getSelectedBrushIndex() const;

private:
    // UI Components
    QGridLayout* m_gridLayout = nullptr;
    
    // Brush data
    QList<RME::core::Brush*> m_brushes;
    QList<BrushIconWidget*> m_brushIcons;
    RME::core::Brush* m_selectedBrush = nullptr;
    
    // Grid configuration
    QSize m_iconSize = QSize(48, 48);
    int m_columnsPerRow = 4;
};

} // namespace palettes
} // namespace ui
} // namespace RME