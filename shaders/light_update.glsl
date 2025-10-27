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

    for (int i = 0; i < RANDOM_DIRECTION_COUNT; i++) {
        vec3 direction = randomDirections[i].xyz;
        vec3 normal = voxelNormal(direction);
        uint face = faceVoxelNormal(direction);
        // 2D offset on face
        vec2 offset = vec2(mod(float(seed), 8.0) / 8.0, mod(float(seed / 8), 8.0) / 8.0);
        // position on face
        vec3 position = vec3(index)
            + (normal * 0.5001 + 0.5) // push to face surface
            + (normal.yzx * offset.x) + (normal.zxy * offset.y); // add offset on face
        Ray ray = Ray(position, direction);

        if (isOutOfBounds(ray.origin)) {
            // faceColor[face] += SKY_COLOR;
            // faceSamples[face] += 1;
            continue;
        }
        // fixes light leaking through 2+ voxel thick walls
        if (isSolid(getVoxel(ivec3(position)))) {
            continue;
        }
        RayCast rayCast = rayCast(ray);

        if (!rayCast.hit) {
            // faceColor[face] += SKY_COLOR;
            // faceSamples[face] += 1;
            continue;
        }
        // cos-angle weight for diffuse surfaces
        float weight = dot(normal, normalize(rayCast.position - position));
        faceColor[face] += weight * getColor(getVoxel(rayCast.voxelIndex), rayCast.face) * voxel.diffuse;
        faceSamples[face] += 1;
    }
    float blendFactor = max(1.0 / (frameNumber + 1.0), 1e-3);

    for (int face = 0; face < 6; face++) {
        uint samples = faceSamples[face];
        vec3 color = faceColor[face];
        if (samples > 0) {
            color /= samples;
            setColor(index, face, mix(getColor(voxel, face), color, blendFactor));
        }
    }
}
