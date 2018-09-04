#pragma once

#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <map>

#include "tilt_escape.hpp"

// The 'Game' struct holds all of the game-relevant state,
// and is called by the main loop.

struct Game {
	//Game creates OpenGL resources (i.e. vertex buffer objects) in its
	//constructor and frees them in its destructor.
	Game();
	~Game();

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	bool handle_event(SDL_Event const &evt, glm::uvec2 window_size);

	//update is called at the start of a new frame, after events are handled:
	void update(float elapsed);

	//draw is called after update:
	void draw(glm::uvec2 drawable_size);

	//check if the current game is over
	bool game_over();

	//resets the level
	void reset();

	// Get ths the next level
	void incement_level();

	// Changes to the the next level
	void next_level();

	// Checks to make sure there are no collisions
	glm::vec2 check_collision(float next_pos_x, float next_pos_y);

	//------- opengl resources -------

	//shader program that draws lit objects with vertex colors:
	struct {
		GLuint program = -1U; //program object

		//uniform locations:
		GLuint object_to_clip_mat4 = -1U;
		GLuint object_to_light_mat4x3 = -1U;
		GLuint normal_to_light_mat3 = -1U;
		GLuint sun_direction_vec3 = -1U;
		GLuint sun_color_vec3 = -1U;
		GLuint sky_direction_vec3 = -1U;
		GLuint sky_color_vec3 = -1U;

		//attribute locations:
		GLuint Position_vec4 = -1U;
		GLuint Normal_vec3 = -1U;
		GLuint Color_vec4 = -1U;
	} simple_shading;

	//mesh data, stored in a vertex buffer:
	GLuint meshes_vbo = -1U; //vertex buffer holding mesh data

	//The location of each mesh in the meshes vertex buffer:
	struct Mesh {
		GLint first = 0;
		GLsizei count = 0;
	};
	
	// Meshes for tilt escape
	Mesh player_mesh;
	Mesh guard_mesh;
	Mesh guard_view_mesh;
	Mesh wall_mesh;
	Mesh floor_mesh;


	GLuint meshes_for_simple_shading_vao = -1U; //vertex array object that describes how to connect the meshes_vbo to the simple_shading_program

	//------- game state -------

	glm::uvec2 board_size = glm::uvec2(5,4);
	
	// The guard will have discreet rotations for its FOV
	std::map<TiltEscape::LookDirection, glm::quat> guard_vision_rotations;

	struct {
		bool tilt_left = false;
		bool tilt_right = false;
		bool tilt_up = false;
		bool tilt_down = false;
	} controls;

	// This is the angle the board is tilted in
	// either direction
	const float TILT_ANGLE = 45.0f;

	// Player physics parameters
	glm::vec2 player_acceleration = glm::vec2(0,0);
	//float player_mass = 1.0f;
	float gravity = -9.8f;

	// How long has the board been tilted
	// *this may or may not be necessary*
	float elapsed_tilt_time = 0;

	// What levels do we support in the game
	std::vector<std::string> level_names;
	int level_index = 0;

	// Reference to the currently loaded level
	TiltEscape::Level level;

};
