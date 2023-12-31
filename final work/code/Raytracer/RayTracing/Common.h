#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>
#include <algorithm>

// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;
const int spp = 1;
const int max_depth = 50;

// Utility Functions
inline double degrees_to_radians(double degrees) { return degrees * pi / 180.0; }
inline double random_double() { return rand() / (RAND_MAX + 1.0); }
inline double random_double(double min, double max) { return min + (max-min) * random_double(); }

// Common Headers
#include "ray.h"
#include "geometry.h"
