#version 460 core
out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
uniform bool useTexture;
uniform sampler2D Texture;
uniform vec3 lightColor;
uniform float ambientStrength;
void main() {
    vec3 ambient = lightColor * ambientStrength;
    if (useTexture)
        FragColor = texture(Texture, TexCoord) * vec4(color, 1.0f);
    else    
        FragColor = vec4(ambient*color, 1.0f);
        // FragColor = vec4(color, 1.0f);
}