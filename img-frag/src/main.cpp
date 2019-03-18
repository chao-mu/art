// STL
#include <iostream>
#include <unordered_set>
#include <climits>

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

// Texture units
#define IMG_UNIT 0
#define IMG_UNIT_GL GL_TEXTURE0
#define LAST_OUTPUT_UNIT 1
#define LAST_OUTPUT_UNIT_GL GL_TEXTURE1

// For ping pong rendering
#define SRC 0
#define DEST 1

// Load an image, return texture id
GLuint loadImage(cv::Mat& frame, GLenum texture_unit) {
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    flip(frame, frame, 0);

    // Store the previous active texture so we can revert to it
    GLint prev_active = 0;
    glGetIntegeri_v(GL_ACTIVE_TEXTURE, 1, &prev_active);

    GLuint tex_id;
    glGenTextures(1, &tex_id);

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

    return tex_id;
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

// GLFW key press callback
void onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_P:
                break;
        }
    }
}

int main(int argc, const char** argv) {
    TCLAP::CmdLine cmd("Art! Woo!");

    TCLAP::ValueArg<std::string> vert_arg("", "vert", "path to vertex shader", false, "vert.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> frag_arg("", "frag", "path to fragment shader", false, "frag.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> img_arg("i", "img", "texture image path", false, "", "string", cmd);
    TCLAP::ValueArg<int> height_arg("", "height", "window height (width will be calculated automatically)", false, 720, "int", cmd);

    // Parse command line arguments
    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    // Load the image provided to us on the command line
    const std::string img_path = img_arg.getValue();
    cv::Mat image = cv::imread(img_path);
    if (image.empty()) {
        std::cerr << "error: unable to load image " << img_path << std::endl;
        return 1;
    }
    cv::Size resolution = image.size();

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

    float height = static_cast<float>(height_arg.getValue());
    float ratio = static_cast<float>(resolution.height) / height;
    float width = static_cast<float>(resolution.width) / ratio;
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Awesome Art", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
        return 1;
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
    GLuint image_id = loadImage(image, IMG_UNIT_GL);

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
    GLuint output_texs[2] = {};
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

        // Use our FBO and destination buffer.
        // We'll write to this before rendering to screen.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glDrawBuffer(draw_bufs[DEST]);

        // Load our shader program
        GLuint program_id = program->getProgram();
        glUseProgram(program_id);

        program->setUniform("img0", [image_id](GLint& id) {
            glActiveTexture(IMG_UNIT_GL);
            glBindTexture(GL_TEXTURE_2D, image_id);
            glUniform1i(id, 0);
        });

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

        program->setUniform("lastOut", [output_texs](GLint& id) {
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

        // Show buffer
        glfwSwapBuffers(window);
        first_pass = false;
    }

    return 0;
}
