#version 450

layout(std140, binding = 0) uniform Transform {
    mat4 model;
    mat4 view;
    mat4 proj;
} transform;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = transform.proj * transform.view * vec4(inPosition, 1.0);
    fragColor = vec3(0.0, 0.5, 0.5);
}
