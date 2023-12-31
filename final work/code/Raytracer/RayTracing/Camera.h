#pragma once
#include "common.h"
class camera {
public:
    camera(Point3f lookFrom,
        Point3f lookAt,
        Vec3f vUp,
        double vfov,
        double aspect_ratio,
        double aperture,
        double focus_dist) {

        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        w = (lookFrom - lookAt).normalize();
        u = (vUp.crossProduct(w)).normalize();
        v = w.crossProduct(u);
        origin = lookFrom;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;
        lens_radius = aperture / 2;
    }
    Ray get_ray(double s, double t) const {
        Vec3f rd = lens_radius * Vec3f().random_in_unit_disk();
        Vec3f offset = u * rd.x + v * rd.y;
        return Ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin-offset);
    }
private:
    Point3f origin;
    Point3f lower_left_corner;
    Vec3f horizontal;
    Vec3f vertical;

    Vec3f u, v, w;
    double lens_radius;
};
