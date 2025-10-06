#version 430

in vec2 inPos;
out vec2 outPos;

void main() {
    outPos = inPos;
    gl_Position = vec4(inPos, 0.0, 1.0);
}
