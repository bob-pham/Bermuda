#include "consumable_factories.hpp"

#include <iostream>

#include "collision_util.hpp"
#include "items.hpp"

/////////////////////////////////////////////////////////////////
// Util
/////////////////////////////////////////////////////////////////
/**
 * @brief Checks whether or not the spawn is valid or invalid based on spawn
 * collisons The entity should already have the position attached
 *
 * @param entity - enemy to check
 * @return true if valid, false otherwise
 */
static bool checkSpawnCollisions(Entity entity) {
  if (!registry.positions.has(entity)) {
    return false;
  }
  const Position& enemyPos = registry.positions.get(entity);

  // Entities can't spawn in the player
  for (Entity player : registry.players.entities) {
    if (!registry.positions.has(player)) {
      continue;
    }
    const Position player_pos = registry.positions.get(player);
    if (box_collides(enemyPos, player_pos)) {
      return false;
    }
  }

  // Entities can't spawn in walls
  for (Entity wall : registry.activeWalls.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(enemyPos, wallPos)) {
      return false;
    }
  }

  // Entities can't spawn in breakables
  for (Entity wall : registry.breakables.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(enemyPos, wallPos)) {
      return false;
    }
  }

  // Entities can't spawn in interactables
  for (Entity interactable : registry.interactable.entities) {
    if (!registry.positions.has(interactable)) {
      continue;
    }
    const Position interactablePos = registry.positions.get(interactable);
    if (box_collides(enemyPos, interactablePos)) {
      return false;
    }
  }

  return true;
}

/////////////////////////////////////////////////////////////////
// Oxygen Canister
/////////////////////////////////////////////////////////////////
/**
 * @brief creates an oxygen canister at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createOxygenCanisterPos(RenderSystem* renderer, vec2 position, bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = OXYGEN_CANISTER_SCALE_FACTOR * OXYGEN_CANISTER_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Consumable &c = registry.consumables.emplace(entity);
  c.respawnFn = respawnOxygenCanister;

  // Add stats
  auto& refill  = registry.oxygenModifiers.emplace(entity);
  refill.amount = OXYGEN_CANISTER_QTY;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::OXYGEN_CANISTER, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief Respawns a Geyser based on it's entity state
 *
 * @param renderer 
 * @param es 
 * @return 
 */
Entity respawnOxygenCanister(RenderSystem *renderer, EntityState es) {
  Entity entity = createOxygenCanisterPos(renderer, es.position.position, false);
  return entity;
}


