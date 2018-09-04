#pragma once

#include "data_path.hpp"

#include "GL.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <deque>
#include <tuple>



// This header provide codes that assists in reading in
// levels from files
namespace TiltEscape
{
    class Entity
    {
    public:
        glm::vec2 position;

        Entity()
        {
            position = glm::vec2(0,0);
        }

        Entity(glm::vec2 _position)
        {
            position = _position;
        }
    };

    class Wall : public Entity
    {
    public:
        glm::vec2 size;

        Wall(glm::vec2 _position, glm::vec2 _size)
        : Entity(_position)
        {
            size = _size;
        }
    };

    class Player : public Entity
    {
    public:
        float radius;
        glm::vec2 velocity;

        Player()
        : Entity()
        {
            radius = 0.5f;
            velocity = glm::vec2(0,0);
        }

        Player(glm::vec2 _position, float _radius)
        : Entity(_position)
        {
            radius = _radius;
            velocity = glm::vec2(0,0);
        }
    };

    enum class LookDirection {
        UP, UP_LEFT, LEFT, DOWN_LEFT, DOWN, DOWN_RIGHT, RIGHT, UP_RIGHT
    };

    class GuardVision {
    public:
        float radius;
        float distance;
        LookDirection look_direction;

        GuardVision()
        {
            radius = 0.5f;
            distance = 1.0f;
            look_direction = LookDirection::DOWN_RIGHT;
        }

        GuardVision(float _radius, float _distance, LookDirection _look_direction)
        {
            radius = _radius;
            distance = _distance;
            look_direction = _look_direction;
        }
    };

    class Guard : public Entity
    {
    public:
        int guard_id;
        float radius;
        GuardVision fov;
        float time_looking_in_direction;
        float look_thresh;
        std::mt19937 mt;
        glm::vec2 current_waypoint;
        glm::vec2 next_waypoint;
        float wait_thresh;
        float time_at_waypoint;
        glm::vec2 velocity;
        std::deque<glm::vec2> waypoints;

        Guard(glm::vec2 _position, float _radius, GuardVision _fov)
        : Entity(_position)
        {
            radius = _radius;
            fov = _fov;
            time_looking_in_direction = 0;
            look_thresh = 2.0f * ((double)std::rand() / RAND_MAX);
            mt = std::mt19937(position.x * position.y);
            current_waypoint = position;
            velocity = glm::vec2(0,0);
            wait_thresh = 2.0f * ((double)std::rand() / RAND_MAX);
            guard_id = -1;
        }

        void update(float elapsed)
        {
            // Always update the velocity
            position += velocity * elapsed;
            
            // Sitting still at a waypoint
            if (at_current_waypoint())
            {
                // Update the amount of time we have been at this waypoint
                time_at_waypoint += elapsed;
                if (time_at_waypoint >= wait_thresh)
                {
                    // Get the next waypoint
                    next_waypoint = waypoints.front();
                    waypoints.pop_front();
                    waypoints.push_back(next_waypoint);

                    //start moving to next waypoint
                    velocity = next_waypoint - current_waypoint;
                    velocity /= velocity.length();
                }
            }

            if (!at_current_waypoint() && !at_next_waypoint())
            {
                velocity = next_waypoint - position;
            }

            // Arriving at the next waypoitn
            if (at_next_waypoint() &&  !at_current_waypoint())
            {
                // Stop Moving
                velocity = glm::vec2(0,0);
                // Restart the timer
                time_at_waypoint = 0;
                
                current_waypoint = next_waypoint;
                
            }

            // Look in different directions randomly 
            time_looking_in_direction += elapsed;
            if (time_looking_in_direction >= look_thresh)
            {
                change_look_dir();
            }

            
        }

        bool at_next_waypoint()
        {
            return std::roundf(position.x) == next_waypoint.x && std::roundf(position.y) == next_waypoint.y;
        }


        bool at_current_waypoint()
        {
            return std::roundf(position.x) == current_waypoint.x && std::roundf(position.y) == current_waypoint.y;
        }

        void change_look_dir()
        {
            time_looking_in_direction = 0;
            int next_dir = (mt() % ((int)LookDirection::UP_RIGHT + 1));
            fov.look_direction = static_cast<LookDirection>(next_dir);
            look_thresh = 2.0f * ((double)std::rand() / RAND_MAX);
        }

        glm::vec2 get_fov_offset()
        {
            glm::vec2 offset(0,0);

            switch(fov.look_direction) {
                case TiltEscape::LookDirection::UP:
                    offset.y = 1;
                    break;
                case TiltEscape::LookDirection::UP_LEFT:
                    offset.x = -1;
                    offset.y = 1;
                    break;
                case TiltEscape::LookDirection::LEFT:
                    offset.x = -1;
                    break;
                case TiltEscape::LookDirection::DOWN_LEFT:
                    offset.x = -1;
                    offset.y = -1;
                    break;
                case TiltEscape::LookDirection::DOWN:
                    offset.y = -1;
                    break;
                case TiltEscape::LookDirection::DOWN_RIGHT:
                    offset.y = -1;
                    offset.x = 1;
                    break;
                case TiltEscape::LookDirection::RIGHT:
                    offset.x = 1;
                    break;
                case TiltEscape::LookDirection::UP_RIGHT:
                    offset.x = 1;
                    offset.y =1;
                    break;
            }

            return offset;
        }


    };

    enum class Direction {
            UP, RIGHT, DOWN, LEFT
    };

    typedef std::tuple<GLboolean, Direction, glm::vec2> Collision;


    static Direction vector_direction(glm::vec2 target)
    {
        glm::vec2 compass[] = {
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, -1.0f),
            glm::vec2(-1.0f, 0.0f)
        };

        GLfloat max = 0.0f;
        GLuint best_match = -1;
        for (GLuint i = 0; i < 4; i++)
        {
            GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
            if (dot_product > max)
            {
                max = dot_product;
                best_match =i;
            }
        }

        return (Direction) best_match;
    }

    struct Level  {

        // Stores the string representation
        std::vector<std::vector<char>> level_matrix;
        // All the walls in the level
        std::vector<Wall> walls;
        // Reference to the player
        Player player;
        // Reference to all the guards in the level
        std::vector<Guard> guards;
        std::vector<glm::vec2> holes;

        Level()
        {

        }

        // Returns 2D array of char from file
        void load_level(std::string filename)
        {
            const glm::vec2 WALL_SIZE = glm::vec2(1,1);

            std::ifstream mapfile(data_path(filename));
            if (!mapfile.is_open())
            {
                std::cerr << "failed to open " << filename << '\n';
            }
            else
            {
                
                std::string line;
                
                while (std::getline(mapfile, line))
                {
                    std::vector<char> row;
                    for (uint32_t i = 0; i < line.length(); i++)
                    {
                        if (line[i] == '#')
                        {
                            // Create a new wall
                            walls.push_back(Wall(glm::vec2(i, level_matrix.size()), WALL_SIZE));
                        }
                        if (line[i] == 'P')
                        {
                            // Creates a new Player
                            player = Player(glm::vec2(i, level_matrix.size()), 0.5f);
                        }
                        if (line[i] == 'H')
                        {
                            // Create a new hole in he board
                            holes.push_back(glm::vec2(i, level_matrix.size()));
                        }
                        if (std::isdigit(line[i]))
                        {
                            // I use numbers to denote the guards
                            int guard_id = line[i] - '0';
                            int guard_index = get_guard_index(guard_id);
                            if (guard_index >= 0)
                            {
                                // Add this to the guards queue of waypoints
                                guards[guard_index].waypoints.push_back(glm::vec2(i, level_matrix.size()));
                            }
                            else
                            {
                                // Create a new guard and add this as a waypoint
                                Guard new_guard(glm::vec2(i, level_matrix.size()), 0.5f, GuardVision());
                                new_guard.guard_id = guard_id;
                                new_guard.current_waypoint = glm::vec2(i, level_matrix.size());
                                new_guard.next_waypoint = glm::vec2(i, level_matrix.size());
                                new_guard.waypoints.push_back(glm::vec2(i, level_matrix.size()));
                                guards.push_back(new_guard);
                            }
                        }

                        row.push_back(line[i]);
                        
                    }
                    level_matrix.push_back(row);

                }
                //std::reverse(std::begin(level_matrix), std::end(level_matrix));
            }
        }

        bool has_guard(int guard_id)
        {
            for(Uint32 i = 0; i < guards.size(); i++)
            {
                if (guards[i].guard_id == guard_id)
                {
                    return true;
                }
            }
            return false;
        }

        int get_guard_index(int guard_id) {
            for(Uint32 i = 0; i < guards.size(); i++)
            {
                if (guards[i].guard_id == guard_id)
                {
                    return i;
                }
            }
            return -1;
        }


        void clear_level()
        {
            level_matrix.clear();
            walls.clear();
            guards.clear();
            holes.clear();
        }


        int get_length()
        {
            if (!level_matrix.empty()) {
                return level_matrix[0].size();
            }
            return 0;
        }

        int get_height()
        {
            return level_matrix.size();
        }

        void print()
        {
            for (Uint32 i = 0; i < level_matrix.size(); i++)
            {
                for (Uint32 j = 0; j < level_matrix[i].size(); j++)
                {
                    std::cout << level_matrix[i][j];
                }
                std::cout << std::endl;
            }
        }

        char at(int row, int col) {
            if (row >= 0 && col >= 0)
            {
                if ((Uint32)row < level_matrix.size() && 
                    (Uint32)col < level_matrix[0].size())
                {
                    return level_matrix[row][col];
                }
                return '\0';
            }
            return '\0';
        }

        // AABB Collision detection from learnopengl.com 2D breakout game tutorial
        Collision check_wall_collision(Wall& wall)
        {
            glm::vec2 player_center(player.position + player.radius);
            glm::vec2 aabb_half_extents(wall.size.x / 2, wall.size.y / 2);
            glm::vec2 aabb_center (
                wall.position.x + aabb_half_extents.x,
                wall.position.y + aabb_half_extents.y
            );
            glm::vec2 difference = player_center - aabb_center;
            glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
            glm::vec2 closest = aabb_center + clamped;
            difference = closest - player_center;

            if (glm::length(difference) < player.radius)
            {
                return std::make_tuple(GL_TRUE, vector_direction(difference), difference);
            }
            else
            {
                return std::make_tuple(GL_FALSE, Direction::UP, glm::vec2(0,0));
            }
        }

        void update(float elapsed)
        {
            for (uint32_t i = 0; i < guards.size(); i++)
            {
                guards[i].update(elapsed);
            }
        }
         

        bool check_caught_by_guard()
        {
            for (Uint32 i = 0; i < guards.size(); i++)
            {
                glm::vec2 hotspot = guards[i].position + guards[i].get_fov_offset();
                // Create an invisible block
                Wall caught_box(hotspot, glm::vec2(1,1));
                Collision collision = check_wall_collision(caught_box);
                if(std::get<0>(collision))
                {
                    return true;
                }
            }
            return false;
        }


        bool fell_in_hole()
        {
            for(uint32_t i = 0; i < holes.size(); i++)
            {
                glm::vec2 player_center = player.position + 0.5f;
                bool col_x = player_center.x > holes[i].x && player_center.x < holes[i].x + 1;
                bool col_y = player_center.y > holes[i].y && player_center.y < holes[i].y + 1;
                if (col_x && col_y)
                {
                    return true;
                }
            }
            return false;
        }

        
    };

    

};

