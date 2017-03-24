
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <portaudio.h>

#include <queue>
#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "translate.h"
#include "renderer.h"

#define WIDTH 352
#define HEIGHT 280
#define FRAMES 10000
#define FRAMERATE 50

#define STREAM_BUFFER_SIZE 512

struct WavInfo {
	short *wavdata;
	unsigned samplepos;
	unsigned n_samples;
	double sample_rate;
	double time_offset;
};

struct WavHeader {
	unsigned riff_tag;
	unsigned file_length;
	unsigned wave_tag;
	unsigned fmt_tag;
	unsigned header_length;
	unsigned short format;
	unsigned short num_channels;
	unsigned sample_rate;
	unsigned byte_rate;
	unsigned short block_align;
	unsigned short bits;
	unsigned data_tag;
	unsigned data_length;
};

int stream_callback(const void *input, void *output, unsigned long frameCount,
					const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	struct WavInfo *info = (struct WavInfo *) userData;
	info->time_offset = timeInfo->outputBufferDacTime - info->samplepos / info->sample_rate;
	int remaining = info->n_samples - info->samplepos;
	int copy = std::max(0, std::min((int)frameCount, remaining));
	memset(output, 0, frameCount * 4);
	memcpy(output, &info->wavdata[info->samplepos * 2], copy * 4);
	info->samplepos += frameCount;
	return paContinue;
}

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
	RoseResult rose_data = translate(filename, frames, print_errors);
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
	char *filename = argv[1];
	void *wav_file = nullptr;
	double sample_rate = 44100.0;
	struct WavInfo info = { nullptr, 0, 0, 0.0, 0.0 };

	int framerate = FRAMERATE;
	if (argc > 2) {
		framerate = atoi(argv[2]);
	}

	int frames = FRAMES;
	if (argc > 3) {
		const char *wav_filename = argv[3];
		FILE *fp = fopen(wav_filename,"rb");
		if (!fp) {
			printf("Unable to open music file %s\n", wav_filename);
			exit(1);
		}
		fseek(fp,0,SEEK_END);
		size_t wav_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		wav_file = malloc(wav_size);
		fread(wav_file, 1, wav_size, fp);
		fclose(fp);

		struct WavHeader *wh = (struct WavHeader *) wav_file;
		sample_rate = wh->sample_rate;
		if (wh->num_channels != 2 || wh->bits != 16) {
			printf("Music must be 16 bit stereo.\n");
			exit(1);
		}
		frames = (int) ((wh->data_length / 4) / sample_rate * framerate);
		info.wavdata = (short *) &wh[1];
		info.n_samples = wh->data_length / 4;
		info.sample_rate = sample_rate;
		info.time_offset = 0.0;
	}

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

	// Load code
	struct stat filestat;
	stat(filename, &filestat);
	RoseRenderer* project = make_renderer(filename, frames, WIDTH, HEIGHT, true);

	// Set up key callback
	std::queue<int> key_queue;
	glfwSetWindowUserPointer(window, &key_queue);
	glfwSetKeyCallback(window, key_callback);

	Pa_Initialize();
	PaStream* stream;
	if (Pa_OpenDefaultStream(&stream, 0, 2, paInt16, sample_rate, STREAM_BUFFER_SIZE, stream_callback, &info)) {
		printf("Error opening audio output!\n");
		exit(1);
	}

	int startframe = 0;
	int frame = 0;
	bool playing = true;
	info.time_offset = Pa_GetStreamTime(stream);
	if (wav_file) Pa_StartStream(stream);
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
		bool frame_set = false;

		// Reload if changed
		struct stat newfilestat;
		stat(filename, &newfilestat);
		if (newfilestat.st_mtime != filestat.st_mtime) {
			// Reload code
			printf("\nReloading at %s\n", ctime(&newfilestat.st_mtime));
			delete project;
			project = make_renderer(filename, frames, WIDTH, HEIGHT, false);
			if (!project) {
				// Try again
				usleep(100*1000);
				project = make_renderer(filename, frames, WIDTH, HEIGHT, true);
			}
			fflush(stdout);
			filestat = newfilestat;
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
					info.time_offset = Pa_GetStreamTime(stream) - frame / (double) framerate;
					if (wav_file) Pa_StartStream(stream);
				} else {
					if (wav_file) Pa_StopStream(stream);
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
			}
		}

		// Clamp frame
		if (frame < 0) frame = 0;
		if (frame > frames-1) frame = frames-1;

		if (frame_set) {
			info.samplepos = (int) (frame * sample_rate / framerate);
			info.time_offset = Pa_GetStreamTime(stream) - frame / (double) framerate;
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
				double music_time = Pa_GetStreamTime(stream) - info.time_offset;
				frame = (int)(music_time * framerate);
			} while (frame == prev_frame);
		} else {
			usleep(100000);
		}
	}

	Pa_CloseStream(stream);
	Pa_Terminate();

	if (project) delete project;

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

