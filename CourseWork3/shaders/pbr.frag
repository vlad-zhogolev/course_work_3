#version 330 core

// output color
out vec4 FragColor;

// input data
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material maps
uniform sampler2D texture_albedo1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metallic1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

uniform vec3 cameraPos;

struct PointLight {
    vec3 position;
    vec3 color;   

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;    
};

struct DirLight {
    vec3 direction;
    vec3 color;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight{
    vec3 position;
    vec3 direction;
    vec3 color;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;  //cosine actually
    float outerCutOff;
};

struct Material {        
    vec3 albedo;
    vec3 normal;    
    float metallic;
    float ao;
    float roughness;
};

const float PI                      = 3.14159265359;
const int   MAX_DIR_LIGHTS_NUMBER   = 4;
const int   MAX_POINT_LIGHTS_NUMBER = 32;
const int   MAX_SPOT_LIGHTS_NUMBER  = 32;

uniform int dirLightsNumber;
uniform DirLight dirLights[MAX_DIR_LIGHTS_NUMBER];
uniform int pointLightsNumber;
uniform PointLight pointLights[MAX_POINT_LIGHTS_NUMBER];
uniform int spotLightsNumber;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS_NUMBER];

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   =  normalize(Normal);
    vec3 T   =  normalize(Q1*st2.t - Q2*st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN =  mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 calcPointLight(PointLight light, Material material, vec3 fragPos, vec3 viewDir, vec3 F0)
{
    // calculate per-light radiance
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfway = normalize(viewDir + lightDir);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.color * attenuation;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(material.normal, halfway, material.roughness);   
    float G   = GeometrySmith(material.normal, viewDir, lightDir, material.roughness);      
    vec3  F   = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
      
    vec3 nominator    = NDF * G * F; 
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float denominator = 4 * NdotV * NdotL + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - material.metallic;     

    // scale light by NdotL add to outgoing radiance Lo 
    return (light.ambient * material.albedo + kD * light.diffuse * material.albedo / PI + light.specular * specular) * radiance * NdotL;
}
// ----------------------------------------------------------------------------
vec3 calcDirLight(DirLight light, Material material, vec3 viewDir, vec3 F0)
{
    // calculate per-light radiance    
    vec3 halfway = normalize(viewDir + light.direction);
    // float distance = length(light.position - fragPos);
    // float attenuation = 1.0 / (distance * distance); 
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(material.normal, halfway, material.roughness);   
    float G   = GeometrySmith(material.normal, viewDir, light.direction, material.roughness);      
    vec3  F   = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
      
    vec3 nominator    = NDF * G * F; 
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, light.direction), 0.0);
    float denominator = 4 * NdotV * NdotL + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - material.metallic;     

    // scale light by NdotL add to outgoing radiance Lo 
    return (light.ambient * material.albedo + kD * light.diffuse * material.albedo / PI + light.specular * specular) * light.color * NdotL;      
}
// ----------------------------------------------------------------------------
vec3 calcSpotLight(SpotLight light, Material material, vec3 fragPos, vec3 viewDir, vec3 F0)
{
    // calculate per-light radiance
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfway = normalize(viewDir + lightDir);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.color * attenuation;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(material.normal, halfway, material.roughness);   
    float G   = GeometrySmith(material.normal, viewDir, lightDir, material.roughness);      
    vec3  F   = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
      
    vec3 nominator    = NDF * G * F; 
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float denominator = 4 * NdotV * NdotL + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - material.metallic;     

    // intensity
    float angle = dot(lightDir, normalize(-light.direction));   
    float intensity = clamp((angle - light.outerCutOff) / 
                            (light.cutOff - light.outerCutOff), 
                            0.0, 1.0);

    // scale light by NdotL add to outgoing radiance Lo 
    return (light.ambient * material.albedo + kD * light.diffuse * material.albedo / PI + light.specular * specular) * intensity * radiance * NdotL;
}
// ----------------------------------------------------------------------------
void main()
{		
    Material material;
    material.albedo    = pow(texture(texture_albedo1, TexCoords).rgb, vec3(2.2));
    material.metallic  = texture(texture_metallic1, TexCoords).r;
    material.roughness = texture(texture_roughness1, TexCoords).r;
    material.ao        = texture(texture_ao1, TexCoords).r;
    material.normal    = getNormalFromMap();

    vec3 V = normalize(cameraPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, material.albedo, material.metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < pointLightsNumber; ++i)     
        Lo += calcPointLight(pointLights[i], material, WorldPos, V, F0);  

    for(int i = 0; i < dirLightsNumber; ++i)
        Lo += calcDirLight(dirLights[i], material, V, F0);  
    
    for(int i = 0; i < spotLightsNumber; ++i)
        Lo += calcSpotLight(spotLights[i], material, WorldPos, V, F0);

    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * material.albedo * material.ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}