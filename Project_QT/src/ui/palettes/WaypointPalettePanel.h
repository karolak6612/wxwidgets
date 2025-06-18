#pragma once

#include "ui/palettes/BasePalettePanel.h"
#include <QWidget>

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

class WaypointData;

namespace ui {

class WaypointPalettePanel : public BasePalettePanel
{
    Q_OBJECT

public:
    explicit WaypointPalettePanel(QWidget* parent = nullptr);
    ~WaypointPalettePanel();

    // BasePalettePanel interface
    void setupUI() override;

    // Waypoint management
    void loadWaypoints();
    void refreshWaypointList();
    void filterWaypoints(const QString& filter);
    
    // Utility methods
    QString getSelectedWaypointName() const;
    void selectWaypoint(const QString& waypointName);

signals:
    void waypointSelected(const QString& waypointName);
    void createWaypointRequested();
    void editWaypointRequested(const QString& waypointName);
    void deleteWaypointRequested(const QString& waypointName);
    void waypointDoubleClicked(const QString& waypointName);

private slots:
    void onWaypointSelectionChanged();
    void onWaypointDoubleClicked(QListWidgetItem* item);
    void onWaypointContextMenu(const QPoint& position);
    void onCreateWaypoint();
    void onEditWaypoint();
    void onDeleteWaypoint();
    void onGoToWaypoint();
    void onSearchTextChanged(const QString& text);

private:
    void setupWaypointList();
    void setupSearchControls();
    void setupWaypointInfo();
    void setupWaypointControls();
    void connectSignals();
    
    void updateWaypointInfo(const QString& waypointName);
    void showWaypointInformation(const QString& waypointName);
    
    // UI components
    QGroupBox* m_searchWidget;
    QLineEdit* m_searchEdit;
    
    QListWidget* m_waypointList;
    
    QGroupBox* m_waypointInfoWidget;
    QLabel* m_waypointInfoLabel;
    
    QGroupBox* m_waypointControlsWidget;
    QPushButton* m_createButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_goToButton;
    
    // Data
    // TODO: Add WaypointManager integration when available
};

} // namespace ui
} // namespace RME