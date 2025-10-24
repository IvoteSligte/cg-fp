// fragment shader
#version 430
#extension GL_ARB_shading_language_include : require

in vec2 fragPos;
out vec4 fragColor;

#include "/common.glsl"

// NOTE: names used for sync with CPU, do not change
// NOTE: location = 0 is already taken by dbColorReadIdx in common.glsl
layout(location = 1) uniform vec3 position;
layout(location = 2) uniform mat3 rotation;

void main() {
    vec2 aspectRatio = vec2(1.0); // TODO:
    
    vec2 screenPos = (fragPos * 2.0 - 1.0) * aspectRatio;
    vec3 direction = rotation * normalize(vec3(screenPos.x, screenPos.y, -1.0));
    Ray ray = Ray(position, direction);

    // camera is outside chunk
    if (any(lessThan(ray.origin, CHUNK_MIN)) || any(greaterThan(ray.origin, CHUNK_MAX))) {
        // magenta (indicating error)
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }
    RayCast rayCast = rayCast(ray);
    vec3 color = vec3(0.0);

    if (rayCast.hit) {
        ivec3 index = rayCast.voxelIndex;
        color = getColor(getVoxel(index));
    } else {
        // sky color
        color = vec3(0.2, 0.2, 0.7);
    }
    fragColor = vec4(color, 1.0);
}
