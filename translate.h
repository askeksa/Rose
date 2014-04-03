#pragma once

#include <vector>
#include <utility>

struct Plot {
	short t,x,y,r,c;
};

struct TintColor {
	short t,i,rgb;
};

std::pair<std::vector<Plot>, std::vector<TintColor>> translate(const char *filename, int max_time);
