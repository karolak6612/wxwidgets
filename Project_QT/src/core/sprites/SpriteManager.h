#ifndef RME_SPRITE_MANAGER_H
#define RME_SPRITE_MANAGER_H

#include "SpriteData.h"
#include "../assets/ClientProfile.h" // For DatFormat
#include <QMap>
#include <QString>
#include <QScopedPointer>

// Forward declare QDataStream if it's only used in .cpp for parameters
class QDataStream;

namespace RME {

// Structure for OTFI (Open Tibia File Information) data if used
struct OtfiData {
    QString customDatPath;
    QString customSprPath;
    bool isExtended = false;
    bool hasTransparency = true; // Default from OTFI
    bool hasFrameDurations = false; // If OTFI specifies frame durations
    // other OTFI specific flags...
};


class SpriteManager {
public:
    SpriteManager();
    ~SpriteManager();

    // Main loading function
    bool loadDatSpr(const QString& datPath, const QString& sprPath, const ClientProfile& clientProfile);
    
    // Convenience method for loading sprites
    bool loadSprites(const QString& datPath, const QString& sprPath);

    // Optional: Load OTFI to override paths or sprite properties
    bool loadOtfi(const QString& otfiPath, OtfiData& otfiDataResult); // Parses OTFI into struct

    const SpriteData* getSpriteData(quint32 spriteID) const;
    const SpriteData& getDefaultSpriteData() const; // For invalid IDs

    int getSpriteCount() const;
    const QMap<quint32, SpriteData>& getAllSprites() const; // For iteration, if needed

private:
    // DAT parsing helpers based on DatFormat
    // bool parseDat(QDataStream& stream, const ClientProfile& clientProfile); // This seems to be integrated into loadDatSpr

    // SPR reading helper
    // It's called multiple times by loadDatSpr for each sprite ID entry.
    // sprStream should be an open QDataStream for the .spr file.
    bool readSpritePixelData(quint32 spriteID, SpriteData& spriteData, QDataStream& sprStream);

    struct SpriteManagerData; // PIMPL
    QScopedPointer<SpriteManagerData> d;

    SpriteData invalidSpriteData;
};

} // namespace RME

#endif // RME_SPRITE_MANAGER_H
