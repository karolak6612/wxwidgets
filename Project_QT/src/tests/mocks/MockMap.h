#ifndef MOCKMAP_H
#define MOCKMAP_H

#include "map_management/Map.h"
#include <QPoint>
#include <QSize>
#include <QVector>
#include <QSharedPointer>
#include "map_editor/Tile.h" // Ensure Tile is included

class MockMap : public Map {
    Q_OBJECT

public:
    MockMap(int width, int height, QObject *parent = nullptr)
        : Map(parent), m_width(width), m_height(height), m_spawnPoint(0,0) {
        m_tiles.resize(width * height);
        // Initialize with default tiles if necessary
        for (int i = 0; i < m_tiles.size(); ++i) {
            // Assuming Tile has a default constructor or a constructor that can be called like this
            m_tiles[i] = QSharedPointer<Tile>(new Tile());
        }
    }
    ~MockMap() override = default;

    // --- Map interface implementation ---
    int getWidth() const override { return m_width; }
    int getHeight() const override { return m_height; }
    QSize getSize() const override { return QSize(m_width, m_height); }

    QSharedPointer<Tile> getTile(int x, int y) const override {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            // Consider logging an error or returning a specific "out of bounds" tile
            return nullptr;
        }
        return m_tiles.value(y * m_width + x, nullptr); // Use .value for safety
    }

    bool setTile(int x, int y, QSharedPointer<Tile> tile) override {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return false;
        }
        if (!tile) { // Prevent setting null tiles if that's an invalid state
            return false;
        }
        m_tiles[y * m_width + x] = tile;
        // In a real map, you might emit a signal here like tileChanged(x, y)
        return true;
    }

    QPoint getSpawnPoint() const override {
        return m_spawnPoint;
    }

    void setSpawnPoint(const QPoint& spawnPoint) override {
        // Basic bounds checking, can be more sophisticated
        if (spawnPoint.x() >= 0 && spawnPoint.x() < m_width &&
            spawnPoint.y() >= 0 && spawnPoint.y() < m_height) {
            m_spawnPoint = spawnPoint;
        } else {
            // Handle invalid spawn point, e.g., clamp to bounds or log error
            // For a mock, simple storage might be enough, or specific testable behavior.
            // m_spawnPoint = QPoint(qBound(0, spawnPoint.x(), m_width-1), qBound(0, spawnPoint.y(), m_height-1));
        }
    }

    QString getTilesetPath() const override {
        return m_tilesetPath;
    }
    void setTilesetPath(const QString& path) override {
        m_tilesetPath = path;
    }

    // Returns a copy of the tiles vector.
    QVector<QSharedPointer<Tile>> getTiles() const override {
        return m_tiles;
    }

    // --- Additional Mock-specific functionalities (if any) ---
    // For example, methods to directly manipulate internal state for testing setup

private:
    int m_width;
    int m_height;
    QPoint m_spawnPoint;
    QVector<QSharedPointer<Tile>> m_tiles;
    QString m_tilesetPath; // Assuming this is part of the Map interface
};

#endif // MOCKMAP_H
