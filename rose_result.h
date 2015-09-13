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

struct RoseStatistics {
	int frames;
	int max_overwait;
	int max_stack_height;
	std::vector<int> circles_in_frame;
	std::vector<int> turtles_born_in_frame;
	std::vector<int> turtles_died_in_frame;

	RoseStatistics(int frames) : frames(frames) {
		max_overwait = 0;
		max_stack_height = 0;
		circles_in_frame.resize(frames, 0);
		turtles_born_in_frame.resize(frames, 0);
		turtles_died_in_frame.resize(frames, 0);
	}

	void print(FILE *out) {
		int max_circles = 0;
		int max_turtles = 0;
		int turtles_alive = 0;
		for (int i = 0 ; i < frames ; i++) {
			if (circles_in_frame[i] > max_circles) max_circles = circles_in_frame[i];
			turtles_alive += turtles_born_in_frame[i];
			if (turtles_alive > max_turtles) max_turtles = turtles_alive;
			turtles_alive -= turtles_died_in_frame[i];
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
