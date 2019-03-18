#pragma once

#include "rose_result.h"

class RoseRenderer {
	static GLuint plot_program;
	static GLuint xyuv_loc;
	static GLuint tint_loc;
	GLuint plot_vertex_buffer;

	static GLuint quad_program;
	static GLuint xy_loc;
	GLuint quad_vertex_buffer;

	GLuint framebuf, render_tex;
	RoseResult rose_data;
	std::vector<int> schedule;

	int prev_frame;

	bool overlay_enabled;

public:
	int width, height;

	RoseRenderer(RoseResult rose_result, int width, int height);

	void draw(int frame);

	void toggle_overlay();

	~RoseRenderer();
};
