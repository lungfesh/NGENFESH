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
uniform vec3 viewPos;

void main() {
    float ambientStrength =  0.1f;
    float specularStrength = 0.5f;

    vec3 ambient = lightColor * ambientStrength;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    vec3 viewDir = normalize(viewPos-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir,reflectDir),0.0f),32);
    vec3 specular = specularStrength*spec*lightColor;
    
    vec3 lighting = (ambient + diffuse + specular) * color.rgb;
    
    if (useTexture)
        FragColor = vec4(texture(Texture, TexCoord).rgb*lighting, texture(Texture, TexCoord).a);    
    else
        FragColor = vec4(lighting, 1.0f);
}