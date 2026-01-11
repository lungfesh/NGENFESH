#version 460 core
out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
uniform bool useTexture;
uniform sampler2D Texture;
void main() {
    // FragColor = vec4(color, 1.0f);
    if (useTexture)
        FragColor = texture(Texture, TexCoord) * vec4(color, 1.0f);
    else    
        FragColor = vec4(color, 1.0f);
}