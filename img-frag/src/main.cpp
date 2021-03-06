// STL
#include <iostream>
#include <unordered_set>
#include <climits>
#include <chrono>
#include <thread>
#include <filesystem>

// OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// TCLAP
#include <tclap/CmdLine.h>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "ShaderProgram.h"
#include "MathUtil.h"
#include "Webcam.h"
#include "types.h"

// Texture units
#define IMG_UNIT 0
#define IMG_UNIT_GL GL_TEXTURE0
#define LAST_OUTPUT_UNIT 1
#define LAST_OUTPUT_UNIT_GL GL_TEXTURE1
#define CAM_UNIT 2
#define CAM_UNIT_GL GL_TEXTURE2

// For ping pong rendering
#define SRC 0
#define DEST 1

// Load an image, return texture id
void populateTexture(GLenum texture_unit, GLuint tex_id, cv::Mat& frame) {
    // Store the previous active texture so we can revert to it
    GLint prev_active = 0;
    glGetIntegeri_v(GL_ACTIVE_TEXTURE, 1, &prev_active);

    cv::Size size = frame.size();

    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.width, size.height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);

    // Restore active texture
    glActiveTexture(prev_active);
}

void saveTexture(GLuint tex_id, const std::string& path="") {
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

    // Store the previous active texture so we can revert to it
    GLint prev_active = 0;
    glGetIntegeri_v(GL_ACTIVE_TEXTURE, 1, &prev_active);

    glBindTexture(GL_TEXTURE_2D, tex_id);

    GLint alignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);

    GLint width;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);

    GLint height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    // Load the actual image daata
    char* data = new char[width * height * 3];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Restore active texture
    glActiveTexture(prev_active);

    cv::Mat image(height, width, CV_8UC3, data);

    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    flip(image, image, 0);
    cv::imwrite(dest, image);
}

// GLFW error callback
void onError(int errc, const char* desc) {
    std::cerr << "Error (" << std::to_string(errc) << "): " << std::string(desc) << std::endl;
}

// GLFW window resizing callback
void onWindowSize(GLFWwindow* /* window */, int width, int height) {
    // Resize the view port when a window resize is requested
    glViewport(0,0, width, height);
}

// Globals EVIL, please refactor this.
GLuint output_texs[2] = {};
std::string img_out;

// GLFW key press callback
void onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_P:
                saveTexture(output_texs[SRC], img_out);
                break;
        }
    }
}

int main(int argc, const char** argv) {
    TCLAP::CmdLine cmd("Art! Woo!");

    TCLAP::ValueArg<std::string> vert_arg("", "vert", "path to vertex shader", false, "vert.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> frag_arg("", "frag", "path to fragment shader", false, "frag.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> img_arg("i", "img", "texture image path", false, "", "string", cmd);
    TCLAP::ValueArg<std::string> img_out_arg("o", "image-out", "output image path", false, "", "string", cmd);
    TCLAP::ValueArg<int> height_arg("", "height", "window height (width will be calculated automatically)", false, 720, "int", cmd);
    TCLAP::ValueArg<int> cam_arg("c", "cam", "camera device id", false, 0, "int", cmd);
    TCLAP::ValueArg<int> pause_arg("p", "pause", "miliseconds to pause between frames", false, 0, "int", cmd);
    TCLAP::ValueArg<int> stages_arg("s", "stages", "number of stages", false, 1, "int", cmd);
    TCLAP::ValueArg<double> cam_fps_arg("", "cam-fps", "FPS to set for cam", false, 0, "int", cmd);
    TCLAP::ValueArg<int> cam_width_arg("", "cam-width", "width of camera resolution", false, 0, "int", cmd);
    TCLAP::ValueArg<int> cam_height_arg("", "cam-height", "height of camera resolution", false, 0, "int", cmd);
    TCLAP::SwitchArg debug_timer_arg("", "debug-timer", "debug time between frames", cmd);
    TCLAP::SwitchArg full_arg("", "full", "maximized, no titlebar", cmd);

    // Parse command line arguments
    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    img_out = img_out_arg.getValue();

    // Resolution of our image
    cv::Size resolution(0, 0);

    // Load the image
    cv::Mat image;
    if (img_arg.isSet()) {
        const std::string img_path = img_arg.getValue();
        image = cv::imread(img_path);
        if (image.empty()) {
            std::cerr << "error: unable to load image " << img_path << std::endl;
            return 1;
        }

        resolution = image.size();

        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        flip(image, image, 0);
    }

    // Load the webcamera
    std::unique_ptr<Webcam> cam = nullptr;
    cv::Mat cam_image;
    if (cam_arg.isSet()) {
        if (resolution.width == 0) {
            resolution.width = cam_width_arg.getValue();
            resolution.height = cam_height_arg.getValue();
        }
        cam = std::make_unique<Webcam>(cam_arg.getValue(), cam_fps_arg.getValue(), resolution);
        Error err = cam->start();
        if (!err.empty()) {
            std::cerr << "error: failed to load webcam: " << err << std::endl;
            return 1;
        }

        cam->read(cam_image);
        resolution = cam_image.size();
    }

    if (resolution.width == 0) {
        resolution.width = 640;
        resolution.height = 480;
    }

    // Set GLFW error callback
    glfwSetErrorCallback(onError);

    // Initialize the GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW!!!\n");
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

    // Load image into texture
    GLuint image_id = GL_FALSE;
    glGenTextures(1, &image_id);
    populateTexture(IMG_UNIT_GL, image_id, image);

    // Load first frame of webcam image
    GLuint cam_image_id = GL_FALSE;
    glGenTextures(1, &cam_image_id);
    populateTexture(CAM_UNIT_GL, cam_image_id, cam_image);

    // Configure shader program. We will check for errors once in our run loop
    auto program = std::make_unique<ShaderProgram>();
    program->registerShader(GL_VERTEX_SHADER, vert_arg.getValue());
    program->registerShader(GL_FRAGMENT_SHADER, frag_arg.getValue());

    // Bind vertex array object
    GLuint vao = GL_FALSE;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Copy indices into element buffer
    GLuint ebo;
    glGenBuffers(1, &ebo);
    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Copy position vetex attributes
    GLuint pos_vbo = GL_FALSE;
    GLfloat pos[] = {
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create and bind the frame buffer we will be rendering to
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Our textures for ping-ponging
    GLuint draw_bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glGenTextures(2, output_texs);
    for (const auto& id : output_texs) {
        glBindTexture(GL_TEXTURE_2D, id);

        // Give an empty image to OpenGL ( the last "0" )
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution.width, resolution.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Bind the textures to the frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, draw_bufs[SRC], output_texs[SRC], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, draw_bufs[DEST], output_texs[DEST], 0);

    // Our run loop
    std::string last_err = "";
    std::unordered_set<std::string> last_warnings;
    bool first_pass = true;
    unsigned int iter = 0;
    int stages = stages_arg.getValue();
    double last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // We will be treating this like an int when passing to shader
        iter = (iter + 1) % INT_MAX;

        double time = glfwGetTime();

        // Reload shaders if needed and report any errors
        std::string err = program->update().value_or("");
        if (err != "") {
            if (err != last_err) {
                std::cout << "ERROR: " << err << std::endl;
            }
        } else if (last_err != "") {
            std::cout << "- No more errors! -" << std::endl;
        }
        last_err = err;

        for (int i=0; i < stages; i++) {
            program->setUniform("stage", [i](GLint& id) {
                glUniform1i(id, i);
            });

            program->setUniform("lastStage", [stages](GLint& id) {
                glUniform1i(id, stages - 1);
            });

            // Use our FBO and destination buffer.
            // We'll write to this before rendering to screen.
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glDrawBuffer(draw_bufs[DEST]);

            // Load our shader program
            GLuint program_id = program->getProgram();
            glUseProgram(program_id);

            // Load the webcam frame if we're using a webcam
            if (cam != nullptr) {
                if (cam->read(cam_image)) {
                    populateTexture(CAM_UNIT_GL, cam_image_id, cam_image);
                }

                program->setUniform("cam0", [cam_image_id](GLint& id) {
                    glActiveTexture(CAM_UNIT_GL);
                    glBindTexture(GL_TEXTURE_2D, cam_image_id);
                    glUniform1i(id, CAM_UNIT);
                });
            }

            // Set the image if we're using an image
            if (!image.empty()) {
                program->setUniform("img0", [image_id](GLint& id) {
                    glActiveTexture(IMG_UNIT_GL);
                    glBindTexture(GL_TEXTURE_2D, image_id);
                    glUniform1i(id, IMG_UNIT);
                });
            }

            program->setUniform("iTime", [time](GLint& id) {
                glUniform1f(id, static_cast<float>(time));
            });

            program->setUniform("iResolution", [resolution](GLint& id) {
                glUniform2f(
                    id,
                    static_cast<float>(resolution.width),
                    static_cast<float>(resolution.height)
                );
            });

            program->setUniform("frame", [iter](GLint& id) {
                glUniform1i(id, static_cast<int>(iter));
            });

            program->setUniform("firstPass", [first_pass](GLint& id) {
                glUniform1i(id, first_pass);
            });

            program->setUniform("lastOut", [](GLint& id) {
                glActiveTexture(LAST_OUTPUT_UNIT_GL);
                glBindTexture(GL_TEXTURE_2D, output_texs[SRC]);
                glUniform1i(id, LAST_OUTPUT_UNIT);
            });


            glViewport(0,0, resolution.width, resolution.height);

            // Draw our vertices
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Swap the ping pong buffer!
            std::swap(draw_bufs[SRC], draw_bufs[DEST]);
            std::swap(output_texs[SRC], output_texs[DEST]);

            // Calculate blit settings
            int win_width, win_height;
            glfwGetWindowSize(window, &win_width, &win_height);
            DrawInfo draw_info = DrawInfo::scaleCenter(
                static_cast<float>(resolution.width),
                static_cast<float>(resolution.height),
                static_cast<float>(win_width),
                static_cast<float>(win_height)
            );

            // Draw to the screen
            glDrawBuffer(GL_BACK);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glReadBuffer(draw_bufs[SRC]);
            glViewport(0,0, win_width, win_height);
            glBlitFramebuffer(
                0,0, resolution.width, resolution.height,
                draw_info.x0, draw_info.y0, draw_info.x1, draw_info.y1,
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                GL_NEAREST
            );

            // Show buffer
            glfwSwapBuffers(window);
            first_pass = false;
        }

        // Report on shader related warnings
        std::unordered_set<std::string> warnings = program->getWarnings();
        if (warnings != last_warnings) {
            if (warnings.empty()) {
                std::cout << "- No more warnings! -" << std::endl;
            } else {
                for (const auto warning : warnings) {
                    std::cout << "WARNING: " << warning << std::endl;
                }
            }

            last_warnings = warnings;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(pause_arg.getValue()));

        if (debug_timer_arg.getValue()) {
            double current_time = glfwGetTime();
            std::cout << "FPS: " << current_time - last_time << std::endl;
            last_time = glfwGetTime();
        }
    }

    return 0;
}
