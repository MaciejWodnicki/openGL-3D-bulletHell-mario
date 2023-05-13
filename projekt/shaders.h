#pragma once

////////////////////////////////////////////
//
//shaderProgram


const char* vertexShaderSource = R"glsl(
#version 430 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UVPos;
layout (location = 3) uniform mat4 u_ProjMatrix;
layout (location = 4) uniform mat4 u_ViewMatrix;
layout (location = 5) uniform mat4 u_ModelMatrix;


uniform mat4 lightSpaceMatrix;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

out vec2 UV;

void main()
{
    vs_out.FragPos = vec3(u_ModelMatrix * vec4(Pos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(u_ModelMatrix))) * Normal;
    vs_out.TexCoords = UVPos;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
   gl_Position = u_ProjMatrix * u_ViewMatrix * u_ModelMatrix * vec4(Pos, 1.0);
    UV = UVPos;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 430 core

out vec4 FragColor;
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

in vec2 UV;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform sampler2D TextureSampler;

uniform vec3 lightPos;
uniform vec3 viewPos;

layout (location = 6) uniform vec4 CubeColor;
layout (location = 7) uniform vec3 u_LightColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r + 0.01; //0.01 shift to address high shadowmap brigthness
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{ 
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = u_LightColor;
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1 - shadow) * (diffuse + specular));
    
    FragColor = vec4(lighting, 1.0) * texture(TextureSampler, UV) * CubeColor;
}
)glsl";

////////////////////////////////////////////
//
//depthShaderProgram

const char* depthVertexShaderSource = R"glsl(
#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) uniform mat4 lightSpaceMatrix;
layout (location = 2) uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)glsl";

const char* depthFragmentShaderSource = R"glsl(
#version 430 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}
)glsl";

