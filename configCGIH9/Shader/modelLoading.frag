#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3  position;
    float constant;
    float linear;
    float quadratic;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
};

uniform sampler2D texture_diffuse1;
uniform vec3      viewPos;
uniform DirLight  dirLight;
uniform PointLight pointLights[4];

// ── Cuánto conserva la textura original (0.0 = iluminación pura, 1.0 = textura pura)
uniform float factorNoche;   // lo mandas desde el CPU

void main()
{
    vec4 texSample = texture(texture_diffuse1, TexCoords);
    vec3 texColor  = texSample.rgb;
    vec3 norm      = normalize(Normal);
    vec3 viewDir   = normalize(viewPos - FragPos);

    // ── Luz direccional ───────────────────────────────────────
    vec3 lightDir = normalize(-dirLight.direction);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 reflDir  = reflect(-lightDir, norm);
    float spec    = pow(max(dot(viewDir, reflDir), 0.0), 16.0);

    vec3 dirResult = (dirLight.ambient + dirLight.diffuse * diff) * texColor
                   +  dirLight.specular * spec;

    // ── Point lights (lámparas de stands) ────────────────────
    vec3 pointResult = vec3(0.0);
    for (int i = 0; i < 4; i++)
    {
        vec3  lDir  = normalize(pointLights[i].position - FragPos);
        float lDiff = max(dot(norm, lDir), 0.0);
        float lDist = length(pointLights[i].position - FragPos);
        float att   = 1.0 / (pointLights[i].constant
                            + pointLights[i].linear    * lDist
                            + pointLights[i].quadratic * lDist * lDist);

        vec3 luzColor = pointLights[i].ambient + pointLights[i].diffuse * lDiff;
pointResult  += mix(luzColor * texColor, luzColor, 0.3) * att;
    }

    // ── Mezcla: de día = textura brillante, de noche = iluminación calculada ──
    // factorNoche 0.0 → textura pura (como antes)
    // factorNoche 1.0 → iluminación completa con point lights
    vec3 resultDia   = texColor * 0.9;                        // textura original, un poco boost
    vec3 resultNoche = dirResult + pointResult;

    vec3 finalColor  = mix(resultDia, resultNoche, factorNoche);

    color = vec4(clamp(finalColor, 0.0, 1.0), texSample.a);
}