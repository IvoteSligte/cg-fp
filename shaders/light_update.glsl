// compute shader
#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#include "common.glsl"

const uint RANDOM_DIRECTION_COUNT = 16;

// NOTE: location = 0 is already taken by dbColorReadIdx in common.glsl

// number of frames since start
layout(location = 1) uniform uint frameNumber;
// packed normalized vec3 directions
layout(location = 2) uniform uint randomDirections[RANDOM_DIRECTION_COUNT];

// TODO: energy preservation or falloff term
// TODO: specular and translucent surfaces?

void main() {
    ivec3 index = ivec3(gl_GlobalInvocationID);
    Voxel voxel = getVoxel(index);

    // skip air voxels
    if (!isSolid(voxel)) {
        return;
    }
    uint seed = hash(uvec4(gl_GlobalInvocationID, frameNumber));
    // just do one face per frame for simplicity and because arrays take a lot of registers
    uint face = umod(seed, 6);
    vec3 normal = voxelFaceToNormal(face);
    // 2D offset on face
    vec2 offset = vec2(mod(float(seed), 8.0) / 8.0, mod(float(seed / 8), 8.0) / 8.0);
    // position on face
    vec3 position = vec3(index)
        + (normal * 0.5001 + 0.5) // push to face surface
        + (normal.yzx * offset.x) + (normal.zxy * offset.y); // add offset on face

    if (voxel.emission != vec3(0.0)) {
        setColor(index, face, voxel.emission);
        return;
    }
    vec3 color = vec3(0.0);
    uint samples = 0;

    for (int i = 0; i < RANDOM_DIRECTION_COUNT; i++) {
        // direction sampled based on cosine-weighted hemisphere
        vec3 direction = normal + unpackSnorm4x8(randomDirections[i]).xyz;
        if (direction == vec3(0.0)) {
            // skip to prevent NaN
            continue;
        }
        direction = normalize(direction);
        Ray ray = Ray(position, direction);

        if (isOutOfBounds(ray.origin)) {
            color += skyColor(ray.direction);
            samples += 1;
            continue;
        }
        // fixes light leaking through 2+ voxel thick walls
        if (isSolid(getVoxel(ivec3(ray.origin)))) {
            continue;
        }
        RayCast rayCast = rayCast(ray);

        if (!rayCast.hit) {
            color += skyColor(ray.direction);
            samples += 1;
            continue;
        }
        color += getColor(getVoxel(rayCast.voxelIndex), rayCast.face) * voxel.diffuse;
        samples += 1;
    }
    if (samples > 0) {
        const float BLEND_FACTOR = max(1.0 / sqrt(1.0 + frameNumber), 0.01);
        color /= samples;
        setColor(index, face, mix(getColor(voxel, face), color, BLEND_FACTOR));
    }
}
