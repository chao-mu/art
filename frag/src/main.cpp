// STL
#include <iostream>
#include <unordered_set>
#include <climits>
#include <chrono>
#include <thread>
#include <filesystem>

// TCLAP
#include <tclap/CmdLine.h>

// OpenCV
#include <opencv2/opencv.hpp>

// SFML
#include <SFML/Audio.hpp>

// Ours
#include "GLUtil.h"
#include "ShaderProgram.h"
#include "MathUtil.h"
#include "Video.h"
#include "types.h"
#include "midi/Device.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Media.h"
#include "PatchParser.h"
#include "PingPongTexture.h"
#include "GLUtil.h"

#define DEBUG_TIME_DECLARE(name) \
     std::chrono::time_point<std::chrono::high_resolution_clock> start_ ## name; \
     std::chrono::time_point<std::chrono::high_resolution_clock> end_ ## name;

#define DEBUG_TIME_START(name) \
    if (debug_time) { \
        start_##name = std::chrono::high_resolution_clock::now(); \
    }

#define DEBUG_TIME_END(name) \
    if (debug_time) { \
        end_##name = std::chrono::high_resolution_clock::now(); \
        std::cout << "debug-timer " # name << " " << std::chrono::duration<double, std::milli>(end_##name - start_##name).count() << "ms" << std::endl; \
    }

void screenshot(std::shared_ptr<frag::Texture> out, const std::string& path="") {
    std::string dest = path;
    if (dest.empty()) {
        std::stringstream s;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
                ).count();
        std::time_t now = std::time(nullptr);
        s << "output-" << std::put_time(std::localtime(&now), "%Y-%m-%d_") << ms << ".png";
        dest = s.str();
    }

    out->save(dest);
}

// GLFW error callback
void onError(int errc, const char* desc) {
    std::cerr << "Error (" << std::to_string(errc) << "): " << std::string(desc) << std::endl;
}

// GLFW window resizing callback
void onWindowSize(GLFWwindow* /* window */, int width, int height) {
    // Resize the view port when a window resize is requested
    GLCall(glViewport(0,0, width, height));
}

std::shared_ptr<frag::Texture> tex_out;
std::string out_path;

bool flip_playback = false;

// GLFW key press callback
void onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_Q:
                GLCall(glfwSetWindowShouldClose(window, GLFW_TRUE));
                break;
            case GLFW_KEY_R:
                flip_playback = true;
                break;
            case GLFW_KEY_P:
                screenshot(tex_out, out_path);
                break;
        }
    }
}

int main(int argc, const char** argv) {
    TCLAP::CmdLine cmd("Art! Woo!");

    TCLAP::ValueArg<std::string> vert_arg("", "vert", "path to vertex shader", false, "vert.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> patch_arg("i", "patch", "path to yaml patch", false, "patch.yml", "string", cmd);
    TCLAP::ValueArg<std::string> img_out_arg("o", "image-out", "output image path", false, "", "string", cmd);
    TCLAP::ValueArg<std::string> sound_arg("s", "sound-path", "path to sound file", false, "", "string", cmd);
    TCLAP::ValueArg<int> height_arg("", "height", "window height (width will be calculated automatically)", false, 720, "int", cmd);
    TCLAP::ValueArg<int> pause_arg("p", "pause", "miliseconds to pause between frames", false, 0, "int", cmd);
    TCLAP::SwitchArg debug_timer_arg("", "debug-timer", "debug time between frames", cmd);
    TCLAP::SwitchArg full_arg("", "full", "maximized, no titlebar", cmd);

    // Parse command line arguments
    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    bool debug_time = debug_timer_arg.getValue();

    sf::Music music;
    const std::string sound_path = sound_arg.getValue();
    if (!sound_path.empty()) {
        if (!music.openFromFile(sound_path)) {
            // openFromFile displays its own helpful error, so none here
            return 1;
        }
    }

    out_path = img_out_arg.getValue();

    // Set GLFW error callback
    glfwSetErrorCallback(onError);

    // Initialize the GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!!!" << std::endl;
        return 1;
    }

    // OpenGL version and compatibility settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (full_arg.getValue()) {
        glfwWindowHint(GLFW_DECORATED, false);
        glfwWindowHint(GLFW_MAXIMIZED, true);
    }

    frag::PatchParser parser(patch_arg.getValue());
    const frag::Resolution resolution = parser.getResolution();

    float height = static_cast<float>(height_arg.getValue());
    float ratio = static_cast<float>(resolution.height) / height;
    float width = static_cast<float>(resolution.width) / ratio;
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Awesome Art", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    if (full_arg.getValue()) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    // Set window resize callback
    glfwSetWindowSizeCallback(window, onWindowSize);

    // Set key press callback
    glfwSetKeyCallback(window, onKey);

    // Make the context of the specified window current for our current thread
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // Bind vertex array object
    frag::VertexArray vao;
    vao.bind();

    // Copy indices into element buffer
    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    frag::IndexBuffer ebo(indices, 6);
    ebo.bind();

    // Copy position vetex attributes
    GLfloat pos[] = {
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    frag::VertexBuffer pos_vbo(pos, sizeof(pos));
    pos_vbo.bind();

    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

    std::vector<std::shared_ptr<frag::Module>> modules = parser.getModules();
    if (modules.empty()) {
        std::cerr << "No modules specified. Nothing to do." << std::endl;
        return 1;
    }

    std::map<std::string, std::shared_ptr<frag::Media>> media = parser.getMedia();
    std::map<std::string, std::shared_ptr<frag::midi::Device>> controllers = parser.getControllers();
    std::shared_ptr<frag::ValueStore> store = parser.getValueStore();

    std::map<std::string, std::shared_ptr<frag::Media>> modules_output;

    // Initialize values
    for (const auto& mod : modules) {
        mod->compile(store);
        const std::string out_name = mod->getOutput();
        store->set(frag::Address(out_name), mod->getLastOutTex());

        const frag::Resolution res = mod->getResolution();
        store->set(frag::Address(out_name, "resolution"),
                frag::Value(std::vector({static_cast<float>(res.width), static_cast<float>(res.height)})));
    }


    if (!sound_path.empty()) {
        music.play();
    }

    // Our run loop
    unsigned int iter = 0;
    bool first_pass = true;

    DEBUG_TIME_DECLARE(render)
    DEBUG_TIME_DECLARE(loop)
    DEBUG_TIME_DECLARE(draw)

    while (!glfwWindowShouldClose(window)) {
        DEBUG_TIME_START(loop)

        if (flip_playback) {
            for (auto& kv : media) {
                kv.second->flipPlayback();
            }
            flip_playback = false;
        }
        GLCall(glfwPollEvents());

        DEBUG_TIME_START(render)

        // We will be treating this like an int when passing to shader
        iter = (iter + 1) % INT_MAX;
        store->set(frag::Address("iteration"), frag::Value(static_cast<float>(iter)));

        for (const auto& kv : controllers) {
            const std::string& controller_name = kv.first;
            for (const frag::midi::Control& ctrl : kv.second->getControls()) {
                store->set(frag::Address(controller_name, ctrl.name), ctrl);
            }
        }

        for (const auto& kv : media) {
            const std::string name = kv.first;
            std::shared_ptr<frag::Media> m = kv.second;
            store->set(frag::Address(name), m);

            const frag::Resolution res = m->getResolution();

            store->set(frag::Address(name, "resolution"),
                    frag::Value(std::vector({static_cast<float>(res.width), static_cast<float>(res.height)})));
        }


        for (size_t i=0; i < modules.size(); i++) {
            auto& mod = modules.at(i);
            mod->bind();
            mod->setValues(store, first_pass);

            glViewport(0,0, resolution.width, resolution.height);

            // Draw our vertices
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Unbind and swap output/input textures
            mod->unbind();

            store->set(frag::Address(mod->getOutput()), mod->getLastOutTex());
            const frag::Resolution res = mod->getResolution();
            store->set(frag::Address(mod->getOutput(), "resolution"),
                frag::Value(std::vector({static_cast<float>(res.width), static_cast<float>(res.height)})));
        }

        DEBUG_TIME_END(render)

        DEBUG_TIME_START(draw)

        // Calculate blit settings
        int win_width, win_height;
        glfwGetWindowSize(window, &win_width, &win_height);
        DrawInfo draw_info = DrawInfo::scaleCenter(
            static_cast<float>(resolution.width),
            static_cast<float>(resolution.height),
            static_cast<float>(win_width),
            static_cast<float>(win_height)
        );

        auto& mod = modules.back();
        tex_out = mod->getLastOutTex();

        // Draw to the screen
        glDrawBuffer(GL_BACK);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mod->getFBO());
        glReadBuffer(mod->getReadableBuf());
        glViewport(0,0, win_width, win_height);
        glBlitFramebuffer(
            0,0, resolution.width, resolution.height,
            draw_info.x0, draw_info.y0, draw_info.x1, draw_info.y1,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );
        DEBUG_TIME_END(draw)

        // Show buffer
        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(pause_arg.getValue()));

        first_pass = false;

        DEBUG_TIME_END(loop)
    }

    return 0;
}
