#version 330 core

in vec3 fColor;
in vec3 cameraPos;
in vec3 ExtentMax;
in vec3 ExtentMin;

out vec4 outColor;

uniform float stepSize;
uniform sampler1D transferfun;
// transferfun is the transfer function texture
uniform sampler3D texture3d;
// texture3d is the volume texture

float screen_width = 1900;
float screen_height = 1080;

vec4 value;
float s;
vec4 fragColor = vec4(0, 0, 0, 0);
vec3 direction;
vec3 dpos;

float aspect_ratio = screen_width / screen_height;
float thetha = 90;
float focal_H = 1.0;
vec3 w = normalize(vec3(cameraPos - vec3(0, 0, 0)));
vec3 u = normalize(cross(vec3(0, 1, 0), w));
// Get vector perpendicular to the camera startPoint and the world up vector
vec3 v = normalize(cross(w, u));
// Get vector perpendicular to the camera startPoint and the vector u to get orthonormal basis

float z_in;
float endPoint;

bool does_hit(vec3 startPoint, vec3 direction) {
    float y_min, y_max, z_min, z_max;
    vec3 n = 1 / direction;
    // if the sign is negative, the ray is going in the negative direction
    if(n.x < 0) {
        z_in = (ExtentMax.x - startPoint.x) / direction.x;
        endPoint = (ExtentMin.x - startPoint.x) / direction.x;
    } else {
        z_in = (ExtentMin.x - startPoint.x) / direction.x;
        endPoint = (ExtentMax.x - startPoint.x) / direction.x;
    }

    if(n.y < 0) {
        y_min = (ExtentMax.y - startPoint.y) / direction.y;
        y_max = (ExtentMin.y - startPoint.y) / direction.y;
    } else {
        y_min = (ExtentMin.y - startPoint.y) / direction.y;
        y_max = (ExtentMax.y - startPoint.y) / direction.y;
    }
     // Check intersection with Z-axis
    if((z_in > y_max)) {
        return false;
    }
    // if the ray y value is out of the volume, return false
    if(y_min > endPoint) {
        return false;
    }
    // replace the z value with the y value if the z value is smaller
    if(y_min > z_in) {
        z_in = y_min;
    }
    // replace the texture length with the y value if the y value is larger
    if(y_max < endPoint) {
        endPoint = y_max;
    }

    if(n.z < 0) {
        z_min = (ExtentMax.z - startPoint.z) / direction.z;
        z_max = (ExtentMin.z - startPoint.z) / direction.z;
    } else {
        z_min = (ExtentMin.z - startPoint.z) / direction.z;
        z_max = (ExtentMax.z - startPoint.z) / direction.z;
    }

    if(z_min > z_in) {
        z_in = z_min;
    }
    if(z_max < endPoint) {
        endPoint = z_max;
    }
    if(z_in > 0) {
        if(endPoint > 0) {
            if(z_in < endPoint) {
                // Ray hits the volume
                return true;
            }
        }
    }
    // if ray does not hit the volume, return false
    return false;
}

void main() {
    float xw = aspect_ratio * (gl_FragCoord.x - screen_width / 2.0 + 0.5) / screen_width;
    float yw = (gl_FragCoord.y - screen_height / 2.0 + 0.5) / screen_height;
    // 1/(2tan(pi*thetha/360)))
    float dirn = focal_H / (2.0 * tan(thetha * 3.14 / (180.0 * 2.0)));
    vec3 startPoint = cameraPos;
    direction = normalize(u * xw + v * yw - dirn * w);
    // Get the direction of the ray using the camera startPoint and the orthonormal basis
    // If the ray does not hit the volume, return black
    if(!does_hit(startPoint, direction)) {
        outColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }
    /*-------------------- Initialize the color and alpha value --------------------*/
    fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    int i = 1;
    float currPoint = z_in;
    while(i ==1) {
        dpos = startPoint + direction * currPoint;
        /*-------------------- Ray exits the volume and hence the loop is broken --------------------*/
        if(currPoint > endPoint) {
            break;
        }
        /*-------------------- Early Ray Termination if reached threashold --------------------*/
        if(fragColor.a > 0.95) {
            break;
        }
        // Get the color from the transfer function and the value from the volume texture
        /*--------------------- Tri-linear interpolation performed ---------------------*/
        value = texture(texture3d, (dpos + ((ExtentMax - ExtentMin) / 2)) / (ExtentMax - ExtentMin));
        s = value.r;
        vec4 src = texture(transferfun, s);
        // calculate the color and alpha value using the transfer function and the volume texture
        fragColor.rgb = fragColor.rgb + (1.0 - fragColor.a) * src.rgb * s;
        fragColor.a = fragColor.a + (1.0 - fragColor.a) * s;
        currPoint += stepSize;
    }
    outColor = fragColor;
}
