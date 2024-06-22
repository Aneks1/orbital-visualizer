#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <random>
#include <iostream>

const float PI = 3.14159265358979323846;
const float a0 = 1.0; // Bohr radius
const int num_points = 100000;

// Function to compute the probability density of the 2p_z orbital
float probability_density_2p_z(float r, float theta) {
    float normalization = 1.0 / (32.0 * PI * pow(a0, 5));
    float radial_part = r * r * exp(-r / a0);
    float angular_part = cos(theta) * cos(theta);
    return normalization * radial_part * angular_part;
}

// Generate random points based on the probability density
std::vector<GLfloat> generate_points_2p_z() {
    std::vector<GLfloat> points;
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0, 1.0);
    std::uniform_real_distribution<float> dist_angle(0.0, 2 * PI);

    for (int i = 0; i < num_points; ++i) {
        float r = -a0 * log(dist(rng));
        float theta = acos(1 - 2 * dist(rng));
        float phi = dist_angle(rng);

        float probability = probability_density_2p_z(r, theta);
        float max_density = 1.0 / (32.0 * PI * pow(a0, 5)) * pow(2.0 * a0, 2) * exp(-2.0);

        if (dist(rng) <= probability / max_density) {
            float x = r * sin(theta) * cos(phi);
            float y = r * sin(theta) * sin(phi);
            float z = r * cos(theta);

            points.push_back(x);
            points.push_back(y);
            points.push_back(z);
        }
    }
    return points;
}

void render_points(const std::vector<GLfloat>& points) {
    glEnable(GL_POINT_SMOOTH);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < points.size(); i += 3) {
        glVertex3f(points[i], points[i + 1], points[i + 2]);
    }
    glEnd();
}

void check_gl_error(const char* statement, const char* file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << error << " after " << statement
                  << " at " << file << ":" << line << std::endl;
    }
}

#define CHECK_GL_ERROR() check_gl_error(__FUNCTION__, __FILE__, __LINE__)

float rotationX = 0.0f;
float rotationY = 0.0f;
bool mouseDragging = false;
double lastMousePosX, lastMousePosY;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouseDragging = true;
            glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
        } else if (action == GLFW_RELEASE) {
            mouseDragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (mouseDragging) {
        rotationY += (xpos - lastMousePosX) * 0.1f;
        rotationX += (ypos - lastMousePosY) * 0.1f;
        lastMousePosX = xpos;
        lastMousePosY = ypos;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        const char* description;
        glfwGetError(&description);
        std::cerr << "Error: " << description << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Schrödinger's Wave Function Visualization", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    std::vector<GLfloat> points = generate_points_2p_z();
    CHECK_GL_ERROR();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECK_GL_ERROR();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
        CHECK_GL_ERROR();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(3.0, 3.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        CHECK_GL_ERROR();

        // Apply rotations
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

        render_points(points);
        CHECK_GL_ERROR();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
