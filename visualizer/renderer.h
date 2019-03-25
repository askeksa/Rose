#pragma once

#include "rose_result.h"

class RoseRenderer {
	static GLuint plot_program;
	static GLuint xyuv_loc;
	static GLuint tint_loc;
	GLuint plot_vertex_buffer;

	static GLuint combine_program;
	static GLuint combine_xy_loc;
	GLuint quad_vertex_buffer;

	static GLuint overlay_program;
	static GLuint overlay_xy_loc;

	std::vector<GLuint> render_tex, framebuf;
	RoseResult rose_data;
	std::vector<int> schedule;

	int prev_frame;

public:
	int width, height;

	RoseRenderer(RoseResult rose_result, int width, int height);

	void draw(int frame, bool overlay_enabled);

	~RoseRenderer();
};
