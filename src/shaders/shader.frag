#version 450


layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 sunDir;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const float ambient = 0.02f;

void main() {
    vec3 amb = ambient * (fragColor);

    float diff = max(dot(fragNormal,sunDir), 0.0);
    vec3 diffuse = vec3(1,1,1);
    diffuse = diffuse * diff * fragColor;


    vec3 result = amb + diffuse;

    outColor = vec4(result * texture(texSampler, fragTexCoord).rgb, texture(texSampler, fragTexCoord).a);
}