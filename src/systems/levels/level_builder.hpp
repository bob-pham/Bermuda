#pragma once

#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <set>

#include "room_builder.hpp"
#include "respawn.hpp"

/**
 * The Following Define Standardized Units For Room Building
 *
 * NOTE: Rooms will have a standard 20 x 10 overall shape
 */

#define MAX_X_UNITS 20
#define MAX_Y_UNITS 10

#define X_1U (window_width_px) / 22.f // One Room Building Unit in the X Direction
#define X_2U 2.f * X_1U
#define X_3U 3.f * X_1U
#define X_4U 4.f * X_1U
#define X_5U 5.f * X_1U
#define X_6U 6.f * X_1U
#define X_7U 7.f * X_1U
#define X_8U 8.f * X_1U
#define X_9U 9.f * X_1U
#define X_10U 10.f * X_1U
#define X_11U 11.f * X_1U
#define X_12U 12.f * X_1U
#define X_13U 13.f * X_1U
#define X_14U 14.f * X_1U
#define X_15U 15.f * X_1U
#define X_16U 16.f * X_1U
#define X_17U 17.f * X_1U
#define X_18U 18.f * X_1U
#define X_19U 19.f * X_1U
#define X_20U 20.f * X_1U

#define Y_1U (window_height_px) / 12.f // One Room Building Unit in the Y Direction
#define Y_2U 2.f * Y_1U
#define Y_3U 3.f * Y_1U
#define Y_4U 4.f * Y_1U
#define Y_5U 5.f * Y_1U
#define Y_6U 6.f * Y_1U
#define Y_7U 7.f * Y_1U
#define Y_8U 8.f * Y_1U
#define Y_9U 9.f * Y_1U
#define Y_10U 10.f * Y_1U

Direction get_opposite_direction(Direction direction);

/**
 * Deals only with the building of levels.
 */
class LevelBuilder
{
private:
    std::unordered_map<std::string, RoomBuilder> rooms;

    void connect_doors(std::unordered_map<int, std::unordered_map<int, Direction>>& adjacency_list);

    void mark_difficulty_regions();
    void mark_tutorial_room();
    void mark_miniboss_rooms();
    void mark_final_boss_room();

    void build_wall_with_random_doors(Direction direction,
                                std::unordered_map<int, Direction> directed_adjacencies,
                                int max_units,
                                int unit_size,
                                std::function<void(int)> draw_segment,
                                std::function<void(std::string, int)> draw_door);
    void randomize_room_shapes(std::unordered_map<int, std::unordered_map<int, Direction>>& adjacency_list);
public:
    /**
     * Returns the RoomBuilder for a room, making a new one if one under the given key does not exist.
     * @param s_id: the room's key.
     */
    RoomBuilder &get_room_by_editor_id(std::string s_id);

    /**
     * Connects two doors together.
     * @param direction: the wall that the first door is on.
     * @param r1_id: the first room's key.
     * @param d1_id: the door in the first room's key.
     * @param r2_id: the second room's key.
     * @param d1_id: the door in the second room's key.
     */
    LevelBuilder &connect(Direction direction, std::string r1_id, std::string d1_id, std::string r2_id, std::string d2_id);

    /**
     * Generates a randomized level.
     * @param rooms: a vector of the number of rooms for each difficulty area, i.e {1,5,5,5} = 16 rooms, each grouped by ascending difficulty class.
     */
    void generate_random_level();

    void mark_all_rooms_unvisited();
};