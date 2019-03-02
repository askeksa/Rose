#pragma once

#include <vector>
#include <utility>
#include <cstdio>

struct Plot {
	short t,x,y,r,c;
};

struct TintColor {
	short t,i,rgb;
};

struct RoseResult {
	std::vector<Plot> plots;
	std::vector<TintColor> colors;

	bool empty() {
		return plots.empty() && colors.empty();
	}
};

struct FrameStatistics {
	int circles = 0;
	int turtles_survived = 0;
	int turtles_died = 0;
};

struct RoseStatistics {
	int frames;
	int max_overwait;
	int max_stack_height;
	std::vector<FrameStatistics> frame;

	RoseStatistics(int frames) : frames(frames), frame(frames) {
		max_overwait = 0;
		max_stack_height = 0;
	}

	void print(FILE *out) {
		int max_circles = 0;
		int max_turtles = 0;
		for (int i = 0 ; i < frames ; i++) {
			if (frame[i].circles > max_circles) max_circles = frame[i].circles;
			int turtles_alive = frame[i].turtles_survived + frame[i].turtles_died + 1;
			if (turtles_alive > max_turtles) max_turtles = turtles_alive;
		}
		fprintf(out, "\n");
		fprintf(out, "Number of frames:     %5d\n", frames);
		fprintf(out, "Max extra wait:       %5d\n", max_overwait);
		fprintf(out, "Max circles in frame: %5d\n", max_circles);
		fprintf(out, "Max turtles alive:    %5d\n", max_turtles);
		fprintf(out, "Max stack height:     %5d\n", max_stack_height);
		fflush(out);
	}
};
