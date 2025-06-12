#ifndef RME_BASEMAP_H
#define RME_BASEMAP_H

#include "QTreeNode.h"
#include "MapIterator.h" // Include for MapIterator return type
#include "../Position.h"
#include "../../assets/AssetManager.h"
#include <memory>

namespace RME {

class Tile;

class BaseMap {
public:
    BaseMap(int mapWidth, int mapHeight, int mapFloors, AssetManager* assetManager);
    virtual ~BaseMap() = default;

    Tile* getTile(const Position& pos);
    const Tile* getTile(const Position& pos) const;
    Tile* getOrCreateTile(const Position& pos, bool& created);
    bool removeTile(const Position& pos);
    void setTile(const Position& pos, std::unique_ptr<Tile> newTile);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getNumFloors() const { return floors; }

    bool isPositionValid(const Position& pos) const;

    // Iterator support
    MapIterator begin();
    MapIterator end();
    // Const iterators would require a const_MapIterator or MapIterator<const Tile>
    // const_MapIterator begin() const;
    // const_MapIterator end() const;


    QTreeNode* getRootNode() const { return rootNode.get(); }
    AssetManager* getAssetManager() const { return assetManager; }

public:
    static int calculateRootNodeSize(int mapWidth, int mapHeight);

protected:
    std::unique_ptr<QTreeNode> rootNode;
    AssetManager* assetManager;

    int width;
    int height;
    int floors;
};

} // namespace RME

#endif // RME_BASEMAP_H
