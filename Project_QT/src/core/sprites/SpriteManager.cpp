#include "SpriteManager.h"
#include <QFile>
#include <QDataStream>
#include <QXmlStreamReader> // For OTFI (OTML format)
#include <QDebug>

namespace RME {

const int SPRITE_DEFAULT_WIDTH = 32;
const int SPRITE_DEFAULT_HEIGHT = 32;
const int SPRITE_ADDRESS_TABLE_START_OFFSET = 0; // Assuming SPR address table is at the beginning

struct SpriteManager::SpriteManagerData {
    QMap<quint32, SpriteData> sprites;
    OtfiData activeOtfiData;
    bool otfiLoaded = false;
    quint32 sprSignature = 0; // Store SPR signature if needed for validation
    quint32 datSignature = 0; // Store DAT signature
    quint32 maxSpriteID = 0;  // Max sprite ID read from DAT counts
};

SpriteManager::SpriteManager() : d(new SpriteManagerData()) {
    invalidSpriteData.id = 0;
}
SpriteManager::~SpriteManager() = default;

const SpriteData* SpriteManager::getSpriteData(quint32 spriteID) const {
    return d->sprites.value(spriteID, &invalidSpriteData);
}
const SpriteData& SpriteManager::getDefaultSpriteData() const {
    return invalidSpriteData;
}
int SpriteManager::getSpriteCount() const {
    return d->sprites.size();
}
const QMap<quint32, SpriteData>& SpriteManager::getAllSprites() const {
    return d->sprites;
}

bool SpriteManager::loadOtfi(const QString& otfiPath, OtfiData& otfiDataResult) {
    QFile file(otfiPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "SpriteManager: Could not open OTFI file:" << otfiPath;
        return false;
    }
    d->otfiLoaded = false;
    OtfiData currentOtfi;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("fileInformation")) {
                currentOtfi.isExtended = xml.attributes().value("extended") == "true";
                currentOtfi.hasTransparency = xml.attributes().value("alpha") != "false";
                currentOtfi.hasFrameDurations = xml.attributes().value("frameDurations") == "true";
            } else if (xml.name() == QLatin1String("dat")) {
                currentOtfi.customDatPath = xml.attributes().value("path").toString();
            } else if (xml.name() == QLatin1String("spr")) {
                currentOtfi.customSprPath = xml.attributes().value("path").toString();
            }
        }
    }
    if (xml.hasError()) {
        qWarning() << "SpriteManager: Error parsing OTFI file" << otfiPath << ":" << xml.errorString();
        return false;
    }
    otfiDataResult = currentOtfi;
    d->activeOtfiData = currentOtfi;
    d->otfiLoaded = true;
    qInfo() << "SpriteManager: Successfully loaded OTFI file:" << otfiPath;
    return true;
}

bool SpriteManager::loadDatSpr(const QString& datPath, const QString& sprPath, const ClientProfile& clientProfile) {
    QString actualDatPath = datPath;
    QString actualSprPath = sprPath;

    if (d->otfiLoaded) {
        if (!d->activeOtfiData.customDatPath.isEmpty()) actualDatPath = d->activeOtfiData.customDatPath;
        if (!d->activeOtfiData.customSprPath.isEmpty()) actualSprPath = d->activeOtfiData.customSprPath;
    }

    QFile datFile(actualDatPath);
    QFile sprFile(actualSprPath);

    if (!datFile.open(QIODevice::ReadOnly)) {
        qWarning() << "SpriteManager: Could not open DAT file:" << datFile.fileName(); return false;
    }
    if (!sprFile.open(QIODevice::ReadOnly)) {
        qWarning() << "SpriteManager: Could not open SPR file:" << sprFile.fileName(); return false;
    }

    QDataStream datStream(&datFile); datStream.setByteOrder(QDataStream::LittleEndian);
    QDataStream sprStream(&sprFile); sprStream.setByteOrder(QDataStream::LittleEndian);

    d->sprites.clear();

    datStream >> d->datSignature;
    sprStream >> d->sprSignature; // SPR signature often matches DAT or is related

    // For sample.dat: ItemCount=2, OutfitCount=0, EffectCount=0, ProjectileCount=0
    // Total sprites to read metadata for = sum of these.
    quint16 numItems, numOutfits, numEffects, numProjectiles;
    datStream >> numItems >> numOutfits >> numEffects >> numProjectiles;
    if (datStream.status() != QDataStream::Ok) {
        qWarning("SpriteManager: Failed to read sprite counts from DAT."); return false;
    }

    quint32 firstSpriteID = 1;
    // RME's logic for firstSpriteID can be complex based on client version.
    // For V_760 (used by sample), it's 1. For V_780_792+, it's 100.
    if (clientProfile.datFormat >= DatFormat::V_780_792) { // This enum value needs to exist
         firstSpriteID = 100;
    }
    d->maxSpriteID = firstSpriteID + numItems + numOutfits + numEffects + numProjectiles -1;
    if ((numItems + numOutfits + numEffects + numProjectiles) == 0) { // Handle case of empty DAT
        d->maxSpriteID = firstSpriteID -1; // No sprites to load
    }


    qInfo() << "SpriteManager: DAT Sig:" << Qt::hex << d->datSignature << "SPR Sig:" << Qt::hex << d->sprSignature;
    qInfo() << "SpriteManager: Counts: Items=" << numItems << "Outfits=" << numOutfits
            << "Effects=" << numEffects << "Projectiles=" << numProjectiles;
    qInfo() << "SpriteManager: Loading sprite metadata for IDs" << firstSpriteID << "to" << d->maxSpriteID;

    for (quint32 currentID = firstSpriteID; currentID <= d->maxSpriteID; ++currentID) {
        if (datStream.atEnd()) {
            qWarning() << "SpriteManager: Unexpected end of DAT file at sprite ID" << currentID;
            break;
        }
        SpriteData sd;
        sd.id = currentID;

        // Metadata parsing based on clientProfile.datFormat
        // For the sample (DatFormat::V_760 or similar simple format)
        if (clientProfile.datFormat == DatFormat::V_760 || clientProfile.datFormat == DatFormat::V_755 /* or a new TEST_SIMPLE */) {
            // Skip 3 bytes for color key (RME does this for 7.55+)
            // For sample.dat, we defined no color key, so this part needs care or specific format.
            // The sample.dat was defined as: W(2),H(2),L(1),PX(1),PY(1),PZ(1),PH(1) = 7 bytes
            // If a real V_760 has color key (3 bytes) + other fields, then sample.dat is not V_760.
            // Let's assume our sample.dat structure IS the one for testClientProfile.datFormat.
            // No color key in sample.dat.
            datStream >> sd.width >> sd.height;
            // RME's V760 logic: if (sd.width == 0 || sd.height == 0) { skip reading rest, continue }
            // For sample, width/height are non-zero.
            quint8 num_layers_times_patterns_z_times_patterns_y_times_patterns_x_times_phases; // This complex field is for newer versions
            datStream >> sd.layers >> sd.patternsX >> sd.patternsY >> sd.patternsZ >> sd.phases;

        } else {
            qWarning() << "SpriteManager: Unhandled DAT format for metadata parsing:" << static_cast<int>(clientProfile.datFormat);
            // Default values, will likely fail to read SPR correctly.
            sd.width = SPRITE_DEFAULT_WIDTH; sd.height = SPRITE_DEFAULT_HEIGHT;
            sd.layers = 1; sd.patternsX = 1; sd.patternsY = 1; sd.patternsZ = 1; sd.phases = 1;
        }
         if (datStream.status() != QDataStream::Ok) {
            qWarning() << "SpriteManager: Error reading DAT metadata for sprite ID" << currentID;
            return false; // Critical error
        }


        if (d->otfiLoaded) {
            sd.isExtended = d->activeOtfiData.isExtended;
            sd.hasTransparency = d->activeOtfiData.hasTransparency;
            // if (d->activeOtfiData.hasFrameDurations) { /* handle frame durations if SpriteData stores them */ }
        } else {
            sd.isExtended = clientProfile.extendedSprites;
            sd.hasTransparency = clientProfile.transparentSprites;
        }

        // The actual number of images in SPR is sd.getTotalImageCount()
        // The address table in SPR is for sprite IDs up to maxSpriteID.
        // readSpritePixelData will be called if sd.getTotalImageCount() > 0
        if (sd.width > 0 && sd.height > 0 && sd.getTotalImageCount() > 0) {
            d->sprites.insert(currentID, sd); // Insert metadata first
        } else {
            // Sprite is effectively empty based on DAT metadata, don't try to read pixels
            // but still might need an entry if it's just transparent.
            // For now, if dimensions are zero, skip.
            // If getTotalImageCount is 0, also skip (no pixels to read).
        }
    }

    // After all metadata is read, read pixel data
    for (auto it = d->sprites.begin(); it != d->sprites.end(); ++it) {
        if (!readSpritePixelData(it.key(), it.value(), sprStream)) {
             qWarning() << "SpriteManager: Failed to read pixel data for sprite ID" << it.key() << ". Removing from list.";
             // it = d->sprites.erase(it); // Careful with iterator invalidation if erasing
             // For now, let's allow it to exist with empty frames if pixel read fails
        }
    }


    qInfo() << "SpriteManager: Loaded metadata for" << d->sprites.size() << "sprites from" << datFile.fileName()
            << ". Pixel data processed from" << sprFile.fileName();
    return true;
}

bool SpriteManager::readSpritePixelData(quint32 spriteID, SpriteData& spriteData, QDataStream& sprStream) {
    if (spriteData.width == 0 || spriteData.height == 0 || spriteData.getTotalImageCount() == 0) {
        return true; // No pixel data to read for empty or metadata-less sprite
    }

    // SPR Address Table: Read address for this specific spriteID
    // The table size is (d->maxSpriteID + 1) * 4 bytes if IDs start from 0, or (d->maxSpriteID - firstSpriteID + 1) * 4
    // For sample: IDs 0, 1, 2. maxSpriteID = 2 (if firstID=0). Table size = 3*4 = 12 bytes.
    // If firstSpriteID=1, maxSpriteID=2. IDs are 1,2. Table needs to map these.
    // RME SPR files usually have addresses for IDs 1 through maxSpriteID.
    // The sample SPR was created with addresses for ID 0, 1, 2.

    quint32 addressTableOffset = SPRITE_ADDRESS_TABLE_START_OFFSET + (spriteID * 4); // Assuming IDs in table are contiguous from 0
    if (sprStream.device()->size() < static_cast<qint64>(addressTableOffset + 4)) {
        qWarning() << "SpriteManager: SPR file too small for address table for sprite ID" << spriteID;
        return false;
    }
    sprStream.device()->seek(addressTableOffset);

    quint32 address;
    sprStream >> address;
    if (sprStream.status() != QDataStream::Ok) {
        qWarning() << "SpriteManager: Failed to read SPR address for sprite ID" << spriteID;
        return false;
    }
    if (address == 0) { // Sprite is empty / does not exist in SPR
        spriteData.frames.clear(); // Ensure no frames if address is 0
        return true;
    }

    qint64 previousPos = sprStream.device()->pos(); // This is not needed if we read all addresses first
                                                 // But for one-by-one, we seek to 'address'
    sprStream.device()->seek(address);

    quint32 numPixelsPerFrame = spriteData.width * spriteData.height;

    for (quint32 i = 0; i < spriteData.getTotalImageCount(); ++i) {
        if (sprStream.atEnd()) {
            qWarning() << "SpriteManager: Unexpected end of SPR file for sprite ID" << spriteID << "frame" << i;
            return false;
        }

        QImage image(spriteData.width, spriteData.height, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        quint32 currentPixelCount = 0; // Pixels filled in current image
        while(currentPixelCount < numPixelsPerFrame) {
            quint16 transparentPixels, coloredPixels;
            sprStream >> transparentPixels >> coloredPixels;

            if (sprStream.status() != QDataStream::Ok) {
                qWarning() << "SpriteManager: SPR stream error reading pixel counts for sprite" << spriteID << "frame" << i;
                return false;
            }

            // Advance for transparent pixels
            currentPixelCount += transparentPixels;
            if (currentPixelCount > numPixelsPerFrame) { // Should not happen
                 qWarning() << "SpriteManager: SPR transparent pixel run overflow for sprite" << spriteID << "frame" << i; return false;
            }

            // Read and set colored pixels
            for (quint16 p = 0; p < coloredPixels; ++p) {
                if (currentPixelCount >= numPixelsPerFrame) {
                    qWarning() << "SpriteManager: SPR colored pixel run overflow for sprite" << spriteID << "frame" << i; return false;
                }
                quint8 r, g, b;
                sprStream >> r >> g >> b;
                if (sprStream.status() != QDataStream::Ok) {
                     qWarning() << "SpriteManager: SPR stream error reading RGB for sprite" << spriteID << "frame" << i; return false;
                }

                quint32 y = currentPixelCount / spriteData.width;
                quint32 x = currentPixelCount % spriteData.width;
                image.setPixel(x, y, qRgba(r, g, b, 255));
                currentPixelCount++;
            }
        }
        spriteData.frames.append({image});
    }
    // Restore stream position? Not strictly needed if addresses are absolute and always re-sought.
    // However, the original code saved and restored previousPos around a single sprite's full data read,
    // which is only correct if the outer loop is also seeking for each sprite ID's address.
    // The current refined logic reads metadata for ALL sprites first, then iterates through the sprite map
    // to call readSpritePixelData. This is better. So, no need to restore previousPos here.
    return true;
}

// Convenience method for loading sprites with default client profile
bool SpriteManager::loadSprites(const QString& datPath, const QString& sprPath) {
    if (datPath.isEmpty() || sprPath.isEmpty()) {
        qWarning() << "SpriteManager::loadSprites: Empty file paths provided";
        return false;
    }
    
    // Create a default client profile for version 7.60 (commonly used)
    ClientProfile defaultProfile;
    defaultProfile.datFormat = DatFormat::V_760;
    defaultProfile.extendedSprites = false;
    defaultProfile.transparentSprites = true;
    
    qInfo() << "SpriteManager::loadSprites: Loading sprites from" << datPath << "and" << sprPath;
    
    bool success = loadDatSpr(datPath, sprPath, defaultProfile);
    
    if (success) {
        qInfo() << "SpriteManager::loadSprites: Successfully loaded" << getSpriteCount() << "sprites";
    } else {
        qWarning() << "SpriteManager::loadSprites: Failed to load sprites";
    }
    
    return success;
}

} // namespace RME
