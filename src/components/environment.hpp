#pragma once

#include "level_util.hpp"
#include "player.hpp"
#include "common.hpp"
#include "respawn.hpp"
#include <limits>

/*
  "To instantiate a global variable; this path poisons the mind and corrupts the code.
   To disguise it as a Component; this path is that to harmony."
   -Confucius"
*/

// An enumeration for programatically defining directions.
// Not strongly-typed (enum class) for now.
enum Direction {
  NORTH = 0,
  EAST = NORTH + 1,
  SOUTH = EAST + 1,
  WEST = SOUTH + 1
};

struct SpaceBoundingBox {
    float minimum_x = std::numeric_limits<float>::max();
    float maximum_x = std::numeric_limits<float>::min();
    float minimum_y = std::numeric_limits<float>::max();
    float maximum_y = std::numeric_limits<float>::min();
};

// TODO:
// This is only used internally for SpaceBuilder, so probably just remove it from the ECS entirely.
// Also the name sucks.
struct Vector {
  vec2 start;
  vec2 end;

  Vector(vec2 start, vec2 end) : start(start), end(end) {};
};

struct Space {
  std::vector<Entity> boundaries;
  std::vector<Entity> walls;
  std::vector<Entity> doors;
};

struct DoorConnection {
  std::string room_id;
  Entity exit_door;
  Direction direction;

  // Gameplay related data.
  Objective objective = Objective::NONE;
  bool assigned = false;
  bool locked = false;
};

struct RoomTransition {
  DoorConnection door_connection;
};

struct ActiveWall {};

struct ActiveDoor {};

struct Interactable {
  ENTITY_TYPE type;
};

struct PressurePlate {
  bool active;
  float mass_activation = 20;
};

struct Floor {};

struct Geyser {
  float bubble_timer;
};

struct Explosion {
  float timer;
  float expiry_time;
  vec2  full_scale;
};

struct Bubble {};

struct Breakable {
  ENTITY_TYPE type;
};

struct Ambient {
  ENTITY_TYPE type;
};
