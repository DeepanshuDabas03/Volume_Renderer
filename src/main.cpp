#include "utils.h"

#define  GLM_FORCE_RADIANS
#define  GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include<cmath>

#include<iostream>
#include <unistd.h>
#include <stdio.h>

//Globals
bool slider_flag = false;
float alpha = 0.0;
int scalar = 0;
float r, g, b;

int screen_width = 800, screen_height=800;
GLint vModel_uniform, vView_uniform, vProjection_uniform;
GLint vColor_uniform, vCam_uniform;
glm::mat4 modelT, viewT, projectionT;//The model, view and projection transformations

double oldX, oldY, currentX, currentY;
bool isDragging=false;

void createBoundingbox(unsigned int &, unsigned int &);
void createLines(unsigned int &, unsigned int &);
void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &);
glm::vec3 getTrackBallVector(double x, double y);
bool load_volume(const char* filename);
void setUniforms(unsigned int &);
GLfloat* createTransferfun(int width, int height);
float x_size = 256;
float y_size = 256;
float z_size = 256;
float step_size = 1;
const int vol_size = x_size*y_size*z_size;
GLubyte* volume = new GLubyte[vol_size];
GLfloat *tf = new GLfloat[256*4];
glm::vec4 camposition = glm::vec4(0, 0, 280.0, 1.0);
glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
GLuint VAO, transferfun, texture3d;

int main(int, char**)
{
    // Setup window
    GLFWwindow *window = setupWindow(screen_width, screen_height);
    ImGuiIO& io = ImGui::GetIO(); // Create IO object

    ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    const char *filepath = "./data/foot.raw";
    if(!load_volume(filepath))                                      // Reading the Volume
    {
        std::cout<<"Volume not loaded succesfully"<<std::endl;
        printf("Current working dir: %s\n", get_current_dir_name());
        return 0;
    }

    tf = createTransferfun(x_size, y_size);                     // Creating transfer function

    unsigned int shaderProgram = createProgram("./shaders/vshader11.vs", "./shaders/fshader11.fs");

    glUseProgram(shaderProgram);

    glGenTextures(1,&texture3d);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture3d);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexImage3D(GL_TEXTURE_3D,0,GL_INTENSITY,x_size,y_size,z_size,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,volume);      //Making a 3D texture with the volume
   //  glTexImage3D(GL_TEXTURE_3D,0,GL_R8,x_size,y_size,z_size,0,GL_RED,GL_UNSIGNED_BYTE,volume);
    delete [] volume;

    
    glGenTextures(1, &transferfun);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, transferfun);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,tf);                    // Making a 1d transfer function
    
    glGenVertexArrays(1, &VAO);

    glUseProgram(shaderProgram);
    setupModelTransformation(shaderProgram);                    // These funs will set and pass the Model, View, Transformation matrix to shaders
    setupViewTransformation(shaderProgram);
    setupProjectionTransformation(shaderProgram);

    createBoundingbox(shaderProgram, VAO);                  // Creating vounding box;

    oldX = oldY = currentX = currentY = 0.0;
    int prevLeftButtonState = GLFW_RELEASE;
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS){
            slider_flag = slider_flag ? false:true;
        }
        // Get current mouse position
        int leftButtonState = glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT);
        double x,y;
        glfwGetCursorPos(window,&x,&y);
        if(leftButtonState == GLFW_PRESS && prevLeftButtonState == GLFW_RELEASE &&  !slider_flag){
            isDragging = true;
            currentX = oldX = x;
            currentY = oldY = y;
        }
        else if(leftButtonState == GLFW_PRESS && prevLeftButtonState == GLFW_PRESS && !slider_flag){
            currentX = x;
            currentY = y;
        }
        else if(leftButtonState == GLFW_RELEASE && prevLeftButtonState == GLFW_PRESS && !slider_flag){
            isDragging = false;
        }
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) {               // Moving camera with key press up/(shift-up)
            camposition.z = camposition.z+2;
            setupViewTransformation(shaderProgram);
        }
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow))){
            camposition.z = camposition.z-2;
            setupViewTransformation(shaderProgram);
        }
        // Rotate based on mouse drag movementsetupViewTransformation(shaderProgram);
        prevLeftButtonState = leftButtonState;
        if(isDragging && (currentX !=oldX || currentY != oldY))
        {
            glm::vec3 va = getTrackBallVector(oldX, oldY);
            glm::vec3 vb = getTrackBallVector(currentX, currentY);
            float angle = acos(std::min(1.0f, glm::dot(va,vb)));
            glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
            glm::mat3 camera2object = glm::inverse(glm::mat3(viewT*modelT));
            glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
            glm::mat4 dummy = glm::rotate(modelT, -angle, axis_in_object_coord);
            camposition = glm::vec4(glm::mat3(dummy)*glm::vec3(camposition),1.0);
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
            
            r = tf[0];
            ImGui::SliderFloat("Red Color Value:", &r, 0, 1);
            g = tf[1];
            ImGui::SliderFloat("Green Color Value: ", &g, 0, 1);
            b = tf[2];
            ImGui::SliderFloat("Blue Color Value: ", &b, 0, 1);
            alpha = tf[3];
            ImGui::SliderFloat("Alpha Value", &alpha, 0, 1);

            tf[0] = r;
            tf[1] = g;
            tf[2] = b;
            tf[3] = alpha;

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_1D, transferfun);
            
            glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,tf); 
            ImGui::End();
        }
        glUseProgram(shaderProgram);
        setUniforms(shaderProgram);                         // This will set all the uniform variable inside shaders


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

bool load_volume(const char* filename)
{
    FILE *file = fopen(filename,"rb");
    if(NULL == file)
    {
        return false;
    }
    fread(volume,sizeof(GLubyte),vol_size,file);
    fclose(file);
    return true;
}

GLfloat* createTransferfun(int width, int height)
{
    for(int i=0; i<256; i++) {
         tf[i*4] = float(i)/255.0;
         tf[i*4 + 1] = float(i)/255.0;
         tf[i*4 + 2] = float(i)/255.0;
         tf[i*4 + 3] = 1;
    }
    return tf;
}

void createBoundingbox(unsigned int &program, unsigned int &cube_VAO)
{
    glUseProgram(program);

    //Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if(vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }
    //Cube data
    GLfloat cube_vertices[] = {
        x_size-1, y_size-1, -z_size+1, 0, y_size-1, -z_size+1, 0, 0, -z_size+1, x_size-1, 0, -z_size+1,
        x_size-1, y_size-1, 0, 0, y_size-1, 0, 0, 0, 0, x_size-1, 0, 0
    };
    GLushort cube_indices[] = {
                0, 1, 2, 0, 2, 3, //Front
                4, 7, 5, 5, 7, 6, //Back
                1, 6, 2, 1, 5, 6, //Left
                0, 3, 4, 4, 7, 3, //Right
                0, 4, 1, 4, 5, 1, //Top
                2, 6, 3, 3, 6, 7 //Bottom
                };

    //Generate VAO object
    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    //Create VBOs for the VAO
    //Position information (data + format)
    int nVertices = (6*2)*3; //(6 faces) * (2 triangles each) * (3 vertices each)
    GLfloat *expanded_vertices = new GLfloat[nVertices*3];
    for(int i=0; i<nVertices; i++) {
        expanded_vertices[i*3] = cube_vertices[cube_indices[i]*3];
        expanded_vertices[i*3 + 1] = cube_vertices[cube_indices[i]*3+1];
        expanded_vertices[i*3 + 2] = cube_vertices[cube_indices[i]*3+2];
    }
    GLuint vertex_VBO;
    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete []expanded_vertices;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); //Unbind the VAO to disable changes outside this function.
}

void setupModelTransformation(unsigned int &program)
{
    //Modelling transformations (Model -> World coordinates)
    modelT = glm::translate(glm::mat4(1.0f), glm::vec3(-x_size/2, -y_size/2, z_size/2));//Model coordinates are the world coordinates
    
    //Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    vModel_uniform = glGetUniformLocation(program, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}

void setupViewTransformation(unsigned int &program)
{
    //Viewing transformations (World -> Camera coordinates
    viewT = glm::lookAt(glm::vec3(camposition), glm::vec3(0.0, 0.0, 0.0), up);

    //Pass-on the viewing matrix to the vertex shader
    glUseProgram(program);
    vView_uniform = glGetUniformLocation(program, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(viewT));
}

void setupProjectionTransformation(unsigned int &program)
{
    //Projection transformation
    projectionT = glm::perspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 0.1f, 800.0f);

    //Pass on the projection matrix to the vertex shader
    glUseProgram(program);
    vProjection_uniform = glGetUniformLocation(program, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projectionT));
}

glm::vec3 getTrackBallVector(double x, double y)
{
	glm::vec3 p = glm::vec3(2.0*x/screen_width - 1.0, 2.0*y/screen_height - 1.0, 0.0); //Normalize to [-1, +1]
	p.y = -p.y; //Invert Y since screen coordinate and OpenGL coordinates have different Y directions.

	float mag2 = p.x*p.x + p.y*p.y;
	if(mag2 <= 1.0f)
		p.z = sqrtf(1.0f - mag2);
	else
		p = glm::normalize(p); //Nearest point, close to the sides of the trackball
	return p;
}

void setUniforms(unsigned int &program)
{
    glUseProgram(program);

    vCam_uniform = glGetUniformLocation(program, "camPosition");
	if(vCam_uniform == -1){
		fprintf(stderr, "Could not bind location: camPosition\n");
		exit(0);
	}
	glUniform3fv(vCam_uniform, 1, glm::value_ptr(glm::vec3(camposition)));

    GLuint vstep_size = glGetUniformLocation(program, "stepSize");
    if(vstep_size == -1){
        fprintf(stderr, "Could not bind location: vstep_size\n");
        // exit(0);
    }
    glUniform1f(vstep_size, step_size);

    GLuint vExtentMin = glGetUniformLocation(program, "extentmin");
    if(vExtentMin == -1){
        fprintf(stderr, "Could not bind location: vExtentMin\n");
        // exit(0);
    }
    glUniform3f(vExtentMin, 0, 0, -z_size);

    GLuint vExtentMax = glGetUniformLocation(program, "extentmax");
    if(vExtentMax == -1){
        fprintf(stderr, "Could not bind location: vExtentMax\n");
        // exit(0);
    }
    glUniform3f(vExtentMax, x_size, y_size, 0);

    GLuint tex1 = glGetUniformLocation(program,"texture3d");
    if(tex1 == -1){
        fprintf(stderr, "Could not bind location: texture3d\n");
        // exit(0);
    }
    else{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, texture3d);
        glUniform1i(tex1, 0);
    }

    GLuint tex2 = glGetUniformLocation(program,"transferfun");
    if(tex2 == -1){
        fprintf(stderr, "Could not bind location: transferfun\n");
        // exit(0);
    }
    else{
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, transferfun);
        glUniform1i(tex2, 1);
    }
}
