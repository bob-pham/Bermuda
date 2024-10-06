#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include <functional>

//////////////////////////////////////////////////////////////
// JellyFish
//////////////////////////////////////////////////////////////
#define JELLY_OXYGEN 10.0
#define JELLY_DAMAGE 5.0
#define JELLY_STUN_MS 2000.0
#define JELLY_BB_WIDTH  0.6f * 165.f
#define JELLY_BB_HEIGHT 0.6f * 165.f

Entity createJellyRoom(RenderSystem *renderer, vec2 (*randPos)(void));
Entity createJellyPos(RenderSystem *renderer, vec2 position);

//////////////////////////////////////////////////////////////
// Fish
//////////////////////////////////////////////////////////////
#define FISH_MS 50.0
#define FISH_OXYGEN 1.0 // one shot
#define FISH_DAMAGE 5.0
#define FISH_BB_WIDTH  0.6f * 165.f
#define FISH_BB_HEIGHT 0.6f * 165.f

Entity createFishRoom(RenderSystem *renderer, vec2 (*randPos)(void));
Entity createFishPos(RenderSystem *renderer, vec2 position);

//////////////////////////////////////////////////////////////
// Sharks
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Octopi
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Krabs
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Sea mine
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Merpeople
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Void Tentacles
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Serpents
//////////////////////////////////////////////////////////////
