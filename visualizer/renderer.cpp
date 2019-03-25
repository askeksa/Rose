
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "shaders.h"

#include <algorithm>

struct CircleVertex {
	float x,y,u,v;
	float tint;
};

struct QuadVertex {
	float x,y;
};

GLuint makeShader(GLenum kind, const char **source) {
	std::vector<char> log;
	GLsizei log_length;
	GLuint s = glCreateShader(kind);
	glShaderSource(s, 1, source, 0);
	glCompileShader(s);
	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &log_length);
	log.resize(log_length + 1);
	glGetShaderInfoLog(s, log_length, nullptr, &log[0]);
	log[log_length] = '\0';
	printf("%s", &log[0]);
	fflush(stdout);
	return s;
}

GLuint makeProgram(const char *vsource, const char *psource) {
	std::vector<char> log;
	GLsizei log_length;
	GLuint program = glCreateProgram();
	GLuint vs = makeShader(GL_VERTEX_SHADER, &vsource);
	glAttachShader(program, vs);
	GLuint ps = makeShader(GL_FRAGMENT_SHADER, &psource);
	glAttachShader(program, ps);
	glLinkProgram(program);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
	log.resize(log_length + 1);
	glGetProgramInfoLog(program, log_length, nullptr, &log[0]);
	log[log_length] = '\0';
	printf("%s", &log[0]);
	fflush(stdout);
	return program;
}

GLuint RoseRenderer::plot_program = 0;
GLuint RoseRenderer::xyuv_loc = 0;
GLuint RoseRenderer::tint_loc = 0;
GLuint RoseRenderer::combine_program = 0;
GLuint RoseRenderer::combine_xy_loc = 0;
GLuint RoseRenderer::overlay_program = 0;
GLuint RoseRenderer::overlay_xy_loc = 0;

RoseRenderer::RoseRenderer(RoseResult rose_result, int width, int height)
	: rose_data(std::move(rose_result)), width(width), height(height)
{
	// Make vertex data
	stable_sort(rose_data.plots.begin(), rose_data.plots.end(), [](const Plot& a, const Plot& b) {
		if (a.t != b.t) return a.t < b.t;
		return a.y - a.r < b.y - b.r;
	});
	const float corners[6][2] = {
		{ 1.0, -1.0 },
		{ -1.0, -1.0 },
		{ -1.0, 1.0 },
		{ -1.0, 1.0 },
		{ 1.0, 1.0 },
		{ 1.0, -1.0 }
	};

	// Data for plot vertex buffer
	std::vector<CircleVertex> plot_vertex_data;
	for (auto p : rose_data.plots) {
		float x = p.x + 0.5f;
		float y = p.y + 0.5f;
		float r = p.r + 0.5f;
		for (int c = 0 ; c < 6 ; c++) {
			float u = corners[c][0];
			float v = corners[c][1];
			CircleVertex vert = {
				(x + u*r) / width * 2 - 1, (y + v*r) / height * -2 + 1, u, v, (float) (p.c & 511)
			};
			plot_vertex_data.push_back(vert);
		}
	}

	// Make plot vertex buffer
	glGenBuffers(1, &plot_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, plot_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, plot_vertex_data.size() * sizeof(CircleVertex), &plot_vertex_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Make quad vertex buffer
	glGenBuffers(1, &quad_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(QuadVertex), &corners[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	int layers = rose_data.layer_count;
	render_tex.resize(layers);
	glGenTextures(layers, &render_tex[0]);
	framebuf.resize(layers);
	glGenFramebuffers(layers, &framebuf[0]);
	for (int l = 0; l < layers; l++) {
		// Make render texture
		glBindTexture(GL_TEXTURE_2D, render_tex[l]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Make frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, framebuf[l]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex[l], 0);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer not complete (%d)\n", status);
			fflush(stdout);
		}
	}

	// Construct schedule
	int n = 0;
	for (int f = 0 ; n < rose_data.plots.size() ; f++) {
		schedule.push_back(n);
		while (n < rose_data.plots.size() && rose_data.plots[n].t <= f) n++;
	}
	schedule.push_back(n);

	// Compile shaders
	if (!plot_program) {
		plot_program = makeProgram(plot_vshader, plot_pshader);
		xyuv_loc = glGetAttribLocation(plot_program, "xyuv");
		tint_loc = glGetAttribLocation(plot_program, "tint");
	}
	if (!combine_program) {
		combine_program = makeProgram(quad_vshader, combine_pshader);
		combine_xy_loc = glGetAttribLocation(combine_program, "xy");
	}
	if (!overlay_program) {
		overlay_program = makeProgram(quad_vshader, overlay_pshader);
		overlay_xy_loc = glGetAttribLocation(overlay_program, "xy");
	}

	// Mark contents invalid
	prev_frame = -1;
}

void RoseRenderer::draw(int frame, bool overlay_enabled) {
	// Initialize colors
	std::vector<float> colors;
	for (int i = 0 ; i < 256 ; i++) {
		colors.push_back(1.0f);
		colors.push_back(0.0f);
		colors.push_back(1.0f);
		colors.push_back(i % rose_data.layer_depth == 0 ? 0.0f : 1.0f);
	}

	// Update colors
	int script_index = 0;
	while (script_index < rose_data.colors.size() && rose_data.colors[script_index].t <= frame + 1) {
		short rgb = rose_data.colors[script_index].rgb;
		short index = rose_data.colors[script_index].i & 255;
		float *color = &colors[index * 4];
		color[0] = ((rgb >> 8) & 15) / 15.0f;
		color[1] = ((rgb >> 4) & 15) / 15.0f;
		color[2] = ((rgb >> 0) & 15) / 15.0f;
		color[3] = index % rose_data.layer_depth == 0 ? 0.0f : 1.0f;
		script_index++;
	}

	// Save target state
	GLuint target;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &target);
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	// Set global render states
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, 0.0);

	// Plot pass

	// Set up vertex streams
	glBindBuffer(GL_ARRAY_BUFFER, plot_vertex_buffer);
	glVertexAttribPointer(xyuv_loc, 4, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), &((CircleVertex *)0)->x);
	glEnableVertexAttribArray(xyuv_loc);
	glVertexAttribPointer(tint_loc, 1, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), &((CircleVertex *)0)->tint);
	glEnableVertexAttribArray(tint_loc);

	// Set program
	glUseProgram(plot_program);

	int draw_frame = std::min(frame + 1, (int) (schedule.size() - 1));
	int layers = rose_data.layer_count;
	for (int l = 0; l < layers; l++) {
		// Render to FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuf[l]);
		glViewport(0, 0, width, height);

		// Set layer uniforms
		GLuint min_tint_loc = glGetUniformLocation(plot_program, "min_tint");
		glUniform1f(min_tint_loc, l * rose_data.layer_depth);
		GLuint max_tint_loc = glGetUniformLocation(plot_program, "max_tint");
		glUniform1f(max_tint_loc, (l + 1) * rose_data.layer_depth - 1);

		// Draw
		if (prev_frame == -1 || draw_frame < prev_frame) {
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, schedule[draw_frame] * 6);
		} else {
			glDrawArrays(GL_TRIANGLES, schedule[prev_frame] * 6, (schedule[draw_frame] - schedule[prev_frame]) * 6);
		}
	}
	prev_frame = draw_frame;

	// Cleanup
	glDisable(GL_ALPHA_TEST);
	glDisableVertexAttribArray(xyuv_loc);
	glDisableVertexAttribArray(tint_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Combine pass

	// Render to original render target
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glViewport(vp[0], vp[1], vp[2], vp[3]);

	// Set up vertex streams
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glVertexAttribPointer(combine_xy_loc, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), &((QuadVertex *)0)->x);
	glEnableVertexAttribArray(combine_xy_loc);

	// Set program
	glUseProgram(combine_program);

	// Set color uniforms
	GLuint colors_loc = glGetUniformLocation(combine_program, "colors");
	glUniform4fv(colors_loc, 256, &colors[0]);

	// Clear
	glClearColor(colors[0], colors[1], colors[2], colors[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int l = 0; l < layers; l++) {
		// Set layer uniforms
		GLuint image_loc = glGetUniformLocation(combine_program, "image");
		glUniform1i(image_loc, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, render_tex[l]);

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(combine_xy_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Overlay pass

	if (overlay_enabled) {
		// Set up vertex streams
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
		glVertexAttribPointer(overlay_xy_loc, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), &((QuadVertex *)0)->x);
		glEnableVertexAttribArray(overlay_xy_loc);

		// Set program
		glUseProgram(overlay_program);

		// Set uniforms
		FrameStatistics& stats = rose_data.stats->frame[frame];
		GLuint copper_cycles_loc = glGetUniformLocation(overlay_program, "copper_cycles");
		glUniform1f(copper_cycles_loc, stats.copper_cycles);
		GLuint blitter_cycles_loc = glGetUniformLocation(overlay_program, "blitter_cycles");
		glUniform1f(blitter_cycles_loc, stats.blitter_cycles);
		GLuint cpu_compute_cycles_loc = glGetUniformLocation(overlay_program, "cpu_compute_cycles");
		glUniform1f(cpu_compute_cycles_loc, stats.cpu_compute_cycles);
		GLuint cpu_draw_cycles_loc = glGetUniformLocation(overlay_program, "cpu_draw_cycles");
		glUniform1f(cpu_draw_cycles_loc, stats.cpu_draw_cycles);

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Cleanup
		glDisableVertexAttribArray(overlay_xy_loc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

RoseRenderer::~RoseRenderer() {
	glFinish();
	int layers = rose_data.layer_count;
	glDeleteTextures(layers, &render_tex[0]);
	glDeleteFramebuffers(layers, &framebuf[0]);
	glDeleteBuffers(1, &quad_vertex_buffer);
	glDeleteBuffers(1, &plot_vertex_buffer);
}
