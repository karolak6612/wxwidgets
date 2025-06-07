import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Using f-string for the C++ code block, ensuring {{ and }} are used for literal braces
cpp_example_in_boilerplate = """\
1.  **`TextureManager.h/.cpp` (Application Side, not mapcore):**
    -   Constructor: Takes `mapcore::GraphicManager*` as input.
    -   `void initializeAtlases()`:
        -   Iterate all sprites from `mapcore::GraphicManager`.
        -   Use a texture packing algorithm (e.g., basic row/column or a more advanced one like Guillotine) to arrange sprite pixel data (obtained from `mapcore::GameSprite::getRGBAData()`) into one or more large `GLuint` OpenGL textures (atlases).
        -   Store a mapping from `(clientID, frame, patternX, patternY, patternZ, layer)` to `(atlasTextureID, QRectF_uvCoords)`.
    -   `SpriteRenderInfo getSpriteInfo(uint16_t clientSpriteId, int frame, ...)`: Looks up and returns the atlas ID and UVs.
    -   `SpriteRenderInfo getCreatureSpriteInfo(const mapcore::Outfit& outfit, mapcore::Direction dir, int frame)`:
        -   Retrieves base creature sprite layers from `mapcore::GraphicManager`.
        -   Handles outfit colors:
            -   **Option A (Simpler, more VRAM):** Create a new texture region in an atlas by dynamically tinting the base sprite layers with outfit colors. Cache these.
            -   **Option B (Complexer, less VRAM):** Return base sprite layer info and expect a special shader in `MapView` to do the tinting using outfit colors as uniforms and potentially mask textures. (The original `GameSprite::TemplateImage` logic in wxWidgets can guide this). Choose one approach.
2.  **`MapView.h/.cpp` Modifications:**
    -   Add `TextureManager* m_textureManager;`. Initialize it in `MapView`'s constructor or `initializeGL` after `mapcore::GraphicManager` is available.
    -   Add `QTimer* m_animationTimer; int m_animationTick = 0;`. In constructor: `m_animationTimer = new QTimer(this); connect(m_animationTimer, &QTimer::timeout, this, &MapView::animate); m_animationTimer->start(50); // ~20 FPS for animations`.
    -   Slot `void animate() {{ m_animationTick++; update(); }}`.
    -   Create and compile GLSL shaders:
        -   `texture_shader.vert/.frag`: Simple pass-through vertex shader, fragment shader samples texture using UVs.
        -   (If Option B for outfits) `outfit_shader.vert/.frag`: Similar, but fragment shader takes outfit colors and possibly a mask texture to tint base sprite.
    -   In `paintGL()`:
        -   For each visible item:
            -   `ItemType& it = g_items[item->getID()]; GameSprite* gs = m_mapcoreGraphicsManager->getSprite(it.clientID);`
            -   `int frame = gs && gs->animator ? gs->animator->getFrameForTick(m_animationTick) : 0;`
            -   `SpriteRenderInfo texInfo = m_textureManager->getSpriteInfo(it.clientID, frame, /* patterns */);`
            -   Bind `texInfo.atlasTextureID`. Use `texture_shader`. Add quad vertices (positions + `texInfo.uvRect`) to a VBO for batch drawing.
        -   For each visible creature:
            -   `CreatureType* ct = g_creatures[creature->getName()]; GameSprite* gs = m_mapcoreGraphicsManager->getCreatureSprite(ct->outfit.lookType);`
            -   `int frame = gs && gs->animator ? gs->animator->getFrameForTick(m_animationTick) : 0;`
            -   `SpriteRenderInfo texInfo = m_textureManager->getCreatureSpriteInfo(ct->outfit, creature->getDirection(), frame, ...);`
            -   Add to VBO. Bind `texInfo.atlasTextureID`. Use appropriate shader (texture_shader or outfit_shader).
        -   After collecting all sprite data for an atlas: `glBindTexture(GL_TEXTURE_2D, atlasId); shaderProgram->bind(); ... glDrawArrays(...);`
3.  Replace the "colored quad" rendering from `RENDER-02` with this new textured quad rendering.\
"""

yaml_content = {
    "id": "RENDER-03",
    "section": "Core Migration Tasks",
    "title": "Implement Sprite Rendering",
    "original_input_files": "graphics.h/cpp, sprites.h",
    "analyzed_input_files": [
        "wxwidgets/graphics.h",
        "wxwidgets/graphics.cpp",
        "wxwidgets/sprites.h"
    ],
    "dependencies": [
        "RENDER-02", # Depends on MapView and basic tile rendering loop
    ],
    "current_functionality_summary": """\
The `GraphicManager` (from `graphics.h/cpp`) is responsible for loading sprite metadata (`.dat`) and pixel data (`.spr`). `GameSprite` objects store this data and provide methods like `getHardwareID()` to obtain an OpenGL texture ID for a specific sprite variation (considering animation, patterns, and creature outfits). `sprites.h` defines various sprite ID constants. The original system would use these texture IDs to render textured quads for items and creatures.\
""",
    "qt6_migration_steps": """\
1. Create a `TextureManager` class (e.g., `TextureManager.h`, `TextureManager.cpp`) within the Qt application (not `mapcore`). This class will be responsible for managing OpenGL textures derived from game sprites.
2. Upon initialization, the `TextureManager` should:
   a. Obtain access to the `mapcore::GraphicManager` (ported in `CORE-02`) to get sprite metadata and raw pixel data.
   b. Implement a texture atlas strategy: Iterate through all available sprites (items, creatures, effects) from `mapcore::GraphicManager`. Pack their pixel data into one or more large `GLuint` textures (atlases). This involves creating large `GLuint` textures and uploading sprite pixel data into specific regions (sub-textures) within these atlases using `glTexSubImage2D`.
   c. Store the UV coordinates (e.g., `QRectF` representing normalized texture coordinates) for each individual sprite (and its variations like animation frames, patterns) within its respective atlas. A map or hash table can be used to look up `(sprite_id, frame, pattern_x, ...)` to `(atlas_texture_id, uv_rect)`.
3. Implement methods in `TextureManager` to retrieve rendering information for a sprite:
   a. `SpriteRenderInfo getSpriteInfo(uint16_t clientSpriteId, int animationFrame = 0, int patternX = 0, int patternY = 0, int patternZ = 0, int layers = 0);` This should return the `GLuint` of the atlas texture and the `QRectF` of the UV coordinates for the specified sprite variation.
   b. `SpriteRenderInfo getCreatureSpriteInfo(const mapcore::Outfit& outfit, mapcore::Direction dir, int animationFrame = 0);` This method needs to handle creature outfits. Options:
      i.  **Dynamic Atlas Entries (More VRAM, simpler shader):** If a specific colorized outfit combination is requested, `TextureManager` could dynamically create a new colorized version of the base creature sprites (using pixel manipulation on `mapcore::GameSprite` data), add it to an atlas (if not already present), and return its info.
      ii. **Shader-based Tinting (Less VRAM, complex shader):** `TextureManager` returns info for the base creature sprite layers. A custom GLSL shader in `MapView` will then apply outfit colors as tints, potentially using mask textures if creature sprites are designed with separate color mask layers. The original `GameSprite::TemplateImage` logic provides clues for this.
4. Modify `MapView::paintGL()` (from `RENDER-02`):
   a. For each visible item on a tile:
      i.  Get its `clientID` from `mapcore::ItemType`.
      ii. Get animation frame (see step 5).
      iii.Call `m_textureManager->getSpriteInfo()` to get its atlas texture ID and UVs.
      iv. Instead of drawing a colored quad, draw a textured quad using this texture ID and UVs.
   b. For each visible creature on a tile:
      i.  Get its `Outfit`, `Direction` from `mapcore::Creature` and `mapcore::CreatureType`.
      ii. Get animation frame.
      iii.Call `m_textureManager->getCreatureSpriteInfo()`.
      iv. Draw a textured quad (potentially with the outfit tinting shader).
5. Implement Animation Handling in `MapView`:
   a. Add a `QTimer* m_animationTimer` to `MapView`.
   b. In its timeout slot, increment a global animation tick or update animation states for visible entities based on `mapcore::Animator` logic associated with `GameSprite`s.
   c. Call `MapView::update()` to trigger repaints, which will use the new animation frames when calling `TextureManager`.
6. All rendering of textured quads should use VBOs and appropriate GLSL shader programs (a basic texture mapping shader, and one for outfit colorization if that approach is chosen).\
""",
    "definition_of_done": """\
`MapView` correctly renders items and creatures from the map data models as textured quads, utilizing a texture atlas managed by a `TextureManager`.
Key requirements:
- A `TextureManager` class is implemented, loads sprite data via `mapcore::GraphicManager`, and creates/manages OpenGL texture atlas(es).
- `MapView::paintGL` retrieves texture ID and UV coordinates from `TextureManager` for each visible item and creature based on their current state (ID, animation frame, pattern, outfit).
- Items and creatures are rendered as textured quads using the correct sprites from the atlas.
- Creature outfits are correctly colorized on rendered sprites.
- Sprite animations are displayed correctly.
- Rendering uses efficient OpenGL techniques (VBOs, shaders) for textured quads.
- The simple colored quads from `RENDER-02` are now replaced with actual sprite rendering.\
""",
    "boilerplate_coder_ai_prompt": cpp_example_in_boilerplate
}

output_file_path = "enhanced_wbs_yaml_files/RENDER-03.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
