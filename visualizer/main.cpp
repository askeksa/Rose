
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


// Defaults
#define WIDTH 352
#define HEIGHT 280
#define LAYERS 1
#define DEPTH 4
#define WINDOW_SCALE 2
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

static RoseRenderer* make_renderer(RoseResult rose_data) {
	if (rose_data.empty()) {
		return nullptr;
	}
	return new RoseRenderer(std::move(rose_data), rose_data.width, rose_data.height);
}

class FileWatches {
	std::vector<FileWatch> watches;
	int index = 0;

public:
	FileWatches(const RoseResult& rose_result) {
		for (const std::string& path : rose_result.paths) {
			watches.emplace_back(path.c_str());
		}
	}

	bool changed() {
		for (int i = 0; i < watches.size(); i++) {
			if (watches[i].changed()) {
				index = i;
				return true;
			}
		}
		return false;
	}

	const char* time_text() {
		return watches[index].time_text();
	}
};

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: rose <filename> [<framerate> [<music>]]\n");
		exit(1);
	}

	int arg = 1;
	const char* main_filename = argv[arg++];

	int window_scale = WINDOW_SCALE;
	if (argc > arg && argv[arg][0] == 'x') {
		window_scale = atoi(&argv[arg++][1]);
	}

	int framerate = FRAMERATE;
	if (argc > arg) {
		framerate = atoi(argv[arg++]);
	}

	MusicPlayer player;
	int frames = FRAMES;
	if (argc > arg) {
		player.load(argv[arg++]);
		frames = (int) (player.length() * framerate);
	}

	// Load code
	RoseResult rose_result = translate(main_filename, frames, WIDTH, HEIGHT, LAYERS, DEPTH);
	std::unique_ptr<FileWatches> watches(new FileWatches(rose_result));
	int width = rose_result.width;
	int height = rose_result.height;

	// Initialize GLFW
	glfwSetErrorCallback(error_callback);
	glfwInit();

	// Initialize Window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(width * window_scale, height * window_scale, "Rose", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	RoseRenderer* project = make_renderer(std::move(rose_result));

	// Set up key callback
	std::queue<int> key_queue;
	glfwSetWindowUserPointer(window, &key_queue);
	glfwSetKeyCallback(window, key_callback);

	player.start(0);
	int startframe = 0;
	int frame = 0;
	bool playing = true;
	bool overlay_enabled = false;
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
		bool frame_set = false;

		// Reload if changed
		if (watches->changed()) {
			// Reload code
			printf("\nReloading at %s\n", watches->time_text());
			if (project) delete project;
			rose_result = translate(main_filename, frames, WIDTH, HEIGHT, LAYERS, DEPTH);
			if (rose_result.empty() && !rose_result.error) {
				// Try again
				usleep(100*1000);
				rose_result = translate(main_filename, frames, WIDTH, HEIGHT, LAYERS, DEPTH);
			}
			watches.reset(new FileWatches(rose_result));
			fflush(stdout);
			if (playing) {
				frame = startframe;
				frame_set = true;
			}
			project = make_renderer(std::move(rose_result));
			if (project) {
				if (project->width != width || project->height != height) {
					width = project->width;
					height = project->height;
					glfwSetWindowSize(window, width * window_scale, height * window_scale);
					glViewport(0, 0, width * window_scale, height * window_scale);
				}
			}
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
			double xpos,ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			frame = (int)(xpos / (width * window_scale) * frames);
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
				overlay_enabled = !overlay_enabled;
				break;
			}
		}

		// Clamp frame
		if (frame < 0) frame = 0;
		if (frame > frames-1) frame = frames-1;

		if (frame_set) {
			player.set_time(frame / (double) framerate);
		}

		// Render
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		if (project) {
			if (project->draw(frame, overlay_enabled)) {
				glfwSwapBuffers(window);
			}
		} else {
			// Error color
			glClearColor(1,0,0,0);
			glClear(GL_COLOR_BUFFER_BIT);
			glfwSwapBuffers(window);
		}

		glfwPollEvents();

		if (playing && project) {
			int prev_frame = frame;
			//do {
			//	usleep(1000);
			//	frame = (int)(player.get_time() * framerate);
			//} while (frame == prev_frame);
                        frame++;
                        usleep(20);
		} else {
			usleep(100000);
		}
	}


	if (project) delete project;

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

