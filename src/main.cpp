#include "utils.h"
/*--------------------------- Include standard headers from OpenGL ---------------------------*/
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
/*--------------------------- Include GLM core features ---------------------------*/
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

/*--------------------------- Include standard headers from C++ ---------------------------*/
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <unistd.h>
#include <vector>
#include <string>
namespace fs = std::filesystem;

/*--------------------------- Window size ---------------------------*/
int screen_width = 1900, screen_height = 1080;
GLint vModel_uniform, vView_uniform, vProjection_uniform, vColor_uniform, vCam_uniform;
glm::mat4 modelT, viewT, projectionT;

double oldX, oldY, currentX, currentY;
bool isDragging = false;
bool is_ok = false;
/*--------------------------- Scaler index of the Transfer function array ---------------------------*/
int trans_coord;
/*---------------------------Function Declarations---------------------------*/
void fileIterator(std::string, std::vector<std::string> &);
std::string UpdateTransferFunction(std::string);
bool saveTransferFunction(std::string);
void computeNormals();
void createBoundingbox(unsigned int &, unsigned int &);
void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &);
glm::vec3 getTrackBallVector(double x, double y);
/* --------------------------- Camera Position---------------------------*/
glm::vec4 camPos = glm::vec4(0, 0, 280.0, 1.0);
float a = 256, b = 256, c = 256;
/* ---------------------------Volume size = 256 * 256 * 256---------------------------*/
const int volume_size = a * b * c;
/*--------------------------- Step size for ray ---------------------------*/
float step_size = 2.0f;
/*--------------------------- Array to store loaded volume data---------------------------*/
GLubyte *volume_data = new GLubyte[volume_size];
GLubyte *normals = new GLubyte[volume_size];
/*--------------------------- Pointer to the location of volume---------------------------*/
const char *location = "Null";
/*--------------------------- File name to save transfer function---------------------------*/
char fileName[1024];
/*--------------------------- Array to store volume data, transfer function file names---------------------------*/
std::string path = "./data";
std::vector<std::string> files;
std::string pathT = "./transferFunction";
std::vector<std::string> transferfiles;

/*--------------------------- Current transfer function file name---------------------------*/
std::string currentTransferFunction;
/* --------------------------- Create a transfer function Array with 256*4 size since we have 256 values and each have 4 values for RGBA where A is alpha and R is red, G is green and B is blue---------------------------*/
GLfloat *transfer_function = new GLfloat[1024];
/* --------------------------- Shader programs ---------------------------*/
GLuint VAO, transferfun, volumeTexture, normalTexture;
int mode=1;

int main()
{
    /*------------------ Setup window------------------*/
    GLFWwindow *window = setupWindow(screen_width, screen_height);
    ImGuiIO &io = ImGui::GetIO(); // Create IO object
    /*------------------ Background Color(Default Color)------------------*/
    ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    /*------------------ RGBA values for the Transfer function ------------------*/
    float RGBA[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    /*
    ------------------ Open directory and iterator over each file and add them to files array ------------------
    ------------------ These files will be used to load volume data from file ------------------
    ------------------ User then can select the desired volume data from the GUI ------------------
    */
    fileIterator(path, files);
    fileIterator(pathT, transferfiles);

    currentTransferFunction = UpdateTransferFunction("./transferFunction/default");

    memset(volume_data, 0, volume_size);
    /*------------------ Create a shader program------------------*/
    unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
    glUseProgram(shaderProgram);
    /*------------------ Create Textures for volume data ------------------*/
    glGenTextures(1, &volumeTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, a, b, c, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, volume_data);
    /*------------------  Create Textures for transfer function ------------------*/
    glGenTextures(1, &transferfun);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, transferfun);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenVertexArrays(1, &VAO);

    /* --------------------------- Bind location of variables from shader program  for volume texture ---------------------------*/
    GLuint tex1 = glGetUniformLocation(shaderProgram, "volumeTexture");

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTexture);
    glUniform1i(tex1, 0);
    /* --------------------------- Bind location of variables from shader program  for transfer function texture ---------------------------*/
    GLuint tex2 = glGetUniformLocation(shaderProgram, "transferfun");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, transferfun);
    glUniform1i(tex2, 1);
    /* --------------------------- Bind location of variables from shader program  for normal texture and also create normal texture ---------------------------*/
    glGenTextures(1, &normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, normalTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, a, b, c, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr); // Provide nullptr as data for now
    glUniform1i(glGetUniformLocation(shaderProgram, "normalTexture"), 2);
    glUseProgram(shaderProgram);
    /*------------------ Setup Transformations------------------*/
    setupModelTransformation(shaderProgram);
    setupViewTransformation(shaderProgram);
    setupProjectionTransformation(shaderProgram);
    /*------------------ Create Bounding box------------------*/
    createBoundingbox(shaderProgram, VAO);

    oldX = oldY = currentX = currentY = 0.0;
    int prevLeftButtonState = GLFW_RELEASE;
    glEnable(GL_DEPTH_TEST);
    /*------------------ Main loop------------------*/
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (!ImGui::IsAnyItemActive())
        {
            /*--------------------------- Handle Keyboard events---------------------------*/
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
            /*------------------ Rotate based on mouse drag movements------------------*/
            float angle = 0.0f;
            prevLeftButtonState = leftButtonState;
            if (mode == 0)
            {
                if (isDragging && (currentX != oldX || currentY != oldY))
                {
                    glm::vec3 va = getTrackBallVector(oldX, oldY);
                    glm::vec3 vb = getTrackBallVector(currentX, currentY);
                    angle = acos(std::min(1.0f, glm::dot(va, vb)));
                    glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
                    glm::mat3 camera2object = glm::inverse(glm::mat3(viewT * modelT));
                    glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
                    glm::mat4 dummy = glm::rotate(modelT, -angle, axis_in_object_coord);
                    camPos = glm::vec4(glm::mat3(dummy) * glm::vec3(camPos), 1.0);
                    setupViewTransformation(shaderProgram);
                    oldX = currentX;
                    oldY = currentY;
                }
            }
            else
            {
                angle += 0.005;
                glm::mat4 dummy = glm::rotate(modelT, angle, glm::vec3(0.0, 1.0, 0.0));
                camPos = glm::vec4(glm::mat3(dummy) * glm::vec3(camPos), 1.0);
                setupViewTransformation(shaderProgram);
                oldX = currentX;
                oldY = currentY;
            }
        }
        /*------------------Start the Dear ImGui frame------------------*/
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            /*------------------ Window Properties ------------------*/
            ImGui::Begin("Window Properties", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetWindowFontScale(1.25);
            std::string title = "Current Volume: " + std::string(location);
            ImGui::Text(title.c_str(), NULL);
            std::string function = "Current Transfer Function: " + std::string(currentTransferFunction);
            ImGui::Text(function.c_str(), NULL);
            if(ImGui::Button("Toggle Auto Rotate", ImVec2(200, 50))){
                if(mode==0){
                    mode=1;
                }
                else{
                    mode=0;
                }
            }
            /*------------------ Create Menu of Options Of Volume ------------------ */
            if (ImGui::BeginMenu("Select Volume Data"))
            {
                for (int i = 0; i < files.size(); i++)
                {
                    if (ImGui::MenuItem(files[i].c_str()))
                    {
                        /*------------------ Reset volume data------------------*/
                        memset(volume_data, 0, volume_size);
                        glDeleteTextures(1, &volumeTexture);
                        glDeleteTextures(1, &normalTexture);

                        location = files[i].c_str();
                        /*--------------------------- Read volume data from file ---------------------------*/
                        FILE *file = fopen(location, "rb");
                        if (NULL == file)
                        {
                            fprintf(stderr, "Error opening file\n");
                            exit(0);
                        }
                        fread(volume_data, sizeof(GLubyte), volume_size, file);
                        fclose(file);

                        /*------------------ Update texture for volume data ------------------*/
                        glUseProgram(shaderProgram);
                        glGenTextures(1, &volumeTexture);
                        glActiveTexture(GL_TEXTURE0);
                        /*------------------ Tri-linear interpolation ------------------*/
                        /*------------------ Reference https://learnopengl.com/Getting-started/Textures ------------------*/
                        glBindTexture(GL_TEXTURE_3D, volumeTexture);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, a, b, c, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, volume_data);

                        computeNormals();
                        /*------------------ Compute normals ------------------*/

                        glGenTextures(1, &normalTexture);
                        glActiveTexture(GL_TEXTURE2);
                        glBindTexture(GL_TEXTURE_3D, normalTexture);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, a, b, c, GL_RGB, GL_UNSIGNED_BYTE, normals);

                        /*-------------------- Update normal texture with the newly calculated normals ------------------*/
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Select Transfer Function"))
            {
                for (int i = 0; i < transferfiles.size(); i++)
                {
                    if (ImGui::MenuItem(transferfiles[i].c_str()))
                    {
                        currentTransferFunction = UpdateTransferFunction(transferfiles[i]);
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_1D, transferfun);
                        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, transfer_function);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::Text("Camera Position");
            ImGui::SliderFloat("X", &camPos.x, -1024, 1024);
            ImGui::SliderFloat("Y", &camPos.y, -1024, 1024);
            ImGui::SliderFloat("Z", &camPos.z, -1024, 1024);
            ImGui::ColorPicker3("Change Background Color", (float *)&clearColor);
            ImGui::PlotHistogram("Histogram", transfer_function, 256, 0, "Transfer Function", 0.0f, 1.0f, ImVec2(0, 80));
            ImGui::End();

            /*------------------Transfer function Window------------------*/
            ImGui::StyleColorsLight();
            ImGui::Begin("Transfer Function", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetWindowFontScale(1.25);
            ImGui::SliderInt("Scalar Index", &trans_coord, 0, 255);
            ImGui::ColorPicker4("Change Color", (float *)&RGBA, ImGuiColorEditFlags_DisplayRGB);
            ImGui::SliderFloat("Step Size", &step_size, 1.0f, 20.0f);
            ImGui::Text("Enter Output TF File Name:");
            ImGui::InputTextWithHint("", "Enter File Name", fileName, IM_ARRAYSIZE(fileName));
            if (ImGui::Button("Save Transfer Function"))
            {
                saveTransferFunction(fileName);
            }
            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float);

            // Update transfer function based on user input of coordinates and RGBA values
            transfer_function[trans_coord * 4] = RGBA[0];
            transfer_function[trans_coord * 4 + 1] = RGBA[1];
            transfer_function[trans_coord * 4 + 2] = RGBA[2];
            transfer_function[trans_coord * 4 + 3] = RGBA[3];

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_1D, transferfun);
            /*------------------Map transfer function to texture------------------*/
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, transfer_function);
            ImGui::End();
        }
        /*------------------Shader varialbles and uniforms and get location from shader program------------------*/
        glUseProgram(shaderProgram);
        {

            vCam_uniform = glGetUniformLocation(shaderProgram, "camPosition");

            glUniform3fv(vCam_uniform, 1, glm::value_ptr(glm::vec3(camPos)));

            GLuint vstep_size = glGetUniformLocation(shaderProgram, "stepSize");

            glUniform1f(vstep_size, step_size);

            GLuint vMin = glGetUniformLocation(shaderProgram, "Mini");

            glUniform3f(vMin, 0, 0, -c);

            GLuint vMax = glGetUniformLocation(shaderProgram, "Maxi");

            glUniform3f(vMax, a, b, 0);
        }

        /*------------------Rendering------------------*/
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
    delete[] volume_data;
    delete[] normals;
    delete[] transfer_function;
    cleanup(window);

    return 0;
}
/* --------------------------- Function to Iterate over files in a directory and add them to a vector ---------------------------*/
void fileIterator(std::string path, std::vector<std::string> &files)
{
    for (const auto &entry : fs::directory_iterator(path))
    {
        std::filesystem::path outfilename = entry.path();
        std::string outfilename_str = outfilename.string();
        const char *path = outfilename_str.c_str();
        for (int i = 0; i < files.size(); i++)
        {
            if (files[i] == path)
            {
                continue;
            }
        }
        files.push_back(path);
    }
}
/* --------------------------- Function to update transfer function based on user choice ---------------------------*/
std::string UpdateTransferFunction(std::string fileName)
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if (NULL == file)
    {
        file = fopen("./transferFunction/default", "rb");
        if (NULL == file)
        {
            fprintf(stderr, "Error opening default transfer function file\n");
            exit(0);
        }
        fileName = "./transferFunction/default";
    }
    fread(transfer_function, sizeof(GLfloat), 1024, file);
    fclose(file);
    return fileName;
}
/* -------------------Save transfer function to a file in transferFunction folder------------------- */
bool saveTransferFunction(std::string fileName)
{
    std::string path = "./transferFunction/" + fileName;
    FILE *file = fopen(path.c_str(), "wb");
    if (NULL == file)
    {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }
    fwrite(transfer_function, sizeof(GLfloat), 1024, file);
    for (int i = 0; i < transferfiles.size(); i++)
    {
        if (transferfiles[i] == path)
        {
            return 0;
        }
    }
    transferfiles.push_back(path);
    fclose(file);
    return 0;
}

/*------------------------------ Function to compute normals using central differences method ------------------------------*/
void computeNormals()
{
    int nx = a;
    int ny = b;
    int nz = c;
    for (int x = 1; x < nx - 1; ++x)
    {
        for (int y = 1; y < ny - 1; ++y)
        {
            for (int z = 1; z < nz - 1; ++z)
            {
                /*--------------- Compute directional derivatives using central differences ------------------*/
                int idx = x * ny * nz + y * nz + z;
                double fx = static_cast<double>(volume_data[(x + 1) * ny * nz + y * nz + z]) -
                            static_cast<double>(volume_data[(x - 1) * ny * nz + y * nz + z]);
                double fy = static_cast<double>(volume_data[x * ny * nz + (y + 1) * nz + z]) -
                            static_cast<double>(volume_data[x * ny * nz + (y - 1) * nz + z]);
                double fz = static_cast<double>(volume_data[x * ny * nz + y * nz + (z + 1)]) -
                            static_cast<double>(volume_data[x * ny * nz + y * nz + (z - 1)]);

                double norm = std::sqrt(fx * fx + fy * fy + fz * fz);

                if (norm == 0)
                {
                    norm = 1;
                }
                /*--------------------------- Compute normal vector and store in the 'normals' array, since need to store in GLubyte format , we need to convert to 0-255 range ---------------------------*/
                normals[idx] = static_cast<GLubyte>(std::round(fx / norm * 255.0));
                normals[idx + 1] = static_cast<GLubyte>(std::round(fy / norm * 255.0));
                normals[idx + 2] = static_cast<GLubyte>(std::round(fz / norm * 255.0));
            }
        }
    }
}
/*------------------ Create Bounding box by using code from assignments------------------*/
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

/*------------------Setup Transformations of Model using code provided in assignments------------------*/
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
/*------------------ View Transform , code credits goes to Assignment------------------*/
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

/*------------------ Projection Transformation, code from assignments------------------*/
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
