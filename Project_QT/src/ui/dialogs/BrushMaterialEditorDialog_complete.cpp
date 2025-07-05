#include "BrushMaterialEditorDialog.h"
#include "core/utils/ResourcePathManager.h"
#include "ItemFinderDialogQt.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QTabWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDomDocument>
#include <QTextStream>
#include <QDebug>

namespace RME {
namespace ui {
namespace dialogs {

BrushMaterialEditorDialog::BrushMaterialEditorDialog(QWidget* parent)
    : QDialog(parent)
    , m_currentBrushType(BrushType::Ground)
    , m_currentBorderType(BorderType::None)
    , m_currentGroundBrush(nullptr)
    , m_currentWallBrush(nullptr)
    , m_currentDoodadBrush(nullptr)
    , m_currentCarpetBrush(nullptr)
    , m_currentTableBrush(nullptr)
    , m_currentBorderBrush(nullptr)
    , m_itemDatabase(nullptr)
    , m_materialManager(nullptr)
    , m_editorController(nullptr)
{
    setWindowTitle(tr("Brush Material Editor"));
    setMinimumSize(800, 600);
    
    createUI();
    loadMaterials();
}

BrushMaterialEditorDialog::~BrushMaterialEditorDialog()
{
    // Clean up any resources if needed
}

void BrushMaterialEditorDialog::setItemDatabase(RME::core::assets::ItemDatabase* itemDatabase)
{
    m_itemDatabase = itemDatabase;
}

void BrushMaterialEditorDialog::setMaterialManager(RME::core::assets::MaterialManager* materialManager)
{
    m_materialManager = materialManager;
}

void BrushMaterialEditorDialog::setEditorController(RME::editor_logic::EditorController* editorController)
{
    m_editorController = editorController;
}

void BrushMaterialEditorDialog::createUI()
{
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Tab widget for different brush types
    QTabWidget* tabWidget = new QTabWidget(this);
    
    // Create tabs for different brush types
    QWidget* groundTab = createGroundTab();
    QWidget* wallTab = createWallTab();
    QWidget* doodadTab = createDoodadTab();
    QWidget* carpetTab = createCarpetTab();
    QWidget* tableTab = createTableTab();
    QWidget* borderTab = createBorderTab();
    
    // Add tabs to tab widget
    tabWidget->addTab(groundTab, tr("Ground"));
    tabWidget->addTab(wallTab, tr("Wall"));
    tabWidget->addTab(doodadTab, tr("Doodad"));
    tabWidget->addTab(carpetTab, tr("Carpet"));
    tabWidget->addTab(tableTab, tr("Table"));
    tabWidget->addTab(borderTab, tr("Border"));
    
    // Connect tab changed signal
    connect(tabWidget, &QTabWidget::currentChanged, this, &BrushMaterialEditorDialog::onTabChanged);
    
    // Add tab widget to main layout
    mainLayout->addWidget(tabWidget);
    
    // Bottom buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    // Save button
    QPushButton* saveButton = new QPushButton(tr("Save"), this);
    connect(saveButton, &QPushButton::clicked, this, &BrushMaterialEditorDialog::onSaveClicked);
    
    // Cancel button
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, &QPushButton::clicked, this, &BrushMaterialEditorDialog::reject);
    
    // Add buttons to button layout
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    // Add button layout to main layout
    mainLayout->addLayout(buttonLayout);
    
    // Set main layout
    setLayout(mainLayout);
}

QWidget* BrushMaterialEditorDialog::createGroundTab()
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // Ground brush list
    m_groundBrushList = new QListWidget(tab);
    connect(m_groundBrushList, &QListWidget::currentRowChanged, this, &BrushMaterialEditorDialog::onGroundBrushSelected);
    
    // Ground brush properties
    QGroupBox* propertiesGroup = new QGroupBox(tr("Properties"), tab);
    QFormLayout* propertiesLayout = new QFormLayout(propertiesGroup);
    
    m_groundNameEdit = new QLineEdit(propertiesGroup);
    m_groundItemIdEdit = new QLineEdit(propertiesGroup);
    m_groundItemButton = new QPushButton(tr("Select Item..."), propertiesGroup);
    connect(m_groundItemButton, &QPushButton::clicked, this, &BrushMaterialEditorDialog::onSelectGroundItemClicked);
    
    propertiesLayout->addRow(tr("Name:"), m_groundNameEdit);
    propertiesLayout->addRow(tr("Item ID:"), m_groundItemIdEdit);
    propertiesLayout->addRow(tr(""), m_groundItemButton);
    
    // Add/Remove buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* addButton = new QPushButton(tr("Add"), tab);
    QPushButton* removeButton = new QPushButton(tr("Remove"), tab);
    
    connect(addButton, &QPushButton::clicked, this, &BrushMaterialEditorDialog::onAddGroundBrushClicked);
    connect(removeButton, &QPushButton::clicked, this, &BrushMaterialEditorDialog::onRemoveGroundBrushClicked);
    
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addStretch();
    
    // Add widgets to layout
    layout->addWidget(m_groundBrushList);
    layout->addWidget(propertiesGroup);
    layout->addLayout(buttonLayout);
    
    return tab;
}

// Placeholder implementations for other tab creation methods
QWidget* BrushMaterialEditorDialog::createWallTab()
{
    QWidget* tab = new QWidget(this);
    // TODO: Implement wall tab
    return tab;
}

QWidget* BrushMaterialEditorDialog::createDoodadTab()
{
    QWidget* tab = new QWidget(this);
    // TODO: Implement doodad tab
    return tab;
}

QWidget* BrushMaterialEditorDialog::createCarpetTab()
{
    QWidget* tab = new QWidget(this);
    // TODO: Implement carpet tab
    return tab;
}

QWidget* BrushMaterialEditorDialog::createTableTab()
{
    QWidget* tab = new QWidget(this);
    // TODO: Implement table tab
    return tab;
}