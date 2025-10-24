// compute shader
#version 430
#extension GL_ARB_shading_language_include : require

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#include "/common.glsl"

void main() {
    ivec3 index = ivec3(gl_GlobalInvocationID);
    Voxel voxel = getVoxel(index);

    // skip air voxels
    if (!isSolid(voxel)) {
        return;
    }
    if (voxel.emission != vec3(0.0)) {
        // TODO: do not early exit and just add to color, also preserve energy
        setColor(index, voxel.emission);
        return;
    }

    vec3 color = getColor(voxel);
    vec3 direction = vec3(1.0, 0.0, 0.0);
    vec3 position = vec3(index) + direction; // TODO: ensure position is not in the same voxel
    Ray ray = Ray(position, direction);
    RayCast rayCast = rayCast(ray);

    if (!rayCast.hit) {
        return;
    }
    float weight = 1.0; // TODO: cos-angle
    color = mix(color, weight * getColor(getVoxel(rayCast.voxelIndex)), 0.01);

    setColor(index, color);
}
