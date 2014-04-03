
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <queue>
#include <cstdio>
#include <algorithm>

#include <sys/stat.h>

#include "translate.h"
#include "shaders.h"

#define WIDTH 352
#define HEIGHT 280
#define FRAMES 10000

void error_callback(int error, const char* description) {
	printf(" *** GLFW error: %s\n", description);
	fflush(stdout);
}

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
	return s;
}

struct CircleVertex {
	float x,y,u,v;
	float tint;
};

class RoseProject {
	static GLuint program;
	static GLuint xyuv_loc;
	static GLuint tint_loc;
	GLuint vertex_buffer;
	std::pair<std::vector<Plot>, std::vector<TintColor>> plots_and_script;
	std::vector<int> schedule;

public:
	RoseProject(const char *filename) {
		plots_and_script = translate(filename, FRAMES);
		std::vector<Plot>& plots = plots_and_script.first;

		// Make vertex data
		stable_sort(plots.begin(), plots.end(), [](const Plot& a, const Plot& b) {
			if (a.t != b.t) return a.t < b.t;
			return a.y - a.r < b.y - b.r;
		});
		const float corners[3][2] = {
			{ 1.75, -1.0 },
			{ -1.75, -1.0 },
			{ 0.0, 2.0 }
		};

		std::vector<CircleVertex> vertex_data;
		for (auto p : plots) {
			float x = p.x + 0.5f;
			float y = p.y + 0.5f;
			float r = p.r + 0.5f;
			for (int c = 0 ; c < 3 ; c++) {
				float u = corners[c][0];
				float v = corners[c][1];
				CircleVertex vert = {
					(x + u*r) / WIDTH * 2 - 1, (y + v*r) / HEIGHT * -2 + 1, u, v, (float) p.c
				};
				vertex_data.push_back(vert);
			}
		}

		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(CircleVertex), &vertex_data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Construct schedule
		int n = 0;
		for (int f = 0 ; f < FRAMES ; f++) {
			schedule.push_back(n);
			while (n < plots.size() && plots[n].t <= f) n++;
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

	void draw(int frame) {
		std::vector<TintColor>& script = plots_and_script.second;

		// Initialize colors
		std::vector<float> colors;
		for (int i = 0 ; i < 256 ; i++) {
			colors.push_back(1.0);
			colors.push_back(0.0);
			colors.push_back(1.0);
			colors.push_back(0.5);
		}

		// Update colors
		int script_index = 0;
		while (script_index < script.size() && script[script_index].t <= frame) {
			short rgb = script[script_index].rgb;
			float *color = &colors[script[script_index].i * 4];
			color[0] = ((rgb >> 8) & 15) / 15.0f;
			color[1] = ((rgb >> 4) & 15) / 15.0f;
			color[2] = ((rgb >> 0) & 15) / 15.0f;
			color[3] = 1.0f;
			script_index++;
		}

		// Set up vertex streams
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(xyuv_loc, 4, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), &((CircleVertex *)0)->x);
		glEnableVertexAttribArray(xyuv_loc);
		glVertexAttribPointer(tint_loc, 1, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), &((CircleVertex *)0)->tint);
		glEnableVertexAttribArray(tint_loc);

		// Set program and uniforms
		glUseProgram(program);
		GLuint colors_loc = glGetUniformLocation(program, "colors");
		glUniform4fv(colors_loc, 256, &colors[0]);

		// Draw
		glClearColor(colors[0],colors[1],colors[2],colors[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_NOTEQUAL, 0.0);
		glDrawArrays(GL_TRIANGLES, 0, schedule[frame] * 3);

		// Cleanup
		glDisable(GL_ALPHA_TEST);
		glDisableVertexAttribArray(xyuv_loc);
		glDisableVertexAttribArray(tint_loc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	~RoseProject() {
		glDeleteBuffers(1, &vertex_buffer);
	}
};

GLuint RoseProject::program = 0;
GLuint RoseProject::xyuv_loc = 0;
GLuint RoseProject::tint_loc = 0;


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	std::queue<int>* key_queue = (std::queue<int>*) glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		key_queue->push(key);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Missing filename!\n");
		exit(1);
	}
	char *filename = argv[1];
/*
	for (auto p : plots) {
		printf("%d: (%d,%d) R=%d C=%d\n", p.t, p.x, p.y, p.r, p.c);
	}
*/
	// Initialize GLFW
	glfwSetErrorCallback(error_callback);
	glfwInit();

	// Initialize Window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(WIDTH*2, HEIGHT*2, "Rose", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Make frame buffer
	GLuint fb, rb;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete (%d)\n", status);
		fflush(stdout);
	}

	struct stat filestat;
	stat(filename, &filestat);
	RoseProject* project = new RoseProject(filename);

	int startframe = 0;
	int frame = 0;
	bool playing = true;
	std::queue<int> key_queue;
	glfwSetWindowUserPointer(window, &key_queue);
	glfwSetKeyCallback(window, key_callback);
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
		// Reload if changed
		struct stat newfilestat;
		stat(filename, &newfilestat);
		if (newfilestat.st_mtime != filestat.st_mtime) {
			delete project;
			project = new RoseProject(filename);
			filestat = newfilestat;
			frame = startframe;
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
			double xpos,ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			int width,height;
			glfwGetWindowSize(window, &width, &height);
			frame = (int)(xpos / width * FRAMES);
			startframe = frame;
		}

		while (!key_queue.empty()) {
			int key = key_queue.front();
			key_queue.pop();
			switch (key) {
			case GLFW_KEY_SPACE:
				playing = !playing;
				if (playing) startframe = frame;
				break;
			case GLFW_KEY_BACKSPACE:
				frame = startframe;
				break;
			case GLFW_KEY_LEFT:
				frame -= 1;
				break;
			case GLFW_KEY_RIGHT:
				frame += 1;
				break;
			case GLFW_KEY_PAGE_UP:
				frame -= 50;
				break;
			case GLFW_KEY_PAGE_DOWN:
				frame += 50;
				break;
			case GLFW_KEY_HOME:
				frame = 0;
				break;
			}
		}

		// Clamp frame
		if (frame < 0) frame = 0;
		if (frame > FRAMES) frame = FRAMES;

		// Clear
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClearColor(1,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render to FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
		glViewport(0,0,WIDTH,HEIGHT);
		project->draw(frame);

		// Copy FBO to screen
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		glBlitFramebuffer(0,0,WIDTH,HEIGHT, 0,0,WIDTH*2,HEIGHT*2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (playing && frame < FRAMES) frame++;
	}

	delete project;

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

