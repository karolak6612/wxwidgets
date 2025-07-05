#ifndef RME_PROPERTIES_PANEL_H
#define RME_PROPERTIES_PANEL_H

#include "ui/palettes/BasePalettePanel.h"
#include <QStackedWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QGroupBox>

// Forward declarations
namespace RME {
namespace core {
    class Position;
    class Tile;
    class Item;
    class Map;
}
}

namespace RME {
namespace ui {
namespace palettes {

/**
 * @brief Properties panel for editing tile, item, and map properties
 * 
 * This panel displays and allows editing of properties for the currently
 * selected tile, item, or map. It provides different property pages for
 * different types of objects.
 */
class PropertiesPanel : public BasePalettePanel {
    Q_OBJECT

public:
    enum class PropertyType {
        None,
        Tile,
        Item,
        Map,
        Selection
    };

    explicit PropertiesPanel(QWidget* parent = nullptr);
    ~PropertiesPanel() override = default;

    // BasePalettePanel interface
    void refreshContent() override;
    void clearSelection() override;

    // Property display
    void showTileProperties(const RME::core::Position& position);
    void showItemProperties(RME::core::Item* item);
    void showMapProperties();
    void showSelectionProperties();
    void showNoProperties();

public slots:
    void onPropertyChanged();
    void onApplyChanges();
    void onResetChanges();
    void onPositionChanged();

signals:
    void propertyModified(const QString& propertyName, const QVariant& value);
    void positionNavigationRequested(const RME::core::Position& position);

protected:
    void setupContentUI() override;
    void connectSignals() override;
    void applySearchFilter(const QString& text) override;

private:
    // UI components
    QStackedWidget* m_stackedWidget = nullptr;
    
    // Property pages
    QWidget* m_noPropertiesPage = nullptr;
    QWidget* m_tilePropertiesPage = nullptr;
    QWidget* m_itemPropertiesPage = nullptr;
    QWidget* m_mapPropertiesPage = nullptr;
    QWidget* m_selectionPropertiesPage = nullptr;
    
    // Tile properties
    QGroupBox* m_tileInfoGroup = nullptr;
    QLineEdit* m_tilePositionEdit = nullptr;
    QPushButton* m_gotoPositionButton = nullptr;
    QCheckBox* m_tileProtectionZoneCheckBox = nullptr;
    QSpinBox* m_tileHouseIdSpinBox = nullptr;
    QTextEdit* m_tileItemsTextEdit = nullptr;
    
    // Item properties
    QGroupBox* m_itemInfoGroup = nullptr;
    QLineEdit* m_itemIdEdit = nullptr;
    QLineEdit* m_itemNameEdit = nullptr;
    QSpinBox* m_itemCountSpinBox = nullptr;
    QSpinBox* m_itemActionIdSpinBox = nullptr;
    QLineEdit* m_itemTextEdit = nullptr;
    QLineEdit* m_itemDescriptionEdit = nullptr;
    
    // Map properties
    QGroupBox* m_mapInfoGroup = nullptr;
    QLineEdit* m_mapNameEdit = nullptr;
    QLineEdit* m_mapAuthorEdit = nullptr;
    QTextEdit* m_mapDescriptionEdit = nullptr;
    QSpinBox* m_mapWidthSpinBox = nullptr;
    QSpinBox* m_mapHeightSpinBox = nullptr;
    
    // Selection properties
    QGroupBox* m_selectionInfoGroup = nullptr;
    QLabel* m_selectionSizeLabel = nullptr;
    QLabel* m_selectionTileCountLabel = nullptr;
    QLabel* m_selectionItemCountLabel = nullptr;
    
    // Control buttons
    QWidget* m_buttonWidget = nullptr;
    QPushButton* m_applyButton = nullptr;
    QPushButton* m_resetButton = nullptr;
    
    // Current state
    PropertyType m_currentPropertyType = PropertyType::None;
    RME::core::Position m_currentPosition;
    RME::core::Item* m_currentItem = nullptr;
    bool m_hasUnsavedChanges = false;
    
    // Helper methods
    void setupNoPropertiesPage();
    void setupTilePropertiesPage();
    void setupItemPropertiesPage();
    void setupMapPropertiesPage();
    void setupSelectionPropertiesPage();
    void setupButtonWidget();
    
    void updateTileProperties(const RME::core::Position& position);
    void updateItemProperties(RME::core::Item* item);
    void updateMapProperties();
    void updateSelectionProperties();
    
    void applyTileChanges();
    void applyItemChanges();
    void applyMapChanges();
    
    void resetTileProperties();
    void resetItemProperties();
    void resetMapProperties();
    
    void setHasUnsavedChanges(bool hasChanges);
    void updateButtonStates();
    
    QWidget* createFormWidget(QFormLayout*& layout);
    void addFormRow(QFormLayout* layout, const QString& label, QWidget* widget);
};

} // namespace palettes
} // namespace ui
} // namespace RME

#endif // RME_PROPERTIES_PANEL_H