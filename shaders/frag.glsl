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

void main() {
    vec2 screenPos = (fragPos * 2.0 - 1.0) * vec2(aspectRatio, 1.0);
    vec3 direction = rotation * normalize(vec3(screenPos.x, screenPos.y, -1.0));
    // TODO: just set position of character to CHUNK_WIDTH/2
    Ray ray = Ray(position, direction);

    // camera is outside chunk
    if (isOutOfBounds(ray.origin)) {
        fragColor = vec4(ERROR_COLOR, 1.0);
        return;
    }
    RayCast rayCast = rayCast(ray);
    vec3 color = vec3(0.0);

    if (rayCast.hit) {
        ivec3 index = rayCast.voxelIndex;
        Voxel hitVoxel = getVoxel(index);
        color = getColor(hitVoxel, rayCast.face);
    } else {
        color = SKY_COLOR;
    }
    fragColor = vec4(color, 1.0);
}
