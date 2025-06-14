#ifndef RME_SPRITE_DATA_H
#define RME_SPRITE_DATA_H

#include <QByteArray>
#include <QList>
#include <QImage> // For storing pixel data, can be converted to texture later

namespace RME {
namespace core {
namespace assets {

// Represents a single animation frame or sprite layer
struct SpriteFrame {
    QImage image; // Decoded pixel data for this frame/layer
    //quint32 duration; // Frame duration, if applicable (from OTFI or client version extensions)
    // Add other frame-specific properties if any (e.g., offsets)
};

struct SpriteData {
    quint32 id = 0; // Sprite ID

    quint16 width = 0;  // Width of one sprite frame/tile
    quint16 height = 0; // Height of one sprite frame/tile

    quint16 layers = 1;    // Number of layers (for layered sprites)
    quint16 patternsX = 1; // Number of patterns in X direction (width diversity)
    quint16 patternsY = 1; // Number of patterns in Y direction (height diversity)
    quint16 patternsZ = 1; // Number of patterns in Z direction (depth diversity, e.g. walls)
    quint16 phases = 1;    // Number of animation phases / frames

    bool isExtended = false;     // Read from OTFI or client profile
    bool hasTransparency = true; // Default, can be overridden by OTFI

    // Calculated properties
    quint16 totalFramesPerSprite() const { // total images for one "version" of the sprite (one pattern x,y,z)
        return layers * phases;
    }
    quint16 totalPatternVariations() const {
        return patternsX * patternsY * patternsZ;
    }
    quint32 getTotalImageCount() const { // Total number of distinct images in the .spr file for this ID
        return layers * patternsX * patternsY * patternsZ * phases;
    }

    // Pixel data - a list of frames.
    // For a simple sprite (1,1,1,1,1), this has 1 SpriteFrame.
    // For animated sprite (1,1,1,1,4), this has 4 SpriteFrames.
    // For layered animated (2,1,1,1,4), this has 8 SpriteFrames (L0F0, L0F1, L0F2, L0F3, L1F0, L1F1, L1F2, L1F3)
    // This list will store all image variations sequentially.
    QList<SpriteFrame> frames;

    // Optional: Original SPR file offset and size for on-demand loading, if not pre-loading all
    // quint32 sprFileOffset = 0;
    // quint32 sprDataSize = 0;

    SpriteData() = default;
};

} // namespace assets
} // namespace core
} // namespace RME

#endif // RME_SPRITE_DATA_H
