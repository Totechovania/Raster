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


RENDER_MODE modes[] = {RENDER_MODE::DEPTH, RENDER_MODE::NORMALS, RENDER_MODE::UV, RENDER_MODE::LAMBERT};
int modes_size = sizeof(modes) / sizeof(RENDER_MODE);
int mode = 0;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        mode = (mode + 1) % modes_size;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    
}



int main(int argc, char **argv)
{
    const int width = argc > 1 ? atoi(argv[1]) : 640;
    const int height = argc > 2 ? atoi(argv[2]) : 480;
    if (argc > 3 || width <= 0 || height <= 0)
    {
        printf("Usage: %s [width] [height]\n", argv[0]);
        return 1;
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
    const size_t n_pixels = (size_t)fb_width * fb_height;
    std::vector<uint8_t> pixels;
    pixels.resize(4 * n_pixels);

    std::vector<Model> models;

    
    Mesh m = load_obj("resources/Bunny.obj").norm();
    models.push_back(Model(m));
    models.push_back(Model(m).scale(vec3(2, 0.8, 1)));

    
 
    Renderer renderer = Renderer(pixels, fb_width, fb_height, m.num_vertices);

    double prev_time = glfwGetTime();
    double report_time = prev_time;
    int report_frames = 0;

    float angle_speed = 3.14 / 5;


    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window))
    {
        const double now = glfwGetTime();
        const float dt = (float)(now - prev_time);
        prev_time = now;



        models[0].rotation_v[1] += angle_speed * dt;
        models[0].update_M();

        renderer.render_scene(models, modes[mode]);



        glDrawPixels(fb_width, fb_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glfwSwapBuffers(window);
        glfwPollEvents();

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