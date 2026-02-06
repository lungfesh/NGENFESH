#version 460 core
out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
uniform sampler2D Texture;
uniform bool useTexture;

void main() {
    if (useTexture)
        FragColor = vec4(texture(Texture, TexCoord).rgb*color, texture(Texture, TexCoord).a);    
    else
        FragColor = vec4(color, 1.0f);
}