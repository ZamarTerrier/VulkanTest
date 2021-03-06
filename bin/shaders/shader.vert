#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 sunDir;
    float size;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 sunDir;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition * ubo.size, 1.0);
    fragColor = inColor;
    fragNormal = normal;
    sunDir = ubo.sunDir;
}