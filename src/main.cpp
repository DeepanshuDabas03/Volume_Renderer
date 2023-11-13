#include "utils.h"
// Include standard headers from OpenGL
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
// include glm headers
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// Include standard headers from C++ 
#include <cmath>
#include <iostream>
#include <unistd.h>
#include <stdio.h>

// Globals
bool is_ok = false;
float alpha = 0.0;
float r, g, b;
// Window size
int screen_width = 800, screen_height = 800;
// Shader program variables
GLint vModel_uniform, vView_uniform, vProjection_uniform,vColor_uniform, vCam_uniform;
// The model, view and projection transformations
glm::mat4 modelT, viewT, projectionT; 
// Mouse movement variables
double oldX, oldY, currentX, currentY;
bool isDragging = false;

void createBoundingbox(unsigned int &, unsigned int &);
void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &);
glm::vec3 getTrackBallVector(double x, double y);
float step_size = 1;
glm::vec4 camPos = glm::vec4(0, 0, 500.0, 1.0);
// Initial camera Position
// Move camera in camera space

GLuint VAO, transferfun, texture3d;
float x_size = 256, y_size = 256, z_size = 256;
// Volume size = 256*256*256
const int volume_size = x_size * y_size * z_size;
GLubyte *volume_data = new GLubyte[volume_size];
GLfloat *transfer_function = new GLfloat[256 * 4];
// create a transfer function with 256*4 size since we have 256 values and each have 4 values for RGBA where A is alpha and R is red, G is green and B is blue
int main(int, char **)
{
    // Setup window
    GLFWwindow *window = setupWindow(screen_width, screen_height);
    ImGuiIO &io = ImGui::GetIO(); // Create IO object
    ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    // Setup ImGui binding
    const char *file = "./data/foot.raw";
    FILE *f = fopen(file, "rb");
    if (NULL == f)
    {   fprintf(stderr, "Data file not found\n");
        exit(0);
    }
    fread(volume_data, sizeof(GLubyte), volume_size, f);
    fclose(f);
    //Transfer function
    // To do
    for (int i = 0; i < 256; i++)
    {
        transfer_function[i * 4] = float(i) / 255.0;
        transfer_function[i * 4 + 1] = float(i) / 255.0;
        transfer_function[i * 4 + 2] = float(i) / 255.0;
        transfer_function[i * 4 + 3] = 1;
    }
    // Generate a texture with the volume_data data and shading values and pass it to the shader program using uniform variables
    unsigned int shader_program = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");

    glUseProgram(shader_program);
    glGenTextures(1, &texture3d);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture3d);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, x_size, y_size, z_size, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, volume_data);
    // Making a texture with the volume_data
    delete[] volume_data;

    glGenTextures(1, &transferfun);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, transferfun);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, transfer_function);
     // 1D transfer function

    glGenVertexArrays(1, &VAO);

    glUseProgram(shader_program);
    setupModelTransformation(shader_program); // These funs will set and pass the Model, View, Transformation matrix to shaders
    setupViewTransformation(shader_program);
    setupProjectionTransformation(shader_program);

    createBoundingbox(shader_program, VAO); // Creating vounding box;

    oldX = oldY = currentX = currentY = 0.0;
    int prevLeftButtonState = GLFW_RELEASE;
    glEnable(GL_DEPTH_TEST);
    // Credit goes to Assignment 3 , Code provided by Prof. Ojaswa Sharma
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
            isDragging = 0;
        }
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
        {
            camPos.z = camPos.z + 1;
            setupViewTransformation(shader_program);
        }
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
        {
            camPos.z = camPos.z - 1;
            setupViewTransformation(shader_program);
        }
        // Rotate based on mouse drag movement
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
            setupViewTransformation(shader_program);
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
            ImGui::SliderFloat("Red Color Value:", &r, 0, 1);
            g = transfer_function[1];
            ImGui::SliderFloat("Green Color Value: ", &g, 0, 1);
            b = transfer_function[2];
            ImGui::SliderFloat("Blue Color Value: ", &b, 0, 1);
            alpha = transfer_function[3];
            ImGui::SliderFloat("Alpha Value", &alpha, 0, 1);

            transfer_function[0] = r;
            transfer_function[1] = g;
            transfer_function[2] = b;
            transfer_function[3] = alpha;

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_1D, transferfun);

            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, transfer_function);
            ImGui::End();
        }
        glUseProgram(shader_program);
        // glUseProgram(shader_program);
        // Bind shader variables
        vCam_uniform = glGetUniformLocation(shader_program, "camPosition");
        glUniform3fv(vCam_uniform, 1, glm::value_ptr(glm::vec3(camPos)));
        GLuint vstep_uniform = glGetUniformLocation(shader_program, "stepSize");
        glUniform1f(vstep_uniform, step_size);
        GLuint vextent_min_uniform = glGetUniformLocation(shader_program, "extentmin");
        glUniform3f(vextent_min_uniform, 0, 0, -z_size);
        GLuint vextent_max_uniform = glGetUniformLocation(shader_program, "extentmax");
        glUniform3f(vextent_max_uniform, x_size, y_size, 0);
        GLuint texture_1 = glGetUniformLocation(shader_program, "texture3d");
        unsigned int VAO;
        glGenVertexArrays(1, &VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, texture3d);
        glUniform1i(texture_1, 0);

        GLuint texture_2 = glGetUniformLocation(shader_program, "transferfun");
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, transferfun);
        glUniform1i(texture_2, 1);
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

// Below functions are referred from Assignment and Labs
// Bounding box required for the volume_data rendering to intersect ray with the volume_data data
void createBoundingbox(unsigned int &program, unsigned int &cube_VAO)
{
    glUseProgram(program);

    // Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if (vVertex_attrib == -1)
    {
        fprintf(stderr, "Could not bind location: vVertex\n");
    }
    // Cube data
    GLfloat cube_vertices[] = {
        x_size - 1, y_size - 1, -z_size + 1, 0, y_size - 1, -z_size + 1, 0, 0, -z_size + 1, x_size - 1, 0, -z_size + 1,
        x_size - 1, y_size - 1, 0, 0, y_size - 1, 0, 0, 0, 0, x_size - 1, 0, 0};
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
    modelT = glm::translate(glm::mat4(1.0f), glm::vec3(-x_size / 2, -y_size / 2, z_size / 2)); // Model coordinates are the world coordinates

    // Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    vModel_uniform = glGetUniformLocation(program, "vModel");
    if (vModel_uniform == -1)
    {
        fprintf(stderr, "Could not bind location: vModel\n");
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}

void setupViewTransformation(unsigned int &program)
{
    // Viewing transformations (World -> Camera coordinates
    viewT = glm::lookAt(glm::vec3(camPos), glm::vec3(0.0, 0.0, 0.0),  glm::vec3(0.0, 1.0, 0.0));

    // Pass-on the viewing matrix to the vertex shader
    glUseProgram(program);
    vView_uniform = glGetUniformLocation(program, "vView");
    if (vView_uniform == -1)
    {
        fprintf(stderr, "Could not bind location: vView\n");
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
