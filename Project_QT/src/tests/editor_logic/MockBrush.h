#ifndef RME_MOCKBRUSH_H
#define RME_MOCKBRUSH_H

#include "core/brush/Brush.h"
#include "core/map/Map.h" // For RME::Map argument type
#include "core/Tile.h"    // For RME::Tile argument type
#include "core/Position.h" // For RME::Position
#include "core/brush/BrushSettings.h" // For RME::BrushSettings
#include <QString>
#include <QList>
#include <QVariantMap> // For storing call arguments

namespace RME {

class MockBrush : public Brush {
public:
    MockBrush(const QString& name = "MockBrush") : Brush(), m_name(name) {}
    ~MockBrush() override = default;

    // Spy members
    mutable int drawCallCount = 0;
    mutable int undrawCallCount = 0;
    mutable QList<Position> lastDrawPositions;
    mutable QList<Position> lastUndrawPositions;
    // Store a simplified version of settings if needed
    // mutable QList<BrushSettings> lastDrawSettings;

    // Brush interface
    // void draw(Map* map, Tile* tile, const BrushSettings* settings) override {
    // Brush.h currently has: virtual void draw(MapDocument* map, Tile* tile, void* data = nullptr) = 0;
    // Assuming MapDocument will be or is compatible with RME::Map for now.
    // And BrushSettings* will be cast from void* or signature will change.
    // For the test, let's use the planned signature:
    void draw(Map* map, Tile* tile, const BrushSettings* settings) override {
        drawCallCount++;
        if(tile) lastDrawPositions.append(tile->getPosition());
        // if(settings) lastDrawSettings.append(*settings);
        // Simulate some change for testing undo/redo if needed
        if (tile) tile->setAttribute("mock_draw_attr", "drawn_by_" + m_name);
    }

    // void undraw(Map* map, Tile* tile, void* data = nullptr) override {
    // Matching the draw signature assumption:
    void undraw(Map* map, Tile* tile, const BrushSettings* settings) override {
        undrawCallCount++;
        if(tile) lastUndrawPositions.append(tile->getPosition());
        // Simulate some change for testing undo/redo
        if (tile) tile->clearAttribute("mock_draw_attr");
    }

    bool canDraw(Map* /*map*/, const Position& /*position*/, const BrushSettings* /*settings*/) const override { return true; }
    QString getName() const override { return m_name; }
    // int getLookID() const override { return 0; } // Not relevant for this mock
    // Other virtual methods can return default values or assert if called unexpectedly

    void resetSpy() {
        drawCallCount = 0;
        undrawCallCount = 0;
        lastDrawPositions.clear();
        lastUndrawPositions.clear();
        // lastDrawSettings.clear();
    }

private:
    QString m_name;
};

} // namespace RME
#endif // RME_MOCKBRUSH_H
