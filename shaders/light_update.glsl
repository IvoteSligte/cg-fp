// compute shader
#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#include "common.glsl"

const uint RANDOM_DIRECTION_COUNT = 256;
const uint RAYS_PER_FRAME = 64;

// NOTE: location = 0 is already taken by dbColorReadIdx in common.glsl

// number of frames since start
layout(location = 1) uniform uint frameNumber;
// vec4 for randomDirections to ensure 16-byte alignment
layout(location = 2) uniform vec4 randomDirections[RANDOM_DIRECTION_COUNT];

// uses integer rounding to calculate the uint modulo `n mod m`
uint umod(uint n, uint m) {
    return n - (n / m) * m;
}

// TODO: energy preservation or falloff term
// TODO: specular and translucent surfaces?

void main() {
    ivec3 index = ivec3(gl_GlobalInvocationID);
    Voxel voxel = getVoxel(index);

    // skip air voxels
    if (!isSolid(voxel)) {
        return;
    }
    if (voxel.emission != vec3(0.0)) {
        // TODO: do not early exit and just add to color, also preserve energy
        for (uint face = 0; face < 6; face++) {
            setColor(index, face, voxel.emission);
        }
        return;
    }

    uint seed = hash(uvec4(gl_GlobalInvocationID, frameNumber));

    // storing a vec3[6] and uint[6] as locals takes a lot of registers
    // and will likely cause thrashing but we ignore that for now
    vec3 faceColor[6];
    uint faceSamples[6];
    for (int face = 0; face < 6; face++) {
        faceColor[face] = vec3(0.0);
        faceSamples[face] = 0;
    }

    // FIXME: weird light falloff based on distance?

    for (int i = 0; i < RAYS_PER_FRAME; i++) {
        vec3 direction = randomDirections[umod(seed + i, RANDOM_DIRECTION_COUNT)].xyz;
        vec3 normal = voxelNormal(direction);
        uint face = faceVoxelNormal(direction);
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
        // cos-angle weight for diffuse surfaces
        float weight = dot(normal, normalize(rayCast.position - position));
        faceColor[face] += weight * getColor(getVoxel(rayCast.voxelIndex), rayCast.face) * voxel.diffuse;
        faceSamples[face] += 1;
    }
    float blendFactor = max(1.0 / (frameNumber + 1.0), 1e-4);

    for (int face = 0; face < 6; face++) {
        uint samples = faceSamples[face];
        vec3 color = faceColor[face];
        if (samples > 0) {
            color /= samples;
            setColor(index, face, mix(getColor(voxel, face), color, blendFactor));
        }
    }
}
