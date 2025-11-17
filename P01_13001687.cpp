// PXX_YYYY.cpp
// 3D Platformer Collectibles Game - Ancient East Asian Warriors
// Single-file OpenGL (legacy) implementation suitable for teaching assignments.
// Controls:
//  - Move: WASD or Arrow Keys (XZ plane)
//  - Jump: Spacebar
//  - Camera: 1=Follow (semi top-down), 2=Top view, 3=Side view, 4=Front view, V=cycle
//  - Camera free move: I/K (forward/back), J/L (left/right), U/O (down/up)
//  - Pause/unpause animations (animations auto-start when collectibles are collected):
//      R = Red platform (rotation), B = Blue platform (scaling),
//      G = Green platform (translation), Y = Yellow platform (color change)
//  - Reset game: ESC
// Notes:
//  - Everything is built from OpenGL primitives (quads/triangles). No imported models.
//  - Uses GLUT for windowing/input and GLU for camera.

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
// GLU for camera

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>

// Optional audio: compile-time/header-availability guard for single-file submission
#ifndef USE_MINIAUDIO
#  if defined(__has_include)
#    if __has_include("third_party/miniaudio.h")
#      define USE_MINIAUDIO 1
#    else
#      define USE_MINIAUDIO 0
#    endif
#  else
#    define USE_MINIAUDIO 0
#  endif
#endif

#if USE_MINIAUDIO
#define MINIAUDIO_IMPLEMENTATION
#include "third_party/miniaudio.h"
#endif

static constexpr float PI_F = 3.14159265358979323846f;

// --------------------------- Math helpers ---------------------------
struct Vec3 { float x, y, z; };
static inline Vec3 makeVec3(float x, float y, float z){ return {x,y,z}; }
static inline Vec3 add(const Vec3&a,const Vec3&b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
static inline Vec3 sub(const Vec3&a,const Vec3&b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
static inline Vec3 mul(const Vec3&a,float s){ return {a.x*s,a.y*s,a.z*s}; } 
// to simplify vector operations

struct AABB {
    Vec3 center; // world position
    Vec3 half;   // half sizes
}; // for the object boundin so we can calculate from (center - half) to (center + half)

static inline bool aabbIntersects(const AABB&a, const AABB&b){
    return std::abs(a.center.x - b.center.x) <= (a.half.x + b.half.x) &&
           std::abs(a.center.y - b.center.y) <= (a.half.y + b.half.y) &&
           std::abs(a.center.z - b.center.z) <= (a.half.z + b.half.z);
} // to check if two boxes intersect


static inline float dist2XZ(const Vec3&a, const Vec3&b){
    float dx=a.x-b.x, dz=a.z-b.z; return dx*dx+dz*dz;
} //. calculates the distance for in xy plane ignoring height y

// --------------------------- Global state ---------------------------
static int winW=1200, winH=800; // this is for window dimensions and size

// World scale and layout
static const float WORLD_HALF = 40.0f; // this is the playable area which is a 80x80 unit sqyare at origin

// Player state
static Vec3 playerPos = {0.0f, 1.0f, 0.0f}; // y=1 to sit above ground (thickness)
static Vec3 playerDir = {0.0f, 0.0f, -1.0f};
static float playerSpeed = 12.0f; // units per second
static float playerYawDeg = 0.0f; // face movement direction
static const Vec3 playerHalf = {0.7f, 1.0f, 0.7f}; // AABB half size
static float playerVelY = 0.0f; // vertical velocity for jumping
static bool playerOnGround = true; // is player standing on ground or platform
static const float GRAVITY = -25.0f; // gravity acceleration
static const float JUMP_VELOCITY = 12.0f; // initial jump velocity

// Camera
static Vec3 camPos = {0.0f, 18.0f, 28.0f};
static Vec3 camTarget = {0.0f, 0.0f, 0.0f};
static Vec3 camUp = {0.0f, 1.0f, 0.0f};

enum CameraPreset { CAM_FOLLOW=0, CAM_TOP, CAM_SIDE, CAM_FRONT, CAM_FREE };
static CameraPreset camMode = CAM_FOLLOW; // Fixed-angle semi top-down camera (isometric style)

// Game state
enum GameState { PLAYING, WON, LOST };
static GameState gameState = PLAYING;
static float gameTime = 120.0f; // seconds countdown

// Simplified flying oracle for game over
struct FlyingOracle {
    Vec3 pos;
    Vec3 vel;
    float rotation;
    float color[3];
};
static FlyingOracle flyingOracles[4];

// Platforms
struct Platform {
    AABB box; // position/size
    float color[3];
};
static Platform platforms[4];

// Obstacles - static and moving
struct Obstacle {
    AABB box;
    float color[3];
    bool isMoving;
    float moveSpeed;
    float moveRange;
    Vec3 basePos; // for moving obstacles
    float moveTime; // accumulated time for movement
};
static std::vector<Obstacle> obstacles;

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

// Simplified sky oracles
struct SkyOracle {
    Vec3 pos;
    float radius;
    float rotation;
    float color[3];
};
static std::vector<SkyOracle> skyOracles;

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

// --------------------------- Audio ---------------------------
#if USE_MINIAUDIO
static ma_engine audioEngine;
static bool audioReady = false;

struct AudioClip {
    ma_sound sound;
    bool loaded = false;
    bool played = false; // Track if one-time sounds have played
};

static AudioClip audioBgm, audioCollect, audioWin, audioLose;

static bool loadAudio(AudioClip& clip, const char* path, bool loop){
    FILE* f = std::fopen(path, "rb");
    if(!f){
        std::fprintf(stderr, "[audio] File not found: %s\n", path);
        return false;
    }
    std::fclose(f);
    
    ma_uint32 flags = loop ? MA_SOUND_FLAG_STREAM : 0;
    if(ma_sound_init_from_file(&audioEngine, path, flags, nullptr, nullptr, &clip.sound) == MA_SUCCESS){
        clip.loaded = true;
        ma_sound_set_looping(&clip.sound, loop ? MA_TRUE : MA_FALSE);
        return true;
    }
    return false;
}

static void playAudio(AudioClip& clip, bool restart = true){
    if(!audioReady || !clip.loaded) return;
    if(restart) ma_sound_seek_to_pcm_frame(&clip.sound, 0);
    ma_sound_start(&clip.sound);
}

static void playOnce(AudioClip& clip){
    if(clip.played) return;
    playAudio(clip);
    clip.played = true;
}

static void initAudioSystem(){
    if(ma_engine_init(nullptr, &audioEngine) != MA_SUCCESS){
        std::fprintf(stderr, "[audio] Failed to initialize audio engine\n");
        return;
    }
    audioReady = true;
    
    loadAudio(audioBgm, "assets/audio/bgd.wav", true);
    loadAudio(audioCollect, "assets/audio/coin.wav", false);
    loadAudio(audioWin, "assets/audio/win.wav", false);
    loadAudio(audioLose, "assets/audio/lose.wav", false);
    
    playAudio(audioBgm);
}

static void shutdownAudioSystem(){
    if(!audioReady) return;
    if(audioBgm.loaded) ma_sound_uninit(&audioBgm.sound);
    if(audioCollect.loaded) ma_sound_uninit(&audioCollect.sound);
    if(audioWin.loaded) ma_sound_uninit(&audioWin.sound);
    if(audioLose.loaded) ma_sound_uninit(&audioLose.sound);
    ma_engine_uninit(&audioEngine);
    audioReady = false;
}
#else
// No-audio stubs for single-file submission or when header is unavailable
struct AudioClip { bool loaded=false; bool played=false; };
static bool audioReady = false;
static AudioClip audioBgm, audioCollect, audioWin, audioLose;
static bool loadAudio(AudioClip&, const char*, bool){ return false; }
static void playAudio(AudioClip&, bool=true){}
static void playOnce(AudioClip&){}
static void initAudioSystem(){}
static void shutdownAudioSystem(){}
#endif

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

static void drawDiamond(const Vec3&center, float radius, float height, const float col[3]){
    glColor3f(col[0], col[1], col[2]);
    float halfH = height * 0.5f;
    glBegin(GL_TRIANGLES);
    for(int i=0;i<6;i++){
        float a0 = (float)i/6.0f * 2.0f * PI_F;
        float a1 = (float)(i+1)/6.0f * 2.0f * PI_F;
        float x0 = cosf(a0) * radius;
        float z0 = sinf(a0) * radius;
        float x1 = cosf(a1) * radius;
        float z1 = sinf(a1) * radius;
        // Top half
        glVertex3f(center.x, center.y + halfH, center.z);
        glVertex3f(center.x + x0, center.y, center.z + z0);
        glVertex3f(center.x + x1, center.y, center.z + z1);
        // Bottom half
        glVertex3f(center.x, center.y - halfH, center.z);
        glVertex3f(center.x + x1, center.y, center.z + z1);
        glVertex3f(center.x + x0, center.y, center.z + z0);
    }
    glEnd();
}

static void drawHaloRing(const Vec3&center, float innerR, float outerR, const float col[3], float alpha){
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=64;i++){
        float ang = (float)i/64.0f * 2.0f * PI_F;
        float c = cosf(ang);
        float s = sinf(ang);
        glColor4f(col[0], col[1], col[2], alpha);
        glVertex3f(center.x + c*outerR, center.y, center.z + s*outerR);
        glColor4f(col[0], col[1], col[2], 0.0f);
        glVertex3f(center.x + c*innerR, center.y, center.z + s*innerR);
    }
    glEnd();
    glDisable(GL_BLEND);
    glPopAttrib();
}

static void drawOrbPlane(const Vec3&center, const Vec3&axisU, const Vec3&axisV, float radius, const float col[3], float alpha){
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(col[0], col[1], col[2], alpha);
    glVertex3f(center.x, center.y, center.z);
    glColor4f(col[0], col[1], col[2], 0.0f);
    for(int i=0;i<=32;i++){
        float ang = (float)i/32.0f * 2.0f * PI_F;
        float c = cosf(ang);
        float s = sinf(ang);
        Vec3 offset = add(mul(axisU, radius*c), mul(axisV, radius*s));
        glVertex3f(center.x + offset.x, center.y + offset.y, center.z + offset.z);
    }
    glEnd();
}

static void drawGlowingOrb(const Vec3&center, float radius, const float col[3], float alpha){
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    drawOrbPlane(center, {1,0,0}, {0,1,0}, radius, col, alpha);
    drawOrbPlane(center, {0,1,0}, {0,0,1}, radius, col, alpha);
    drawOrbPlane(center, {1,0,0}, {0,0,1}, radius, col, alpha);
    glDisable(GL_BLEND);
    glPopAttrib();
}

static void drawTaikoDrum(float radius, float height, const float bodyCol[3], const float frameCol[3], const float ropeCol[3]){
    drawSolidBox({{0.0f, height*0.7f, 0.0f}, {radius, height*0.7f, radius}}, bodyCol[0], bodyCol[1], bodyCol[2]);
    drawSolidBox({{0.0f, height*1.35f, 0.0f}, {radius*0.95f, 0.15f, radius*0.95f}}, ropeCol[0], ropeCol[1], ropeCol[2]);
    drawSolidBox({{0.0f, height*0.05f, 0.0f}, {radius*0.95f, 0.15f, radius*0.95f}}, ropeCol[0]*0.9f, ropeCol[1]*0.9f, ropeCol[2]*0.9f);
    drawSolidBox({{ radius*0.95f, height*0.7f, 0.0f}, {0.15f, height*0.6f, radius*0.35f}}, ropeCol[0], ropeCol[1], ropeCol[2]);
    drawSolidBox({{-radius*0.95f, height*0.7f, 0.0f}, {0.15f, height*0.6f, radius*0.35f}}, ropeCol[0], ropeCol[1], ropeCol[2]);
    drawSolidBox({{0.0f, height*0.7f,  radius*0.95f}, {radius*0.35f, height*0.6f, 0.15f}}, ropeCol[0], ropeCol[1], ropeCol[2]);
    drawSolidBox({{0.0f, height*0.7f, -radius*0.95f}, {radius*0.35f, height*0.6f, 0.15f}}, ropeCol[0], ropeCol[1], ropeCol[2]);
    drawSolidBox({{-radius*1.25f, height*0.4f, 0.0f}, {0.25f, height*0.4f, 0.35f}}, frameCol[0], frameCol[1], frameCol[2]);
    drawSolidBox({{ radius*1.25f, height*0.4f, 0.0f}, {0.25f, height*0.4f, 0.35f}}, frameCol[0], frameCol[1], frameCol[2]);
    drawSolidBox({{0.0f, height*0.35f, 0.0f}, {radius*1.45f, 0.12f, radius*0.45f}}, frameCol[0]*0.9f, frameCol[1]*0.9f, frameCol[2]*0.9f);
    drawSolidBox({{0.0f, height*0.15f, 0.0f}, {radius*1.45f, 0.12f, radius*0.55f}}, frameCol[0]*0.75f, frameCol[1]*0.75f, frameCol[2]*0.75f);
}

static void drawStoneLantern(float scale, const float stoneCol[3], const float glowCol[3]){
    drawSolidBox({{0.0f, 0.15f*scale, 0.0f}, {0.7f*scale, 0.15f*scale, 0.7f*scale}}, stoneCol[0]*0.9f, stoneCol[1]*0.9f, stoneCol[2]*0.9f);
    drawSolidBox({{0.0f, 0.55f*scale, 0.0f}, {0.22f*scale, 0.4f*scale, 0.22f*scale}}, stoneCol[0], stoneCol[1], stoneCol[2]);
    drawSolidBox({{0.0f, 1.05f*scale, 0.0f}, {0.45f*scale, 0.2f*scale, 0.45f*scale}}, stoneCol[0]*1.05f, stoneCol[1]*1.05f, stoneCol[2]*1.05f);
    drawGlowingOrb({0.0f, 1.15f*scale, 0.0f}, 0.25f*scale, glowCol, 0.75f);
    drawPyramid({0.0f, 1.55f*scale, 0.0f}, 1.5f*scale, 0.5f*scale, stoneCol[0]*0.85f, stoneCol[1]*0.85f, stoneCol[2]*0.85f);
    drawSolidBox({{0.0f, 1.85f*scale, 0.0f}, {0.35f*scale, 0.08f*scale, 0.35f*scale}}, stoneCol[0]*1.1f, stoneCol[1]*1.1f, stoneCol[2]*1.1f);
}

static void drawLotusOracleModel(float radius, float height, const float col[3]){
    glPushMatrix();
    for(int i=0;i<6;i++){
        glPushMatrix();
        glRotatef(i * 60.0f, 0, 1, 0);
        glTranslatef(radius*0.6f, 0.0f, 0.0f);
        drawPyramid({0,0,0}, radius*0.8f, height, col[0], col[1], col[2]);
        glPopMatrix();
    }
    float coreCol[3]={std::min(1.0f,col[0]+0.2f), std::min(1.0f,col[1]+0.2f), std::min(1.0f,col[2]+0.2f)};
    drawDiamond({0,height*0.6f,0}, radius*0.4f, height*1.2f, coreCol);
    glPopMatrix();
}

static void drawCrystalColumn(float radius, float height, const float col[3]){
    float accent[3]={col[0]*0.8f+0.2f, col[1]*0.8f+0.2f, col[2]*0.8f+0.2f};
    drawDiamond({0,height*0.4f,0}, radius*0.5f, height*1.1f, col);
    drawDiamond({0,height*1.2f,0}, radius*0.35f, height*0.8f, accent);
    drawDiamond({0,height*0.0f,0}, radius*0.35f, height*0.8f, accent);
}

static void drawWindBell(float radius, float height, const float col[3]){
    float capCol[3]={col[0]*0.6f, col[1]*0.9f, col[2]*0.6f};
    drawDiamond({0,height*0.6f,0}, radius*0.5f, height, col);
    drawDiamond({0,height*1.2f,0}, radius*0.25f, height*0.5f, capCol);
    // Hanging chimes
    glColor3f(capCol[0], capCol[1], capCol[2]);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-0.2f, 0.0f, 0.0f); glVertex3f(-0.05f, -height*1.2f, 0.0f);
    glVertex3f(0.2f, 0.0f, 0.0f);  glVertex3f(0.05f, -height*1.2f, 0.0f);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(0.0f, 0.0f, -0.2f); glVertex3f(0.0f, -height*1.3f, -0.05f);
    glVertex3f(0.0f, 0.0f, 0.2f);  glVertex3f(0.0f, -height*1.3f, 0.05f);
    glEnd();
}

static void drawLanternOracle(float radius, float height, const float col[3]){
    float bodyCol[3]={col[0]*0.9f+0.1f, col[1]*0.9f+0.1f, col[2]*0.6f+0.4f};
    drawGlowingOrb({0,height*0.8f,0}, radius*0.8f, bodyCol, 0.8f);
    drawDiamond({0,height*0.8f,0}, radius*0.45f, height, col);
    drawDiamond({0,height*0.1f,0}, radius*0.3f, height*0.5f, bodyCol);
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

// Player model (ninja warrior): head with mask, torso (dark gi), legs (hakama pants), arms, katana sword, ninja hood
static void drawPlayer(){
    glPushMatrix();
    glTranslatef(playerPos.x, playerPos.y, playerPos.z);
    glRotatef(playerYawDeg, 0,1,0);

    // Torso - dark ninja gi/outfit
    drawSolidBox({{0, 1.0f, 0}, {0.6f, 0.8f, 0.35f}}, 0.1f, 0.1f, 0.15f);

    // Head - skin tone (face visible)
    drawSolidBox({{0, 2.0f, 0}, {0.35f, 0.35f, 0.35f}}, 0.85f, 0.75f, 0.65f);

    // Ninja mask/hood - dark cloth covering lower face and head
    drawSolidBox({{0, 1.85f, 0}, {0.38f, 0.25f, 0.36f}}, 0.08f, 0.08f, 0.12f);

    // Headband - red cloth band (traditional ninja/samurai)
    drawSolidBox({{0, 2.25f, 0}, {0.4f, 0.08f, 0.38f}}, 0.7f, 0.1f, 0.1f);

    // Legs - dark hakama pants (traditional Japanese)
    drawSolidBox({{-0.25f, 0.2f, 0}, {0.22f, 0.6f, 0.22f}}, 0.12f, 0.1f, 0.15f);
    drawSolidBox({{ 0.25f, 0.2f, 0}, {0.22f, 0.6f, 0.22f}}, 0.12f, 0.1f, 0.15f);

    // Arms - wrapped in dark cloth
    drawSolidBox({{-0.7f, 1.1f, 0}, {0.18f, 0.6f, 0.15f}}, 0.1f, 0.1f, 0.15f);
    drawSolidBox({{ 0.7f, 1.1f, 0}, {0.18f, 0.6f, 0.15f}}, 0.1f, 0.1f, 0.15f);

    // Hands - gloved/wrapped hands
    drawSolidBox({{-0.9f, 0.6f, 0}, {0.1f, 0.12f, 0.1f}}, 0.15f, 0.1f, 0.1f);
    drawSolidBox({{ 0.9f, 0.6f, 0}, {0.1f, 0.12f, 0.1f}}, 0.15f, 0.1f, 0.1f);

    // Katana sword - silver blade with dark handle held on back
    // Blade
    drawSolidBox({{-0.3f, 1.8f, -0.45f}, {0.05f, 0.8f, 0.08f}}, 0.7f, 0.75f, 0.8f);
    // Handle (tsuka)
    drawSolidBox({{-0.3f, 0.85f, -0.45f}, {0.08f, 0.25f, 0.1f}}, 0.15f, 0.1f, 0.08f);
    // Guard (tsuba)
    drawSolidBox({{-0.3f, 1.15f, -0.45f}, {0.15f, 0.02f, 0.15f}}, 0.6f, 0.5f, 0.2f);

    // Tabi boots - traditional split-toe footwear
    drawSolidBox({{-0.25f, -0.5f, 0.1f}, {0.2f, 0.1f, 0.28f}}, 0.95f, 0.95f, 0.95f);
    drawSolidBox({{ 0.25f, -0.5f, 0.1f}, {0.2f, 0.1f, 0.28f}}, 0.95f, 0.95f, 0.95f);

    glPopMatrix();
}

// Feature object draw variants
static void drawFeatureObj(const FeatureObj&f){
    glPushMatrix();
    glTranslatef(f.box.center.x, f.box.center.y, f.box.center.z);

    float r=f.baseColor[0], g=f.baseColor[1], b=f.baseColor[2];
    float glowPulse = f.animEnabled ? (0.5f + 0.5f*sinf(f.t*3.0f)) : 0.3f;

    switch(f.type){
        case ANIM_ROTATE: {
            float spin = f.animEnabled ? fmodf(f.t*90.0f, 360.0f) : 0.0f;
            glPushMatrix();
            glRotatef(spin, 0, 1, 0);
            float toriiCol[3]={r,g,b};
            drawTorii({0,0,0}, 1.6f, toriiCol);
            glPopMatrix();

            glPushMatrix();
            float rise = f.animEnabled ? (0.4f + 0.3f*sinf(f.t*2.2f)) : 0.2f;
            glTranslatef(0.0f, 4.8f + rise, 0.0f);
            drawGlowingOrb({0,0,0}, 0.7f + glowPulse*0.25f, toriiCol, 0.55f + glowPulse*0.35f);
            glPopMatrix();

            glPushMatrix();
            float petalSpin = f.animEnabled ? fmodf(f.t*140.0f, 360.0f) : 0.0f;
            glRotatef(petalSpin, 0, 1, 0);
            drawHaloRing({0, 3.0f, 0}, 1.0f, 3.5f, toriiCol, 0.25f + glowPulse*0.3f);
            glPopMatrix();

            drawHaloRing({0, 0.6f, 0}, 0.5f, 2.5f, toriiCol, 0.3f + glowPulse*0.3f);
        } break;
        case ANIM_SCALE: {
            float scalePulse = f.animEnabled ? (1.0f + 0.18f*sinf(f.t*1.8f)) : 1.0f;
            glPushMatrix();
            glScalef(scalePulse, 1.0f + 0.25f*sinf(f.t*2.1f), scalePulse);
            float pagodaCol[3]={r,g,b};
            drawPagoda({0,0,0}, 1.0f, pagodaCol);
            glPopMatrix();

            glPushMatrix();
            glRotatef(f.animEnabled ? fmodf(f.t*60.0f, 360.0f) : 0.0f, 0, 1, 0);
            drawHaloRing({0, 3.1f, 0}, 0.8f, 2.6f, pagodaCol, 0.35f + glowPulse*0.35f);
            glPopMatrix();
            drawGlowingOrb({0, 4.2f, 0}, 0.55f + glowPulse*0.2f, pagodaCol, 0.4f + glowPulse*0.4f);
        } break;
        case ANIM_TRANSLATE: {
            float bob = f.animEnabled ? 0.7f*sinf(f.t*1.6f) : 0.0f;
            glPushMatrix();
            glTranslatef(0, bob, 0);
            float bodyCol[3]={std::min(1.0f, r*1.1f), std::min(1.0f, g*0.6f + 0.2f), std::min(1.0f, b*0.5f + 0.15f)};
            float frameCol[3]={0.45f, 0.2f, 0.12f};
            float ropeCol[3]={0.95f, 0.9f, 0.8f};
            drawTaikoDrum(1.2f, 0.9f, bodyCol, frameCol, ropeCol);
            glPopMatrix();

            auto drawMallet = [&](float side){
                glPushMatrix();
                glTranslatef(side * 2.1f, 1.5f, 0.0f);
                float swing = f.animEnabled ? 20.0f*sinf(f.t*2.4f + side) : 4.0f;
                glRotatef(swing, 0, 0, 1);
                drawSolidBox({{0.0f, 0.45f, 0.0f}, {0.08f, 0.45f, 0.08f}}, 0.75f, 0.7f, 0.65f);
                drawSolidBox({{0.0f, 1.0f, 0.0f}, {0.28f, 0.18f, 0.28f}}, 0.3f, 0.3f, 0.3f);
                glPopMatrix();
            };
            drawMallet(-1.0f);
            drawMallet(1.0f);

            drawHaloRing({0, 0.2f, 0}, 0.5f, 1.9f, bodyCol, 0.3f + glowPulse*0.45f);
        } break;
        case ANIM_COLOR: {
            float colorShift = f.animEnabled ? (0.3f + 0.7f*(0.5f+0.5f*sinf(f.t*2.4f))) : 0.4f;
            float stoneCol[3]={0.65f + 0.2f*r, 0.6f + 0.2f*g, 0.55f + 0.2f*b};
            float glowCol[3]={0.9f, 0.8f + 0.15f*colorShift, 0.4f + 0.25f*colorShift};
            glPushMatrix();
            glScalef(1.0f, 1.0f + 0.15f*sinf(f.t*3.0f), 1.0f);
            drawStoneLantern(1.0f, stoneCol, glowCol);
            glPopMatrix();

            drawHaloRing({0, 0.4f, 0}, 0.4f, 2.0f, glowCol, 0.35f + glowPulse*0.5f);
        } break;
    }

    glPopMatrix();
}

// --------------------------- Scene setup ---------------------------
static void resetGame(){
    playerPos = {0.0f, 1.0f, 0.0f};
    playerDir = {0.0f, 0.0f, -1.0f};
    playerYawDeg = 0.0f;
    playerVelY = 0.0f;
    playerOnGround = true;
    camPos = {0.0f, 18.0f, 28.0f};
    camTarget = {0.0f, 0.0f, 0.0f};
    camUp = {0.0f, 1.0f, 0.0f};
    camMode = CAM_FOLLOW;
    gameTime = 120.0f;
    gameState = PLAYING;
    
    // Reset audio
    audioWin.played = false;
    audioLose.played = false;
    if(audioBgm.loaded) playAudio(audioBgm);

    // Ground
    groundBox = {{0.0f, 0.0f, 0.0f}, {WORLD_HALF, 0.2f, WORLD_HALF}};

    // Walls: make U-shape (3 walls)
    walls.clear();
    walls.push_back({{0.0f, 2.0f, -WORLD_HALF+1.0f}, {WORLD_HALF, 2.0f, 1.0f}}); // back
    walls.push_back({{-WORLD_HALF+1.0f, 2.0f, 0.0f}, {1.0f, 2.0f, WORLD_HALF}}); // left
    walls.push_back({{ WORLD_HALF-1.0f, 2.0f, 0.0f}, {1.0f, 2.0f, WORLD_HALF}}); // right

    // Platforms in four quadrants with different colors, sizes, AND heights for visual distinction
    platforms[0] = {{ {-20, 0.3f, -20}, {8, 0.3f, 6} }, {0.8f,0.2f,0.2f}}; // red - lowest
    platforms[1] = {{ { 20, 0.4f, -15}, {6, 0.4f, 8} }, {0.2f,0.6f,0.9f}}; // blue - medium-low
    platforms[2] = {{ {-18, 0.5f,  20}, {7, 0.5f, 7} }, {0.2f,0.8f,0.3f}}; // green - highest
    platforms[3] = {{ { 18, 0.35f, 18}, {9, 0.35f, 5} }, {0.9f,0.8f,0.2f}}; // yellow - medium

    // Setup obstacles for each platform to create platformer challenges
    obstacles.clear();

    // Platform 0 (Red/Torii): Small walls as barriers
    obstacles.push_back({{{-23.0f, 1.5f, -20.0f}, {0.5f, 1.2f, 2.0f}}, {0.6f, 0.15f, 0.15f}, false, 0, 0, {0,0,0}, 0});
    obstacles.push_back({{{-17.0f, 1.5f, -20.0f}, {0.5f, 1.2f, 2.0f}}, {0.6f, 0.15f, 0.15f}, false, 0, 0, {0,0,0}, 0});
    obstacles.push_back({{{-20.0f, 1.0f, -17.0f}, {3.0f, 0.7f, 0.5f}}, {0.6f, 0.15f, 0.15f}, false, 0, 0, {0,0,0}, 0});

    // Platform 1 (Blue/Pagoda): Multi-level elevated sections (stairs-like)
    obstacles.push_back({{{ 17.5f, 1.5f, -15.0f}, {2.0f, 1.2f, 2.5f}}, {0.15f, 0.4f, 0.7f}, false, 0, 0, {0,0,0}, 0});
    obstacles.push_back({{{ 21.0f, 2.5f, -15.0f}, {2.0f, 2.2f, 2.5f}}, {0.15f, 0.4f, 0.7f}, false, 0, 0, {0,0,0}, 0});
    obstacles.push_back({{{ 24.0f, 3.5f, -15.0f}, {2.0f, 3.2f, 2.5f}}, {0.15f, 0.4f, 0.7f}, false, 0, 0, {0,0,0}, 0});

    // Platform 2 (Green/Taiko): Moving horizontal obstacles
    Vec3 moveBase1 = {-18.0f, 1.5f, 18.0f};
    obstacles.push_back({{moveBase1, {1.5f, 1.2f, 0.5f}}, {0.15f, 0.6f, 0.2f}, true, 3.0f, 4.0f, moveBase1, 0});
    Vec3 moveBase2 = {-18.0f, 1.5f, 22.0f};
    obstacles.push_back({{moveBase2, {1.5f, 1.2f, 0.5f}}, {0.15f, 0.6f, 0.2f}, true, 2.5f, 3.5f, moveBase2, 1.5f});

    // Platform 3 (Yellow/Lantern): Mix - elevated sections and static barriers
    obstacles.push_back({{{ 15.0f, 2.0f, 18.0f}, {2.5f, 1.7f, 2.0f}}, {0.7f, 0.6f, 0.15f}, false, 0, 0, {0,0,0}, 0});
    obstacles.push_back({{{ 21.0f, 1.2f, 16.0f}, {1.0f, 0.9f, 1.0f}}, {0.7f, 0.6f, 0.15f}, false, 0, 0, {0,0,0}, 0});
    obstacles.push_back({{{ 18.0f, 1.0f, 21.0f}, {2.0f, 0.7f, 0.5f}}, {0.7f, 0.6f, 0.15f}, false, 0, 0, {0,0,0}, 0});

    // Feature objects centered on each platform
    // Red oracle - on ground
    features[0].box.center = {platforms[0].box.center.x, 0.0f, platforms[0].box.center.z};
    features[0].box.half = {1.6f,2.6f,1.0f};
    features[0].baseColor[0] = 0.8f; features[0].baseColor[1] = 0.15f; features[0].baseColor[2] = 0.15f;
    features[0].type = ANIM_ROTATE;
    features[0].allCollected = false;
    features[0].animEnabled = false;
    features[0].t = 0.0f;

    // Blue oracle - floating 3 units above ground
    features[1].box.center = {platforms[1].box.center.x, 3.0f, platforms[1].box.center.z};
    features[1].box.half = {1.6f,2.6f,1.6f};
    features[1].baseColor[0] = 0.7f; features[1].baseColor[1] = 0.4f; features[1].baseColor[2] = 0.9f;
    features[1].type = ANIM_SCALE;
    features[1].allCollected = false;
    features[1].animEnabled = false;
    features[1].t = 0.0f;

    // Green oracle - floating 2.5 units above ground
    features[2].box.center = {platforms[2].box.center.x, 2.5f, platforms[2].box.center.z};
    features[2].box.half = {1.6f,2.0f,1.6f};
    features[2].baseColor[0] = 0.9f; features[2].baseColor[1] = 0.3f; features[2].baseColor[2] = 0.3f;
    features[2].type = ANIM_TRANSLATE;
    features[2].allCollected = false;
    features[2].animEnabled = false;
    features[2].t = 0.0f;

    // Yellow oracle - floating 3.5 units above ground
    features[3].box.center = {platforms[3].box.center.x, 3.5f, platforms[3].box.center.z};
    features[3].box.half = {1.6f,2.2f,1.6f};
    features[3].baseColor[0] = 0.6f; features[3].baseColor[1] = 0.6f; features[3].baseColor[2] = 0.7f;
    features[3].type = ANIM_COLOR;
    features[3].allCollected = false;
    features[3].animEnabled = false;
    features[3].t = 0.0f;

    // Collectibles: 3 per platform arranged in small triangle pattern
    collectibles.clear();
    for(int i=0;i<4;i++){ collectedPerPlatform[i]=0; }

    auto platformSurfaceY = [&](int pi){
        const Platform& p = platforms[pi];
        return p.box.center.y + p.box.half.y;
    };

    auto addCollectible = [&](int pi, float offx, float offz, float heightAboveSurface, float r, float g, float b){
        const Platform& p = platforms[pi];
        Vec3 c = {p.box.center.x + offx, platformSurfaceY(pi) + heightAboveSurface, p.box.center.z + offz};
        Collectible col;
        col.box = { c, {0.18f, 0.35f, 0.18f} };
        col.color[0]=r; col.color[1]=g; col.color[2]=b;
        col.collected=false; col.platformIndex=pi;
        collectibles.push_back(col);
    };

    // Red platform - keep as is (accessible)
    addCollectible(0, -2.0f, -1.5f, 0.25f, 0.9f,0.3f,0.3f);
    addCollectible(0,  2.2f, -1.2f, 0.65f, 0.9f,0.5f,0.3f);
    addCollectible(0,  0.0f,  2.0f, 1.2f, 0.9f,0.3f,0.5f);

    // Blue platform - repositioned to avoid stair obstacles
    addCollectible(1, -4.0f, -5.0f, 0.35f, 0.3f,0.7f,0.9f); // Front left, clear of obstacles
    addCollectible(1,  4.0f, -5.0f, 1.0f, 0.3f,0.9f,0.7f); // Front right, higher ledge
    addCollectible(1,  0.0f,  5.0f, 1.6f, 0.5f,0.8f,0.9f); // Back center, atop stairs

    // Green platform - keep as is (accessible)
    addCollectible(2, -2.0f,  1.4f, 0.35f, 0.2f,0.9f,0.3f);
    addCollectible(2,  2.0f,  0.0f, 0.7f, 0.2f,0.7f,0.4f);
    addCollectible(2,  0.0f, -1.8f, 1.2f, 0.2f,0.9f,0.6f);

    // Yellow platform - repositioned to avoid large obstacles
    addCollectible(3, -7.0f,  0.0f, 0.4f, 0.9f,0.9f,0.3f); // Left edge, clear
    addCollectible(3,  6.0f,  0.0f, 1.1f, 0.9f,0.8f,0.2f); // Right edge, elevated
    addCollectible(3,  0.0f, -4.0f, 0.8f, 0.9f,0.7f,0.2f); // Front, mid-height

    // Reset feature gates
    for(int i=0;i<4;i++){
        features[i].allCollected=false;
        features[i].animEnabled=false;
        features[i].t=0.0f;
    }

    skyOracles.clear();
    for(int i=0; i<4; i++){
        const Platform& p = platforms[i];
        for(int j=0; j<2; j++){
            SkyOracle o;
            float offx = (j==0) ? -3.0f : 3.5f;
            float offz = (j==0) ? -2.0f : 1.5f;
            float height = 5.0f + j * 2.0f;
            o.pos = {p.box.center.x + offx, p.box.center.y + p.box.half.y + height, p.box.center.z + offz};
            o.radius = 1.5f + j * 0.5f;
            o.rotation = (float)(rand()%360);
            o.color[0] = features[i].baseColor[0];
            o.color[1] = features[i].baseColor[1];
            o.color[2] = features[i].baseColor[2];
            skyOracles.push_back(o);
        }
    }
}

// --------------------------- Collision ---------------------------
static bool collidesWithWorld(const AABB&box){
    // Against walls
    for(const auto&w : walls){ if(aabbIntersects(box,w)) return true; }
    // Against platforms - special handling to allow walking on top
    for(const auto&p : platforms){
        // Check if player is standing on top of platform (player's bottom is above or at platform's surface)
        float playerBottom = box.center.y - box.half.y;
        float platformTop = p.box.center.y + p.box.half.y;
        float platformBottom = p.box.center.y - p.box.half.y;

        // If player's bottom is above the platform surface (with small tolerance),
        // they're standing on top - don't block horizontal movement
        const float tolerance = 0.5f; // Allow some overlap for standing on top
        if(playerBottom >= platformTop - tolerance){
            continue; // Skip collision check, player is on top
        }

        // If player is significantly below platform top, check for collision
        // This handles side collisions and prevents clipping through platforms
        if(aabbIntersects(box, p.box)) return true;
    }
    // Against obstacles - same handling as platforms to allow standing on elevated obstacles
    for(const auto&o : obstacles){
        float playerBottom = box.center.y - box.half.y;
        float obstacleTop = o.box.center.y + o.box.half.y;

        const float tolerance = 0.5f;
        if(playerBottom >= obstacleTop - tolerance){
            continue; // Player is on top of obstacle
        }

        if(aabbIntersects(box, o.box)) return true;
    }
    // Against feature objects (platform oracles) - treat as solid; allow standing on top
    for(const auto& f : features){
        float playerBottom = box.center.y - box.half.y;
        float featureTop = f.box.center.y + f.box.half.y;
        const float tolerance = 0.5f;
        if(playerBottom >= featureTop - tolerance){
            continue; // Player is on top of the feature
        }
        if(aabbIntersects(box, f.box)) return true;
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

// Check if player is standing on ground or a platform
static bool isPlayerOnSurface(){
    AABB pb = { playerPos, playerHalf };
    // Check a small distance below player
    pb.center.y -= 0.1f;

    // Check against ground
    if(aabbIntersects(pb, groundBox)) return true;

    // Check against platforms
    for(const auto& p : platforms){
        if(aabbIntersects(pb, p.box)) return true;
    }

    // Check against obstacles (for elevated platforms)
    for(const auto& o : obstacles){
        if(aabbIntersects(pb, o.box)) return true;
    }

    return false;
}

// --------------------------- Game logic ---------------------------
static void updateCollectibles(){
    AABB pb = { playerPos, playerHalf };
    int completedCount=0;
    bool collectedSomething=false;
    for(auto &c : collectibles){
        if(!c.collected && aabbIntersects(pb, c.box)){
            c.collected = true;
            collectedPerPlatform[c.platformIndex]++;
            collectedSomething = true;
        }
    }
    // Check platform completions, auto-start animations
    for(int i=0;i<4;i++){
        if(!features[i].allCollected && collectedPerPlatform[i] >= totalCollectiblesPerPlatform){
            features[i].allCollected = true; // Animation unlocked
            features[i].animEnabled = true;  // Auto-start animation!
        }
        if(collectedPerPlatform[i] >= totalCollectiblesPerPlatform) completedCount++;
    }
    if(collectedSomething) playAudio(audioCollect);
    if(completedCount==4){
        if(gameState == PLAYING){
            gameState = WON;
            playOnce(audioWin);
        }
    }
}

static void updateFeatures(float dt){
    for(int i=0;i<4;i++){
        if(features[i].animEnabled) features[i].t += dt;
    }
}

static void updateSkyOracles(float dt){
    for(auto& o : skyOracles){
        o.rotation = fmodf(o.rotation + 30.0f * dt, 360.0f);
    }
}

// --------------------------- Game Over Scene ---------------------------
static void initFlyingOracles(){
    for(int i=0; i<4; i++){
        flyingOracles[i].pos = features[i].box.center;
        float vx = (rand()%200 - 100) / 20.0f;
        float vy = (rand()%100 + 50) / 20.0f;
        float vz = (rand()%200 - 100) / 20.0f;
        flyingOracles[i].vel = {vx, vy, vz};
        flyingOracles[i].rotation = 0.0f;
        flyingOracles[i].color[0] = features[i].baseColor[0];
        flyingOracles[i].color[1] = features[i].baseColor[1];
        flyingOracles[i].color[2] = features[i].baseColor[2];
    }
}

static void updateFlyingOracles(float dt){
    const float gravity = -9.8f;
    for(int i=0; i<4; i++){
        flyingOracles[i].pos.x += flyingOracles[i].vel.x * dt;
        flyingOracles[i].pos.y += flyingOracles[i].vel.y * dt;
        flyingOracles[i].pos.z += flyingOracles[i].vel.z * dt;
        flyingOracles[i].vel.y += gravity * dt;
        
        if(flyingOracles[i].pos.y < 0.0f){
            flyingOracles[i].pos.y = 0.0f;
            flyingOracles[i].vel.y = -flyingOracles[i].vel.y * 0.7f;
        }
        
        flyingOracles[i].rotation += 180.0f * dt;
        if(flyingOracles[i].rotation > 360.0f) flyingOracles[i].rotation -= 360.0f;
    }
}

static void updateObstacles(float dt){
    for(auto& obs : obstacles){
        if(obs.isMoving){
            obs.moveTime += dt;
            // Move horizontally back and forth
            float offset = sinf(obs.moveTime * obs.moveSpeed) * obs.moveRange;
            obs.box.center.x = obs.basePos.x + offset;
        }
    }
}

// --------------------------- Rendering ---------------------------
static void drawEastAsianBackground(){
    // Draw East Asian landscape in the background (mountains, temples, bamboo)

    // Mountain range in the far background - varying heights
    for(int i = -4; i <= 4; i++){
        float x = i * 18.0f;
        float height = 35.0f + (i % 3) * 12.0f;
        float width = 12.0f + (i % 2) * 5.0f;

        // Mountain body - dark gray/brown stone
        float mountainR = 0.25f + (i % 3) * 0.05f;
        float mountainG = 0.28f + (i % 2) * 0.04f;
        float mountainB = 0.22f + (i % 3) * 0.03f;

        // Draw mountain as tapered shape (wider at base)
        drawSolidBox({{x, height/3, -70.0f}, {width/2, height/3, 8.0f}},
                     mountainR, mountainG, mountainB);
        drawSolidBox({{x, height*0.7f, -70.0f}, {width/3, height*0.2f, 7.0f}},
                     mountainR + 0.1f, mountainG + 0.1f, mountainB + 0.1f);

        // Snow caps on peaks
        drawSolidBox({{x, height - 2.0f, -70.0f}, {width/4, 3.0f, 6.0f}},
                     0.9f, 0.92f, 0.95f);
    }

    // Traditional pagoda temples along the sides
    for(int i = 0; i < 2; i++){
        float z = -15.0f + i * 25.0f;

        // Left side temple
        drawSolidBox({{-65.0f, 8.0f, z}, {6.0f, 8.0f, 6.0f}},
                     0.35f, 0.25f, 0.2f); // Dark wood base
        drawPyramid({-65.0f, 16.0f, z}, 14.0f, 6.0f, 0.6f, 0.15f, 0.15f); // Red roof

        // Right side temple
        drawSolidBox({{65.0f, 10.0f, z + 10.0f}, {7.0f, 10.0f, 7.0f}},
                     0.4f, 0.3f, 0.25f);
        drawPyramid({65.0f, 20.0f, z + 10.0f}, 16.0f, 7.0f, 0.55f, 0.18f, 0.18f);
    }

    // Bamboo forest effect - tall thin boxes in clusters
    for(int cluster = 0; cluster < 5; cluster++){
        float baseX = -50.0f + cluster * 25.0f;
        for(int stalk = 0; stalk < 4; stalk++){
            float x = baseX + (stalk - 2) * 1.5f;
            float z = -55.0f + (stalk % 2) * 2.0f;
            float height = 18.0f + (stalk % 3) * 4.0f;
            // Bamboo stalks - green
            drawSolidBox({{x, height/2, z}, {0.3f, height/2, 0.3f}},
                       0.25f, 0.5f + (stalk % 2) * 0.1f, 0.25f);
        }
    }
}

static void drawGround(){
    // Traditional East Asian ground - earth/stone courtyard style
    drawSolidBox(groundBox, 0.35f, 0.32f, 0.28f); // Earthy brown/tan

    // Stone tile pattern - darker squares creating traditional courtyard look
    glColor3f(0.28f, 0.26f, 0.24f);
    glBegin(GL_QUADS);
    for(int i=-35; i<=35; i+=8){
        for(int j=-35; j<=35; j+=8){
            // Alternating pattern like traditional stone tiles
            if((i/8 + j/8) % 2 == 0){
                glVertex3f((float)i, 0.21f, (float)j);
                glVertex3f((float)i+7.5f, 0.21f, (float)j);
                glVertex3f((float)i+7.5f, 0.21f, (float)j+7.5f);
                glVertex3f((float)i, 0.21f, (float)j+7.5f);
            }
        }
    }
    glEnd();

    // Gravel/sand paths - lighter colored paths crossing the courtyard
    glColor3f(0.5f, 0.48f, 0.42f);
    glBegin(GL_QUADS);
    // Horizontal path
    glVertex3f(-40.0f, 0.22f, -2.0f);
    glVertex3f(40.0f, 0.22f, -2.0f);
    glVertex3f(40.0f, 0.22f, 2.0f);
    glVertex3f(-40.0f, 0.22f, 2.0f);
    // Vertical path
    glVertex3f(-2.0f, 0.22f, -40.0f);
    glVertex3f(2.0f, 0.22f, -40.0f);
    glVertex3f(2.0f, 0.22f, 40.0f);
    glVertex3f(-2.0f, 0.22f, 40.0f);
    glEnd();
}

static void drawWalls(){
    // Traditional East Asian walls - stone/wood fortress walls
    for(const auto&w : walls){
        // Main wall - gray stone
        drawSolidBox(w, 0.45f, 0.42f, 0.40f);

        // Wooden top rail - dark wood beam along top of wall
        glColor3f(0.25f, 0.18f, 0.12f);
        glBegin(GL_QUADS);
        float y = w.center.y + w.half.y + 0.15f;

        // Draw wooden beam on top
        if(std::abs(w.half.x - w.half.z) > 0.5f){ // Long wall (back/side walls)
            // Top beam
            glVertex3f(w.center.x - w.half.x, y, w.center.z - w.half.z - 0.3f);
            glVertex3f(w.center.x + w.half.x, y, w.center.z - w.half.z - 0.3f);
            glVertex3f(w.center.x + w.half.x, y, w.center.z + w.half.z + 0.3f);
            glVertex3f(w.center.x - w.half.x, y, w.center.z + w.half.z + 0.3f);
        }
        glEnd();

        // Stone texture - horizontal lines suggesting stacked stones
        glColor3f(0.35f, 0.33f, 0.32f);
        glBegin(GL_LINES);
        for(float h = w.center.y - w.half.y + 0.8f; h < w.center.y + w.half.y; h += 0.8f){
            glVertex3f(w.center.x - w.half.x, h, w.center.z - w.half.z);
            glVertex3f(w.center.x + w.half.x, h, w.center.z - w.half.z);
            glVertex3f(w.center.x - w.half.x, h, w.center.z + w.half.z);
            glVertex3f(w.center.x + w.half.x, h, w.center.z + w.half.z);
        }
        glEnd();
    }
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

static void drawSkyOracles(){
    static float time = 0.0f;
    time += 0.016f; // Approximate frame time
    
    for(const auto& o : skyOracles){
        float bob = sinf(time + o.rotation * 0.01f) * 0.6f;
        Vec3 center = {o.pos.x, o.pos.y + bob, o.pos.z};
        float pulse = 0.5f + 0.5f*sinf(time * 2.0f);
        
        drawGlowingOrb(center, o.radius * 0.5f * (0.8f + 0.2f*pulse), o.color, 0.6f + 0.4f*pulse);
        drawHaloRing({center.x, center.y - 0.2f, center.z}, o.radius * 0.4f, o.radius, o.color, 0.3f + 0.4f*pulse);

        glPushMatrix();
        glTranslatef(center.x, center.y, center.z);
        glRotatef(o.rotation, 0, 1, 0);
        glColor3f(o.color[0]*0.85f, o.color[1]*0.85f, o.color[2]*0.85f);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<48;i++){
            float ang = (float)i/48.0f * 2.0f * PI_F;
            glVertex3f(cosf(ang) * o.radius * 0.85f, 0.0f, sinf(ang) * o.radius * 0.85f);
        }
        glEnd();
        glPopMatrix();
    }
}

static void drawObstacles(){
    for(const auto& obs : obstacles){
        drawSolidBox(obs.box, obs.color[0], obs.color[1], obs.color[2]);
    }
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

    if(gameState == WON){ 
        glColor3f(0.2f,1.0f,0.3f); 
        drawText(winW/2-60, winH-60, "GAME WIN!"); 
    }

    if(gameState == LOST){ 
        glColor3f(1.0f,0.2f,0.2f); 
        drawText(winW/2-70, winH/2, "GAME OVER"); 
        drawText(winW/2-90, winH/2-20, "Press ESC to Restart"); 
    }

    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
}

static void drawGameOverScene(){
    // Dark background for Game Over scene
    glClearColor(0.1f, 0.05f, 0.15f, 1.0f); // Dark purple/black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Fixed camera for Game Over scene - looking at center from above and behind
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60.0, (double)winW/(double)winH, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    gluLookAt(0, 15, 25,   // eye position
              0, 5, 0,     // look at center above ground
              0, 1, 0);    // up vector

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);

    // Draw each flying oracle
    for(int i=0; i<4; i++){
        glPushMatrix();

        // Position oracle
        glTranslatef(flyingOracles[i].pos.x, flyingOracles[i].pos.y, flyingOracles[i].pos.z);

        // Apply simple Y-axis rotation
        glRotatef(flyingOracles[i].rotation, 0, 1, 0);

        float r = flyingOracles[i].color[0];
        float g = flyingOracles[i].color[1];
        float b = flyingOracles[i].color[2];

        // Draw the oracle based on its type (same as features)
        switch(i){
            case 0: {
                float col[3]={r,g,b};
                drawTorii({0,0,0}, 1.8f, col);
            } break;
            case 1: {
                float col[3]={r,g,b};
                drawPagoda({0,0,0}, 1.2f, col);
            } break;
            case 2: {
                float bodyCol[3]={std::min(1.0f, r*1.1f), std::min(1.0f, g*0.6f + 0.2f), std::min(1.0f, b*0.5f + 0.15f)};
                float frameCol[3]={0.45f, 0.2f, 0.12f};
                float ropeCol[3]={0.95f, 0.9f, 0.8f};
                drawTaikoDrum(1.1f, 0.9f, bodyCol, frameCol, ropeCol);
            } break;
            case 3: {
                float stoneCol[3]={0.65f + 0.2f*r, 0.6f + 0.2f*g, 0.55f + 0.2f*b};
                float glowCol[3]={0.9f, 0.8f, 0.45f};
                drawStoneLantern(1.0f, stoneCol, glowCol);
            } break;
        }

        glPopMatrix();
    }

    // Draw "GAME OVER" text overlay
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    auto drawText = [&](int x,int y,const char* s){
        glRasterPos2i(x,y);
        for(const char* p=s; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    };

    glColor3f(1.0f,0.2f,0.2f);
    drawText(winW/2-70, winH/2, "GAME OVER");
    drawText(winW/2-90, winH/2-20, "Press ESC to Restart");

    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
}

static void setCamera(){
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60.0, (double)winW/(double)winH, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    Vec3 eye=camPos, target=camTarget, up=camUp;

    if(camMode==CAM_FOLLOW){
        // Fixed-angle semi top-down follow camera (like isometric/Diablo style)
        // Camera follows player position but maintains fixed viewing angle
        float camHeight = 20.0f;  // height above player
        float camBackOffset = 15.0f; // fixed distance back from player

        // Fixed angle camera - always looking from the same direction
        // Position camera behind and above player at a fixed angle
        eye.x = playerPos.x + camBackOffset;
        eye.y = playerPos.y + camHeight;
        eye.z = playerPos.z + camBackOffset;

        // Always look at the player's position
        target.x = playerPos.x;
        target.y = playerPos.y;
        target.z = playerPos.z;

        up = {0, 1, 0};
    }
    else if(camMode==CAM_TOP){ eye = {0.0f, 80.0f, 0.01f}; target = {0,0,0}; up={0,0,-1}; }
    else if(camMode==CAM_SIDE){ eye = {55.0f, 15.0f, 0.01f}; target = {0,0,0}; up={0,1,0}; } // Moved from 100 to 55 to be between play area and temples
    else if(camMode==CAM_FRONT){ eye = {0.01f, 15.0f, 80.0f}; target = {0,0,0}; up={0,1,0}; }

    gluLookAt(eye.x,eye.y,eye.z, target.x,target.y,target.z, up.x,up.y,up.z);
}

static void display(){
    if(gameState == LOST){
        // Replace entire scene with Game Over scene showing flying oracles
        drawGameOverScene();
        glutSwapBuffers();
        return;
    }

    // Normal gameplay rendering
    // East Asian sky - misty mountain atmosphere
    glClearColor(0.65f, 0.7f, 0.75f, 1); // Soft blue-gray for misty sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setCamera();

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);

    // Draw East Asian environment
    drawEastAsianBackground();
    drawGround();
    drawWalls();
    drawPlatforms();
    drawObstacles();
    drawFeatures();
    drawSkyOracles();
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
    if(gameState == LOST) return; // no control on game over

    // Horizontal movement
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

    // Vertical movement (jumping and gravity)
    playerOnGround = isPlayerOnSurface();

    // Apply gravity
    if(!playerOnGround){
        playerVelY += GRAVITY * dt;
    } else {
        // On ground, reset vertical velocity
        if(playerVelY < 0.0f) playerVelY = 0.0f;
    }

    // Update vertical position
    float nextY = playerPos.y + playerVelY * dt;

    // Check if new position would collide
    AABB testBox = { playerPos, playerHalf };
    testBox.center.y = nextY;

    // Only update Y if no collision or moving down to ground
    if(!collidesWithWorld(testBox) || nextY < playerPos.y){
        playerPos.y = nextY;

        // Clamp to ground level (minimum Y position)
        if(playerPos.y < 1.0f){
            playerPos.y = 1.0f;
            playerVelY = 0.0f;
            playerOnGround = true;
        }
    } else {
        // Hit ceiling or obstacle
        if(playerVelY > 0.0f) playerVelY = 0.0f;
    }
}

static void idle(){
    int t = glutGet(GLUT_ELAPSED_TIME);
    if(prevTicks==0) prevTicks=t;
    float dt = (t - prevTicks) / 1000.0f;
    prevTicks = t;

    if(gameState == PLAYING){
        gameTime -= dt;
        if(gameTime<=0.0f){
            gameTime=0.0f;
            gameState = LOST;
            playOnce(audioLose);
            initFlyingOracles();
        }
    }

    if(gameState == LOST){
        // Update flying oracles animation
        updateFlyingOracles(dt);
    } else {
        // Normal game updates
        updateCameraFreeMove(dt);
        updatePlayerMovement(dt);
        updateCollectibles();
        updateFeatures(dt);
        updateObstacles(dt);
        updateSkyOracles(dt);
    }

    glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y){
    keyDown[key] = true;

    if(key=='1') camMode = CAM_FOLLOW;  // Semi top-down follow camera
    if(key=='2') camMode = CAM_TOP;     // Full top-down view
    if(key=='3') camMode = CAM_SIDE;    // Side view
    if(key=='4') camMode = CAM_FRONT;   // Front view
    if(key=='v' || key=='V'){
        // Cycle through camera modes
        if(camMode==CAM_FOLLOW) camMode=CAM_TOP;
        else if(camMode==CAM_TOP) camMode=CAM_SIDE;
        else if(camMode==CAM_SIDE) camMode=CAM_FRONT;
        else if(camMode==CAM_FRONT) camMode=CAM_FREE;
        else camMode=CAM_FOLLOW;
    }
    if(key==27) resetGame(); // ESC key to reset game

    // Jump with spacebar (allowed during PLAYING and after win)
    if((key==' ') && playerOnGround && (gameState == PLAYING || gameState == WON)){
        playerVelY = JUMP_VELOCITY;
        playerOnGround = false;
    }

    // Pause/unpause animations (animations auto-start when collectibles are collected)
    // Use first letter of platform color: R=Red, B=Blue, G=Green, Y=Yellow
    if((key=='r' || key=='R') && features[0].allCollected) features[0].animEnabled = !features[0].animEnabled;
    if((key=='b' || key=='B') && features[1].allCollected) features[1].animEnabled = !features[1].animEnabled;
    if((key=='g' || key=='G') && features[2].allCollected) features[2].animEnabled = !features[2].animEnabled;
    if((key=='y' || key=='Y') && features[3].allCollected) features[3].animEnabled = !features[3].animEnabled;
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
    glutCreateWindow("3D Platformer - Ancient East Asian Warriors");

    initGL();
    resetGame();
    initAudioSystem();
    atexit(shutdownAudioSystem);

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
