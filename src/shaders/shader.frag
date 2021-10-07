#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 sunDir;

layout(location = 0) out vec4 outColor;

const float ambient = 0.02f;

void main() {
    vec3 amb = ambient * (fragColor);

    float diff = max(dot(fragNormal,sunDir), 0.0);
    vec3 diffuse = vec3(1,1,1);
    diffuse = diffuse * diff * fragColor;


    vec3 result = amb + diffuse;

    outColor = vec4(result, 1.0f);
}