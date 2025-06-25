#pragma once

#include "ui/palettes/BasePalettePanel.h"
#include <QWidget>

// Service interfaces
#include "core/services/IBrushStateService.h"
#include "core/services/IClientDataService.h"

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QLineEdit;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

namespace RME {

class HouseData;

namespace ui {

class EditHouseDialog;

class HousePalettePanel : public BasePalettePanel
{
    Q_OBJECT

public:
    explicit HousePalettePanel(
        RME::core::IBrushStateService* brushStateService,
        RME::core::IClientDataService* clientDataService,
        QWidget* parent = nullptr
    );
    ~HousePalettePanel();

    // BasePalettePanel interface
    void setupUI() override;

    // House management
    void loadHouses();
    void refreshHouseList();
    void filterHouses(const QString& filter);
    
    // Utility methods
    int getSelectedHouseId() const;
    QString getSelectedHouseName() const;
    void selectHouse(int houseId);

signals:
    void houseSelected(int houseId);
    void editHouseRequested(int houseId);
    void createHouseRequested();
    void deleteHouseRequested(int houseId);
    void houseDoubleClicked(int houseId);

private slots:
    void onHouseSelectionChanged();
    void onHouseDoubleClicked(QListWidgetItem* item);
    void onHouseContextMenu(const QPoint& position);
    void onCreateHouse();
    void onEditHouse();
    void onDeleteHouse();
    void onSearchTextChanged(const QString& text);

private:
    // Services
    RME::core::IBrushStateService* m_brushStateService;
    RME::core::IClientDataService* m_clientDataService;
    
    void setupHouseList();
    void setupSearchControls();
    void setupHouseInfo();
    void setupHouseControls();
    void connectSignals();
    
    void updateHouseInfo(int houseId);
    void showHouseInformation(int houseId);
    
    // UI components
    QGroupBox* m_searchWidget;
    QLineEdit* m_searchEdit;
    
    QListWidget* m_houseList;
    
    QGroupBox* m_houseInfoWidget;
    QLabel* m_houseInfoLabel;
    
    QGroupBox* m_houseControlsWidget;
    QPushButton* m_createButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    
    // Data
    // TODO: Add HouseManager integration when available
};

} // namespace ui
} // namespace RME