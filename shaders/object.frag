#version 460 core
out vec4 FragColor;
in vec3 color;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
uniform sampler2D Texture;
uniform bool useTexture;

uniform vec3 viewPos;

struct PointLight {    
    vec3 position;
    vec3 color;
    float specularStrength;
};
uniform PointLight pointLights[16];
uniform int numPointLights;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color; // diffuse lighting
    // specular shading
    float spec = pow(max(dot(viewDir,reflectDir),0.0f),32); // specular lighting
    vec3 specular = light.specularStrength*spec*light.color;

    return (diffuse);
} 

void main() {
    float ambientStrength = 0.1f;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 ambient = ambientStrength * vec3(1.0f,1.0f,1.0f);
    // if (!gl_FrontFacing) {
    //     norm = -norm; // if not facing front side, reverse normal
    // }
    vec3 lighting = ambient; 
    for (int i=0;i < 2; i++) {
        lighting += CalcPointLight(pointLights[i], norm, viewDir);
    }

    vec4 result = vec4(lighting*color.rgb, 1.0f);
    if (useTexture)
        result = vec4(result.rgb * texture(Texture, TexCoord).rgb, texture(Texture, TexCoord).a);
    FragColor = result;
    // if (gl_FrontFacing) {
    //     FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // } else {
    //     FragColor = vec4(0.0, 0.0, 1.0, 1.0);
}
