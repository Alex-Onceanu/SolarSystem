#version 300 es
precision highp float;

in vec2 aPos;
out vec2 vFragPos;

void main() {
    gl_Position = vec4(aPos, 1.0, 1.0);
    vFragPos = aPos;
}