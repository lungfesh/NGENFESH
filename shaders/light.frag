#version 460 core
out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
uniform vec3 lightColor;
void main() {
    FragColor = vec4(lightColor, 1.0f);
}