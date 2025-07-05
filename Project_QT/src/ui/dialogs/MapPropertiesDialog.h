#ifndef RME_MAP_PROPERTIES_DIALOG_H
#define RME_MAP_PROPERTIES_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>

// Forward declarations
namespace RME {
namespace core {
    class Map;
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog for viewing and editing map properties
 * 
 * This dialog allows users to view and modify map properties such as
 * name, description, author, dimensions, and other metadata.
 */
class MapPropertiesDialog : public QDialog {
    Q_OBJECT

public:
    explicit MapPropertiesDialog(RME::core::Map* map, QWidget* parent = nullptr);
    ~MapPropertiesDialog() override = default;

public slots:
    void accept() override;
    void onCalculateStatistics();

private:
    void setupUI();
    void loadMapProperties();
    void saveMapProperties();
    void calculateMapStatistics();
    
    // Map reference
    RME::core::Map* m_map;
    
    // UI components
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // General tab
    QWidget* m_generalTab;
    QLineEdit* m_nameEdit;
    QLineEdit* m_authorEdit;
    QTextEdit* m_descriptionEdit;
    QSpinBox* m_widthSpin;
    QSpinBox* m_heightSpin;
    
    // Statistics tab
    QWidget* m_statisticsTab;
    QLabel* m_totalTilesLabel;
    QLabel* m_usedTilesLabel;
    QLabel* m_emptyTilesLabel;
    QLabel* m_itemCountLabel;
    QLabel* m_creatureCountLabel;
    QLabel* m_spawnCountLabel;
    QLabel* m_houseCountLabel;
    QLabel* m_waypointCountLabel;
    QPushButton* m_calculateButton;
    
    // Button layout
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_MAP_PROPERTIES_DIALOG_H