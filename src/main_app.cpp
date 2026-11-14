// Пример вывода массива пикселей в окно в реальном времени.
//
// Каждый кадр программа на CPU заполняет буфер RGBA8 цветом, который плавно
// меняется со временем, и выводит его в окно через glDrawPixels. OpenGL здесь
// используется только для того, чтобы показать готовый массив пикселей.
// Раз в секунду в консоль печатается среднее время кадра.

#include <GLFW/glfw3.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>


#include "vectors.h"
#include "Renderer.h"


static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

static float clamp01(float x)
{
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

static uint8_t to_u8(float x)
{
    return (uint8_t)(clamp01(x) * 255.0f + 0.5f);
}

// Цвет радуги по оттенку hue из [0, 1)
static void hue_to_rgb(float hue, float rgb[3])
{
    const float h = hue * 6.0f;
    rgb[0] = clamp01(fabsf(h - 3.0f) - 1.0f);
    rgb[1] = clamp01(2.0f - fabsf(h - 2.0f));
    rgb[2] = clamp01(2.0f - fabsf(h - 4.0f));
}

struct {
    //mode swith
    RENDER_MODE modes[4] = { RENDER_MODE::LAMBERT, RENDER_MODE::DEPTH, RENDER_MODE::NORMALS, RENDER_MODE::UV,};
    int modes_size = sizeof(modes) / sizeof(RENDER_MODE);
    int mode = 0;

    //mouse
    struct {
        float dx = 0;
        float dy = 0;
        float last_x = 0;
        float last_y = 0;
        bool first = true;
    } mouse;
    

} HInfo;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        HInfo.mode = (HInfo.mode + 1) % HInfo.modes_size;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    
}

static void cursor_callback(GLFWwindow* w, double x, double y) {
    if (HInfo.mouse.first) {
        HInfo.mouse.last_x = x;
        HInfo.mouse.last_y = y;
        HInfo.mouse.first  = false;
        return;
    }

    HInfo.mouse.dx += (float)(x - HInfo.mouse.last_x);
    HInfo.mouse.dy += (float)(y - HInfo.mouse.last_y);
    HInfo.mouse.last_x = x;
    HInfo.mouse.last_y = y;
}

enum class ResolutionMode{
        NO_SCALE,
        INT_SCALE,
        DIM_SCALE,
};

int main(int argc, char **argv)
{
    int r_width = argc > 1 ? atoi(argv[1]) : 640;
    int r_height = argc > 2 ? atoi(argv[2]) : 480;

    const float scale = (argc == 4) ? atof(argv[3]) : 0;

    int width = argc > 4 ? atoi(argv[3]) : 640;
    int height = argc > 4 ? atoi(argv[4]) : 480;

    if (argc > 5 || width <= 0 || height <= 0)
    {
        printf("Usage: %s [r_width] [r_height] [width] [height] \n", argv[0]);
        return 1;
    } 

    

    if (scale > 0) { 
        width = std::ceil(r_width * scale);
        height = std::ceil(r_height * scale);

    }

   

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Размер буфера пикселей жёстко связан с размером окна, поэтому менять его нельзя
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(width, height, "Software Renderer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    // 0 - не ждать вертикальной синхронизации: время кадра показывает реальную
    // скорость программы, а не частоту монитора. 1 - включить vsync.
    glfwSwapInterval(0);

    // Число пикселей окна может отличаться от width x height (масштабирование
    // экрана в ОС), поэтому размер буфера берём у самого окна.
    int fb_width = 0, fb_height = 0;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    // RGBA8, строки снизу вверх: первая строка буфера - нижняя строка окна
    const size_t n_window_pixels = (size_t)fb_width * fb_height;
    std::vector<uint8_t> window_pixels;
    window_pixels.resize(4 * n_window_pixels);

    //resolution_mode
    ResolutionMode resolution_mode = ResolutionMode::NO_SCALE;

    const size_t n_render_pixels = (size_t)r_width * r_height;
    std::vector<uint8_t> render_pixels;
    render_pixels.resize(n_render_pixels * 4);

    int int_scale = (int) scale;
    if ( width == r_width && height == r_height) {
         resolution_mode = ResolutionMode::NO_SCALE;
    } else if (width == int_scale * r_width && height == int_scale * r_height) {
        resolution_mode = ResolutionMode::INT_SCALE;
    } else {
        resolution_mode = ResolutionMode::DIM_SCALE;
    } 

    std::cout << (int) resolution_mode << "\n";

    //input handeling
    glfwSetKeyCallback(window, key_callback);


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetCursorPosCallback(window, cursor_callback);

    const float mouse_sensitivity = 0.0025f;
    const float pitch_limit       = 3.12f / 2;

    

    // scene creation
    std::vector<Model> models;

    Mesh b = Mesh("resources/Bunny.obj");
    Mesh m = Mesh("resources/Roman Centurion.obj");

    Model b1 = Model(b, "resources/textures/porcelain.jpg", 640, 480).scale(vec3(1, 1, 1));
    b1.id = 0;
    models.push_back(b1);
    
    Model m1 = Model(m).scale(vec3(2, 0.8, 1)).translate(vec3(-1, 1, 0));
    m1.base_color = vec3(1, 1, 1);
    m1.id = 0;
    models.push_back(m1);

    Model m2 = Model(m).translate(vec3(1, -1)).scale(vec3(0.3, 0.3, 0.3));
    m2.id = 1;
    models.push_back(m2);
 

    int buf_size = 0;
    for (auto model: models) {
        buf_size = std::max(model.mesh.num_vertices, buf_size);
    }

    
    //renderer

    Renderer renderer = Renderer(render_pixels, r_width, r_height, buf_size);
    renderer.PSI.light_direction = vec3(0, 1, 1).norm();
    
    Camera &cam = renderer.cam;
    float cam_speed = 1;

    double prev_time = glfwGetTime();
    double report_time = prev_time;
    int report_frames = 0;

    float angle_speed = 3.14 / 5;


    // main loop
    while (!glfwWindowShouldClose(window))
    {
        const double now = glfwGetTime();
        const float dt = (float)(now - prev_time);
        prev_time = now;


        // camera
        vec3 f = (cam.target - cam.eye).norm();
        vec3 r = cross(f, cam.up).norm();
        vec3 move = vec3(0);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move = move + f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move = move - f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move = move - r;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move = move + r;

        if (move.len() > 1e-6f) {
            move = move.norm() * cam_speed * dt;
            
        }

        cam.pitch -= HInfo.mouse.dy * mouse_sensitivity;
        cam.yaw += HInfo.mouse.dx * mouse_sensitivity;
        HInfo.mouse.dy = 0;
        HInfo.mouse.dx = 0;

        f = vec3(cosf(cam.pitch) * cosf(cam.yaw),
            sinf(cam.pitch),
            cosf(cam.pitch) * sinf(cam.yaw));

        if (f.len() > 1e-6) {
            f.norm();
        }

        cam.pitch  = std::clamp(cam.pitch, -pitch_limit, pitch_limit);
        if (cam.yaw >  3.14f) cam.yaw -= 2 * 3.14f;
        if (cam.yaw < -3.14f) cam.yaw += 2 * 3.14f;

        cam.look_at(cam.eye + move, cam.eye + f + move, cam.up);


        // models
        models[0].rotation_v[1] += angle_speed * dt;
        models[0].update_M();


        // actual rendering
        renderer.render_scene(models, HInfo.modes[HInfo.mode]);

        //upscaling
        if (resolution_mode == ResolutionMode::NO_SCALE) {
            glDrawPixels(fb_width, fb_height, GL_RGBA, GL_UNSIGNED_BYTE, render_pixels.data());

        } else if (resolution_mode == ResolutionMode::INT_SCALE) {
            std::vector<uint32_t> expanded_row(fb_width);

            for (int y = 0; y < r_height; ++y) {
                const uint32_t* src_row = reinterpret_cast<const uint32_t*>(
                    render_pixels.data() + (size_t)y * r_width * 4);

                for (int x = 0; x < r_width; ++x) {
                    for (int s = 0; s < int_scale; ++s) {
                        expanded_row[x * int_scale + s] = src_row[x];
                    } 
                }

                for (int s = 0; s < int_scale; ++s) {
                    size_t dst_offset = ((size_t)(y * int_scale + s) * fb_width) * 4;

                    std::memcpy(window_pixels.data() + dst_offset,
                                expanded_row.data(),
                                (size_t)fb_width * 4);
                }
            }

            glDrawPixels(fb_width, fb_height, GL_RGBA, GL_UNSIGNED_BYTE, window_pixels.data());
        } else if (resolution_mode == ResolutionMode::DIM_SCALE) { 
            
            uint8_t out[4];
            for (int y = 0; y < fb_height; y++) {
                for (int x = 0; x < fb_width; x++) {
                    bilinear_inter(render_pixels.data(), r_width, r_height, (x + 0.5f) / fb_width, (y + 0.5f) / fb_height, out);
                    std::memcpy(window_pixels.data() + (y*fb_width + x) * 4, out, 4);
                }
            }


            glDrawPixels(fb_width, fb_height, GL_RGBA, GL_UNSIGNED_BYTE, window_pixels.data());
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();

        //framerate
        report_frames++;
        const double elapsed = glfwGetTime() - report_time;
        if (elapsed >= 1.0)
        {


            printf("frame time: %.3f ms (%.1f FPS)\n",
                   1000.0 * elapsed / report_frames, report_frames / elapsed);
            fflush(stdout);
            report_time += elapsed;
            report_frames = 0;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}