
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <queue>
#include <cstdio>
#include <cstdlib>

#include "translate.h"
#include "renderer.h"
#include "music.h"
#include "filewatch.h"

#define WIDTH 352
#define HEIGHT 280
#define FRAMES 10000
#define FRAMERATE 50



void error_callback(int error, const char* description) {
	printf(" *** GLFW error: %s\n", description);
	fflush(stdout);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	std::queue<int>* key_queue = (std::queue<int>*) glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		key_queue->push(key);
	}
}

static RoseRenderer* make_renderer(const char *filename, int frames, int width, int height, bool print_errors) {
	RoseResult rose_data = translate(filename, frames, width, height, print_errors);
	if (rose_data.empty()) {
		return nullptr;
	}
	return new RoseRenderer(std::move(rose_data), width, height);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: rose <filename> [<framerate> [<music>]]\n");
		exit(1);
	}

	int framerate = FRAMERATE;
	if (argc > 2) {
		framerate = atoi(argv[2]);
	}

	MusicPlayer player;
	int frames = FRAMES;
	if (argc > 3) {
		player.load(argv[3]);
		frames = (int) (player.length() * framerate);
	}

	// Initialize GLFW
	glfwSetErrorCallback(error_callback);
	glfwInit();

	// Initialize Window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(WIDTH*2, HEIGHT*2, "Rose", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Load code
	FileWatch rose_file(argv[1]);
	RoseRenderer* project = make_renderer(rose_file.name(), frames, WIDTH, HEIGHT, true);

	// Set up key callback
	std::queue<int> key_queue;
	glfwSetWindowUserPointer(window, &key_queue);
	glfwSetKeyCallback(window, key_callback);

	player.start(0);
	int startframe = 0;
	int frame = 0;
	bool playing = true;
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
		bool frame_set = false;

		// Reload if changed
		if (rose_file.changed()) {
			// Reload code
			printf("\nReloading at %s\n", rose_file.time_text());
			delete project;
			project = make_renderer(rose_file.name(), frames, WIDTH, HEIGHT, false);
			if (!project) {
				// Try again
				usleep(100*1000);
				project = make_renderer(rose_file.name(), frames, WIDTH, HEIGHT, true);
			}
			fflush(stdout);
			if (playing) {
				frame = startframe;
				frame_set = true;
			}
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
			double xpos,ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			int width,height;
			glfwGetWindowSize(window, &width, &height);
			frame = (int)(xpos / width * frames);
			frame_set = true;
			startframe = frame;
		}

		while (!key_queue.empty()) {
			int key = key_queue.front();
			key_queue.pop();
			switch (key) {
			case GLFW_KEY_SPACE:
				playing = !playing;
				if (playing) {
					startframe = frame;
					player.start(frame / (double) framerate);
				} else {
					player.stop();
					frame_set = true;
				}
				break;
			case GLFW_KEY_BACKSPACE:
				frame = startframe;
				frame_set = true;
				break;
			case GLFW_KEY_LEFT:
				frame -= 1;
				frame_set = true;
				break;
			case GLFW_KEY_RIGHT:
				frame += 1;
				frame_set = true;
				break;
			case GLFW_KEY_PAGE_UP:
				frame -= 50;
				frame_set = true;
				break;
			case GLFW_KEY_PAGE_DOWN:
				frame += 50;
				frame_set = true;
				break;
			case GLFW_KEY_HOME:
				frame = 0;
				frame_set = true;
				break;
			case GLFW_KEY_TAB:
				project->toggle_overlay();
				break;
			}
		}

		// Clamp frame
		if (frame < 0) frame = 0;
		if (frame > frames-1) frame = frames-1;

		if (frame_set) {
			player.set_time(frame / (double) framerate);
		}

		// Clear
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClearColor(1,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render
		if (project) project->draw(frame);

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (playing) {
			int prev_frame = frame;
			do {
				usleep(1000);
				frame = (int)(player.get_time() * framerate);
			} while (frame == prev_frame);
		} else {
			usleep(100000);
		}
	}


	if (project) delete project;

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

