#pragma once

#include "rose_result.h"

RoseResult translate(const char *filename, int max_time,
                     int width, int height,
                     int layer_count, int layer_depth,
                     bool print);
