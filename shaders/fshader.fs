#version 330 core

in vec3 fColor;
in vec3 cameraPos;
in vec3 ExtentMax;
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
float delta_t = 0.0001;
vec3 curren_pos, pos_max, pos_min;
float tentry, texit;
vec3 model_center = vec3(0, 0, 0);
float distance = length(model_center - cameraPos);
float radius = length(ExtentMax - ExtentMin)/2.0;
float tmin = distance - radius;
float tmax = distance + radius;

out vec4 outColor;

// bool rayintersection(vec3 position, vec3 dir)
// {
//         // vec3 tMin = (ExtentMin - position) / dir;
//         // vec3 tMax = (ExtentMax - position) / dir;
//         // vec3 t1 = min(tMin, tMax);
//         // vec3 t2 = max(tMin, tMax);
//         // float tentry = max(max(t1.x, t1.y), t1.z);
//         // float texit = min(min(t2.x, t2.y), t2.z);

//         float tx1 = (ExtentMin.x - dir.x)/dir.x;
//         float tx2 = (ExtentMax.x - dir.x)/dir.x;
//         tentry = min(tx1, tx2);
//         texit = max(tx1, tx2);
//         float ty1 = (ExtentMin.y - dir.y)/dir.y;
//         float ty2 = (ExtentMax.y - dir.y)/dir.y;
//         tentry = max(tentry, min(ty1, ty2));
//         texit = min(texit, max(ty1, ty2));
//         if(tentry > texit)
//         {
//                 return false;
//         }
//         return true;
// }

bool rayintersection(vec3 position, vec3 dir)
{
        float temp;
        tentry = (ExtentMin.x - position.x) / dir.x; 
        texit = (ExtentMax.x - position.x) / dir.x; 
        
        if (tentry > texit){
                temp = tentry;
                tentry = texit;
                texit = temp;
        }
        
        float tymin = (ExtentMin.y - position.y) / dir.y; 
        float tymax = (ExtentMax.y - position.y) / dir.y; 
        
        if (tymin > tymax){
                temp = tymin;
                tymin = tymax;
                tymax = temp;
        } 
        
        if ((tentry > tymax) || (tymin > texit)) 
                return false; 
        
        if (tymin > tentry) 
                tentry = tymin; 
        
        if (tymax < texit) 
                texit = tymax; 
        
        float tzmin = (ExtentMin.z - position.z) / dir.z; 
        float tzmax = (ExtentMax.z - position.z) / dir.z; 
        
        if (tzmin > tzmax){
                temp = tzmin;
                tzmin = tzmax;
                tzmax = temp;
        }
        
        if ((tentry > tzmax) || (tzmin > texit)) 
                return false; 
        
        if (tzmin > tentry) 
                tentry = tzmin; 
        
        if (tzmax < texit) 
                texit = tzmax; 
        return true; 
}

void main()
{
        vec4 ndc = vec4((gl_FragCoord.x/screen_width - 0.5)*2.0, (gl_FragCoord.y/screen_height - 0.5)*2.0, 
                                (gl_FragCoord.z - 0.5)*2.0, 1.0);
        ndc.z = (2*gl_FragCoord.z - (gl_DepthRange.near + gl_DepthRange.far))/gl_DepthRange.diff;
        // vec4 glposition = ndc/gl_FragCoord.w;
        // vec3 position = vec3(inverse_viewproj*glposition);
        vec4 glposition = inverse_viewproj*ndc;
        vec3 position = (glposition/glposition.w).xyz;

        direction = normalize(position - cameraPos);

        if(!rayintersection(position,direction)){
                outColor = vec4(1.0,0.0,0.0,1.0);
                return;
        }

        dst = vec4(0,0,0,0);
        curren_pos = position + tentry*direction;
        float sum = 0;
        int i = 0;
        float t = tentry;
        float tnorm = ((tentry - tmin)/(tmax- tmin));
        for(i=0;;i+=1){
                value = texture(texture3d, (curren_pos+((ExtentMax - ExtentMin)/2))/(ExtentMax-ExtentMin));
                sum += value.r;
                scalar = value.r;
                vec4 src = texture(transferfun,scalar);

                dst = (1.0-dst.a)*src + dst;

                t += delta_t;
                curren_pos = position + direction*t;
                // if(curren_pos.x<ExtentMin.x || curren_pos.y<ExtentMin.y || curren_pos.z<ExtentMin.z
                //         || curren_pos.x>ExtentMax.x || curren_pos.y>ExtentMax.y || curren_pos.z>ExtentMax.z)
                //         {
                //                 break;
                //         }
                if(t>texit){
                        break;
                }
                if(dst.a > 0.95){
                        break;
                }
        }
        outColor = vec4(tnorm, tnorm, tnorm, 1.0) + dst*0;
        sum /= i;
        // outColor = dst;
        // outColor = vec4(sum, sum, sum, 1.0)+dst*0;
}
