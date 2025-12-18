#version 410 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirectionalLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutoff;
    float outerCutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;


void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 materialDiffuse = vec3(texture(material.diffuse, texCoord));
    vec3 materialSpecular = vec3(texture(material.specular, texCoord));
    
    vec3 result = vec3(0.0);
    
    // ============================================
    // Directional Light (sun-like, no position/attenuation)
    // ============================================
    vec3 dirLightDir = normalize(-directionalLight.direction);
    float dirDiff = max(dot(norm, dirLightDir), 0.0);
    vec3 dirReflectDir = reflect(-dirLightDir, norm);
    float dirSpec = pow(max(dot(viewDir, dirReflectDir), 0.0), material.shininess);
    
    vec3 dirAmbient = directionalLight.ambient * materialDiffuse;
    vec3 dirDiffuse = directionalLight.diffuse * dirDiff * materialDiffuse;
    vec3 dirSpecular = directionalLight.specular * dirSpec * materialSpecular;
    
    result += dirAmbient + dirDiffuse + dirSpecular;
    
    // ============================================
    // Point Light (light bulb, has position and attenuation)
    // ============================================
    vec3 pointLightDir = normalize(pointLight.position - fragPos);
    float pointDistance = length(pointLight.position - fragPos);
    float pointAttenuation = 1.0 / (pointLight.constant + pointLight.linear * pointDistance + pointLight.quadratic * (pointDistance * pointDistance));
    
    float pointDiff = max(dot(norm, pointLightDir), 0.0);
    vec3 pointReflectDir = reflect(-pointLightDir, norm);
    float pointSpec = pow(max(dot(viewDir, pointReflectDir), 0.0), material.shininess);
    
    vec3 pointAmbient = pointLight.ambient * materialDiffuse;
    vec3 pointDiffuse = pointLight.diffuse * pointDiff * materialDiffuse;
    vec3 pointSpecular = pointLight.specular * pointSpec * materialSpecular;
    
    result += pointAttenuation * (pointAmbient + pointDiffuse + pointSpecular);
    
    // ============================================
    // Spot Light (flashlight, has position, direction, cutoff, and attenuation)
    // ============================================
    vec3 spotLightDir = normalize(spotLight.position - fragPos);
    vec3 spotDirection = normalize(spotLight.direction);
    float theta = dot(spotLightDir, -spotDirection); // Angle between fragment-to-light and light direction
    
    // Check if fragment is within the outer cone (includes falloff region)
    if(theta > spotLight.outerCutoff)
    {
        float spotDistance = length(spotLight.position - fragPos);
        // Using same attenuation formula as point light (you might want separate constants)
        float spotAttenuation = 1.0 / (1.0 + 0.09 * spotDistance + 0.032 * (spotDistance * spotDistance));
        
        // Smooth edges - intensity falls off from inner cutoff (1.0) to outer cutoff (0.0)
        // When theta = cutoff: intensity = 1.0 (full brightness)
        // When theta = outerCutoff: intensity = 0.0 (no light)
        float epsilon = spotLight.cutoff - spotLight.outerCutoff;
        float intensity = clamp((theta - spotLight.outerCutoff) / epsilon, 0.0, 1.0);
        
        float spotDiff = max(dot(norm, spotLightDir), 0.0);
        vec3 spotReflectDir = reflect(-spotLightDir, norm);
        float spotSpec = pow(max(dot(viewDir, spotReflectDir), 0.0), material.shininess);
        
        vec3 spotAmbient = spotLight.ambient * materialDiffuse;
        vec3 spotDiffuse = spotLight.diffuse * spotDiff * materialDiffuse;
        vec3 spotSpecular = spotLight.specular * spotSpec * materialSpecular;
        
        result += spotAttenuation * intensity * (spotAmbient + spotDiffuse + spotSpecular);
    }
    
    FragColor = vec4(result, 1.0f);
}