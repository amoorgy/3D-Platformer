# 3D Collectibles Game - Ancient East Asian Warriors

A 3D OpenGL platformer game inspired by traditional East Asian aesthetics, created for Computer Graphics Assignment 2.

## Game Objective

Navigate the warrior character across four distinct platforms, collecting all the ancient relics before the timer runs out. Each platform contains unique 3D objects that animate after collecting all items on that platform.

## Features

- **Theme**: Ancient East Asian Warriors with elements like torii gates, pagodas, taiko drums, and stone lanterns
- **All models constructed from OpenGL primitives** (no imported models)
- **4 distinct platforms** with different colors and designs
- **12 collectible relics** (3 per platform)
- **4 unique 3D objects**: Torii gate, Pagoda, Taiko drum, Stone lantern
- **Complete collision detection** system
- **4 animation types**: rotation, scaling, transformation, color change
- **Timer system** with Game Over condition
- **Multiple camera views** with free movement along 3 axes

## Controls

### Player Movement
- **WASD or Arrow Keys**: Move on X and Z axes
- **Character automatically rotates** to face movement direction

### Camera Controls
- **I/K**: Move camera forward/back
- **J/L**: Move camera left/right
- **U/O**: Move camera down/up
- **1**: Top View (fixed)
- **2**: Side View (fixed)
- **3**: Front View (fixed)
- **V**: Cycle through camera views
- **R**: Reset game

### Animation Toggles (unlocked after collecting all platform items)
- **5**: Toggle rotation animation (Platform 1 - Torii)
- **6**: Toggle scaling animation (Platform 2 - Pagoda)
- **7**: Toggle transformation animation (Platform 3 - Taiko)
- **8**: Toggle color change animation (Platform 4 - Lantern)

## How to Compile and Run

### Prerequisites
- OpenGL development libraries
- GLUT library
- C++11 compatible compiler

### macOS (using Homebrew)
```bash
# Install required packages
brew install freeglut mesa

# Compile
g++ -std=c++11 -Wall -Wextra -o PXX_YYYY PXX_YYYY.cpp -framework OpenGL -framework GLUT

# OR use the provided Makefile
make

# Run
./PXX_YYYY
```

### Linux
Modify the Makefile's LIBS line to use Linux glut libraries:
```
LIBS = -lGL -lGLU -lglut
```

Then compile:
```bash
make
./PXX_YYYY
```

### Windows
You'll need to install GLUT and set up your Visual Studio environment appropriately.

## Game Rules

1. Collect all relics on each platform (3 per platform)
2. After collecting all relics on a platform, the platform's object begins animating
3. Toggle animations on/off using the corresponding number keys
4. Win by collecting all 12 relics from all platforms
5. Lose if the timer runs out (120 seconds)
6. Navigate using the collision boundaries - you cannot pass through walls or objects

## Technical Implementation

This implementation follows all assignment requirements:

- **21+ objects**: ground, 3 walls, player, 4 platforms, 4 feature objects, 12+ collectibles
- **Minimum primitive counts per object**:
  - Player: 6+ primitives (torso, head, arms, legs, headband)
  - Walls: 1 primitive each
  - Ground: 1 primitive
  - Platforms: 2+ primitives each
  - Feature objects: 4+ primitives each
  - Collectibles: 3+ primitives each
- **Animation types**: rotation, scaling, transformation, color change
- **Camera system**: free movement + 3 fixed views
- **Timer system**: countdown with Game Over screen
- **Collision detection**: walls, platforms, objects, and collectibles

## Submission

For assignment submission, rename the file according to your tutorial group and student ID:
- Format: `PXX_YYYY.cpp`
- Replace XX with your tutorial group number
- Replace YYYY with your student ID

Only submit the `.cpp` file - do not submit any `.sln` or project files.

## Known Issues

- Game uses legacy OpenGL for compatibility with teaching environments
- Sound effects and advanced lighting are not implemented per assignment focus on fundamental primitives

## License

This code is created for educational purposes as part of a computer graphics assignment.