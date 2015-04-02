
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <portaudio.h>

#include <vector>
#include <queue>
#include <cstdio>
#include <algorithm>

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "translate.h"
#include "shaders.h"

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

struct CircleVertex {
	float x,y,u,v;
	float tint;
};

class RoseProject {
	static GLuint program;
	static GLuint xyuv_loc;
	static GLuint tint_loc;
	int frames;
	GLuint vertex_buffer;
	std::pair<std::vector<Plot>, std::vector<TintColor>> plots_and_script;
	std::vector<int> schedule;

public:
	static RoseProject* make(const char *filename, int frames, bool print_errors) {
		auto pas = translate(filename, frames, print_errors);
		if (pas.first.empty() && pas.second.empty()) {
			return nullptr;
		}
		return new RoseProject(std::move(pas), frames);
	}

	RoseProject(std::pair<std::vector<Plot>, std::vector<TintColor>> pas, int frames)
			: frames(frames), plots_and_script(std::move(pas))
	{
		std::vector<Plot>& plots = plots_and_script.first;

		// Make vertex data
		stable_sort(plots.begin(), plots.end(), [](const Plot& a, const Plot& b) {
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
		for (auto p : plots) {
			float x = p.x + 0.5f;
			float y = p.y + 0.5f;
			float r = p.r + 0.5f;
			for (int c = 0 ; c < 6 ; c++) {
				float u = corners[c][0];
				float v = corners[c][1];
				CircleVertex vert = {
					(x + u*r) / WIDTH * 2 - 1, (y + v*r) / HEIGHT * -2 + 1, u, v, (float) (p.c & 511)
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
		for (int f = 0 ; f < frames ; f++) {
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
		for (int i = 0 ; i < 512 ; i++) {
			colors.push_back(1.0);
			colors.push_back(0.0);
			colors.push_back(1.0);
			colors.push_back(i >= 256);
		}

		// Update colors
		int script_index = 0;
		while (script_index < script.size() && script[script_index].t <= frame) {
			short rgb = script[script_index].rgb;
			short index = script[script_index].i & 255;
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
		glClearColor(colors[0],colors[1],colors[2],colors[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_NOTEQUAL, 0.0);
		glDrawArrays(GL_TRIANGLES, 0, schedule[frame] * 6);

		// Cleanup
		glDisable(GL_ALPHA_TEST);
		glDisableVertexAttribArray(xyuv_loc);
		glDisableVertexAttribArray(tint_loc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	~RoseProject() {
		glFinish();
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

	// Load code
	struct stat filestat;
	stat(filename, &filestat);
	RoseProject* project = RoseProject::make(filename, frames, true);

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
			project = RoseProject::make(filename, frames, false);
			if (!project) {
				// Try again
				usleep(100*1000);
				project = RoseProject::make(filename, frames, true);
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

		// Render to FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
		glViewport(0,0,WIDTH,HEIGHT);
		if (project) project->draw(frame);

		// Copy FBO to screen
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
		glBlitFramebuffer(0,0,WIDTH,HEIGHT, 0,0,WIDTH*2,HEIGHT*2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (playing) {
			int prev_frame = frame;
			do {
				usleep(1000);
				double music_time = Pa_GetStreamTime(stream) - info.time_offset;
				frame = (int)(music_time * framerate);
			} while (frame == prev_frame);
		}
	}

	Pa_CloseStream(stream);
	Pa_Terminate();

	if (project) delete project;

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

