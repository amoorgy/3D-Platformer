# Computer Graphics Assignment 2
## Ancient East Asian Warriors - 3D Collectibles Game

**Deadline:** Tuesday, November 18th, 2025 @ 11:59 PM  
**Implementation:** OpenGL  
**Theme:** Ancient East Asian Warriors (ninjas, samurais, traditional Eastern aesthetics)

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

## What to Implement

### 1. Player Movement
- Control with Arrow keys OR WASD
- Movement on X and Z axes only
- Player rotates to face direction of movement
- Cannot pass through walls, platforms, or 3D objects

### 2. Collision System
- Detect collision with walls → block movement
- Detect collision with platforms → block movement
- Detect collision with 3D objects → block movement
- Detect collision with collectibles → make collectible disappear

### 3. Collectible System
- Place 3+ collectibles on each platform (fixed or random positions)
- Track how many collected per platform
- Remove collectible from scene when player touches it

### 4. Animation System
Implement 4 different animation types (one per platform):
1. Rotation
2. Scaling
3. Transformation
4. Color change

**Animation Behavior:**
- Starts when ALL collectibles on that platform are collected
- Runs continuously once started
- Can be toggled on/off with keyboard keys (choose your own keys)
- Toggle only works after collectibles are collected

**Final Platform:**
- Same animation rules apply
- PLUS display "Game Win" message on screen
- Player can still move after winning

### 5. Timer System
- Implement countdown timer
- When timer reaches 0:
  - Replace entire scene with "Game Over" scene
  - Remove player control

### 6. Camera System
Implement camera with:
- Free movement along X, Y, Z axes
- 3 fixed views:
  - Top View
  - Side View
  - Front View
- Controls: keyboard, mouse, or both

---

## Bonus (Optional)

Choose ONE:

### Option A: Sound Effects (all 4 required)
- Background music when game starts
- Sound when collecting objects
- Sound when winning
- Sound when losing

### Option B: Exceptional Graphics
- Create 4 complex models with 10+ primitives each

---

## Submission

### File Format
- Filename: `PXX_YYYY.cpp`
  - XX = tutorial group number
  - YYYY = student ID
- Submit ONLY the .cpp file
- DO NOT submit .sln file (= automatic ZERO)

### Where to Submit
https://forms.gle/vjdSmUStKFuAmigy5

### Academic Integrity
- Copying code = ZERO
- You must understand all code (including AI-generated)

---

## Implementation Checklist

### Models to Create
- [ ] Ground plane (1 primitive)
- [ ] Wall 1 (1 primitive)
- [ ] Wall 2 (1 primitive)
- [ ] Wall 3 (1 primitive)
- [ ] Player character (6 primitives: head, torso, limbs)
- [ ] Platform 1 (2 primitives)
- [ ] Platform 2 (2 primitives)
- [ ] Platform 3 (2 primitives)
- [ ] Platform 4 (2 primitives)
- [ ] 3D Object for Platform 1 (4 primitives)
- [ ] 3D Object for Platform 2 (4 primitives)
- [ ] 3D Object for Platform 3 (4 primitives)
- [ ] 3D Object for Platform 4 (4 primitives)
- [ ] 3 collectibles for Platform 1 (3 primitives each)
- [ ] 3 collectibles for Platform 2 (3 primitives each)
- [ ] 3 collectibles for Platform 3 (3 primitives each)
- [ ] 3 collectibles for Platform 4 (3 primitives each)
- [ ] All models colored
- [ ] Platforms look different from each other
- [ ] Theme applied (Ancient East Asian Warriors)

### Systems to Implement
- [ ] Player movement (Arrow keys or WASD, X and Z axes only)
- [ ] Player rotation based on movement direction
- [ ] Collision detection: walls
- [ ] Collision detection: platforms
- [ ] Collision detection: 3D objects
- [ ] Collision detection: collectibles
- [ ] Collectibles disappear when collected
- [ ] Track collectibles per platform
- [ ] Rotation animation (Platform 1)
- [ ] Scaling animation (Platform 2)
- [ ] Transformation animation (Platform 3)
- [ ] Color change animation (Platform 4)
- [ ] Animations trigger after collecting all platform items
- [ ] Animations run continuously
- [ ] Keyboard toggle for each animation
- [ ] Toggles only work after items collected
- [ ] Timer countdown
- [ ] "Game Win" message when final platform complete
- [ ] Player can move after winning
- [ ] "Game Over" scene when timer expires
- [ ] Remove control on game over
- [ ] Camera: free movement (X, Y, Z)
- [ ] Camera: Top View
- [ ] Camera: Side View
- [ ] Camera: Front View
- [ ] Camera controls implemented

### Bonus (Pick ONE)
- [ ] Background music
- [ ] Collection sound effect
- [ ] Win sound effect
- [ ] Lose sound effect

OR

- [ ] Complex model 1 (10+ primitives)
- [ ] Complex model 2 (10+ primitives)
- [ ] Complex model 3 (10+ primitives)
- [ ] Complex model 4 (10+ primitives)

### Before Submission
- [ ] File named correctly: PXX_YYYY.cpp
- [ ] Only .cpp file (no .sln)
- [ ] Code tested and works
- [ ] Understand all code
- [ ] Theme is visible in design