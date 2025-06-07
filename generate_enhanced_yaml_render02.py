import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Using f-string for the C++ code block, ensuring {{ and }} are used for literal braces
cpp_example_in_boilerplate = """\
// Inside paintGL, after setting up projection and modelview matrices
// (ModelView should be translated by -m_viewCenterMapCoords * TILE_SIZE * m_zoomFactor, then scaled by m_zoomFactor)
// Or adjust vertex coordinates directly if using a fixed ortho projection.

for (int z = renderMaxZ; z >= renderMinZ; --z) {{
    for (int y = minMapY; y <= maxMapY; ++y) {{
        for (int x = minMapX; x <= maxMapX; ++x) {{
            Tile* tile = m_map->getTile(x, y, z); // Assuming m_map is accessible
            if (!tile || (tile->empty() && !tile->isHouseTile())) {{ // Render house tiles even if empty for area indication
                continue;
            }}

            QColor tileColor = Qt::darkGray; // Default for empty or no-ground tiles
            float alpha = 1.0f;

            if (tile->ground && tile->ground->getMiniMapColor() != 0xFF) {{
                // Convert minimap color (0-215) to RGB
                // uint8_t miniColor = tile->ground->getMiniMapColor();
                // tileColor = QColor(some_conversion_from_miniColor_to_RGB);
                tileColor = QColor(100, 80, 60); // Placeholder brown for ground
            }} else if (tile->isHouseTile()) {{
                tileColor = QColor(50, 50, 100); // Placeholder blue for house area
            }}


            if (z < m_currentFloor) {{
                alpha = 0.3f; // Ghost lower floors
            }} else if (z > m_currentFloor) {{
                if (AppSettings::instance()->getBoolean(Config::TRANSPARENT_FLOORS)) {{
                    alpha = 0.2f; // Ghost higher floors
                }} else {{
                    continue; // Don't draw higher floors if not transparent
                }}
            }}
            tileColor.setAlphaF(alpha);

            // Calculate screen coordinates for the tile's top-left corner
            // This depends on your specific matrix setup.
            // For example, if (0,0) of your projection is top-left of screen:
            // float screenX = (x - m_viewCenterMapCoords.x()) * 32.0f * m_zoomFactor + width() / 2.0f;
            // float screenY = (y - m_viewCenterMapCoords.y()) * 32.0f * m_zoomFactor + height() / 2.0f;
            // float scaledTileSize = 32.0f * m_zoomFactor;

            // If using QPainter:
            // painter.fillRect(QRectF(screenX, screenY, scaledTileSize, scaledTileSize), tileColor);

            // If using direct OpenGL (assuming simple_color_shader_program is bound):
            // simple_color_shader_program->setUniformValue("ourColor", tileColor);
            // GLfloat vertices[] = {{ screenX, screenY,  screenX + scaledTileSize, screenY, ... }};
            // Update VBO and draw quad for this tile.
        }}
    }}
}}\
"""

yaml_content = {
    "id": "RENDER-02",
    "section": "Core Migration Tasks",
    "title": "Render Map Data",
    "original_input_files": "map_drawer.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/map_drawer.h",
        "wxwidgets/map_drawer.cpp"
    ],
    "dependencies": [
        "RENDER-01" # Depends on MapView (QOpenGLWidget) being set up
    ],
    "current_functionality_summary": """\
`MapDrawer` (in `map_drawer.h/cpp`) is responsible for the OpenGL rendering of the map. Its core methods (`DrawMap`, `DrawTile`) iterate through visible map tiles for the current and adjacent floors (for effects like shading or ghosting). For each tile and its contents (ground, items, creatures), it determines the correct sprite, animation frame, and pattern, then calls internal `BlitItem`, `BlitCreature`, or `BlitSpriteType` methods. These `Blit*` methods ultimately use `GameSprite::getHardwareID()` to get an OpenGL texture ID and render a textured quad via immediate mode OpenGL calls (`glBegin`, `glEnd`, `glVertex2f`, `glTexCoord2f`). It also handles drawing overlays like selection boxes, grids, and tooltips (using `glutBitmapCharacter`).\
""",
    "qt6_migration_steps": """\
1. Port the core map iteration logic from `MapDrawer::DrawMap()` and `MapDrawer::DrawTile()` into `MapView::paintGL()` (created in `RENDER-01`) or into a new helper class `QtMapDrawer` called by `MapView::paintGL()`.
2. In `MapView::paintGL()`, after clearing the screen and setting up the ModelViewProjection matrices based on current zoom and view center:
   a. Calculate the visible range of map coordinates (minX, minY, maxX, maxY) for the current viewport.
   b. Determine the range of floors to draw (`z_draw_start` to `z_draw_end`) based on `MapView::m_currentFloor` and settings like `Config::SHOW_ALL_FLOORS` (obtained from `AppSettings`). Typically, this means drawing from `m_currentFloor + N` down to `m_currentFloor - M` or floor 0, respecting map boundaries.
3. Iterate `z` from `z_draw_start` down to `z_draw_end` (to ensure correct drawing order if relying on painter's algorithm, or if depth buffer is used, order might be less critical but still good for floor distinction).
   a. For each floor `z`, iterate `y` from `minVisibleMapY` to `maxVisibleMapY`.
   b. For each `y`, iterate `x` from `minVisibleMapX` to `maxVisibleMapX`.
      i.   Retrieve `Tile* tile = m_map->getTile(x, y, z);` (where `m_map` is the `mapcore::Map` instance accessible to `MapView`).
      ii.  If `tile` is null or `tile->empty()` (and not a special case like a house tile that should be colored), continue.
      iii. **Render Tile as Colored Quad (as per WBS for this specific task):**
           - Determine a base color for the tile. A simple approach is to use `tile->ground->getMiniMapColor()` if `tile->ground` exists, converting this 8-bit minimap color index to an actual RGB `QColor`. If no ground, use a default placeholder color (e.g., dark gray).
           - Apply "ghosting" for floors not equal to `m_currentFloor`:
             - If `z < m_currentFloor` (lower floors), reduce the alpha of the `QColor` (e.g., to 50-70%).
             - If `z > m_currentFloor` (higher floors, if `Config::TRANSPARENT_FLOORS` is enabled via `AppSettings`), reduce alpha significantly (e.g., to 20-40%).
           - Calculate the screen coordinates `(screenX, screenY)` and `tileSizeOnScreen` for this tile based on map coordinates `(x,y)`, `MapView::m_viewCenterMapCoords`, `MapView::m_zoomFactor`, and the base tile size (32px).
           - If using `QPainter` (e.g., `QPainter painter(this);` in `paintGL` after `beginNativePainting()` if mixing with GL, or if `QOpenGLWidget` is just a surface):
             `painter.fillRect(screenX, screenY, tileSizeOnScreen, tileSizeOnScreen, tileColorWithAlpha);`
           - If using direct OpenGL:
             Set up a simple shader program for rendering flat colored quads. Pass the `tileColorWithAlpha` as a uniform or vertex attribute. Draw a quad with vertices corresponding to `(screenX, screenY)` and `tileSizeOnScreen`.
4. Sprite rendering and detailed item/creature drawing are deferred to `RENDER-03` and `RENDER-04`. This task focuses on the tile iteration and basic colored quad representation with floor ghosting.\
""",
    "definition_of_done": """\
`MapView::paintGL` correctly iterates through the currently visible map tiles for the active floor (and other floors if ghosting is enabled) and renders each existing tile as a simple colored quad.
Key requirements:
- The range of visible map tiles (X, Y, Z coordinates) is accurately calculated based on the `MapView`'s current view parameters (center/scroll, zoom level, current floor, and settings for showing other floors).
- `MapView::paintGL` iterates through these visible tiles.
- Each non-empty `Tile` is rendered as a colored quad at its correct on-screen position and scale. The color can be derived from its ground item's minimap color or a default placeholder.
- "Ghosting" logic for different floors is implemented: tiles on floors other than the `m_currentFloor` are rendered with modified alpha or color intensity to visually distinguish them (e.g., lower floors are dimmer, higher floors are more transparent if `Config::TRANSPARENT_FLOORS` is active).
- The rendering uses either `QPainter` on the `QOpenGLWidget` surface or direct OpenGL calls with a simple shader program designed for drawing colored quads. Full sprite-based rendering is not part of this task.\
""",
    "boilerplate_coder_ai_prompt": f"""\
In `MapView::paintGL()` (from `RENDER-01`), implement the logic to iterate through visible map tiles and render them as colored quads, including floor ghosting. Sprite rendering is for a later task.
1.  **Calculate Visible Range:**
    -   In `paintGL`, based on `m_viewCenterMapCoords`, `m_zoomFactor`, widget `width()`, `height()`, and tile size (32), determine `minMapX, maxMapX, minMapY, maxMapY`.
    -   Determine `renderMinZ` and `renderMaxZ` based on `m_currentFloor` and `AppSettings` for `Config::SHOW_ALL_FLOORS`. Iterate `z` from `renderMaxZ` down to `renderMinZ`.
2.  **Tile Iteration and Rendering Loop:**
    ```cpp
{cpp_example_in_boilerplate}
    ```
3.  **Ghosting:** Implement the alpha modification for floors not equal to `m_currentFloor` as shown in the loop.
4.  **Coordinate System:** Be very careful with map coordinates, OpenGL world coordinates, and screen coordinates, especially considering zoom and view center. The `MapView`'s projection and modelview matrices must be set up to correctly transform map tile positions to the screen.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/RENDER-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
