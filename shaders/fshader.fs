#version 330 core
/* ---------------------- Input from the vertex shader ---------------------- */
in vec3 eye;
in vec3 tMax;
in vec3 tMin;
/*----------------------  Output Color of the fragment shader ---------------------- */
out vec4 outColor;
/* ----------------------  Get the uniform variables from the main function ---------------------- */
uniform float stepSize;
uniform sampler1D transferfun;
/* ----------------------  1D transfer function is used to get the color and alpha value from the volume texture ----------------------  */
uniform sampler3D volumeTexture;
/* ---------------------- 3D volume texture is used to get the value from the volume data texture ----------------------  */

uniform sampler3D normalTexture;
/* ---------------------- 3D normal texture is used to get the normal vector from the volume data texture ----------------------  */
float screen_width = 1900;
float screen_height = 1080;
/* ----------------------  Screen width and height is used to get the aspect ratio ----------------------  */

vec4 sample;
vec4 accColor = vec4(0, 0, 0, 0);
vec3 direction;
vec3 p;

#define shineConst 32.0

float aspect_ratio = screen_width / screen_height;
float thetha = 90;
float focal_H = 1.0;
vec3 w = normalize(vec3(eye - vec3(0, 0, 0)));
vec3 u = normalize(cross(vec3(0, 1, 0), w));
// Get vector perpendicular to the camera startPoint and the world up vector
vec3 v = normalize(cross(w, u));
// Get vector perpendicular to the camera startPoint and the vector u to get orthonormal basis

float start;
float endPoint;
/* ----------------------  Function to check if the ray hits the volume ---------------------- */
/* ----------------------  Reference: https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm ----------------------*/
bool liangBarsky(vec3 startPoint, vec3 direction) {
    float tuMin=0.0, tuMax=0.0, tvMin=0.0, tvMax=0.0;
    vec3 n = 1 / direction;
    if(n.x < 0) {
        start = (tMax.x - startPoint.x) * n.x;
        endPoint = (tMin.x - startPoint.x) * n.x;
    } else {
        start = (tMin.x - startPoint.x) * n.x;
        endPoint = (tMax.x - startPoint.x) * n.x;
    }

    if(n.y < 0) {
        tuMin = (tMax.y - startPoint.y) * n.y;
        tuMax = (tMin.y - startPoint.y) * n.y;
    } else {
        tuMin = (tMin.y - startPoint.y) * n.y;
        tuMax = (tMax.y - startPoint.y) * n.y;
    }
    if((start > tuMax)) {
        return false;
    }
    /* ----------------------  If the ray is outside volume, return false ---------------------- */
    if(tuMin > endPoint) {
        return false;
    }
    start = max(start, tuMin);
    endPoint = min(endPoint, tuMax);
    if(n.z < 0) {
        tvMin = (tMax.z - startPoint.z) * n.z;
        tvMax = (tMin.z - startPoint.z) * n.z;
    } else {
        tvMin = (tMin.z - startPoint.z) * n.z;
        tvMax = (tMax.z - startPoint.z) * n.z;
    }
    start = max(start, tvMin);
    endPoint = min(endPoint, tvMax);
    if(start <= 0 || endPoint <= 0  || start >= endPoint) {
        /* ---------------------- If any of the above conditions are true, the ray does not hit the volume ---------------------- */
        return false;
    } else {
        /* ----------------------  Ray hits the volume, return true ---------------------- */
        return true;
    }
}

/* ----------------------  Bing Phong shading is used to calculate the diffuse and specular component ---------------------- */
vec4 bingPhongShading(vec3 fPos, vec4 fColor, vec3 dir, vec3 normal) {
    /*----------------------------- Directional light properties -----------------------------*/
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));  // Light direction
    vec3 lightColor = vec3(1.0, 1.0, 1.0);  // Light color
    float diff = max(dot(normalize(normal), -lightDir), 0.0);
    /*----------------------------- Calculate the diffuse component-----------------------------*/
    vec3 diffuse = fColor.rgb * diff * lightColor;
    /*----------------------------- Calculate the specular component-----------------------------*/
    vec3 reflected = reflect(-lightDir, normal);
    float specularIntensity = max(dot(dir, reflected), 0.0);
    vec3 specular = vec3(pow(specularIntensity, shineConst));
    /* ----------------------  Return the color and alpha value ---------------------- */
    return vec4(diffuse + specular, fColor.a);
}

void main() {
    float xw = aspect_ratio * (gl_FragCoord.x - screen_width / 2.0 + 0.5) / screen_width;
    float yw = (gl_FragCoord.y - screen_height / 2.0 + 0.5) / screen_height;
    // 1/(2tan(pi*thetha/360)))
    float dirn = focal_H / (2.0 * tan(thetha * 3.14 / (180.0 * 2.0)));
    vec3 startPoint = eye;
    direction = normalize(u * xw + v * yw - dirn * w);
    // Get the direction of the ray using the camera startPoint and the orthonormal basis
    // If the ray does not hit the volume, return black
    if(!liangBarsky(startPoint, direction)) {
        outColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }
    vec3 dir = normalize((gl_FragCoord.xyz - eye));

    /*-------------------- Initialize the color and alpha value --------------------*/
    accColor = vec4(0.0, 0.0, 0.0, 0.0);
    int i = 1;
    float currPoint = start;

    while(i == 1) {
        /*-------------------- Propagate the ray down the ray direction --------------------*/
        /*--------------- p = eye + t*direction ----------------*/
        p = startPoint + direction * currPoint;
        /*-------------------- Ray exits the volume and hence the loop is broken --------------------*/
        if(currPoint > endPoint) {
            break;
        }
        /*-------------------- Early Ray Termination if reached threshold --------------------*/
        if(accColor.a > 0.95) {
            break;
        }
        /*--------------------- Tri-linear interpolation performed ---------------------*/
        sample = texture(volumeTexture, (p + ((tMax - tMin) / 2)) / (tMax - tMin));
        /*--------------- Amount of color and alpha value is determined by the volume density---------------*/

        /* ---------------------- calculate the color and alpha value using the transfer function and the volume texture ---------------------- */
        vec4 transferFuncColor = texture(transferfun, sample.r);
        /* ----------------------  Calculate the normal vector from the normal texture ---------------------- */
        vec3 normalFromTexture = texture(normalTexture, gl_FragCoord.xyz).rgb;
        vec3 normal = normalize(normalFromTexture * 2.0 - 1.0);
        /* ---------------------- Reference: https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal ---------------------- */
        /* ---------------------- Reference: https://learnopengl.com/Advanced-Lighting/Normal-Mapping ---------------------- */
        float normal_mag = length(normal);
        if(normal_mag > 0.01 && currPoint > stepSize) {

            transferFuncColor = bingPhongShading(p, transferFuncColor, -dir, normalize(normal));
        }
        /* ---------------------- Perform the shading only if the normal vector is not zero ---------------------- */
        if(transferFuncColor.a > 0.0) {
            accColor = accColor + (1.0 - accColor.a) * transferFuncColor * sample.r;
        }
        /* ---------------------- Composite the color and alpha sample using the front-to-back compositing if opactiy at that point is greater than zero---------------------- */
        currPoint += stepSize;
        /* ---------------------- Increment the current point by the step size to get the next point on the ray ---------------------- */
        /* ----------------------  Step size determines the quality of the image ---------------------- */
    }
    outColor = accColor;
    /* ----------------------  Final color and alpha value is assigned to the output color ---------------------- */
}
