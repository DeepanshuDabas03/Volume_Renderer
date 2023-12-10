#version 330 core
/* ----------------------- Inputs to vShader ----------------------- */
layout (location = 0) in vec3 vVertex;

/* ----------------------- Uniforms from Main----------------------- */
uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;
uniform vec3 camPosition;
uniform vec3 Mini;
uniform vec3 Maxi;

/* ----------------------- Outputs to fShader ----------------------- */
out vec3 eye;
out vec3 tMax;
out vec3 tMin;

void main() {
	gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);
    eye = camPosition;
	tMax = vec3(vModel*vec4(Maxi,1.0));
	tMin = vec3(vModel*vec4(Mini,1.0));
}
