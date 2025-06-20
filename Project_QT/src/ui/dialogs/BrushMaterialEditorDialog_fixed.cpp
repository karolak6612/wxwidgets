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

QWidget* BrushMaterialEditorDialog::createBorderTab()
{
    QWidget* tab = new QWidget(this);
    // TODO: Implement border tab
    return tab;
}

void BrushMaterialEditorDialog::loadMaterials()
{
    // TODO: Load borders from borders.xml
    loadBordersFromXml();
    
    // TODO: Load ground brushes from grounds.xml
    loadGroundBrushesFromXml();
    
    // TODO: Load wall brushes from walls.xml
    loadWallBrushesFromXml();
    
    // TODO: Load doodad brushes from doodads.xml
    loadDoodadBrushesFromXml();
    
    // TODO: Load tilesets from tilesets.xml
    loadTilesetsFromXml();
}

void BrushMaterialEditorDialog::loadBordersFromXml()
{
    // Placeholder implementation
}

void BrushMaterialEditorDialog::loadGroundBrushesFromXml()
{
    // Placeholder implementation
}

void BrushMaterialEditorDialog::loadWallBrushesFromXml()
{
    // Placeholder implementation
}

void BrushMaterialEditorDialog::loadDoodadBrushesFromXml()
{
    // Placeholder implementation
}

void BrushMaterialEditorDialog::loadTilesetsFromXml()
{
    // Placeholder implementation
}

// Event handlers
void BrushMaterialEditorDialog::onTabChanged(int index)
{
    // Update current brush type based on selected tab
    switch (index) {
        case 0: m_currentBrushType = BrushType::Ground; break;
        case 1: m_currentBrushType = BrushType::Wall; break;
        case 2: m_currentBrushType = BrushType::Doodad; break;
        case 3: m_currentBrushType = BrushType::Carpet; break;
        case 4: m_currentBrushType = BrushType::Table; break;
        case 5: m_currentBrushType = BrushType::Border; break;
        default: m_currentBrushType = BrushType::Ground; break;
    }
}

void BrushMaterialEditorDialog::onGroundBrushSelected(int index)
{
    if (index < 0 || index >= m_groundBrushes.size()) {
        // Clear selection
        m_currentGroundBrush = nullptr;
        m_groundNameEdit->clear();
        m_groundItemIdEdit->clear();
        return;
    }
    
    // Update UI with selected brush
    m_currentGroundBrush = m_groundBrushes[index];
    m_groundNameEdit->setText(m_currentGroundBrush->name);
    m_groundItemIdEdit->setText(QString::number(m_currentGroundBrush->itemId));
}

void BrushMaterialEditorDialog::onSelectGroundItemClicked()
{
    // Open item finder dialog
    ItemFinderDialogQt itemFinder(m_itemDatabase, this);
    if (itemFinder.exec() == QDialog::Accepted) {
        uint32_t selectedItemId = itemFinder.getSelectedItemId();
        m_groundItemIdEdit->setText(QString::number(selectedItemId));
    }
}

void BrushMaterialEditorDialog::onAddGroundBrushClicked()
{
    // Create new ground brush
    GroundBrushData* newBrush = new GroundBrushData();
    newBrush->name = tr("New Ground Brush");
    newBrush->itemId = 0;
    
    // Add to list
    m_groundBrushes.append(newBrush);
    
    // Update UI
    m_groundBrushList->addItem(newBrush->name);
    m_groundBrushList->setCurrentRow(m_groundBrushList->count() - 1);
}

void BrushMaterialEditorDialog::onRemoveGroundBrushClicked()
{
    int currentRow = m_groundBrushList->currentRow();
    if (currentRow < 0 || currentRow >= m_groundBrushes.size()) {
        return;
    }
    
    // Remove from list
    delete m_groundBrushes[currentRow];
    m_groundBrushes.removeAt(currentRow);
    
    // Update UI
    delete m_groundBrushList->takeItem(currentRow);
    
    // Select next item
    if (currentRow < m_groundBrushList->count()) {
        m_groundBrushList->setCurrentRow(currentRow);
    } else if (m_groundBrushList->count() > 0) {
        m_groundBrushList->setCurrentRow(m_groundBrushList->count() - 1);
    }
}

void BrushMaterialEditorDialog::onSaveClicked()
{
    // Save all materials to XML files
    bool success = true;
    
    success &= saveBorderToXml();
    success &= saveGroundBrushesToXml();
    success &= saveWallBrushesToXml();
    success &= saveDoodadBrushesToXml();
    
    if (success) {
        accept();
    }
}

QString BrushMaterialEditorDialog::getXmlFilePath(const QString& filename) const
{
    // Use ResourcePathManager to resolve the path
    QString resolvedPath = RME::core::utils::ResourcePathManager::instance().resolvePath(filename, "xml");
    
    // If the file doesn't exist at the resolved path, use the app data path for creation
    if (!QFile::exists(resolvedPath)) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        return appDataPath + "/XML/" + filename;
    }
    
    return resolvedPath;
}

bool BrushMaterialEditorDialog::ensureXmlDirectoryExists() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString xmlDirPath = appDataPath + "/XML";
    
    QDir xmlDir(xmlDirPath);
    if (!xmlDir.exists()) {
        return xmlDir.mkpath(".");
    }
    return true;
}

// XML operations (placeholder implementations)
bool BrushMaterialEditorDialog::saveBorderToXml()
{
    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString bordersPath = getXmlFilePath("borders.xml");
    
    QDomDocument doc;
    QDomElement root;
    
    // Try to load existing file
    QFile existingFile(bordersPath);
    if (existingFile.exists() && existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&existingFile)) {
            existingFile.close();
            QMessageBox::warning(this, "Error", "Could not parse existing borders.xml file.");
            return false;
        }
        existingFile.close();
        root = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\"");
        doc.appendChild(xmlDeclaration);
        root = doc.createElement("borders");
        doc.appendChild(root);
    }
    
    // TODO: Save actual border data
    
    // Write to file
    QFile file(bordersPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open borders.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    
    return true;
}

bool BrushMaterialEditorDialog::saveGroundBrushesToXml()
{
    if (!ensureXmlDirectoryExists()) {
        QMessageBox::warning(this, "Error", "Could not create XML directory.");
        return false;
    }
    
    QString groundsPath = getXmlFilePath("grounds.xml");
    
    QDomDocument doc;
    QDomElement root;
    
    // Try to load existing file
    QFile existingFile(groundsPath);
    if (existingFile.exists() && existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (!doc.setContent(&existingFile)) {
            existingFile.close();
            QMessageBox::warning(this, "Error", "Could not parse existing grounds.xml file.");
            return false;
        }
        existingFile.close();
        root = doc.documentElement();
    } else {
        // Create new document
        QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\"");
        doc.appendChild(xmlDeclaration);
        root = doc.createElement("materials");
        doc.appendChild(root);
    }
    
    // TODO: Save actual ground brush data
    
    // Write to file
    QFile file(groundsPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open grounds.xml for writing.");
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toString(2);
    file.close();
    
    return true;
}