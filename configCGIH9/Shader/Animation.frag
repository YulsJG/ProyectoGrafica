#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct Material {
    float shininess;
};
uniform Material material;

void main()
{
    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;

    // Ambient
    vec3 ambient = dirLight.ambient * texColor;

    // Diffuse
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(-dirLight.direction);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = dirLight.diffuse * diff * texColor;

    // Specular
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular   = dirLight.specular * spec;

    color = vec4(ambient + diffuse + specular, 1.0);
}