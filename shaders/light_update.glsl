// compute shader
#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#include "common.glsl"

const uint RANDOM_DIRECTION_COUNT = 256;
const uint RAYS_PER_FRAME = 16;

// NOTE: location = 0 is already taken by dbColorReadIdx in common.glsl

// number of frames since start
layout(location = 1) uniform uint frameNumber;
// vec4 for randomDirections to ensure 16-byte alignment
layout(location = 2) uniform vec4 randomDirections[RANDOM_DIRECTION_COUNT];

// uses integer rounding to calculate the uint modulo `n mod m`
uint umod(uint n, uint m) {
    return n - (n / m) * m;
}

// Returns the normal of the face that a ray from the center of the voxel
// with direction OUT_DIRECTION goes through.
vec3 voxelNormal(vec3 outDirection) {
    vec3 dir = abs(outDirection);
    // normal is that of one of six faces, of three dimensions (d)
    int d = 2;
    if (dir.x > dir.y) {
        if (dir.x > dir.z) d = 0;
    } else {
        if (dir.y > dir.z) d = 1;
    }
    vec3 norm = vec3(0.0);
    norm[d] = sign(outDirection)[d];
    return norm;
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

    uint seed = hash(uvec4(gl_GlobalInvocationID, frameNumber));
    vec3 color = vec3(0.0);
    uint samples = 0;

    for (int i = 0; i < RAYS_PER_FRAME; i++) {
        vec3 direction = randomDirections[umod(seed + i, RANDOM_DIRECTION_COUNT)].xyz;
        vec3 normal = voxelNormal(direction);
        // using the normal as offset (plus a small epsilon) ensures the ray origin
        // is not in the same voxel
        vec3 position = vec3(index) + (normal * 0.5001 + 0.5);
        Ray ray = Ray(position, direction);

        if (isOutOfBounds(ray.origin)) {
            // TODO: proper skybox or sky color function
            continue;
        }
        // fix light leaking through 2+ voxel thick walls
        if (isSolid(getVoxel(ivec3(position)))) {
            continue;
        }
        RayCast rayCast = rayCast(ray);

        if (!rayCast.hit) {
            // TODO: proper skybox or sky color function
            continue;
        }

        float weight = 1.0; // TODO: cos-angle
        color += weight * getColor(getVoxel(rayCast.voxelIndex));
        samples += 1;
    }
    if (samples > 0) {
        const float BLEND_FACTOR = 0.1;

        color /= samples;
        setColor(index, mix(getColor(voxel), color, BLEND_FACTOR));
    }
}
