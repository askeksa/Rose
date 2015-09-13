#pragma once

#include "rose_result.h"

class RoseRenderer {
	static GLuint program;
	static GLuint xyuv_loc;
	static GLuint tint_loc;
	GLuint vertex_buffer;
	GLuint fb, rb;
	RoseResult rose_data;
	std::vector<int> schedule;
	int width, height;

public:
	RoseRenderer(RoseResult rose_result, int width, int height);

	void draw(int frame);

	~RoseRenderer();
};
