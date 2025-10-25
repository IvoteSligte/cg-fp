// compute shader
#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#include "common.glsl"

const uint RANDOM_DIRECTION_COUNT = 256;

// NOTE: location = 0 is already taken by dbColorReadIdx in common.glsl

// number of frames since start
layout(location = 1) uniform uint frameNumber;
// vec4 for randomDirections to ensure 16-byte alignment
layout(location = 2) uniform vec4 randomDirections[RANDOM_DIRECTION_COUNT];

// uses integer rounding to calculate the uint modulo `n mod m`
uint umod(uint n, uint m) {
    return n - (n / m) * m;
}

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
    vec3 direction = randomDirections[umod(frameNumber, RANDOM_DIRECTION_COUNT)].xyz;
    vec3 position = vec3(index) + direction; // TODO: ensure position is not in the same voxel
    Ray ray = Ray(position, direction);

    // FIXME: some positions are always out of bounds, others always in walls...

    if (isOutOfBounds(ray.origin)) {
        return;
    }
    // fix light leaking through 2+ voxel thick walls
    if (isSolid(getVoxel(ivec3(position)))) {
        return;
    }
    RayCast rayCast = rayCast(ray);

    if (!rayCast.hit) {
        // sky color
        // TODO: proper skybox or sky color function
        setColor(index, SKY_COLOR);
        return;
    }
    const float BLEND_FACTOR = 0.01;
    float weight = 1.0; // TODO: cos-angle
    color = mix(color, weight * getColor(getVoxel(rayCast.voxelIndex)), BLEND_FACTOR);
    // color = vec3(rayCast.steps / 10.0);

    setColor(index, color);
}
