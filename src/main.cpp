#include "utils.h"
// Include standard headers from OpenGL
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
// Include GLM core features
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// Include standard headers from C++
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <unistd.h>

// Window size
int screen_width = 800, screen_height = 800;
GLint vModel_uniform, vView_uniform, vProjection_uniform, vColor_uniform, vCam_uniform;
glm::mat4 modelT, viewT, projectionT;

double oldX, oldY, currentX, currentY;
bool isDragging = false;
bool is_ok = false;
// Variables for the rgba values of the transfer function
float r, g, bl;
float alpha = 0.0;

void createBoundingbox(unsigned int &, unsigned int &);
void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &);
glm::vec3 getTrackBallVector(double x, double y);
void setUniforms(unsigned int &);
glm::vec4 camPos = glm::vec4(0, 0, 280.0, 1.0);
float a = 256, b = 256, c = 256;
const int volume_size = a * b * c;
// Volume size = 256 * 256 * 256
float step_size = 1;
// Load volume data into this array
GLubyte *volume_data = new GLubyte[volume_size];
// Array for the transfer function
GLfloat *transfer_function = new GLfloat[1024]; // 256 * 4
// create a transfer function with 256*4 size since we have 256 values and each have 4 values for RGBA where A is alpha and R is red, G is green and B is blue
GLuint VAO, transferfun, texture3d;

int main(int, char **)
{
    // Setup window
    GLFWwindow *window = setupWindow(screen_width, screen_height);
    ImGuiIO &io = ImGui::GetIO(); // Create IO object

    ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    // Setup ImGui binding
    // Read volume data from file and create a 3D texture
    const char *location = "./data/foot.raw";
    {
        FILE *file = fopen(location, "rb");
        if (NULL == file)
        {
            fprintf(stderr, "Error opening file\n");
            exit(0);
        }
        fread(volume_data, sizeof(GLubyte), volume_size, file);
        fclose(file);
    }
    // Transfer function
    // To do

    for (int i = 0; i < 256; i++)
    {

        transfer_function[i * 4] = float(i) / 255.0;
        transfer_function[i * 4 + 1] = float(i) / 255.0;
        transfer_function[i * 4 + 2] = float(i) / 255.0;
        transfer_function[i * 4 + 3] = 1;
    }

    unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
    glUseProgram(shaderProgram);

    glGenTextures(1, &texture3d);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture3d);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, a, b, c, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, volume_data);
    // create a texture with the volume_data
    delete[] volume_data;

    glGenTextures(1, &transferfun);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, transferfun);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, transfer_function);
    // Map transfer function to texture

    glGenVertexArrays(1, &VAO);

    glUseProgram(shaderProgram);
    setupModelTransformation(shaderProgram);
    setupViewTransformation(shaderProgram);
    setupProjectionTransformation(shaderProgram);

    createBoundingbox(shaderProgram, VAO); //  Bounding box;

    oldX = oldY = currentX = currentY = 0.0;
    int prevLeftButtonState = GLFW_RELEASE;
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            if (is_ok)
            {
                is_ok = false;
            }
            else
            {
                is_ok = true;
            }
        }
        int leftButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        if (leftButtonState == GLFW_PRESS && prevLeftButtonState == GLFW_RELEASE && !is_ok)
        {
            isDragging = true;
            currentX = oldX = x;
            currentY = oldY = y;
        }
        else if (leftButtonState == GLFW_PRESS && prevLeftButtonState == GLFW_PRESS && !is_ok)
        {
            currentX = x;
            currentY = y;
        }
        else if (leftButtonState == GLFW_RELEASE && prevLeftButtonState == GLFW_PRESS && !is_ok)
        {
            isDragging = false;
        }
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
        { // Moving camera with key press up/(shift-up)
            camPos.z = camPos.z + 1;
            setupViewTransformation(shaderProgram);
        }
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
        {
            camPos.z = camPos.z - 1;
            setupViewTransformation(shaderProgram);
        }
        // Rotate based on mouse drag movementsetupViewTransformation(shaderProgram);
        prevLeftButtonState = leftButtonState;
        if (isDragging && (currentX != oldX || currentY != oldY))
        {
            glm::vec3 va = getTrackBallVector(oldX, oldY);
            glm::vec3 vb = getTrackBallVector(currentX, currentY);
            float angle = acos(std::min(1.0f, glm::dot(va, vb)));
            glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
            glm::mat3 camera2object = glm::inverse(glm::mat3(viewT * modelT));
            glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
            glm::mat4 dummy = glm::rotate(modelT, -angle, axis_in_object_coord);
            camPos = glm::vec4(glm::mat3(dummy) * glm::vec3(camPos), 1.0);
            setupViewTransformation(shaderProgram);
            oldX = currentX;
            oldY = currentY;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Transfer Function");
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            r = transfer_function[0];
            g = transfer_function[1];
            bl = transfer_function[2];
            alpha = transfer_function[3];
            ImGui::SliderFloat("Red Colour Value", &r, 0, 1);
            ImGui::SliderFloat("Green Colour Value", &g, 0, 1);
            ImGui::SliderFloat("Blue Colour Value", &bl, 0, 1);
            ImGui::SliderFloat("Alpha Value", &alpha, 0, 1);
            transfer_function[0] = r;
            transfer_function[1] = g;
            transfer_function[2] = bl;
            transfer_function[3] = alpha;

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_1D, transferfun);

            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, transfer_function);
            ImGui::End();
        }
        // get shader variables and uniforms from the shader program
        glUseProgram(shaderProgram);
        {

            vCam_uniform = glGetUniformLocation(shaderProgram, "camPosition");

            glUniform3fv(vCam_uniform, 1, glm::value_ptr(glm::vec3(camPos)));

            GLuint vstep_size = glGetUniformLocation(shaderProgram, "stepSize");

            glUniform1f(vstep_size, step_size);

            GLuint vExtentMin = glGetUniformLocation(shaderProgram, "extentmin");

            glUniform3f(vExtentMin, 0, 0, -c);

            GLuint vExtentMax = glGetUniformLocation(shaderProgram, "extentmax");

            glUniform3f(vExtentMax, a, b, 0);

            GLuint tex1 = glGetUniformLocation(shaderProgram, "texture3d");

            unsigned int VAO;
            glGenVertexArrays(1, &VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, texture3d);
            glUniform1i(tex1, 0);

            GLuint tex2 = glGetUniformLocation(shaderProgram, "transferfun");
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_1D, transferfun);
            glUniform1i(tex2, 1);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    cleanup(window);

    return 0;
}

void createBoundingbox(unsigned int &program, unsigned int &cube_VAO)
{
    glUseProgram(program);

    // Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if (vVertex_attrib == -1)
    {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }
    // Cube data
    GLfloat cube_vertices[] = {
        a - 1, b - 1, -c + 1, 0, b - 1, -c + 1, 0, 0, -c + 1, a - 1, 0, -c + 1,
        a - 1, b - 1, 0, 0, b - 1, 0, 0, 0, 0, a - 1, 0, 0};
    GLushort cube_indices[] = {
        0, 1, 2, 0, 2, 3, // Front
        4, 7, 5, 5, 7, 6, // Back
        1, 6, 2, 1, 5, 6, // Left
        0, 3, 4, 4, 7, 3, // Right
        0, 4, 1, 4, 5, 1, // Top
        2, 6, 3, 3, 6, 7  // Bottom
    };

    // Generate VAO object
    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    // Create VBOs for the VAO
    // Position information (data + format)
    int nVertices = (6 * 2) * 3; //(6 faces) * (2 triangles each) * (3 vertices each)
    GLfloat *expanded_vertices = new GLfloat[nVertices * 3];
    for (int i = 0; i < nVertices; i++)
    {
        expanded_vertices[i * 3] = cube_vertices[cube_indices[i] * 3];
        expanded_vertices[i * 3 + 1] = cube_vertices[cube_indices[i] * 3 + 1];
        expanded_vertices[i * 3 + 2] = cube_vertices[cube_indices[i] * 3 + 2];
    }
    GLuint vertex_VBO;
    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete[] expanded_vertices;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind the VAO to disable changes outside this function.
}

void setupModelTransformation(unsigned int &program)
{
    // Modelling transformations (Model -> World coordinates)
    modelT = glm::translate(glm::mat4(1.0f), glm::vec3(-a / 2, -b / 2, c / 2)); // Model coordinates are the world coordinates

    // Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    vModel_uniform = glGetUniformLocation(program, "vModel");
    if (vModel_uniform == -1)
    {
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}

void setupViewTransformation(unsigned int &program)
{
    // Viewing transformations (World -> Camera coordinates
    viewT = glm::lookAt(glm::vec3(camPos), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    // Pass-on the viewing matrix to the vertex shader
    glUseProgram(program);
    vView_uniform = glGetUniformLocation(program, "vView");
    if (vView_uniform == -1)
    {
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(viewT));
}

void setupProjectionTransformation(unsigned int &program)
{
    // Projection transformation
    projectionT = glm::perspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 0.1f, 800.0f);

    // Pass on the projection matrix to the vertex shader
    glUseProgram(program);
    vProjection_uniform = glGetUniformLocation(program, "vProjection");
    if (vProjection_uniform == -1)
    {
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projectionT));
}

glm::vec3 getTrackBallVector(double x, double y)
{
    glm::vec3 p = glm::vec3(2.0 * x / screen_width - 1.0, 2.0 * y / screen_height - 1.0, 0.0); // Normalize to [-1, +1]
    p.y = -p.y;                                                                                // Invert Y since screen coordinate and OpenGL coordinates have different Y directions.

    float mag2 = p.x * p.x + p.y * p.y;
    if (mag2 <= 1.0f)
        p.z = sqrtf(1.0f - mag2);
    else
        p = glm::normalize(p); // Nearest point, close to the sides of the trackball
    return p;
}
