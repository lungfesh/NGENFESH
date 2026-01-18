#version 460 core
out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
uniform sampler2D Texture;
uniform bool useTexture;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform float ambientStrength;

void main() {
    vec3 ambient = lightColor * ambientStrength;
    // if (useTexture)
    //     FragColor = texture(Texture, TexCoord) * vec4(color, 1.0f);
    // else {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0f);
        vec3 diffuse = diff * lightColor;

        FragColor = vec4((ambient+diffuse)*color, 1.0f);
    // }
}