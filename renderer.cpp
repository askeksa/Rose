
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

GLuint makeShader(GLenum kind, const char **source) {
	std::vector<char> log;
	GLsizei log_length;
	GLuint s = glCreateShader(kind);
	glShaderSource(s, 1, source, 0);
	glCompileShader(s);
	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &log_length);
	log.resize(log_length);
	glGetShaderInfoLog(s, log_length, nullptr, &log[0]);
	printf("%s", &log[0]);
	fflush(stdout);
	return s;
}

GLuint RoseRenderer::program = 0;
GLuint RoseRenderer::xyuv_loc = 0;
GLuint RoseRenderer::tint_loc = 0;

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

	std::vector<CircleVertex> vertex_data;
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
			vertex_data.push_back(vert);
		}
	}

	// Make vertex buffer
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(CircleVertex), &vertex_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Make frame buffer
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete (%d)\n", status);
		fflush(stdout);
	}

	// Construct schedule
	int n = 0;
	for (int f = 0 ; n < rose_data.plots.size() ; f++) {
		schedule.push_back(n);
		while (n < rose_data.plots.size() && rose_data.plots[n].t <= f) n++;
	}
	schedule.push_back(n);

	// Compile shaders
	if (!program) {
		program = glCreateProgram();
		GLuint vs = makeShader(GL_VERTEX_SHADER, &vshader);
		glAttachShader(program, vs);
		GLuint ps = makeShader(GL_FRAGMENT_SHADER, &pshader);
		glAttachShader(program, ps);
		glLinkProgram(program);
		xyuv_loc = glGetAttribLocation(program, "xyuv");
		tint_loc = glGetAttribLocation(program, "tint");
	}

}

void RoseRenderer::draw(int frame) {
	// Initialize colors
	std::vector<float> colors;
	for (int i = 0 ; i < 512 ; i++) {
		colors.push_back(1.0);
		colors.push_back(0.0);
		colors.push_back(1.0);
		colors.push_back(i >= 256);
	}

	// Update colors
	int script_index = 0;
	while (script_index < rose_data.colors.size() && rose_data.colors[script_index].t <= frame) {
		short rgb = rose_data.colors[script_index].rgb;
		short index = rose_data.colors[script_index].i & 255;
		float *color = &colors[index * 4];
		color[0] = ((rgb >> 8) & 15) / 15.0f;
		color[1] = ((rgb >> 4) & 15) / 15.0f;
		color[2] = ((rgb >> 0) & 15) / 15.0f;
		color[3] = 0.0f;
		float *square_color = &colors[(index ^ 511) * 4];
		square_color[0] = color[0];
		square_color[1] = color[1];
		square_color[2] = color[2];
		square_color[3] = 1.0f;
		script_index++;
	}

	// Save target state
	GLuint target;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &target);
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	// Render to FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glViewport(0,0,width,height);

	// Set up vertex streams
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(xyuv_loc, 4, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), &((CircleVertex *)0)->x);
	glEnableVertexAttribArray(xyuv_loc);
	glVertexAttribPointer(tint_loc, 1, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), &((CircleVertex *)0)->tint);
	glEnableVertexAttribArray(tint_loc);

	// Set program and uniforms
	glUseProgram(program);
	GLuint colors_loc = glGetUniformLocation(program, "colors");
	glUniform4fv(colors_loc, 512, &colors[0]);

	// Draw
	int schedule_frame = std::min(frame, (int) (schedule.size() - 1));
	glClearColor(colors[0],colors[1],colors[2],colors[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, 0.0);
	glDrawArrays(GL_TRIANGLES, 0, schedule[schedule_frame] * 6);

	// Cleanup
	glDisable(GL_ALPHA_TEST);
	glDisableVertexAttribArray(xyuv_loc);
	glDisableVertexAttribArray(tint_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Copy FBO to screen
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
	glViewport(vp[0],vp[1],vp[2],vp[3]);
	glBlitFramebuffer(0,0,width,height, vp[0],vp[1],vp[2],vp[3], GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

RoseRenderer::~RoseRenderer() {
	glFinish();
	glDeleteFramebuffers(1, &fb);
	glDeleteRenderbuffers(1, &rb);
	glDeleteBuffers(1, &vertex_buffer);
}
