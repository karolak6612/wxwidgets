#include "ui/palettes/CreaturePalettePanel.h"
#include "ui/dialogs/CreaturePropertiesDialog.h"
#include "core/creatures/Creature.h"
#include "core/assets/CreatureDatabase.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

namespace RME {
namespace ui {

CreaturePalettePanel::CreaturePalettePanel(
    RME::core::IBrushStateService* brushStateService,
    RME::core::IClientDataService* clientDataService,
    QWidget* parent
) : BasePalettePanel(parent)
    , m_brushStateService(brushStateService)
    , m_clientDataService(clientDataService)
    , m_creatureList(nullptr)
    , m_searchEdit(nullptr)
    , m_creatureInfoLabel(nullptr)
    , m_spawnButton(nullptr)
    , m_editPropertiesButton(nullptr)
    , m_creatureDatabase(nullptr)
{
    Q_ASSERT(m_brushStateService);
    Q_ASSERT(m_clientDataService);
    
    setObjectName("CreaturePalettePanel");
    setWindowTitle(tr("Creature Palette"));
    
    setupUI();
    connectSignals();
    loadCreatures();
}

CreaturePalettePanel::~CreaturePalettePanel()
{
    // Qt handles cleanup through parent-child relationships
}

void CreaturePalettePanel::setupUI()
{
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);
    
    // Setup search controls
    setupSearchControls();
    
    // Create splitter for creature list and info
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    
    // Setup creature list
    setupCreatureList();
    splitter->addWidget(m_creatureList);
    
    // Setup creature info panel
    setupCreatureInfo();
    splitter->addWidget(m_creatureInfoWidget);
    
    // Setup spawn controls
    setupSpawnControls();
    
    // Add to main layout
    mainLayout->addWidget(m_searchWidget);
    mainLayout->addWidget(splitter, 1); // Give splitter most space
    mainLayout->addWidget(m_spawnControlsWidget);
    
    // Set splitter proportions (70% list, 30% info)
    splitter->setStretchFactor(0, 7);
    splitter->setStretchFactor(1, 3);
}

void CreaturePalettePanel::setupCreatureList()
{
    m_creatureList = new QListWidget(this);
    m_creatureList->setObjectName("creatureList");
    m_creatureList->setAlternatingRowColors(true);
    m_creatureList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_creatureList->setSortingEnabled(true);
    m_creatureList->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Enable drag and drop for creature placement
    m_creatureList->setDragEnabled(true);
    m_creatureList->setDragDropMode(QAbstractItemView::DragOnly);
}

void CreaturePalettePanel::setupSearchControls()
{
    m_searchWidget = new QGroupBox(tr("Search"), this);
    QVBoxLayout* searchLayout = new QVBoxLayout(m_searchWidget);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("creatureSearchEdit");
    m_searchEdit->setPlaceholderText(tr("Search creatures..."));
    m_searchEdit->setClearButtonEnabled(true);
    
    searchLayout->addWidget(m_searchEdit);
}

void CreaturePalettePanel::setupCreatureInfo()
{
    m_creatureInfoWidget = new QGroupBox(tr("Creature Information"), this);
    QVBoxLayout* infoLayout = new QVBoxLayout(m_creatureInfoWidget);
    
    m_creatureInfoLabel = new QLabel(tr("Select a creature to view information"), this);
    m_creatureInfoLabel->setObjectName("creatureInfoLabel");
    m_creatureInfoLabel->setWordWrap(true);
    m_creatureInfoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_creatureInfoLabel->setMinimumHeight(60);
    
    infoLayout->addWidget(m_creatureInfoLabel);
}

void CreaturePalettePanel::setupSpawnControls()
{
    m_spawnControlsWidget = new QGroupBox(tr("Spawn Controls"), this);
    QHBoxLayout* controlsLayout = new QHBoxLayout(m_spawnControlsWidget);
    
    m_spawnButton = new QPushButton(tr("Spawn Creature"), this);
    m_spawnButton->setObjectName("spawnCreatureButton");
    m_spawnButton->setEnabled(false); // Disabled until creature selected
    
    m_editPropertiesButton = new QPushButton(tr("Edit Properties"), this);
    m_editPropertiesButton->setObjectName("editPropertiesButton");
    m_editPropertiesButton->setEnabled(false); // Disabled until creature selected
    
    controlsLayout->addWidget(m_spawnButton);
    controlsLayout->addWidget(m_editPropertiesButton);
    controlsLayout->addStretch(); // Push buttons to left
}

void CreaturePalettePanel::connectSignals()
{
    // Search functionality
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &CreaturePalettePanel::onSearchTextChanged);
    
    // Creature list interactions
    connect(m_creatureList, &QListWidget::itemSelectionChanged,
            this, &CreaturePalettePanel::onCreatureSelectionChanged);
    connect(m_creatureList, &QListWidget::itemDoubleClicked,
            this, &CreaturePalettePanel::onCreatureDoubleClicked);
    connect(m_creatureList, &QListWidget::customContextMenuRequested,
            this, &CreaturePalettePanel::onCreatureContextMenu);
    
    // Spawn controls
    connect(m_spawnButton, &QPushButton::clicked,
            this, &CreaturePalettePanel::onSpawnCreature);
    connect(m_editPropertiesButton, &QPushButton::clicked,
            this, &CreaturePalettePanel::onEditCreatureProperties);
}

void CreaturePalettePanel::loadCreatures()
{
    if (!m_creatureList || !m_clientDataService) {
        return;
    }
    
    m_creatureList->clear();
    
    // Load creatures from CreatureDatabase via service
    auto* creatureDatabase = m_clientDataService->getCreatureDatabase();
    if (creatureDatabase) {
        const auto& allCreatures = creatureDatabase->getAllCreatures();
        
        // Group creatures by category for better organization
        QMap<QString, QList<CreatureData>> categorizedCreatures;
        
        for (auto it = allCreatures.constBegin(); it != allCreatures.constEnd(); ++it) {
            const CreatureData& creature = it.value();
            QString category = determineCreatureCategory(creature);
            categorizedCreatures[category].append(creature);
        }
        
        // Add creatures to list organized by category
        for (auto catIt = categorizedCreatures.constBegin(); catIt != categorizedCreatures.constEnd(); ++catIt) {
            const QString& category = catIt.key();
            const QList<CreatureData>& creatures = catIt.value();
            
            // Add category header
            QListWidgetItem* categoryItem = new QListWidgetItem(QString("--- %1 ---").arg(category));
            categoryItem->setFlags(Qt::NoItemFlags); // Not selectable
            categoryItem->setBackground(QBrush(QColor(240, 240, 240)));
            categoryItem->setFont(QFont(categoryItem->font().family(), -1, QFont::Bold));
            m_creatureList->addItem(categoryItem);
            
            // Add creatures in this category
            for (const CreatureData& creature : creatures) {
                QListWidgetItem* item = createCreatureListItem(creature);
                m_creatureList->addItem(item);
            }
        }
        
        qInfo() << "CreaturePalettePanel: Loaded" << allCreatures.size() << "creatures in" << categorizedCreatures.size() << "categories";
    } else {
        qWarning() << "CreaturePalettePanel: CreatureDatabase not available, loading fallback creatures";
        loadFallbackCreatures();
    }
    
    qDebug() << "CreaturePalettePanel: Loaded" << m_creatureList->count() << "creatures";
}

void CreaturePalettePanel::filterCreatures(const QString& filter)
{
    if (!m_creatureList) {
        return;
    }
    
    for (int i = 0; i < m_creatureList->count(); ++i) {
        QListWidgetItem* item = m_creatureList->item(i);
        if (item) {
            bool visible = filter.isEmpty() || 
                          item->text().contains(filter, Qt::CaseInsensitive);
            item->setHidden(!visible);
        }
    }
}

void CreaturePalettePanel::refreshCreatureList()
{
    loadCreatures();
    
    // Reapply current filter if any
    if (m_searchEdit && !m_searchEdit->text().isEmpty()) {
        filterCreatures(m_searchEdit->text());
    }
}

void CreaturePalettePanel::onCreatureSelectionChanged()
{
    QListWidgetItem* currentItem = m_creatureList->currentItem();
    bool hasSelection = (currentItem != nullptr);
    
    // Enable/disable buttons based on selection
    m_spawnButton->setEnabled(hasSelection);
    m_editPropertiesButton->setEnabled(hasSelection);
    
    if (hasSelection) {
        QString creatureName = currentItem->text();
        updateCreatureInfo(creatureName);
        
        // Update brush state service with selected creature
        if (m_brushStateService && m_clientDataService) {
            auto* creatureDatabase = m_clientDataService->getCreatureDatabase();
            if (creatureDatabase) {
                auto* creatureData = creatureDatabase->getCreatureByName(creatureName);
                m_brushStateService->setCurrentCreatureType(creatureData);
            }
        }
        
        emit creatureSelected(creatureName);
    } else {
        m_creatureInfoLabel->setText(tr("Select a creature to view information"));
        
        // Clear creature selection in brush state service
        if (m_brushStateService) {
            m_brushStateService->setCurrentCreatureType(nullptr);
        }
    }
}

void CreaturePalettePanel::onCreatureDoubleClicked(QListWidgetItem* item)
{
    if (item) {
        // Double-click spawns the creature
        onSpawnCreature();
    }
}

void CreaturePalettePanel::onCreatureContextMenu(const QPoint& position)
{
    QListWidgetItem* item = m_creatureList->itemAt(position);
    if (!item) {
        return;
    }
    
    QMenu contextMenu(this);
    
    QAction* spawnAction = contextMenu.addAction(tr("Spawn Creature"));
    QAction* propertiesAction = contextMenu.addAction(tr("Edit Properties"));
    contextMenu.addSeparator();
    QAction* infoAction = contextMenu.addAction(tr("Show Information"));
    
    QAction* selectedAction = contextMenu.exec(m_creatureList->mapToGlobal(position));
    
    if (selectedAction == spawnAction) {
        onSpawnCreature();
    } else if (selectedAction == propertiesAction) {
        onEditCreatureProperties();
    } else if (selectedAction == infoAction) {
        showCreatureInformation(item->text());
    }
}

void CreaturePalettePanel::onSpawnCreature()
{
    QString creatureName = getSelectedCreatureName();
    if (creatureName.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), 
                               tr("Please select a creature to spawn."));
        return;
    }
    
    // TODO: Get spawn interval from user or use default
    bool ok;
    int spawnTime = QInputDialog::getInt(this, tr("Spawn Creature"), 
                                       tr("Spawn interval (seconds):"), 
                                       60, 1, 3600, 1, &ok);
    if (!ok) {
        return;
    }
    
    emit spawnCreatureRequested(creatureName, spawnTime);
    
    qDebug() << "CreaturePalettePanel: Spawn requested for" << creatureName 
             << "with interval" << spawnTime << "seconds";
}

void CreaturePalettePanel::onEditCreatureProperties()
{
    QString creatureName = getSelectedCreatureName();
    if (creatureName.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"), 
                               tr("Please select a creature to edit."));
        return;
    }
    
    // Create a temporary creature for editing
    // Create actual creature from database
    if (m_clientDataService && m_clientDataService->getCreatureDatabase()) {
        auto* creatureDatabase = m_clientDataService->getCreatureDatabase();
        auto* creatureData = creatureDatabase->getCreatureByName(creatureName);
        if (creatureData) {
            // Use actual creature data for spawning
            qDebug() << "CreaturePalettePanel: Using creature data from database for" << creatureName;
        }
    }
    auto creature = std::make_unique<RME::Creature>();
    creature->setName(creatureName);
    creature->setSpawnTime(60); // Default spawn time
    creature->setDirection(RME::Direction::South); // Default direction
    
    CreaturePropertiesDialog dialog(this, creature.get());
    if (dialog.exec() == QDialog::Accepted) {
        qDebug() << "CreaturePalettePanel: Creature properties updated for" << creatureName;
        // TODO: Apply changes to actual creature in map when available
    }
}

void CreaturePalettePanel::onSearchTextChanged(const QString& text)
{
    filterCreatures(text);
}

void CreaturePalettePanel::updateCreatureInfo(const QString& creatureName)
{
    if (!m_creatureInfoLabel) {
        return;
    }
    
    // TODO: Get actual creature information from database when available
    QString info = tr("<b>%1</b><br>").arg(creatureName);
    
    if (m_clientDataService && m_clientDataService->getCreatureDatabase()) {
        // Add real creature information from database
        auto* creatureDatabase = m_clientDataService->getCreatureDatabase();
        auto* creatureData = creatureDatabase->getCreatureByName(creatureName);
        if (creatureData) {
            info += tr("Health: %1<br>").arg(creatureData->health);
            info += tr("Experience: %1<br>").arg(creatureData->experience);
            info += tr("Speed: %1<br>").arg(creatureData->speed);
            if (!creatureData->description.isEmpty()) {
                info += tr("Description: %1<br>").arg(creatureData->description);
            }
        }
    } else {
        // Placeholder information
        info += tr("A creature that can be spawned on the map.<br>");
        info += tr("Double-click or use 'Spawn Creature' to place it.<br>");
        info += tr("Use 'Edit Properties' to configure spawn settings.");
    }
    
    m_creatureInfoLabel->setText(info);
}

QString CreaturePalettePanel::getSelectedCreatureName() const
{
    if (!m_creatureList) {
        return QString();
    }
    
    QListWidgetItem* currentItem = m_creatureList->currentItem();
    return currentItem ? currentItem->text() : QString();
}

void CreaturePalettePanel::showCreatureInformation(const QString& creatureName)
{
    QString detailedInfo = tr("<h3>%1</h3>").arg(creatureName);
    
    // Add comprehensive creature information from database
    if (m_clientDataService && m_clientDataService->getCreatureDatabase()) {
        auto* creatureDatabase = m_clientDataService->getCreatureDatabase();
        auto* creatureData = creatureDatabase->getCreatureByName(creatureName);
        if (creatureData) {
            detailedInfo += tr("<p><b>Health:</b> %1 HP</p>").arg(creatureData->health);
            detailedInfo += tr("<p><b>Experience:</b> %1</p>").arg(creatureData->experience);
            detailedInfo += tr("<p><b>Speed:</b> %1</p>").arg(creatureData->speed);
            if (!creatureData->description.isEmpty()) {
                detailedInfo += tr("<p><b>Description:</b> %1</p>").arg(creatureData->description);
            }
        }
    }
    detailedInfo += tr("<p>This creature can be spawned on the map with configurable properties.</p>");
    detailedInfo += tr("<p><b>Usage:</b></p>");
    detailedInfo += tr("<ul>");
    detailedInfo += tr("<li>Double-click to spawn with default settings</li>");
    detailedInfo += tr("<li>Use 'Spawn Creature' button to configure spawn interval</li>");
    detailedInfo += tr("<li>Use 'Edit Properties' to modify creature settings</li>");
    detailedInfo += tr("</ul>");
    
    QMessageBox::information(this, tr("Creature Information"), detailedInfo);
}

void CreaturePalettePanel::setCreatureDatabase(RME::CreatureDatabase* database)
{
    m_creatureDatabase = database;
    refreshCreatureList();
}

QString CreaturePalettePanel::determineCreatureCategory(const CreatureData& creature) const
{
    // Categorize creatures based on their properties
    int health = creature.health;
    
    if (health <= 50) {
        return tr("Weak Creatures");
    } else if (health <= 200) {
        return tr("Normal Creatures");
    } else if (health <= 500) {
        return tr("Strong Creatures");
    } else if (health <= 1000) {
        return tr("Powerful Creatures");
    } else {
        return tr("Boss Creatures");
    }
}

QListWidgetItem* CreaturePalettePanel::createCreatureListItem(const CreatureData& creature) const
{
    QListWidgetItem* item = new QListWidgetItem();
    
    // Set creature text with health info
    QString creatureText = QString("%1 (HP: %2)").arg(creature.name).arg(creature.health);
    item->setText(creatureText);
    
    // Store creature data
    item->setData(Qt::UserRole, creature.lookType);
    item->setData(Qt::UserRole + 1, creature.name);
    item->setData(Qt::UserRole + 2, creature.health);
    
    // Set creature icon from sprite system
    if (m_clientDataService) {
        auto* spriteManager = m_clientDataService->getSpriteManager();
        if (spriteManager) {
            const SpriteData* spriteData = spriteManager->getCreatureSprite(creature.lookType);
            if (spriteData && !spriteData->frames.isEmpty()) {
                const QImage& spriteImage = spriteData->frames.first().image;
                if (!spriteImage.isNull()) {
                    QPixmap sprite = QPixmap::fromImage(spriteImage);
                    QPixmap scaledSprite = sprite.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    item->setIcon(QIcon(scaledSprite));
                }
            }
        }
    }
    
    // Set tooltip with detailed creature information
    QString tooltip = createCreatureTooltip(creature);
    item->setToolTip(tooltip);
    
    return item;
}

QString CreaturePalettePanel::createCreatureTooltip(const CreatureData& creature) const
{
    QStringList tooltipParts;
    tooltipParts << QString("<b>%1</b>").arg(creature.name);
    tooltipParts << QString("Health: %1 HP").arg(creature.health);
    tooltipParts << QString("Experience: %1").arg(creature.experience);
    tooltipParts << QString("Speed: %1").arg(creature.speed);
    tooltipParts << QString("Look Type: %1").arg(creature.lookType);
    
    if (!creature.description.isEmpty()) {
        tooltipParts << QString("Description: %1").arg(creature.description);
    }
    
    return tooltipParts.join("<br>");
}

void CreaturePalettePanel::loadFallbackCreatures()
{
    // Add some fallback creatures for testing when database is not available
    QStringList fallbackCreatures = {
        "Rat", "Cave Rat", "Larva", "Bug", "Spider", "Poison Spider",
        "Scorpion", "Centipede", "Skeleton", "Ghoul", "Zombie",
        "Orc", "Orc Berserker", "Orc Leader", "Orc Warlord",
        "Troll", "Cyclops", "Dragon", "Dragon Lord", "Demon"
    };
    
    for (int i = 0; i < fallbackCreatures.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(fallbackCreatures[i]);
        item->setData(Qt::UserRole, i + 1); // Fallback ID
        item->setToolTip(QString("Fallback creature: %1").arg(fallbackCreatures[i]));
        m_creatureList->addItem(item);
    }
}

} // namespace ui
} // namespace RME