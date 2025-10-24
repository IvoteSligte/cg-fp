// compute shader
#version 430
#extension GL_ARB_shading_language_include : require

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#include "/common.glsl"

void main() {
    ivec3 index = ivec3(gl_GlobalInvocationID);
    vec3 emission = getVoxel(index).emission;
    vec3 color = getColor(getVoxel(index));
    color = max(color, emission);

    for (int x = -1; x <= 1; x += 1) {
        for (int y = -1; y <= 1; y += 1) {
            for (int z = -1; z <= 1; z += 1) {
                vec3 nbColor = getColor(getVoxel(ivec3(mod(ivec3(index) + ivec3(x, y, z), ivec3(CHUNK_SIZE)))));
                color = mix(color, nbColor, 0.01);
            }
        }
    }

    setColor(index, color);
}
