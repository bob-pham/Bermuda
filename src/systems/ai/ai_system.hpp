#pragma once

#include <vector>

#include "common.hpp"
#include "physics.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "random.hpp"

class AISystem {
  private:
  RenderSystem * renderer;
  void do_boss_ai(float elapsed_ms);
  void do_wander_ai(float elapsed_ms);
  void do_wander_ai_line(float elapsed_ms);
  void do_wander_ai_square(float elapsed_ms);
  void do_track_player(float elapsed_ms);
  void do_track_player_ranged(float elapsed_ms);
  bool in_range_of_player(Position &pos, Position &player_pos, float range);

  float sharkman_texture_num = 0.f;
  public:
  void step(float elapsed_ms);
  void init(RenderSystem* renderer_arg);
};

bool is_tracking(Entity e);
void choose_new_direction(Entity enemy, Entity other);
bool can_see_player(Position &pos, Position &player_pos);
void removeFromAI(Entity& e);
void addSharkmanWander();
