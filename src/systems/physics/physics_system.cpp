#include "physics_system.hpp"

#include <cstdio>
#include <iostream>
#include <world_system.hpp>

#include "audio_system.hpp"
#include "boss_factories.hpp"
#include "consumable_utils.hpp"
#include "debuff.hpp"
#include "enemy_util.hpp"
#include "map_util.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"

void PhysicsSystem::step(float elapsed_ms) {
  // Calculate 't value': time loop / loop duration
  float lerp = elapsed_ms / LOOP_DURATION;

  // Set player acceleration (If player is alive)
  if (!registry.deathTimers.has(player)) {
    setPlayerAcceleration();
  } else if (registry.motions.has(player)) {
    registry.motions.get(player).acceleration = {0.f, 0.f};
  }

  // If dash is on cooldown, we need to decrement the dash cooldown timer
  if (registry.players.get(player).dashCooldownTimer > 0) {
    Player& player_comp = registry.players.get(player);
    player_comp.dashCooldownTimer -= elapsed_ms;
    // change color intensity based on cooldown timer and original cooldown
    // duration
    if (player_comp.dashCooldownTimer <= 0) {
      registry.colors.get(player_comp.dashIndicator) = vec3(1.0f);
    } else {
      float time_proportion =
          1.0f - (player_comp.dashCooldownTimer / DASH_COOLDOWN_DURATION);
      registry.colors.get(player_comp.dashIndicator) = vec3(time_proportion);
    }
  }

  // Poof bubbles
  for (Entity entity : registry.bubbles.entities) {
    calculateVelocity(entity, lerp);
    if (registry.motions.has(entity) &&
        registry.motions.get(entity).velocity.y > 0) {
      registry.remove_all_components_of(entity);
    }
  }

  // Apply water friction
  applyWaterFriction(player);
  for (Entity entity : registry.masses.entities) {
    if (!registry.players.has(entity)) {
      registry.motions.get(entity).acceleration = {0.f, 0.f};
      applyWaterFriction(entity);
      calculateVelocity(entity, lerp);
    }
  }

  // Update player velocity with lerp if player not dashing
  if (!registry.players.get(player).dashing) {
    calculatePlayerVelocity(lerp);
  } else {
    // apply Dash or decrement the current dash timer (length of the dash)
    playerDash(elapsed_ms);
  }

  // Update Entity positions with lerp
  for (Entity entity : registry.motions.entities) {
    Motion&   motion   = registry.motions.get(entity);
    Position& position = registry.positions.get(entity);

    if (!debuff_entity_can_move(entity)) {
      motion.velocity = vec2(0.0f);
    }

    if (debuff_entity_knockedback(entity)) {
      KnockedBack& knockedback = registry.knockedback.get(entity);
      motion.velocity          = knockedback.knocked_velocity;
    }

    if (registry.enemyProjectiles.has(entity) &&
        registry.enemyProjectiles.get(entity).type == ENTITY_TYPE::SHOCKWAVE) {
      // shockwaves don't move, they just expand
      position.scale += vec2(SHOCKWAVE_GROW_RATE) * lerp;
    } else {
      position.position += motion.velocity * lerp;
    }

    if (registry.players.has(entity)) {
      Player&   player = registry.players.get(entity);
      Position& player_mesh_position =
          registry.positions.get(player.collisionMesh);
      player_mesh_position.position += motion.velocity * lerp;
    }

    if (registry.oxygen.has(entity) && entity != player) {
      // make sure health bars follow moving enemies
      updateEnemyHealthBarPos(entity);
    }

    if (registry.emoting.has(entity)) {
      updateEmotePos(entity);
    }
  }
}

void updateWepProjPos(vec2 mouse_pos) {
  Position& player_comp    = registry.positions.get(player);
  vec2      player_pos     = player_comp.position;
  vec2      pos_cursor_vec = mouse_pos - player_pos;
  vec2      arm_offset     = (player_comp.scale.x < 0)
                                 ? vec2(-ARM_OFFSET.x, ARM_OFFSET.y)
                                 : ARM_OFFSET;
  pos_cursor_vec -= arm_offset;
  float     angle      = atan2(pos_cursor_vec.y, pos_cursor_vec.x);
  Position& weapon_pos = registry.positions.get(player_weapon);
  Position& proj_pos   = registry.positions.get(player_projectile);
  weapon_pos.angle     = (player_comp.scale.x < 0) ? angle + M_PI : angle;

  float flipped = (player_comp.scale.x < 0) ? -1 : 1;
  switch (wep_type) {
    case (PROJECTILES::HARPOON):
      weapon_pos.position =
          calculate_pos_vec(GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::NET):
      weapon_pos.position =
          calculate_pos_vec(NET_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::CONCUSSIVE):
      weapon_pos.position =
          calculate_pos_vec(CONCUSSIVE_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::TORPEDO):
      weapon_pos.position =
          calculate_pos_vec(TORPEDO_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::SHRIMP):
      weapon_pos.position =
          calculate_pos_vec(SHRIMP_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
  }

  if (registry.playerProjectiles.get(player_projectile).is_loaded) {
    vec2 relative_pos = HARPOON_RELATIVE_POS_FROM_GUN;
    switch (wep_type) {
      case (PROJECTILES::NET):
        relative_pos = NET_RELATIVE_POS_FROM_GUN;
        break;
      case (PROJECTILES::CONCUSSIVE):
        relative_pos   = CONCUSSIVE_RELATIVE_POS_FROM_GUN;
        proj_pos.scale = proj_pos.originalScale;
        if (registry.positions.get(player).scale.x < 0) {
          proj_pos.scale.x = -proj_pos.scale.x;
        }
        break;
      case (PROJECTILES::TORPEDO):
        relative_pos = TORPEDO_RELATIVE_POS_FROM_GUN;
        break;
      case (PROJECTILES::SHRIMP):
        relative_pos = SHRIMP_RELATIVE_POS_FROM_GUN;
        break;
      case (PROJECTILES::PROJ_COUNT):
        break;
    }
    if (player_comp.scale.x < 0) {
      relative_pos.x *= -1;
      if (proj_pos.scale.x > 0) {
        proj_pos.scale.x *= -1;
      }
    } else if (proj_pos.scale.x < 0) {
      proj_pos.scale.x *= -1;
    }
    proj_pos.angle = weapon_pos.angle;
    proj_pos.position =
        calculate_pos_vec(relative_pos.x, weapon_pos.position, proj_pos.angle,
                          {0.f, relative_pos.y});
  }
}

void updatePlayerDirection(vec2 mouse_pos) {
  Position& player_pos = registry.positions.get(player);
  bool      mouse_right_face_left =
      player_pos.position.x < mouse_pos.x && player_pos.scale.x < 0;
  bool mouse_left_face_right =
      player_pos.position.x > mouse_pos.x && player_pos.scale.x > 0;
  if (mouse_right_face_left || mouse_left_face_right) {
    Player&   player_comp = registry.players.get(player);
    Position& player_mesh_pos =
        registry.positions.get(player_comp.collisionMesh);

    player_mesh_pos.scale.x *= -1;
    player_pos.scale.x *= -1;

    Position& weapon = registry.positions.get(player_weapon);
    weapon.scale.x *= -1;
    weapon.position.x *= -1;

    if (registry.playerProjectiles.get(player_projectile).is_loaded) {
      Position& projectile = registry.positions.get(player_projectile);
      projectile.scale.x *= -1;
      projectile.position.x *= -1;
    }
  }
}

void setFiredProjVelo() {
  PlayerProjectile& proj = registry.playerProjectiles.get(player_projectile);
  proj.is_loaded         = false;
  registry.positions.get(player).scale.x < 0 ? proj.is_flipped = true
                                                     : proj.is_flipped = false;
  float   angle       = registry.positions.get(player_projectile).angle;
  Motion& proj_motion = registry.motions.get(player_projectile);
  vec2&   proj_scale  = registry.positions.get(player_projectile).scale;
  vec2&   proj_original_scale =
      registry.positions.get(player_projectile).originalScale;
  float direction = registry.positions.get(player).scale.x /
                    abs(registry.positions.get(player).scale.x);
  switch (proj.type) {
    case PROJECTILES::SHRIMP:
      proj_motion.velocity = {SHRIMP_SPEED * cos(angle) * direction,
                              SHRIMP_SPEED * sin(angle) * direction};
      if (!registry.sounds.has(player_projectile)) {
        registry.sounds.insert(player_projectile,
                               Sound(SOUND_ASSET_ID::PROJECTILE_SHRIMP));
      }
      break;
    case PROJECTILES::CONCUSSIVE:
      proj_scale = vec2(5.f) * proj_original_scale;
      if (registry.positions.get(player).scale.x < 0) {
        proj_scale.x = -proj_scale.x;
      }
      if (!registry.sounds.has(player_projectile)) {
        registry.sounds.insert(player_projectile,
                               Sound(SOUND_ASSET_ID::PROJECTILE_CONCUSSIVE));
      }
      break;
    case PROJECTILES::TORPEDO:
      if (!registry.sounds.has(player_projectile)) {
        registry.sounds.insert(player_projectile,
                               Sound(SOUND_ASSET_ID::PROJECTILE_TORPEDO));
      }
      break;
    case PROJECTILES::NET:
      if (!registry.sounds.has(player_projectile)) {
        registry.sounds.insert(player_projectile,
                               Sound(SOUND_ASSET_ID::PROJECTILE_NET));
      }
      break;
    default:
      if (!registry.sounds.has(player_projectile)) {
        registry.sounds.insert(player_projectile,
                               Sound(SOUND_ASSET_ID::PLAYER_OXYGEN_BLAST));
      }
      break;
  }
  if (proj.type != PROJECTILES::SHRIMP) {
    proj_motion.velocity = {HARPOON_SPEED * cos(angle) * direction,
                            HARPOON_SPEED * sin(angle) * direction};
  }
}

void setPlayerAcceleration() {
  Motion& motion      = registry.motions.get(player);
  Player& keys        = registry.players.get(player);
  motion.acceleration = {0.f, 0.f};

  // If player is dashing, double acceleration
  float accel_inc = registry.players.get(player).gliding ? GLIDE_ACCELERATION
                                                         : PLAYER_ACCELERATION;

  if (keys.upHeld) {
    motion.acceleration.y -= accel_inc;
  }
  if (keys.downHeld) {
    motion.acceleration.y += accel_inc;
  }
  if (keys.leftHeld) {
    motion.acceleration.x -= accel_inc;
  }
  if (keys.rightHeld) {
    motion.acceleration.x += accel_inc;
  }
}

void calculatePlayerVelocity(float lerp) {
  Motion& motion = registry.motions.get(player);

  motion.velocity += motion.acceleration * lerp;

  if (abs(motion.acceleration.x) == WATER_FRICTION &&
      abs(motion.velocity.x) < abs(motion.acceleration.x * lerp) &&
      motion.velocity.x * motion.acceleration.x > 0) {
    motion.velocity.x = 0;
  }
  if (abs(motion.acceleration.y) == WATER_FRICTION &&
      abs(motion.velocity.y) < abs(motion.acceleration.y * lerp) &&
      motion.velocity.y * motion.acceleration.y > 0) {
    motion.velocity.y = 0;
  }

  // If player is gliding, double max speed
  float max_velocity =
      registry.players.get(player).gliding ? MAX_GLIDE_SPEED : MAX_PLAYER_SPEED;

  if (motion.velocity.x > max_velocity) {
    motion.velocity.x = max_velocity;
  } else if (motion.velocity.x < -max_velocity) {
    motion.velocity.x = -max_velocity;
  }
  if (motion.velocity.y > max_velocity) {
    motion.velocity.y = max_velocity;
  } else if (motion.velocity.y < -max_velocity) {
    motion.velocity.y = -max_velocity;
  }
}

void calculateVelocity(Entity entity, float lerp) {
  Motion& motion = registry.motions.get(entity);

  motion.velocity += motion.acceleration * lerp;

  if (abs(motion.velocity.x) < abs(motion.acceleration.x * lerp) &&
      motion.velocity.x * motion.acceleration.x > 0) {
    motion.velocity.x = 0;
  }
  if (abs(motion.velocity.y) < abs(motion.acceleration.y * lerp) &&
      motion.velocity.y * motion.acceleration.y > 0) {
    motion.velocity.y = 0;
  }
}

void playerDash(float elapsed_ms) {
  Motion& motion = registry.motions.get(player);
  Player& keys   = registry.players.get(player);

  if (keys.dashTimer > 0) {
    keys.dashTimer -= elapsed_ms;
    if (keys.dashTimer <= 0) {
      keys.dashing = false;
    }
  }

  if (keys.dashCooldownTimer > 0) {
    return;
  }

  bool activated_and_can_dash = keys.dashing && keys.dashCooldownTimer <= 0;
  if (!activated_and_can_dash) {
    return;
  }
  if (!registry.sounds.has(player)) {
    registry.sounds.insert(player, Sound(SOUND_ASSET_ID::PLAYER_OXYGEN_BLAST));
  }
  keys.dashCooldownTimer = DASH_COOLDOWN_DURATION;
  keys.dashTimer         = DASH_DURATION;

  if (keys.upHeld) {
    motion.velocity.y = -1 * DASH_SPEED;
  }
  if (keys.downHeld) {
    motion.velocity.y = DASH_SPEED;
  }
  if (keys.leftHeld) {
    motion.velocity.x = -1 * DASH_SPEED;
  }
  if (keys.rightHeld) {
    motion.velocity.x = DASH_SPEED;
  }
}

void applyWaterFriction(Entity entity) {
  Motion& motion = registry.motions.get(entity);

  float water_friction = WATER_FRICTION;
  // Keep this here just in case, but acceleration by friction is NOT
  // proportional to mass, which is why we can use a constant
  /*float(registry.masses.get(entity).mass) / float(PLAYER_MASS) *
  WATER_FRICTION;*/

  // Water friction should accelerate in the opposite direction of the player's
  // velocity. If the player isn't moving, friction has no effect
  if (motion.velocity.x) {
    motion.velocity.x > 0 ? motion.acceleration.x -= water_friction
                          : motion.acceleration.x += water_friction;
  }
  if (motion.velocity.y) {
    motion.velocity.y > 0 ? motion.acceleration.y -= water_friction
                          : motion.acceleration.y += water_friction;
  }
}

Entity createGeyserBubble(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();
  // physics and pos
  Position& pos = registry.positions.emplace(entity);
  pos.angle     = 0.f;
  pos.position  = position;
  pos.scale     = BUBBLE_SCALE_FACTOR * BUBBLE_BOUNDING_BOX;

  Motion& motion      = registry.motions.emplace(entity);
  motion.acceleration = {0.f, WATER_FRICTION};
  motion.velocity     = INITIAL_BUBBLE_VELOCITY;

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  registry.bubbles.emplace(entity);

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::GEYSER_BUBBLE, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}
