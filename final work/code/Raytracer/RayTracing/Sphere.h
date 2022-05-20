#pragma once
#include "hittable.h"
#include "Geometry.h"

class Sphere : public hittable {
public:
	Sphere() {};
	Sphere(Point3f cen, double r, shared_ptr<material> m): centre(cen), radius(r), mat_ptr(m) {};
	
	virtual bool hit(const Ray& r, double t_min, double t_max, hit_record& rec) const override;

	virtual bool bounding_box(aabb& output_box) const override;

public:
	Point3f centre;
	double radius;
	shared_ptr<material>mat_ptr;

};

bool Sphere::hit(const Ray& r, double t_min,double t_max, hit_record& rec) const {
	Vec3f oc = r.origin() - centre;
	auto a = r.direction().norm();
	auto half_b = oc.dotProduct(r.direction());
	auto c = oc.norm() - radius * radius;
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	auto sqrtd = sqrt(discriminant);

	// this finds the neerest root that lies in acceptable range
	auto root = (-half_b - sqrtd) / a;
	if (root < t_min || t_max < root) {
		root = (-half_b + sqrtd) / a;
		if (root < t_min || t_max < root)
			return false;

	}	
	rec.t = root;
	rec.p = r.at(rec.t);
	Vec3f outward_normal = (rec.p - centre) / radius;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat_ptr;
	return true;
}

inline bool Sphere::bounding_box(aabb& output_box) const {
	output_box = aabb(centre - Vec3f(radius, radius, radius), centre + Vec3f(radius, radius, radius));
	return true;
}
	
