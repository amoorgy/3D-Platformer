# Computer Graphics Assignment 2
## Ancient East Asian Warriors - 3D Collectibles Game

**Deadline:** Tuesday, November 18th, 2025 @ 11:59 PM
**Implementation:** OpenGL
**Theme:** Ancient East Asian Warriors (ninjas, samurais, traditional Eastern aesthetics)

---

## ✅ CRITICAL FIXES COMPLETED (Nov 15, 2025)

### 🔴 **FIXED: Platform Collision System**
**Problem:** Platforms were treated as solid obstacles, preventing the player from walking on top of them. Player couldn't jump between platforms.

**Solution:** Modified `collidesWithWorld()` function in [PXX_YYYY.cpp:468-504](PXX_YYYY.cpp#L468-L504) to:
- Check if player's bottom is above platform surface
- Allow movement when standing on top of platforms
- Block movement when trying to clip through from sides
- Applied same logic to obstacles for multi-level platforming

### 🔴 **FIXED: Wrong Theme**
**Problem:** Implementation had "Businessman Adventure" theme with city environment, not East Asian Warriors.

**Changes Made:**
1. **Player Model** ([PXX_YYYY.cpp:255-298](PXX_YYYY.cpp#L255-L298))
   - Changed from businessman (suit, tie, briefcase) to ninja warrior
   - Added: ninja mask/hood, headband, hakama pants, katana sword, tabi boots
   - Dark gi outfit with traditional Japanese elements

2. **Environment** ([PXX_YYYY.cpp:587-639](PXX_YYYY.cpp#L587-L639))
   - Replaced city skyline with East Asian landscape
   - Added: mountain ranges with snow caps, pagoda temples, bamboo forests
   - Changed ground from asphalt to stone courtyard with gravel paths
   - Changed walls from concrete to stone fortress walls with wooden beams

3. **Window Title** ([PXX_YYYY.cpp:959](PXX_YYYY.cpp#L959))
   - Changed to "Ancient East Asian Warriors"

### 🔴 **FIXED: Collectible Accessibility**
**Problem:** Blue and Yellow platform collectibles were positioned inside or blocked by obstacles. Players couldn't collect all items.

**Solution:** Repositioned collectibles in [PXX_YYYY.cpp:456-469](PXX_YYYY.cpp#L456-L469):
- **Blue platform:** Moved to front left, front right, and back center (clear of stair obstacles)
- **Yellow platform:** Moved to left edge, right edge, and front (clear of large obstacles)
- All 12 collectibles now obtainable ✅

### 🔴 **FIXED: Improved Controls**
**Problem:** Animation toggle keys (5-8) were not intuitive.

**Solution:** Changed to color-based keys in [PXX_YYYY.cpp:937-943](PXX_YYYY.cpp#L937-L943):
- **R** = Red platform rotation
- **B** = Blue platform scaling
- **G** = Green platform translation
- **Y** = Yellow platform color change
- **ESC** = Reset game (changed from R key)

---

## What You Need to Build

### Scene Requirements (21+ Objects)
Build a 3D game with:
- 1 Ground plane
- 3 Bounding walls
- 1 Player character
- 4 Platforms (visually distinct)
- 4 3D objects (one per platform)
- 12+ Collectibles (minimum 3 per platform)

### Game Objective
- Player collects items on each platform
- Collecting all items on a platform triggers an animation
- Collecting all items on the final platform shows "Game Win" message
- Timer runs out = "Game Over" scene

---

## Modeling Requirements

### ⚠️ CRITICAL RULES
- All models built from OpenGL primitives ONLY
- NO imported models
- All models must be fully colored
- Models must look realistic

### Primitive Counts

| Object | Minimum Primitives |
|--------|-------------------|
| Player Character | 6 (must show head, torso, limbs) |
| Each Wall | 1 |
| Ground | 1 |
| Each Collectible | 3 |
| Each 3D Object (on platform) | 4 |
| Each Platform | 2 |

### Platform Requirements
Make each platform visibly different using:
- Different colors
- Different designs
- Different heights

---

## Implementation Checklist

### Models to Create
- [x] Ground plane (1 primitive) ✅
- [x] Wall 1 (1 primitive) ✅
- [x] Wall 2 (1 primitive) ✅
- [x] Wall 3 (1 primitive) ✅
- [x] Player character (11 primitives: head, mask, headband, torso, legs, arms, hands, sword parts, boots) ✅ **EXCEEDS REQUIREMENT**
- [x] Platform 1 (2 primitives: base + rim) ✅
- [x] Platform 2 (2 primitives: base + rim) ✅
- [x] Platform 3 (2 primitives: base + rim) ✅
- [x] Platform 4 (2 primitives: base + rim) ✅
- [x] 3D Object for Platform 1 (4+ primitives: Torii gate) ✅
- [x] 3D Object for Platform 2 (4+ primitives: Pagoda) ✅
- [x] 3D Object for Platform 3 (4+ primitives: Taiko drum) ✅
- [x] 3D Object for Platform 4 (4+ primitives: Stone lantern) ✅
- [x] 3 collectibles for Platform 1 (3 primitives each: base, roof, ornament) ✅
- [x] 3 collectibles for Platform 2 (3 primitives each) ✅
- [x] 3 collectibles for Platform 3 (3 primitives each) ✅
- [x] 3 collectibles for Platform 4 (3 primitives each) ✅
- [x] All models colored ✅
- [x] Platforms look different from each other ✅
- [x] Theme applied (Ancient East Asian Warriors) ✅ **FIXED**

### Systems to Implement
- [x] Player movement (Arrow keys AND WASD, X and Z axes) ✅
- [x] Player rotation based on movement direction ✅
- [x] Collision detection: walls ✅
- [x] Collision detection: platforms ✅ **FIXED - now allows standing on top**
- [x] Collision detection: 3D objects ✅
- [x] Collision detection: collectibles ✅
- [x] Collectibles disappear when collected ✅
- [x] Track collectibles per platform ✅
- [x] Rotation animation (Platform 1: Torii gate) ✅
- [x] Scaling animation (Platform 2: Pagoda) ✅
- [x] Transformation animation (Platform 3: Taiko drum) ✅
- [x] Color change animation (Platform 4: Stone lantern) ✅
- [x] Animations trigger after collecting all platform items ✅
- [x] Animations run continuously ✅
- [x] Keyboard toggle for each animation (keys 5,6,7,8) ✅
- [x] Toggles only work after items collected ✅
- [x] Timer countdown ✅
- [x] "Game Win" message when final platform complete ✅
- [x] Player can move after winning ✅
- [x] "Game Over" scene when timer expires ✅
- [x] Remove control on game over ✅
- [x] Camera: free movement (X, Y, Z with IJKL/UO keys) ✅
- [x] Camera: Top View (key 2) ✅
- [x] Camera: Side View (key 3) ✅
- [x] Camera: Front View (key 4) ✅
- [x] Camera controls implemented (1-4 for presets, V to cycle, IJKLUO for free move) ✅

### Bonus Features
- [x] **BONUS: Exceptional Graphics** ✅
  - Player character: 11 primitives (exceeds 10+) ✅
  - Torii gate: 4+ primitives ✅
  - Pagoda: 6+ primitives ✅
  - Stone lantern: 4+ primitives ✅
  - **4+ complex models achieved**

### Additional Features (Beyond Requirements)
- [x] Jumping mechanic (Spacebar) with gravity ✅
- [x] Multi-level obstacles for platforming challenges ✅
- [x] Moving obstacles on Platform 2 ✅
- [x] Detailed East Asian background (mountains, temples, bamboo) ✅
- [x] Ground texturing (stone tiles, gravel paths) ✅
- [x] Wall detailing (stone texture, wooden beams) ✅

### Before Submission
- [ ] File named correctly: PXX_YYYY.cpp ⚠️ **RENAME BEFORE SUBMITTING**
- [x] Only .cpp file (no .sln) ✅
- [ ] Code tested and works ⚠️ **TEST ALL FEATURES**
- [x] Understand all code ✅
- [x] Theme is visible in design ✅ **FIXED**

---

## Controls Summary

**Movement:**
- WASD or Arrow Keys: Move on XZ plane
- Spacebar: Jump
- ESC: Reset game

**Camera:**
- 1: Follow camera (semi top-down)
- 2: Top view
- 3: Side view
- 4: Front view
- V: Cycle through views
- I/K: Forward/backward (free camera)
- J/L: Left/right (free camera)
- U/O: Down/up (free camera)

**Animations (after collecting platform items):**
- R: Toggle Red platform rotation (Torii gate)
- B: Toggle Blue platform scaling (Pagoda)
- G: Toggle Green platform translation (Taiko drum)
- Y: Toggle Yellow platform color change (Stone lantern)

---

## Testing Checklist

- [ ] Game starts and displays correctly
- [ ] Player can move in all directions
- [ ] Player can jump
- [ ] Player cannot walk through walls
- [ ] Player can walk ON TOP of platforms ✅ **FIXED**
- [ ] Player can collect items
- [ ] Collectibles disappear when collected
- [ ] Animations unlock after collecting all items on each platform
- [ ] Animation toggles work (keys 5-8)
- [ ] "Game Win" appears after collecting all items on final platform
- [ ] Player can still move after winning
- [ ] Timer counts down
- [ ] "Game Over" appears when timer reaches 0
- [ ] Player loses control on game over
- [ ] All camera views work correctly
- [ ] Theme is clearly Ancient East Asian Warriors ✅ **FIXED**