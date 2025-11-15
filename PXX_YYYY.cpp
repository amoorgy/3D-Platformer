// PXX_YYYY.cpp
// 3D Collectibles Game - Ancient East Asian Warriors Theme
// Single-file OpenGL (legacy) implementation suitable for teaching assignments.
// Controls:
//  - Move: WASD or Arrow Keys (XZ plane)
//  - Camera free move: I/K (forward/back), J/L (left/right), U/O (down/up)
//  - Camera preset views: 1=Top, 2=Side, 3=Front, V=cycle
//  - Toggle animations (only after collecting that platform's items):
//      5 = Platform 1 (rotation), 6 = Platform 2 (scaling), 7 = Platform 3 (translation), 8 = Platform 4 (color)
//  - Reset: R
// Notes:
//  - Everything is built from OpenGL primitives (quads/triangles). No imported models.
//  - Uses GLUT for windowing/input and GLU for camera.
//  - Keep code readable and straightforward at a bachelor level, but complete.

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>

// --------------------------- Math helpers ---------------------------
struct Vec3 { float x, y, z; };
static inline Vec3 makeVec3(float x, float y, float z){ return {x,y,z}; }
static inline Vec3 add(const Vec3&a,const Vec3&b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
static inline Vec3 sub(const Vec3&a,const Vec3&b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
static inline Vec3 mul(const Vec3&a,float s){ return {a.x*s,a.y*s,a.z*s}; }

struct AABB {
    Vec3 center; // world position
    Vec3 half;   // half sizes
};

static inline bool aabbIntersects(const AABB&a, const AABB&b){
    return std::abs(a.center.x - b.center.x) <= (a.half.x + b.half.x) &&
           std::abs(a.center.y - b.center.y) <= (a.half.y + b.half.y) &&
           std::abs(a.center.z - b.center.z) <= (a.half.z + b.half.z);
}

// Distance squared in XZ plane
static inline float dist2XZ(const Vec3&a, const Vec3&b){
    float dx=a.x-b.x, dz=a.z-b.z; return dx*dx+dz*dz;
}

// --------------------------- Global state ---------------------------
static int winW=1200, winH=800;

// World scale and layout
static const float WORLD_HALF = 40.0f; // square world [-40,40] in XZ

// Player state
static Vec3 playerPos = {0.0f, 1.0f, 0.0f}; // y=1 to sit above ground (thickness)
static Vec3 playerDir = {0.0f, 0.0f, -1.0f};
static float playerSpeed = 12.0f; // units per second
static float playerYawDeg = 0.0f; // face movement direction
static const Vec3 playerHalf = {0.7f, 1.0f, 0.7f}; // AABB half size

// Camera
static Vec3 camPos = {0.0f, 18.0f, 28.0f};
static Vec3 camTarget = {0.0f, 0.0f, 0.0f};
static Vec3 camUp = {0.0f, 1.0f, 0.0f};

enum CameraPreset { CAM_FREE=0, CAM_TOP, CAM_SIDE, CAM_FRONT };
static CameraPreset camMode = CAM_FREE;

// Timer
static float gameTime = 120.0f; // seconds countdown
static bool gameOver = false;
static bool gameWin = false;

// Platforms
struct Platform {
    AABB box; // position/size
    float color[3];
};
static Platform platforms[4];

// Platform featured objects + animation states
enum AnimType { ANIM_ROTATE=0, ANIM_SCALE, ANIM_TRANSLATE, ANIM_COLOR };
struct FeatureObj {
    AABB box; // base AABB for collision (not animated extents)
    float baseColor[3];
    AnimType type;
    bool allCollected = false; // becomes true after collectibles on its platform are done
    bool animEnabled = false;  // can toggle only after collected
    float t = 0.0f; // time accumulator
};
static FeatureObj features[4];

// Collectibles
struct Collectible {
    AABB box;
    float color[3];
    bool collected=false;
    int platformIndex=0; // which platform
};
static std::vector<Collectible> collectibles;
static int collectedPerPlatform[4] = {0,0,0,0};
static int totalCollectiblesPerPlatform = 3; // configurable

// Walls and ground
static AABB groundBox; // thin box as ground
static std::vector<AABB> walls; // 3 bounding walls

// Input state
static bool keyDown[256];
static bool specialDown[512];

// Time step
static int prevTicks = 0;

// ------------------------ Drawing primitives ------------------------
static void setColor3f(float r,float g,float b){ glColor3f(r,g,b); }

static void drawQuad(const Vec3&a,const Vec3&b,const Vec3&c,const Vec3&d){
    glBegin(GL_QUADS);
    glVertex3f(a.x,a.y,a.z);
    glVertex3f(b.x,b.y,b.z);
    glVertex3f(c.x,c.y,c.z);
    glVertex3f(d.x,d.y,d.z);
    glEnd();
}

static void drawBox(const AABB& box){
    const float x=box.center.x, y=box.center.y, z=box.center.z;
    const float hx=box.half.x, hy=box.half.y, hz=box.half.z;
    // 6 faces
    // +Y top
    setColor3f(0.85f,0.85f,0.85f);
    drawQuad({x-hx,y+hy,z-hz},{x+hx,y+hy,z-hz},{x+hx,y+hy,z+hz},{x-hx,y+hy,z+hz});
    // -Y bottom
    setColor3f(0.5f,0.5f,0.5f);
    drawQuad({x-hx,y-hy,z+hz},{x+hx,y-hy,z+hz},{x+hx,y-hy,z-hz},{x-hx,y-hy,z-hz});
    // +X
    setColor3f(0.75f,0.75f,0.8f);
    drawQuad({x+hx,y-hy,z-hz},{x+hx,y+hy,z-hz},{x+hx,y+hy,z+hz},{x+hx,y-hy,z+hz});
    // -X
    setColor3f(0.7f,0.7f,0.75f);
    drawQuad({x-hx,y-hy,z+hz},{x-hx,y+hy,z+hz},{x-hx,y+hy,z-hz},{x-hx,y-hy,z-hz});
    // +Z
    setColor3f(0.8f,0.7f,0.7f);
    drawQuad({x-hx,y-hy,z+hz},{x-hx,y+hy,z+hz},{x+hx,y+hy,z+hz},{x+hx,y-hy,z+hz});
    // -Z
    setColor3f(0.7f,0.8f,0.7f);
    drawQuad({x+hx,y-hy,z-hz},{x+hx,y+hy,z-hz},{x-hx,y+hy,z-hz},{x-hx,y-hy,z-hz});
}

// A simple colored box with a single color
static void drawSolidBox(const AABB&box, float r, float g, float b){
    glColor3f(r,g,b);
    const float x=box.center.x, y=box.center.y, z=box.center.z;
    const float hx=box.half.x, hy=box.half.y, hz=box.half.z;
    glBegin(GL_QUADS);
    // top
    glVertex3f(x-hx,y+hy,z-hz); glVertex3f(x+hx,y+hy,z-hz); glVertex3f(x+hx,y+hy,z+hz); glVertex3f(x-hx,y+hy,z+hz);
    // bottom
    glVertex3f(x-hx,y-hy,z+hz); glVertex3f(x+hx,y-hy,z+hz); glVertex3f(x+hx,y-hy,z-hz); glVertex3f(x-hx,y-hy,z-hz);
    // +X
    glVertex3f(x+hx,y-hy,z-hz); glVertex3f(x+hx,y+hy,z-hz); glVertex3f(x+hx,y+hy,z+hz); glVertex3f(x+hx,y-hy,z+hz);
    // -X
    glVertex3f(x-hx,y-hy,z+hz); glVertex3f(x-hx,y+hy,z+hz); glVertex3f(x-hx,y+hy,z-hz); glVertex3f(x-hx,y-hy,z-hz);
    // +Z
    glVertex3f(x-hx,y-hy,z+hz); glVertex3f(x-hx,y+hy,z+hz); glVertex3f(x+hx,y+hy,z+hz); glVertex3f(x+hx,y-hy,z+hz);
    // -Z
    glVertex3f(x+hx,y-hy,z-hz); glVertex3f(x+hx,y+hy,z-hz); glVertex3f(x-hx,y+hy,z-hz); glVertex3f(x-hx,y-hy,z-hz);
    glEnd();
}

// Pyramid (square base) for some East Asian aesthetic (roof-like)
static void drawPyramid(const Vec3&center, float base, float height, float r, float g, float b){
    glColor3f(r,g,b);
    float x=center.x, y=center.y, z=center.z;
    float h=height; float b2=base*0.5f;
    // Base quad
    glBegin(GL_QUADS);
    glVertex3f(x-b2,y,z-b2); glVertex3f(x+b2,y,z-b2); glVertex3f(x+b2,y,z+b2); glVertex3f(x-b2,y,z+b2);
    glEnd();
    // 4 side triangles
    glBegin(GL_TRIANGLES);
    // +Z face
    glVertex3f(x-b2,y,z+b2); glVertex3f(x+b2,y,z+b2); glVertex3f(x, y+h, z);
    // -Z face
    glVertex3f(x+b2,y,z-b2); glVertex3f(x-b2,y,z-b2); glVertex3f(x, y+h, z);
    // +X face
    glVertex3f(x+b2,y,z-b2); glVertex3f(x+b2,y,z+b2); glVertex3f(x, y+h, z);
    // -X face
    glVertex3f(x-b2,y,z+b2); glVertex3f(x-b2,y,z-b2); glVertex3f(x, y+h, z);
    glEnd();
}

// A small torii-like gateway made from boxes (Japanese aesthetic)
static void drawTorii(const Vec3&center, float scale, const float col[3]){
    float r=col[0], g=col[1], b=col[2];
    // pillars
    drawSolidBox({{center.x-1.0f*scale, center.y+2.0f*scale, center.z}, {0.3f*scale, 2.0f*scale, 0.3f*scale}}, r,g,b);
    drawSolidBox({{center.x+1.0f*scale, center.y+2.0f*scale, center.z}, {0.3f*scale, 2.0f*scale, 0.3f*scale}}, r,g,b);
    // cross beam
    drawSolidBox({{center.x, center.y+4.2f*scale, center.z}, {1.8f*scale, 0.25f*scale, 0.4f*scale}}, r*0.9f,g*0.9f,b*0.9f);
    // top cap
    drawSolidBox({{center.x, center.y+4.7f*scale, center.z}, {2.1f*scale, 0.15f*scale, 0.5f*scale}}, r*0.8f,g*0.8f,b*0.8f);
}

// A simple pagoda-like stack: boxes + pyramids
static void drawPagoda(const Vec3&center, float scale, const float col[3]){
    float r=col[0], g=col[1], b=col[2];
    float y=center.y;
    // base box
    drawSolidBox({{center.x, y+0.5f*scale, center.z}, {1.8f*scale, 0.5f*scale, 1.8f*scale}}, r*0.6f,g*0.6f,b*0.6f);
    // roof 1
    drawPyramid({center.x, y+1.0f*scale, center.z}, 4.0f*scale, 0.8f*scale, r,g,b);
    // middle box
    drawSolidBox({{center.x, y+1.8f*scale, center.z}, {1.2f*scale, 0.4f*scale, 1.2f*scale}}, r*0.6f,g*0.6f,b*0.6f);
    // roof 2
    drawPyramid({center.x, y+2.2f*scale, center.z}, 3.2f*scale, 0.7f*scale, r*0.95f,g*0.95f,b*0.95f);
}

// Collectible: a small shrine-like piece (3+ primitives): base box + roof pyramid + ornament box
static void drawCollectibleGeom(const Collectible&c){
    float r=c.color[0], g=c.color[1], b=c.color[2];
    // base
    drawSolidBox({{c.box.center.x, c.box.center.y-0.1f, c.box.center.z}, {c.box.half.x, 0.1f, c.box.half.z}}, r*0.6f,g*0.6f,b*0.6f);
    // roof
    drawPyramid({c.box.center.x, c.box.center.y+0.0f, c.box.center.z}, c.box.half.x*3.0f, c.box.half.y*1.8f, r,g,b);
    // ornament
    drawSolidBox({{c.box.center.x, c.box.center.y+0.45f, c.box.center.z}, {0.08f,0.2f,0.08f}}, r*0.9f,g*0.9f,b*0.2f);
}

// Player model (at least 6 primitives): torso box, head box, 4 limb boxes, plus a headband slab for theme
static void drawPlayer(){
    glPushMatrix();
    glTranslatef(playerPos.x, playerPos.y, playerPos.z);
    glRotatef(playerYawDeg, 0,1,0);
    // torso
    drawSolidBox({{0, 1.0f, 0}, {0.6f, 0.8f, 0.35f}}, 0.9f,0.1f,0.1f); // red gi
    // head
    drawSolidBox({{0, 2.0f, 0}, {0.35f, 0.35f, 0.35f}}, 0.9f,0.85f,0.7f);
    // headband
    drawSolidBox({{0, 2.1f, 0.36f}, {0.36f, 0.06f, 0.06f}}, 0.2f,0.2f,0.2f);
    // legs
    drawSolidBox({{-0.25f, 0.2f, 0}, {0.2f, 0.6f, 0.2f}}, 0.1f,0.1f,0.1f);
    drawSolidBox({{ 0.25f, 0.2f, 0}, {0.2f, 0.6f, 0.2f}}, 0.1f,0.1f,0.1f);
    // arms
    drawSolidBox({{-0.9f, 1.1f, 0}, {0.3f, 0.15f, 0.15f}}, 0.9f,0.85f,0.7f);
    drawSolidBox({{ 0.9f, 1.1f, 0}, {0.3f, 0.15f, 0.15f}}, 0.9f,0.85f,0.7f);
    glPopMatrix();
}

// Feature object draw variants
static void drawFeatureObj(const FeatureObj&f){
    glPushMatrix();
    glTranslatef(f.box.center.x, f.box.center.y, f.box.center.z);

    float r=f.baseColor[0], g=f.baseColor[1], b=f.baseColor[2];

    if(f.type==ANIM_ROTATE && f.animEnabled){
        glRotatef(fmodf(f.t*60.0f,360.0f), 0,1,0);
    }
    if(f.type==ANIM_SCALE && f.animEnabled){
        float s = 1.0f + 0.25f*sinf(f.t*2.0f);
        glScalef(s,s,s);
    }
    if(f.type==ANIM_TRANSLATE && f.animEnabled){
        float d = 0.8f*sinf(f.t*1.5f);
        glTranslatef(0, d, 0);
    }
    if(f.type==ANIM_COLOR && f.animEnabled){
        r = 0.5f + 0.5f*sinf(f.t*2.0f);
        g = 0.3f + 0.7f*fabsf(cosf(f.t*1.7f));
        b = 0.2f + 0.8f*(0.5f+0.5f*sinf(f.t*1.1f+1.1f));
    }

    // Give each feature a distinct East Asian object: torii, pagoda, drum, lantern
    switch(f.type){
        case ANIM_ROTATE: {
            float col[3]={r,g,b};
            drawTorii({0,0,0}, 1.8f, col);
        } break;
        case ANIM_SCALE: {
            float col[3]={r,g,b};
            drawPagoda({0,0,0}, 1.2f, col);
        } break;
        case ANIM_TRANSLATE: {
            // taiko drum-like: two boxes
            drawSolidBox({{0,0.9f,0}, {1.2f,0.9f,1.2f}}, r*0.8f,g*0.1f,b*0.1f);
            drawSolidBox({{0,1.9f,0}, {1.0f,0.15f,1.0f}}, r*0.9f,g*0.9f,b*0.9f);
        } break;
        case ANIM_COLOR: {
            // stone lantern-like stack
            drawSolidBox({{0,0.3f,0}, {0.6f,0.3f,0.6f}}, r*0.6f,g*0.6f,b*0.6f);
            drawSolidBox({{0,0.9f,0}, {0.25f,0.6f,0.25f}}, r*0.8f,g*0.8f,b*0.8f);
            drawPyramid({0,1.3f,0}, 1.2f, 0.5f, r,g,b);
        } break;
    }

    glPopMatrix();
}

// --------------------------- Scene setup ---------------------------
static void resetGame(){
    playerPos = {0.0f, 1.0f, 0.0f};
    playerDir = {0.0f, 0.0f, -1.0f};
    playerYawDeg = 0.0f;
    camPos = {0.0f, 18.0f, 28.0f};
    camTarget = {0.0f, 0.0f, 0.0f};
    camUp = {0.0f, 1.0f, 0.0f};
    camMode = CAM_FREE;
    gameTime = 120.0f;
    gameOver=false; gameWin=false;

    // Ground
    groundBox = {{0.0f, 0.0f, 0.0f}, {WORLD_HALF, 0.2f, WORLD_HALF}};

    // Walls: make U-shape (3 walls)
    walls.clear();
    walls.push_back({{0.0f, 2.0f, -WORLD_HALF+1.0f}, {WORLD_HALF, 2.0f, 1.0f}}); // back
    walls.push_back({{-WORLD_HALF+1.0f, 2.0f, 0.0f}, {1.0f, 2.0f, WORLD_HALF}}); // left
    walls.push_back({{ WORLD_HALF-1.0f, 2.0f, 0.0f}, {1.0f, 2.0f, WORLD_HALF}}); // right

    // Platforms in four quadrants with different colors and sizes
    float ph = 0.3f;
    platforms[0] = {{ {-20, ph, -20}, {8, ph, 6} }, {0.8f,0.2f,0.2f}}; // red
    platforms[1] = {{ { 20, ph, -15}, {6, ph, 8} }, {0.2f,0.6f,0.9f}}; // blue
    platforms[2] = {{ {-18, ph,  20}, {7, ph, 7} }, {0.2f,0.8f,0.3f}}; // green
    platforms[3] = {{ { 18, ph,  18}, {9, ph, 5} }, {0.9f,0.8f,0.2f}}; // yellow

    // Feature objects centered on each platform
    features[0] = { { {platforms[0].box.center.x, 0.0f, platforms[0].box.center.z}, {1.6f,2.6f,1.0f} }, {0.8f,0.15f,0.15f}, ANIM_ROTATE, false, false, 0.0f };
    features[1] = { { {platforms[1].box.center.x, 0.0f, platforms[1].box.center.z}, {1.6f,2.6f,1.6f} }, {0.7f,0.4f,0.9f}, ANIM_SCALE, false, false, 0.0f };
    features[2] = { { {platforms[2].box.center.x, 0.0f, platforms[2].box.center.z}, {1.6f,2.0f,1.6f} }, {0.9f,0.3f,0.3f}, ANIM_TRANSLATE, false, false, 0.0f };
    features[3] = { { {platforms[3].box.center.x, 0.0f, platforms[3].box.center.z}, {1.6f,2.2f,1.6f} }, {0.6f,0.6f,0.7f}, ANIM_COLOR, false, false, 0.0f };

    // Collectibles: 3 per platform arranged in small triangle pattern
    collectibles.clear();
    for(int i=0;i<4;i++){ collectedPerPlatform[i]=0; }

    auto addCollectible = [&](int pi, float offx, float offz, float r, float g, float b){
        const Platform& p = platforms[pi];
        Vec3 c = {p.box.center.x + offx, 0.6f, p.box.center.z + offz};
        Collectible col;
        col.box = { c, {0.18f, 0.35f, 0.18f} };
        col.color[0]=r; col.color[1]=g; col.color[2]=b;
        col.collected=false; col.platformIndex=pi;
        collectibles.push_back(col);
    };

    addCollectible(0, -2.0f, -1.5f, 0.9f,0.3f,0.3f);
    addCollectible(0,  2.2f, -1.2f, 0.9f,0.5f,0.3f);
    addCollectible(0,  0.0f,  2.0f, 0.9f,0.3f,0.5f);

    addCollectible(1, -1.2f,  2.2f, 0.3f,0.7f,0.9f);
    addCollectible(1,  1.8f,  0.0f, 0.3f,0.9f,0.7f);
    addCollectible(1,  0.0f, -2.2f, 0.5f,0.8f,0.9f);

    addCollectible(2, -2.0f,  1.4f, 0.2f,0.9f,0.3f);
    addCollectible(2,  2.0f,  0.0f, 0.2f,0.7f,0.4f);
    addCollectible(2,  0.0f, -1.8f, 0.2f,0.9f,0.6f);

    addCollectible(3, -2.4f,  0.4f, 0.9f,0.9f,0.3f);
    addCollectible(3,  2.4f, -0.4f, 0.9f,0.8f,0.2f);
    addCollectible(3,  0.0f, -2.0f, 0.9f,0.7f,0.2f);

    // Reset feature gates
    for(int i=0;i<4;i++){
        features[i].allCollected=false;
        features[i].animEnabled=false;
        features[i].t=0.0f;
    }
}

// --------------------------- Collision ---------------------------
static bool collidesWithWorld(const AABB&box){
    // Against walls
    for(const auto&w : walls){ if(aabbIntersects(box,w)) return true; }
    // Against platform feature objects
    for(const auto&f : features){ if(aabbIntersects(box,f.box)) return true; }
    // Against platforms treated as low boxes (prevent walking through their bodies)
    for(const auto&p : platforms){
        // make collidable volume a bit thicker
        AABB pb = p.box; pb.half.y = 2.0f; pb.center.y = 1.0f;
        if(aabbIntersects(box,pb)) return true;
    }
    return false;
}

static void tryMovePlayer(const Vec3&delta){
    // Separate axis resolution to avoid sticking too much
    AABB pb = { playerPos, playerHalf };

    // X
    Vec3 attempt = playerPos; attempt.x += delta.x;
    pb.center = attempt; if(!collidesWithWorld(pb)) playerPos.x = attempt.x;

    // Z
    attempt = playerPos; attempt.z += delta.z;
    pb.center = attempt; if(!collidesWithWorld(pb)) playerPos.z = attempt.z;
}

// --------------------------- Game logic ---------------------------
static void updateCollectibles(){
    AABB pb = { playerPos, playerHalf };
    int completedCount=0;
    for(auto &c : collectibles){
        if(!c.collected && aabbIntersects(pb, c.box)){
            c.collected = true;
            collectedPerPlatform[c.platformIndex]++;
        }
    }
    // Check platform completions, unlock animation toggles
    for(int i=0;i<4;i++){
        if(!features[i].allCollected && collectedPerPlatform[i] >= totalCollectiblesPerPlatform){
            features[i].allCollected = true; // toggles now allowed for this platform
        }
        if(collectedPerPlatform[i] >= totalCollectiblesPerPlatform) completedCount++;
    }
    if(completedCount==4) gameWin = true;
}

static void updateFeatures(float dt){
    for(int i=0;i<4;i++){
        if(features[i].animEnabled) features[i].t += dt;
    }
}

// --------------------------- Rendering ---------------------------
static void drawGround(){
    // ground plane as colored box (stone)
    glColor3f(0.25f,0.25f,0.28f);
    drawSolidBox(groundBox, 0.25f,0.28f,0.25f);
    // subtle grid lines for aesthetics (optional, lines)
    glColor3f(0.15f,0.15f,0.16f);
    glBegin(GL_LINES);
    for(int i=-40;i<=40;i+=4){
        glVertex3f((float)i, 0.201f, -WORLD_HALF); glVertex3f((float)i, 0.201f, WORLD_HALF);
        glVertex3f(-WORLD_HALF, 0.201f, (float)i); glVertex3f(WORLD_HALF, 0.201f, (float)i);
    }
    glEnd();
}

static void drawWalls(){
    glColor3f(0.35f,0.35f,0.4f);
    for(const auto&w : walls){ drawSolidBox(w, 0.33f,0.33f,0.36f); }
}

static void drawPlatforms(){
    for(int i=0;i<4;i++){
        const Platform&p = platforms[i];
        glColor3f(p.color[0],p.color[1],p.color[2]);
        drawSolidBox(p.box, p.color[0],p.color[1],p.color[2]);
        // Add a decorative rim to make platforms visually distinct
        glColor3f(0.1f,0.1f,0.1f);
        AABB rim = p.box; rim.half.x += 0.5f; rim.half.z += 0.5f; rim.half.y = 0.05f; rim.center.y = p.box.center.y + p.box.half.y + rim.half.y;
        drawSolidBox(rim, 0.1f,0.1f,0.1f);
    }
}

static void drawCollectibles(){
    for(const auto&c : collectibles){ if(!c.collected) drawCollectibleGeom(c); }
}

static void drawFeatures(){
    for(int i=0;i<4;i++) drawFeatureObj(features[i]);
}

static void drawHUD(){
    // 2D overlay
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    auto drawText = [&](int x,int y,const char* s){
        glRasterPos2i(x,y);
        for(const char* p=s; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    };

    char buf[128];
    snprintf(buf, sizeof(buf), "Time: %ds", (int)std::max(0.0f, gameTime));
    glColor3f(1,1,1); drawText(10, winH-20, buf);
    snprintf(buf, sizeof(buf), "Collected: [%d/%d] [%d/%d] [%d/%d] [%d/%d]",
        collectedPerPlatform[0], totalCollectiblesPerPlatform,
        collectedPerPlatform[1], totalCollectiblesPerPlatform,
        collectedPerPlatform[2], totalCollectiblesPerPlatform,
        collectedPerPlatform[3], totalCollectiblesPerPlatform);
    drawText(10, winH-40, buf);

    if(gameWin){ glColor3f(0.2f,1.0f,0.3f); drawText(winW/2-60, winH-60, "GAME WIN!"); }

    if(gameOver){ glColor3f(1.0f,0.2f,0.2f); drawText(winW/2-70, winH/2, "GAME OVER"); drawText(winW/2-110, winH/2-20, "Press R to Restart"); }

    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
}

static void setCamera(){
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60.0, (double)winW/(double)winH, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    Vec3 eye=camPos, target=camTarget, up=camUp;
    if(camMode==CAM_TOP){ eye = {0.0f, 80.0f, 0.01f}; target = {0,0,0}; up={0,0,-1}; }
    else if(camMode==CAM_SIDE){ eye = {80.0f, 15.0f, 0.01f}; target = {0,0,0}; up={0,1,0}; }
    else if(camMode==CAM_FRONT){ eye = {0.01f, 15.0f, 80.0f}; target = {0,0,0}; up={0,1,0}; }

    gluLookAt(eye.x,eye.y,eye.z, target.x,target.y,target.z, up.x,up.y,up.z);
}

static void display(){
    glClearColor(0.04f,0.05f,0.07f,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setCamera();

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);

    // Light-ish directional effect via simple colored faces (no real lighting for simplicity)

    drawGround();
    drawWalls();
    drawPlatforms();
    drawFeatures();
    drawCollectibles();
    drawPlayer();

    drawHUD();

    glutSwapBuffers();
}

// --------------------------- Input & update ---------------------------
static void updateCameraFreeMove(float dt){
    if(camMode!=CAM_FREE) return;
    // basic strafing with IJKL/UO
    Vec3 fwd = sub(camTarget, camPos); fwd.y=0; float fl = std::max(0.0001f, std::sqrt(fwd.x*fwd.x+fwd.z*fwd.z)); fwd = mul(fwd, 1.0f/fl);
    Vec3 right = { fwd.z, 0.0f, -fwd.x };
    float speed = 25.0f;
    if(keyDown['i']||keyDown['I']){ camPos = add(camPos, mul(fwd, speed*dt)); camTarget = add(camTarget, mul(fwd, speed*dt)); }
    if(keyDown['k']||keyDown['K']){ camPos = sub(camPos, mul(fwd, speed*dt)); camTarget = sub(camTarget, mul(fwd, speed*dt)); }
    if(keyDown['j']||keyDown['J']){ camPos = sub(camPos, mul(right, speed*dt)); camTarget = sub(camTarget, mul(right, speed*dt)); }
    if(keyDown['l']||keyDown['L']){ camPos = add(camPos, mul(right, speed*dt)); camTarget = add(camTarget, mul(right, speed*dt)); }
    if(keyDown['u']||keyDown['U']){ camPos.y -= speed*dt; camTarget.y -= speed*dt; }
    if(keyDown['o']||keyDown['O']){ camPos.y += speed*dt; camTarget.y += speed*dt; }
}

static void updatePlayerMovement(float dt){
    if(gameOver) return; // no control on game over

    Vec3 move = {0,0,0};
    if(keyDown['w'] || specialDown[GLUT_KEY_UP]) move.z -= 1;
    if(keyDown['s'] || specialDown[GLUT_KEY_DOWN]) move.z += 1;
    if(keyDown['a'] || specialDown[GLUT_KEY_LEFT]) move.x -= 1;
    if(keyDown['d'] || specialDown[GLUT_KEY_RIGHT]) move.x += 1;

    float len = std::sqrt(move.x*move.x + move.z*move.z);
    if(len>0.0001f){
        move = mul(move, 1.0f/len);
        tryMovePlayer(mul(move, playerSpeed*dt));
        // face movement direction
        playerYawDeg = std::atan2f(move.x, -move.z) * 180.0f / 3.14159265f; // z- forward
    }
}

static void idle(){
    int t = glutGet(GLUT_ELAPSED_TIME);
    if(prevTicks==0) prevTicks=t;
    float dt = (t - prevTicks) / 1000.0f;
    prevTicks = t;

    if(!gameOver){
        gameTime -= dt; if(gameTime<=0.0f){ gameTime=0.0f; gameOver=true; }
    }

    updateCameraFreeMove(dt);
    updatePlayerMovement(dt);
    updateCollectibles();
    updateFeatures(dt);

    glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y){
    keyDown[key] = true;

    if(key=='1') camMode = CAM_TOP;
    if(key=='2') camMode = CAM_SIDE;
    if(key=='3') camMode = CAM_FRONT;
    if(key=='v' || key=='V'){
        if(camMode==CAM_FREE) camMode=CAM_TOP; else if(camMode==CAM_TOP) camMode=CAM_SIDE; else if(camMode==CAM_SIDE) camMode=CAM_FRONT; else camMode=CAM_FREE;
    }
    if(key=='r' || key=='R') resetGame();

    // Toggle animations only after collectibles are complete for that platform
    if(key=='5' && features[0].allCollected) features[0].animEnabled = !features[0].animEnabled;
    if(key=='6' && features[1].allCollected) features[1].animEnabled = !features[1].animEnabled;
    if(key=='7' && features[2].allCollected) features[2].animEnabled = !features[2].animEnabled;
    if(key=='8' && features[3].allCollected) features[3].animEnabled = !features[3].animEnabled;
}

static void keyboardUp(unsigned char key, int x, int y){ keyDown[key] = false; }

static void special(int key, int x, int y){ specialDown[key] = true; }
static void specialUp(int key, int x, int y){ specialDown[key] = false; }

static void reshape(int w, int h){ winW=w; winH=h>0?h:1; glViewport(0,0,winW,winH); }

// --------------------------- Init ---------------------------
static void initGL(){
    glEnable(GL_DEPTH_TEST);
}

int main(int argc, char** argv){
    std::memset(keyDown, 0, sizeof(keyDown));
    std::memset(specialDown, 0, sizeof(specialDown));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Ancient East Asian Collectibles - Assignment 2");

    initGL();
    resetGame();

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
