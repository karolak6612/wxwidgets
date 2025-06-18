#ifndef RME_EDIT_HOUSE_DIALOG_QT_H
#define RME_EDIT_HOUSE_DIALOG_QT_H

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QtGlobal>

// Forward declarations
namespace RME {
namespace core {
    namespace houses { class HouseData; }
    namespace world { class TownManager; }
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Dialog for editing house properties
 * 
 * Provides a modal dialog for editing house properties including name, town,
 * rent, ID, and guildhall status with proper validation.
 */
class EditHouseDialogQt : public QDialog {
    Q_OBJECT

public:
    explicit EditHouseDialogQt(QWidget* parent, 
                               RME::core::houses::HouseData* houseCopy,
                               RME::core::world::TownManager* townManager);
    ~EditHouseDialogQt() override = default;

private slots:
    void onAccepted();
    void onRejected();

private:
    void setupUI();
    void connectSignals();
    void loadData();
    bool applyChanges();
    bool validateInputs();
    void populateTownCombo();

    // UI components
    QFormLayout* m_formLayout = nullptr;
    QLineEdit* m_nameEdit = nullptr;
    QComboBox* m_townCombo = nullptr;
    QSpinBox* m_rentSpinBox = nullptr;
    QSpinBox* m_idSpinBox = nullptr;
    QCheckBox* m_guildhallCheck = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;

    // Data
    RME::core::houses::HouseData* m_houseCopy = nullptr;
    RME::core::world::TownManager* m_townManager = nullptr;
    quint32 m_originalHouseId = 0; // Store original ID for validation
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_EDIT_HOUSE_DIALOG_QT_H