#ifndef RME_ITEM_DATABASE_H
#define RME_ITEM_DATABASE_H

#include "ItemData.h"
#include <QMap>
#include <QString>
#include <QReadWriteLock> // For thread-safe access if needed later
#include <QScopedPointer> // For PIMPL

class QXmlStreamReader; // Forward declaration
class QDataStream;      // Forward declaration

namespace RME {

class ItemDatabase {
public:
    ItemDatabase();
    ~ItemDatabase();

    bool loadFromOTB(const QString& filePath);
    bool loadFromXML(const QString& filePath); // For items.xml

    const ItemData* getItemData(quint16 serverID) const;
    const ItemData& getDefaultItemData() const; // Returns a default/invalid ItemData

    int getItemCount() const;
    const QMap<quint16, ItemData>& getAllItems() const;

private:
    // OTB parsing helpers
    bool parseOtbNode(QDataStream& stream, quint8& nodeType);
    bool parseOtbItem(QDataStream& stream, quint16 serverID, ItemGroup group); // Added group
    bool parseOtbAttributes(QDataStream& stream, ItemData& itemData);

    // XML parsing helpers
    void parseXmlItem(QXmlStreamReader& xml, ItemData& itemData);
    void parseXmlAttribute(QXmlStreamReader& xml, ItemData& itemData);

    struct ItemDatabaseData; // PIMPL
    QScopedPointer<ItemDatabaseData> d;

    ItemData invalidItemData; // Returned for invalid IDs
};

} // namespace RME

#endif // RME_ITEM_DATABASE_H
