# pragma once
#include "geometry.h"
#include "Hittable.h"
#include "HittableList.h"
#include "SDL.h" 
#include <chrono>
#include <fstream>
#include "Common.h"
#include "ray.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Camera.h"
#include "Material.h"
#include "theadpool.h"
#include "Model.h"
#include "tgaImage.h"
#include "bvh.h"


//#define M_PI 3.14159265359

SDL_Window * window;
SDL_Renderer* renderer;
SDL_Surface* screen;

TGAImage imageOut(1920, 1080, TGAImage::RGB);



void init() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Software Ray Tracer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1920,
        1080,
        0
    );

    screen = SDL_GetWindowSurface(window);
    

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;
    }
}




// method to ensure colours don’t go out of 8 bit range in RGB​
void clamp255(Vec3f& col) {
    col.x = (col.x > 255) ? 255 : (col.x < 0) ? 0 : col.x;
    col.y = (col.y > 255) ? 255 : (col.y < 0) ? 0 : col.y;
    col.z = (col.z > 255) ? 255 : (col.z < 0) ? 0 : col.z;
}


Colour ray_colour(const Ray& r, const Colour& background, const hittable& world, int depth ){
    
    hit_record rec;
    if (depth <= 0) return Colour(0, 0, 0);
    if (!world.hit(r, 0.001, infinity, rec)) return background; // bounded t from 0 -> infinity
 
    Ray scattered;
    Colour attenuation;
    Colour emitted = rec.mat_ptr->emitted();
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
        return emitted;
    }

    return emitted + attenuation * ray_colour(scattered, background, world, depth - 1);

    //    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
    //        return Colour(0,0,0);

    //    return attenuation * ray_colour(scattered, background, world, depth - 1);

    //    

    //    //return (rec.normal + Colour(1, 1, 1)) * 255 * 0.5;
    //
    //Vec3f unit_direction = r.direction().normalize();
    //auto t = 0.5 * (unit_direction.y + 1.0);
    //return (1.0 - t) * Colour(1.0, 1.0, 1.0) + t * Colour(0.5, 0.7, 1.0) * 255;
}

hittable_list random_scene() {
    hittable_list world;
    auto ground_material = make_shared<lambertian>(Colour(0.5, 0.5, 0.5));
    world.add(make_shared<Sphere>(Point3f(0, -1000, 0), 1000, ground_material));
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            Point3f centre(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((centre - Point3f(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;
                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = Colour::random() * Colour::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<Sphere>(centre, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Colour::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(centre, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<Sphere>(centre, 0.2, sphere_material));
                }
            }
        }
    }
    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<Sphere>(Point3f(0, 1, 0), 1.0, material1));
    auto material2 = make_shared<lambertian>(Colour(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(Point3f(-4, 1, 0), 1.0, material2));
    auto material3 = make_shared<metal>(Colour(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(Point3f(4, 1, 0), 1.0, material3));


    auto light1 = make_shared<diffuse_light>(Colour(255, 255, 255));
    world.add(make_shared<Sphere>(Point3f(0, 5, 0), 1.0, light1));


  // return world; // without bvh
    return hittable_list(make_shared<bvh_node>(world)); // with bvh
}

hittable_list test_scene() {
    hittable_list world;

    Model* model = new Model("cc.obj");

    Vec3f transform(0, 0.8, 0);
    auto glass = make_shared<dielectric>(1.5);
    for (uint32_t i = 0; i < model->nfaces(); ++i) {
        const Vec3f& v0 = model->vert(model->face(i)[0]);
        const Vec3f& v1 = model->vert(model->face(i)[1]);
        const Vec3f& v2 = model->vert(model->face(i)[2]);

        const Vec3f& n0 = model->norm(model->vNorms(i)[0]);
        const Vec3f& n1 = model->norm(model->vNorms(i)[1]);
        const Vec3f& n2 = model->norm(model->vNorms(i)[2]);


       // const Vec3f& n = model->norm(i);
        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform,n0, n1, n2, glass));
    }

     transform=Vec3f(1.2, 0.8, 0);
    auto mat_diffuse = make_shared<lambertian>(Colour(0.4,0.2,0.1));
    for (uint32_t i = 0; i < model->nfaces(); ++i) {
        const Vec3f& v0 = model->vert(model->face(i)[0]);
        const Vec3f& v1 = model->vert(model->face(i)[1]);
        const Vec3f& v2 = model->vert(model->face(i)[2]);

        const Vec3f& n0 = model->norm(model->vNorms(i)[0]);
        const Vec3f& n1 = model->norm(model->vNorms(i)[1]);
        const Vec3f& n2 = model->norm(model->vNorms(i)[2]);

        // const Vec3f& n = model->norm(i);
        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, n0, n1, n2, mat_diffuse));
        //world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform,n, mat_diffuse));
    }

     transform= Vec3f(-1.2, 0.8, 0);
    auto mat_metal = make_shared<metal>(Colour(0.4, 0.2, 0.1),0.0);
    for (uint32_t i = 0; i < model->nfaces(); ++i) {
        const Vec3f& v0 = model->vert(model->face(i)[0]);
        const Vec3f& v1 = model->vert(model->face(i)[1]);
        const Vec3f& v2 = model->vert(model->face(i)[2]);

        const Vec3f& n0 = model->norm(model->vNorms(i)[0]);
        const Vec3f& n1 = model->norm(model->vNorms(i)[1]);
        const Vec3f& n2 = model->norm(model->vNorms(i)[2]);


        // const Vec3f& n = model->norm(i);
        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, n0, n1, n2, mat_metal));
        //world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform,n, mat_metal));
    }

   auto light1 = make_shared<diffuse_light>(Colour(255, 255, 255));
   world.add(make_shared<Sphere>(Point3f(0, 5, 0), 1.0, light1));


    auto ground_material = make_shared<lambertian>(Colour(0.5, 0.5, 0.5));
    world.add(make_shared<Sphere>(Point3f(0, -1000, 0), 1000, ground_material));

    //return world;
    return hittable_list(make_shared<bvh_node>(world));
}

template<typename T>
void AddtoWorld(Model* model, T material, Vec3f transform, hittable_list& world)
{
    //std::string location
    //Model* model = new Model(location);
    for (uint32_t i = 0; i < model->nfaces(); ++i) {
        const Vec3f& v0 = model->vert(model->face(i)[0]);
        const Vec3f& v1 = model->vert(model->face(i)[1]);
        const Vec3f& v2 = model->vert(model->face(i)[2]);

        const Vec3f& n0 = model->norm(model->vNorms(i)[0]);
        const Vec3f& n1 = model->norm(model->vNorms(i)[1]);
        const Vec3f& n2 = model->norm(model->vNorms(i)[2]);

        // const Vec3f& n = model->norm(i);
        world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform, n0, n1, n2, material));
        //world.add(make_shared<triangle>(v0 + transform, v1 + transform, v2 + transform,n, mat_diffuse));
    }
}

hittable_list main_scene() {
    hittable_list world;

    Vec3f transCarlo(-6.357, 35.555, -4.835);
    Vec3f transform(0, 0, 0);

    Model* carlo = new Model("cc.obj");

    Model* back_vase = new Model("Assets/back_vase.obj");
    Model* front_vase = new Model("Assets/front_vase.obj");

    Model* flowertop1 = new Model("Assets/flowertop1.obj");
    Model* flowertop2 = new Model("Assets/flowertop2.obj");

    Model* rosestem1 = new Model("Assets/rosestem1.obj");
    Model* rosestem2 = new Model("Assets/rosestem2.obj");

    Model* bowl= new Model("Assets/bowl.obj");
    Model* glass = new Model("Assets/glass2.obj");   

    Model* table = new Model("Assets/table.obj");
    Model* background = new Model("Assets/background.obj");

    Model* orange1 = new Model("Assets/orange1.obj");
    Model* orange2 = new Model("Assets/orange2.obj");
    Model* orange3 = new Model("Assets/orange3.obj");
    Model* orange4 = new Model("Assets/orange4.obj");
    Model* orange5 = new Model("Assets/orange5.obj");
    Model* orange6 = new Model("Assets/orange6.obj");
    Model* orange7 = new Model("Assets/orange7.obj");
    Model* orange8 = new Model("Assets/orange8.obj");
    Model* orange9 = new Model("Assets/orange9.obj");
    Model* orange10 = new Model("Assets/orange10.obj");


   
    // materials
    auto mat_bluevase = make_shared<lambertian>(Colour(0, 0.4, 1));
    auto mat_redvase = make_shared<lambertian>(Colour(0.4,0,0));

    auto mat_roseflower = make_shared<lambertian>(Colour(1, 0.07, 0.07));
    auto mat_rosestem = make_shared<lambertian>(Colour(0.15, 0.27, 0.13));

    auto mat_table = make_shared<lambertian>(Colour(0.4, 0.2, 0.1));
    auto mat_background = make_shared<lambertian>(Colour(0.4, 0.2, 0.1));

    auto mat_metalBowl = make_shared<metal>(Colour(0.50288, 0.50288, 0.50288),0.2);

    auto mat_glass = make_shared<dielectric>(1.5f);
    //orange
    auto mat_orange = make_shared<lambertian>(Colour(1, 0.55, 0));

   


    AddtoWorld(back_vase, mat_bluevase, transform, world);
    AddtoWorld(front_vase, mat_redvase, transform, world);

    AddtoWorld(flowertop1, mat_roseflower, transform, world);
    AddtoWorld(flowertop2, mat_roseflower, transform, world);

    AddtoWorld(rosestem1, mat_rosestem, transform, world);
    AddtoWorld(rosestem2, mat_rosestem, transform, world);

    AddtoWorld(table, mat_table, transform, world);
    AddtoWorld(background, mat_background, transform, world);

    AddtoWorld(bowl, mat_metalBowl, transform, world);

    AddtoWorld(glass, mat_glass, transform, world);

    AddtoWorld(orange1, mat_orange, transform, world);
    AddtoWorld(orange2, mat_orange, transform, world);
    AddtoWorld(orange3, mat_orange, transform, world);
    AddtoWorld(orange4, mat_orange, transform, world);
    AddtoWorld(orange5, mat_orange, transform, world);
    AddtoWorld(orange6, mat_orange, transform, world);
    AddtoWorld(orange7, mat_orange, transform, world);
    AddtoWorld(orange8, mat_orange, transform, world);
    AddtoWorld(orange9, mat_orange, transform, world);
    AddtoWorld(orange10, mat_orange, transform, world);

    

    //AddtoWorld(carlo, mat_diffuse, transCarlo, world);


   auto light1 = make_shared<diffuse_light>(Colour(255, 255, 255));
    world.add(make_shared<Sphere>(Point3f(-11, 71, -9), 4, light1));


    /*auto ground_material = make_shared<lambertian>(Colour(0.5, 0.5, 0.5));
    world.add(make_shared<Sphere>(Point3f(0, -1000, 0), 1000, ground_material));*/

    
    return hittable_list(make_shared<bvh_node>(world));
}

void LineRender(SDL_Surface* screen, hittable_list world, int y, int spp, int max_depth, camera* cam) {
    const float aspect_ratio = 16.0 / 9;
    const int image_width = screen->w;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    const Colour black(0, 0, 0);
    Colour pix_col(black);
    Colour backgroundCol(0, 0, 0);

    for (int x = 0; x < screen->w; ++x) {
        pix_col = black;
        for (int s = 0; s < spp; s++) {
            auto u = double(x + random_double()) / (image_width - 1);
            auto v = double(y + random_double()) / (image_height - 1);
            Ray ray = cam->get_ray(u, v);
            Vec3f unit_direction = ray.direction().normalize();
            auto t = 0.5 * (unit_direction.y + 1.0);
            backgroundCol = (1.0 - t) * Colour(1.0, 1.0, 1.0) + t * Colour(0.5, 0.7, 1.0) * 255;
            pix_col = pix_col + ray_colour(ray, backgroundCol, world, max_depth);
        }
        pix_col /= 255.f * spp;
        pix_col.x = sqrt(pix_col.x);
        pix_col.y = sqrt(pix_col.y);
        pix_col.z = sqrt(pix_col.z);
        pix_col *= 255;

        Uint32 colour = SDL_MapRGB(screen->format, pix_col.x, pix_col.y, pix_col.z);
        putpixel(screen, x, y, colour);

        TGAColor tgaCol(pix_col.x, pix_col.y, pix_col.z, 255);
        imageOut.set(x, y, tgaCol);

       
    }
}

int main(int argc, char** argv)
{
    init();
    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = screen->w;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    //const int spp = 4;

    const float scale = 1.f / spp;
       

    const Colour white(255, 255, 255);
    const Colour black(0, 0, 0);
    const Colour red(255, 0, 0);

    
    // World
    
   auto world = main_scene();
   // auto world = random_scene();
    
    /*hittable_list world;
    auto material_ground = make_shared<lambertian>(Colour(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(Colour(0.1, 0.2, 0.5));
    auto material_left = make_shared<dielectric>(1.5);
    auto material_right = make_shared<metal>(Colour(0.8, 0.6, 0.2), 0.0);

    world.add(make_shared<Sphere>(Point3f(0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<Sphere>(Point3f(0.0, 0.0, -1.0), 0.5, material_center));
    world.add(make_shared<Sphere>(Point3f(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<Sphere>(Point3f(-1.0, 0.0, -1.0), -0.45, material_left));
    world.add(make_shared<Sphere>(Point3f(1.0, 0.0, -1.0), 0.5, material_right));*/

    
    // Camera
    Point3f lookfrom(-34.0993f, 43.4304f, -2.5941f);
    Point3f lookat(0, 34.994f, -7.0f);

    

    /*Point3f lookfrom(0, 1, 10);
    Point3f lookat(0, 1, -0);*/

    Vec3f vup(0, 1, 0);
    auto dist_to_focus = 35.0;
    auto aperture = 0.0;
    camera cam(lookfrom,lookat, vup, 32, aspect_ratio, aperture, dist_to_focus);

    double t;
    Colour pix_col(black);

    SDL_Event e;
    bool running = true;

    

    while (running) {       

        auto t_start = std::chrono::high_resolution_clock::now();

        // clear back buffer, pixel data on surface and depth buffer (as movement)
        SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_RenderClear(renderer);

        {
            // start of the thre4ad pooling 
            t_start = std::chrono::high_resolution_clock::now();
            ThreadPool pool(std::thread::hardware_concurrency());

            int start = screen->h - 1;
            int step = screen->h / std::thread::hardware_concurrency();
            for (int y = 0; y < screen->h - 1; y++)
            {
                //std::cerr << "\rY: " << y << std::flush;
                pool.Enqueue(std::bind(LineRender, screen, world, y, spp, max_depth, &cam));
            }
        }

        // TGA image 
        imageOut.flip_vertically();
        imageOut.write_tga_file("RaytraceOutput.tga");

        



        /// no multi threading 
        //for (int y = screen->h - 1; y >= 0; --y) {
        //    std::cerr << "\rScanlines remaining: " << y << std::flush;
        //    for (int x = 0; x < screen->w; ++x) {
        //        pix_col = black; // reset colour per pixel
        //        for (int s = 0; s < spp; s++) {
        //            auto u = double(x + random_double()) / (image_width - 1);
        //            auto v = double(y + random_double()) / (image_height - 1);
        //            Ray ray = cam.get_ray(u, v);
        //            // accumulate colours for every sample
        //            pix_col = pix_col + ray_colour(ray, world,max_depth);
        //        }
        //        // Gamma Correction
        //        pix_col /= 255.f * spp;
        //        pix_col.x = sqrt(pix_col.x);
        //        pix_col.y = sqrt(pix_col.y);
        //        pix_col.z = sqrt(pix_col.z);
        //        pix_col *= 255;
        //        sqrt(pix_col.x);
        //        // scale cumulative colour values accordingly to spp
        //        Uint32 colour = SDL_MapRGB(screen->format, pix_col.x, pix_col.y, pix_col.z);
        //        putpixel(screen, x, y, colour);

        //        out << (int)pix_col.x << ' '
        //            << (int)pix_col.y << ' '
        //            << (int)pix_col.z << '\n';
        //    }
        //}

        auto t_end = std::chrono::high_resolution_clock::now();
        auto passedTime = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        std::cerr << "Frame render time:  " << passedTime << " ms" << std::endl;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, screen);
        if (texture == NULL) {
            fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
            exit(1);
        }
        SDL_FreeSurface(screen);
        SDL_RenderCopyEx(renderer, texture, nullptr, nullptr, 0, 0, SDL_FLIP_VERTICAL);
        //SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(texture);

        if (SDL_PollEvent(&e))
        {
            switch (e.type) {
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                }
                break;
            }
        }
    }

    return 0;
}
