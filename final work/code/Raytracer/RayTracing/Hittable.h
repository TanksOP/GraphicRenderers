#pragma once
#include "ray.h"
#include "Common.h"
#include "aabb.h"

class material;

struct hit_record
{
	shared_ptr<material> mat_ptr;
	Point3f p;
	Vec3f normal;
	double t;
	bool front_face;
	inline void set_face_normal(const Ray& r, const Vec3f& outward_normal) {
		front_face = (r.dir.dotProduct(outward_normal)) < 0;
		normal = front_face ? outward_normal : -outward_normal;

	}
};

class hittable {
public:
	// virtual this must be overiddern in the derived classes
	virtual bool hit(const Ray& r, double t_min, double t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(aabb& output_box) const = 0;
};
