#include <stdio.h>
#include <random>
#include <chrono>
#include <thread>

#include "room.hpp"

#include "tiny_ecs_registry.hpp"

Room::Room(): Entity() {};

RoomBuilder::RoomBuilder(): SpaceBuilder<Room>() {
    new_entity(Room());
};