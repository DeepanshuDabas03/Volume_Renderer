#version 330 core

in vec3 fColor;
in vec3 cameraPos;
in vec3 ExtentMax;
in vec4 fragPos;
in vec3 ExtentMin;
in mat4 inverse_viewproj;

uniform float stepSize;

uniform sampler1D transferfun;
uniform sampler3D texture3d;

float screen_width = 640;
float screen_height = 640;

vec4 value;
float scalar;
vec4 dst = vec4(0, 0, 0, 0);
vec3 direction;
vec3 curren_pos, pos_max, pos_min;
float tmin_y, tmin_z, tmax_y, tmax_z, tmin, tmax;

vec3 up = vec3(0,1,0);
float aspect = screen_width/screen_height;
float focalHeight = 1.0; //Let's keep this fixed to 1.0
float focalDistance = focalHeight/(2.0 * tan(45 * 3.14/(180.0 * 2.0))); //More the fovy, close is focal plane
vec3 w = vec3(cameraPos - vec3(0,0,0));
vec3 w1 = normalize(w);
vec3 u = cross(up,w1);
vec3 u1 = normalize(u);
vec3 v = cross(w1,u1);
vec3 v1 = normalize(v);

out vec4 outColor;

bool rayintersection(vec3 position, vec3 dir)
{
        vec3 tMin = (ExtentMin - position) / dir;
        vec3 tMax = (ExtentMax - position) / dir;
        vec3 t1 = min(tMin, tMax);
        vec3 t2 = max(tMin, tMax);
        float tmin = max(max(t1.x, t1.y), t1.z);
        float tmax = min(min(t2.x, t2.y), t2.z);
        if(tmin > tmax)
        {
                return false;
        }
        return true;
}

void main()
{
        vec3 position = vec3(fragPos);
        direction += -(w1)*focalDistance;
        float xw = aspect*(fragPos.x - screen_width/2.0 + 0.5)/screen_width;
        float yw = (fragPos.y - screen_height/2.0 + 0.5)/screen_height;
        direction += u1 * xw;
        direction += v1 * yw;
        // direction = normalize(direction);

        if(!rayintersection(position,direction)){
                outColor = vec4(1.0,0.0,0.0,1.0);
                return;
        }

        dst = vec4(0,0,0,0);
        curren_pos = position + tmin*direction;
        float sum = 0;
        int i = 0;
        float t = tmin;
        for(i=0;;i+=1){
                value = texture(texture3d, (curren_pos+((ExtentMax - ExtentMin)/2))/(ExtentMax-ExtentMin));
                sum += value.a;
                scalar = value.a;
                vec4 src = texture(transferfun,scalar);

                dst = (1.0-dst.a)*src + dst;

                t += stepSize;
                curren_pos = curren_pos + direction*t;
                
                if(curren_pos.x<ExtentMin.x || curren_pos.y<ExtentMin.y || curren_pos.z<ExtentMin.z
                        || curren_pos.x>ExtentMax.x || curren_pos.y>ExtentMax.y || curren_pos.z>ExtentMax.z)
                        {
                                break;
                        }
                if(t>tmax){
                        break;
                }
                if(dst.a > 0.95){
                        break;
                }
        }
        sum /=i;
         outColor = dst;
         // outColor = vec4(sum, sum, sum, 1.0);

        // outColor = vec4(tmin, tmin, tmin,1.0);
}
