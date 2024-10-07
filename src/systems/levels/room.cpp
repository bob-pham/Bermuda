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

/*
void place_enemy_in_room_at_random_position(Entity enemy) {
    // If we don't want a room to know about its enemies, we can just have the enemy creation module call get get_random_position(),
    // and edit the registry however it wants, but if we want the rooms to know about its enemies, we probably want to go through a specialized function.
    registry.inside.insert({enemy, entity});
}
*/

bool RoomBuilder::is_in_room(vec2& position) {
    // All well-formed rooms are rectilinear polygons. Therefore, if a point is inside a room, it must be bounded in all four
    // directions by at least one wall.
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    
    for (Entity& entity : registry.spaces.get(entity).boundaries) {
        Vector& vector = registry.vectors.get(entity);
        if ((!down or !up) and vector.start[1] == vector.end[1]) {
            if (position[0] > std::min(vector.start[0], vector.end[0]) and position[0] < std::max(vector.start[0], vector.end[0])) {
                if (position[1] > vector.start[1]) {
                    down = true;
                } else {
                    up = true;
                }
            }
        }

        if ((!left or !right) and vector.start[0] == vector.end[0]) {
            if (position.y > std::min(vector.start[1], vector.end[1]) and position.y < std::max(vector.start[1], vector.end[1])) {
                if (position[0] > vector.start[0]) {
                    left = true;
                } else {
                    right = true;
                }
            }
        }
    }

    return left && right && up && down;
};

/*
Returns a random vec2 inside a well-formed room's exact dimensions. 
*/
vec2 RoomBuilder::rejection_sample() {
    SpaceBoundingBox& box = registry.bounding_boxes.get(entity);
    std::random_device rd; 
    std::mt19937 generate(rd()); 
    // We generate random coordinates inside the bounding box. Theoretically, this could be slow if the bounding box's area is far larger than a room;
    // think of a case like an 'L'-shaped room. Since randomized levels are only generated once, and we can always code level generation
    // so that our rooms are 'decently' rectangular however, it should suffice.
    std::uniform_int_distribution<> random_x_generator(box.minimum_x, box.maximum_x); 
    std::uniform_int_distribution<> random_y_generator(box.minimum_y, box.maximum_y);
    while (true) {
        vec2 position = {random_x_generator(generate), random_y_generator(generate)};
        if (is_in_room(position)) {
            return position;
        }
    };
};

vec2 RoomBuilder::get_random_position() {
    return rejection_sample();
}