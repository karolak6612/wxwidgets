#ifndef RME_CREATURE_PROPERTIES_DIALOG_H
#define RME_CREATURE_PROPERTIES_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>

// Forward declarations
namespace RME {
namespace core {
namespace creatures {
    class Creature;
}
}
}

namespace RME {
namespace ui {
namespace dialogs {

/**
 * @brief Properties dialog for editing creature properties
 * 
 * Dialog for editing creature spawn interval and direction. Follows the UI-04 
 * specification for CreaturePropertiesDialogQt.
 */
class CreaturePropertiesDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreaturePropertiesDialog(QWidget* parent, RME::core::creatures::Creature* creatureCopy);
    ~CreaturePropertiesDialog() override = default;

    // Result access
    bool wasModified() const { return m_wasModified; }
    RME::core::creatures::Creature* getModifiedCreature() const { return m_creatureCopy; }

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onSpawnIntervalChanged();
    void onDirectionChanged();

signals:
    void creatureModified(RME::core::creatures::Creature* creature);

private:
    // Core data
    RME::core::creatures::Creature* m_creatureCopy = nullptr;
    RME::core::creatures::Creature* m_originalCreature = nullptr; // Backup for cancel
    bool m_wasModified = false;

    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QFormLayout* m_formLayout = nullptr;
    QLabel* m_creatureNameLabel = nullptr;
    QLabel* m_positionLabel = nullptr;
    QSpinBox* m_spawnIntervalSpin = nullptr;
    QComboBox* m_directionCombo = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;

    // Helper methods
    void setupUI();
    void setupDirectionCombo();
    void loadCreatureData();
    void saveCreatureData();
    bool validateInput();
    void markAsModified();
    void createBackup();
    void restoreBackup();
    void connectSignals();
};

} // namespace dialogs
} // namespace ui
} // namespace RME

#endif // RME_CREATURE_PROPERTIES_DIALOG_H