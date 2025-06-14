#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include "core/assets/CreatureDatabase.h" // Class to test
#include "core/assets/CreatureData.h"     // For enums and struct access

using namespace RME;

class TestCreatureDatabase : public QObject
{
    Q_OBJECT

private:
    QString sampleRmeCreaturesXmlPath;
    QString sampleOtsMonsterXmlPath;
    QString tempOtsRatPath; // For specific merge test

    void ensureSampleRmeCreaturesXmlExists() {
        if (QFile::exists(sampleRmeCreaturesXmlPath)) return;
        QFile file(sampleRmeCreaturesXmlPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<creatures>\n"
                << "  <creature name=\"Rat\" script=\"creatures/rat.lua\" type=\"monster\">\n"
                << "    <look type=\"21\" head=\"10\" body=\"20\" legs=\"30\" feet=\"40\" addons=\"0\" mount=\"0\"/>\n"
                << "    <attribute key=\"health_max\" value=\"25\"/>\n"
                << "    <attribute key=\"description\" value=\"A common city rat.\"/>\n"
                << "  </creature>\n"
                << "  <creature name=\"Sam\" script=\"npcs/sam.lua\" type=\"npc\">\n"
                << "    <look type=\"136\" head=\"78\" body=\"88\" legs=\"67\" feet=\"95\" addons=\"1\"/>\n"
                << "    <attribute key=\"mana_max\" value=\"1000\"/>\n"
                << "  </creature>\n"
                << "</creatures>";
            file.close();
        } else { qWarning() << "Could not create " << sampleRmeCreaturesXmlPath; }
    }

    void ensureSampleOtsMonsterXmlExists() {
        if (QFile::exists(sampleOtsMonsterXmlPath)) return;
        QFile file(sampleOtsMonsterXmlPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<monster name=\"Dragon Lord\" nameDescription=\"a dragon lord\" race=\"blood\" experience=\"2100\" speed=\"280\" manacost=\"0\">\n"
                << "  <health now=\"1900\" max=\"1900\"/>\n"
                << "  <look type=\"39\" head=\"0\" body=\"0\" legs=\"0\" feet=\"0\" addons=\"0\" corpse=\"5984\"/>\n"
                << "  <flags>\n"
                << "    <flag summonable=\"0\"/>\n"
                << "    <flag hostile=\"1\"/>\n"
                << "  </flags>\n"
                << "  <defenses armor=\"30\" defense=\"35\" />\n"
                << "  <voices interval=\"5000\" chance=\"10\"><voice sentence=\"GROOOOAAAAR!\"/></voices>\n"
                << "</monster>";
            file.close();
        } else { qWarning() << "Could not create " << sampleOtsMonsterXmlPath; }
    }

    void createTempOtsRatFile() {
        QFile file(tempOtsRatPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<monster name=\"Rat\" nameDescription=\"a sewer rat\" speed=\"150\">\n"
                << "  <health now=\"30\" max=\"30\"/>\n"
                << "  <look type=\"57\" corpse=\"2900\"/>\n" // Different looktype, new corpse
                << "  <flags><flag convinceable=\"1\"/></flags>\n"
                << "</monster>";
            file.close();
        } else { qWarning() << "Could not create " << tempOtsRatPath; }
    }


private slots:
    void initTestCase();
    void cleanupTestCase();

    void testLoadNonExistentXml();
    void testLoadRmeCreaturesXml();
    void testImportOtServerMonsterXml();
    void testMergeAndOverride();
};

void TestCreatureDatabase::initTestCase() {
    sampleRmeCreaturesXmlPath = "test_sample_creatures_rme.xml";
    sampleOtsMonsterXmlPath = "test_sample_monster_ots.xml";
    tempOtsRatPath = "test_temp_ots_rat.xml";
    ensureSampleRmeCreaturesXmlExists();
    ensureSampleOtsMonsterXmlExists();
}

void TestCreatureDatabase::cleanupTestCase() {
    QFile::remove(sampleRmeCreaturesXmlPath);
    QFile::remove(sampleOtsMonsterXmlPath);
    QFile::remove(tempOtsRatPath);
}

void TestCreatureDatabase::testLoadNonExistentXml() {
    CreatureDatabase cdb;
    QVERIFY(!cdb.loadFromXML("non_existent_creatures.xml"));
    QVERIFY(!cdb.importFromOtServerXml("non_existent_monster.xml"));
}

void TestCreatureDatabase::testLoadRmeCreaturesXml() {
    CreatureDatabase cdb;
    QVERIFY(cdb.loadFromXML(sampleRmeCreaturesXmlPath));
    QCOMPARE(cdb.getCreatureCount(), 2);

    const RME::core::assets::CreatureData* rat = cdb.getCreatureData("Rat");
    QVERIFY(rat != nullptr && rat->name == "Rat");
    if (rat) {
        QCOMPARE(rat->scriptName, QString("creatures/rat.lua"));
        QVERIFY(!(rat->flags & RME::core::assets::CreatureTypeFlag::IS_NPC));
        QCOMPARE(rat->outfit.lookType, 21);
        QCOMPARE(rat->outfit.head, 10);
        QCOMPARE(rat->outfit.body, 20);
        QCOMPARE(rat->outfit.legs, 30);
        QCOMPARE(rat->outfit.feet, 40);
        QCOMPARE(rat->healthMax, 25); // Parsed from <attribute key="health_max">
        QCOMPARE(rat->genericAttributes.value("description").toString(), QString("A common city rat."));
    }

    const RME::core::assets::CreatureData* sam = cdb.getCreatureData("Sam");
    QVERIFY(sam != nullptr && sam->name == "Sam");
    if (sam) {
        QCOMPARE(sam->scriptName, QString("npcs/sam.lua"));
        QVERIFY(sam->flags & RME::core::assets::CreatureTypeFlag::IS_NPC);
        QCOMPARE(sam->outfit.lookType, 136);
        QCOMPARE(sam->outfit.addons, 1);
        QCOMPARE(sam->manaMax, 1000); // Parsed from <attribute key="mana_max">
    }
}

void TestCreatureDatabase::testImportOtServerMonsterXml() {
    CreatureDatabase cdb;
    QVERIFY(cdb.importFromOtServerXml(sampleOtsMonsterXmlPath));
    QCOMPARE(cdb.getCreatureCount(), 1);

    const RME::core::assets::CreatureData* dl = cdb.getCreatureData("Dragon Lord");
    QVERIFY(dl != nullptr && dl->name == "Dragon Lord");
    if (dl) {
        QVERIFY(!(dl->flags & RME::core::assets::CreatureTypeFlag::IS_NPC));
        QCOMPARE(dl->outfit.lookType, 39);
        QCOMPARE(dl->genericAttributes.value("corpseid").toUInt(), 5984u);
        QCOMPARE(dl->healthMax, 1900);
        QVERIFY(dl->genericAttributes.value("flag_hostile").toBool());
        QVERIFY(!dl->genericAttributes.value("flag_summonable").toBool());
        QCOMPARE(dl->genericAttributes.value("armor").toInt(), 30);
        QCOMPARE(dl->genericAttributes.value("defense").toInt(), 35);
        QCOMPARE(dl->genericAttributes.value("voices").toString(), QString("GROOOOAAAAR!"));
        QCOMPARE(dl->genericAttributes.value("nameDescription").toString(), QString("a dragon lord"));
        QCOMPARE(dl->genericAttributes.value("race").toString(), QString("blood"));
    }
}

void TestCreatureDatabase::testMergeAndOverride() {
    CreatureDatabase cdb;
    QVERIFY(cdb.loadFromXML(sampleRmeCreaturesXmlPath)); // Loads Rat (health 25, look 21) & Sam

    createTempOtsRatFile(); // Create OTS Rat file (health 30, look 57)
    QVERIFY(cdb.importFromOtServerXml(tempOtsRatPath));

    QCOMPARE(cdb.getCreatureCount(), 2); // Rat should be updated, Sam remains

    const RME::core::assets::CreatureData* rat = cdb.getCreatureData("Rat");
    QVERIFY(rat != nullptr && rat->name == "Rat");
    if (rat) {
        QCOMPARE(rat->outfit.lookType, 57); // Overridden by OTS
        QCOMPARE(rat->healthMax, 30);       // Overridden by OTS
        // Script name from RME should persist if not in OTS Rat file
        QCOMPARE(rat->scriptName, QString("creatures/rat.lua"));
        // Description from RME should persist
        QCOMPARE(rat->genericAttributes.value("description").toString(), QString("A common city rat."));
        // New attributes from OTS Rat
        QCOMPARE(rat->genericAttributes.value("nameDescription").toString(), QString("a sewer rat"));
        QCOMPARE(rat->genericAttributes.value("speed").toInt(), 150);
        QCOMPARE(rat->genericAttributes.value("corpseid").toUInt(), 2900u);
        QVERIFY(rat->genericAttributes.value("flag_convinceable").toBool());
    }

    const RME::core::assets::CreatureData* sam = cdb.getCreatureData("Sam"); // Verify Sam is untouched
    QVERIFY(sam != nullptr && sam->name == "Sam");
    if (sam) {
        QCOMPARE(sam->outfit.lookType, 136);
    }
}

QTEST_MAIN(TestCreatureDatabase)
#include "TestCreatureDatabase.moc"
