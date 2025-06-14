#ifndef RME_CREATURE_DATABASE_H
#define RME_CREATURE_DATABASE_H

#include "CreatureData.h"
#include <QMap>
#include <QString>
#include <QScopedPointer> // For PIMPL

class QXmlStreamReader; // Forward declaration

namespace RME {

class CreatureDatabase {
public:
    CreatureDatabase();
    ~CreatureDatabase();

    bool loadFromXML(const QString& filePath); // For main creatures.xml
    bool importFromOtServerXml(const QString& filePath); // For individual monster/NPC XML files

    const CreatureData* getCreatureData(const QString& name) const;
    const CreatureData& getDefaultCreatureData() const;

    int getCreatureCount() const;
    QMap<QString, CreatureData> getAllCreatures() const; // Returns a copy

private:
    void parseCreatureNode(QXmlStreamReader& xml, CreatureData& creatureData, bool isServerFormat);

    struct CreatureDatabaseData; // PIMPL
    QScopedPointer<CreatureDatabaseData> d;

    CreatureData invalidCreatureData;
};

} // namespace RME

#endif // RME_CREATURE_DATABASE_H
