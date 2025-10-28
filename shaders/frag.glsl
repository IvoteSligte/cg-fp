// fragment shader
#version 430

in vec2 fragPos;
out vec4 fragColor;

#include "common.glsl"

// NOTE: names used for sync with CPU, do not change
// NOTE: location = 0 is already taken by dbColorReadIdx in common.glsl
layout(location = 1) uniform vec3 position;
layout(location = 2) uniform mat3 rotation;
layout(location = 3) uniform float aspectRatio;

// Maps Linear RGB to sRGB
// By Tynach from https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
vec3 toSRGB(vec3 linear) {
    bvec3 cutoff = lessThan(linear, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linear, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = linear * vec3(12.92);
    return mix(higher, lower, cutoff);
}

void main() {
    vec2 screenPos = (fragPos * 2.0 - 1.0) * vec2(aspectRatio, 1.0);
    vec3 direction = rotation * normalize(vec3(screenPos.x, screenPos.y, -1.0));
    Ray ray = Ray(position, direction);

    // camera is outside chunk
    if (isOutOfBounds(ray.origin)) {
        // magenta as error color
        fragColor = vec4(vec3(1.0, 0.0, 1.0), 1.0);
        return;
    }
    RayCast rayCast = rayCast(ray);
    vec3 color = vec3(0.0);

    if (rayCast.hit) {
        ivec3 index = rayCast.voxelIndex;
        Voxel hitVoxel = getVoxel(index);
        color = getColor(hitVoxel, rayCast.face);
    } else {
        color = skyColor(ray.direction);
    }
    fragColor = vec4(toSRGB(color), 1.0);
}
