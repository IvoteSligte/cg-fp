// vertex shader
#version 430

in vec2 inPos;
out vec2 fragPos;

void main() {
    // map [-1, 1] to [0, 1]
    fragPos = inPos * 0.5 + 0.5;
    gl_Position = vec4(inPos, 0.0, 1.0);
}
