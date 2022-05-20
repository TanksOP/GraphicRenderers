#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "geometry.h"


class Model {
private:
	std::vector<Vec3f> verts_;              // Stores Vec3f for every model vertex world position
	  // Stores a vector of vector<int> that represent indices in verts_ for vertices comprising a face
	std::vector<Vec2f> vts_;	
	std::vector<Vec3f>vns_;


	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > Vnorms_;
public:
	Model(const char* filename);
	~Model();

	

	int nverts();
	int nfaces();

	Vec3f vert(int i);
	Vec3f norm(int i);
	Vec2f vt(int i);

	std::vector<int> face(int idx);
	std::vector<int> vNorms(int idx);

	
};

