#version 330 core

in vec3 fColor;
in vec3 cameraPos;
in vec3 ExtentMax;
in vec3 ExtentMin;

out vec4 outColor;

uniform float stepSize;
uniform sampler1D transferfun;
uniform sampler3D texture3d;

float screen_width = 800;
float screen_height = 800;

float z_in;
float texture_l;

bool does_hit(vec3 position, vec3 direction) {
    float y_min, y_max, z_min, z_max;
    vec3 n = 1 / direction;
    // if the sign is negative, the ray is going in the negative direction
    if(n.x < 0) {
        z_in = (ExtentMax.x - position.x) / direction.x;
        texture_l = (ExtentMin.x - position.x) / direction.x;
    } else {
        z_in = (ExtentMin.x - position.x) / direction.x;
        texture_l = (ExtentMax.x - position.x) / direction.x;
    }

    if(n.y < 0) {
        y_min = (ExtentMax.y - position.y) / direction.y;
        y_max = (ExtentMin.y - position.y) / direction.y;
    } else {
        y_min = (ExtentMin.y - position.y) / direction.y;
        y_max = (ExtentMax.y - position.y) / direction.y;
    }
     // Check intersection with Z-axis
    if((z_in > y_max)) {
        return false;
    }
    // if the ray y value is out of the volume, return false
    if(y_min > texture_l) {
        return false;
    }
    // replace the z value with the y value if the z value is smaller
    if(y_min > z_in) {
        z_in = y_min;
    }
    // replace the texture length with the y value if the y value is larger
    if(y_max < texture_l) {
        texture_l = y_max;
    }

    if(n.z < 0) {
        z_min = (ExtentMax.z - position.z) / direction.z;
        z_max = (ExtentMin.z - position.z) / direction.z;
    } else {
        z_min = (ExtentMin.z - position.z) / direction.z;
        z_max = (ExtentMax.z - position.z) / direction.z;
    }

    if(z_min > z_in) {
        z_in = z_min;
    }
    if(z_max < texture_l) {
        texture_l = z_max;
    }
    if(z_in > 0) {
        if(texture_l > 0) {
            if(z_in < texture_l) {
                // Ray hits the volume
                return true;
            }
        }
    }
    // if ray does not hit the volume, return false
    return false;
}

vec4 trilinearInterpolation(vec3 pos) {
    vec3 voxelPos = pos * textureSize(texture3d, 0);
    vec3 voxelMin = floor(voxelPos);
    vec3 voxelMax = voxelMin + vec3(1.0);
    vec3 t = voxelPos - voxelMin;

    vec4 c000 = texture(texture3d, voxelMin / textureSize(texture3d, 0));
    vec4 c100 = texture(texture3d, vec3(voxelMax.x, voxelMin.yz) / textureSize(texture3d, 0));
    vec4 c010 = texture(texture3d, vec3(voxelMin.x, voxelMax.y, voxelMin.z) / textureSize(texture3d, 0));
    vec4 c001 = texture(texture3d, vec3(voxelMin.xy, voxelMax.z) / textureSize(texture3d, 0));
    vec4 c110 = texture(texture3d, vec3(voxelMax.xy, voxelMin.z) / textureSize(texture3d, 0));
    vec4 c101 = texture(texture3d, vec3(voxelMax.x, voxelMin.y, voxelMax.z) / textureSize(texture3d, 0));
    vec4 c011 = texture(texture3d, vec3(voxelMin.x, voxelMax.yz) / textureSize(texture3d, 0));
    vec4 c111 = texture(texture3d, voxelMax / textureSize(texture3d, 0));

    vec4 c00 = mix(c000, c100, t.x);
    vec4 c01 = mix(c001, c101, t.x);
    vec4 c10 = mix(c010, c110, t.x);
    vec4 c11 = mix(c011, c111, t.x);

    vec4 c0 = mix(c00, c10, t.y);
    vec4 c1 = mix(c01, c11, t.y);

    return mix(c0, c1, t.z);
}

void main() {
    vec3 dir = normalize(fColor - cameraPos);
    vec3 start = ExtentMin;
    vec3 end = ExtentMax;
    vec3 delta = stepSize * dir;
    if(!does_hit(cameraPos, dir)) {
        outColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }
    vec4 accumColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 sampleColor;
    float alpha;

    for (float t = 0.0; t < 1.0; t += stepSize) {
        vec3 pos = start + t * (end - start);
        float voxel = trilinearInterpolation(pos).r;
        sampleColor = texture(transferfun, voxel);
        alpha = sampleColor.a * stepSize;

        accumColor.rgb += (1.0 - accumColor.a) * sampleColor.rgb * alpha;
        accumColor.a += alpha;

        if (accumColor.a >= 1.0) break;
    }

    outColor = accumColor;
}
