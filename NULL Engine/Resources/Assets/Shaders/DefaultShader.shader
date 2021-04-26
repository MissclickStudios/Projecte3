
#ifdef __Vertex_Shader__

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 boneIds; 
layout(location = 4) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

out vec2 TexCoord;
out vec3 modelNormal;
out vec3 fragPos; 
out vec4 objectColor;

uniform vec4 inColor;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform bool activeAnimation;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{    
    vec4 totalPosition = vec4(0.0f);
    vec4 localPosition = vec4(0.0f);

    if(activeAnimation == true)
    { 
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
         if(boneIds[i] == 4294967295 || weights[i] == 0) 
              continue;

         if(boneIds[i] >= MAX_BONES) 
            {
                totalPosition = vec4(position,1.0f);
                break;
            }

        localPosition = finalBonesMatrices[int(boneIds[i])] * vec4(position,1.0f);
        totalPosition += localPosition * weights[i];
        }   

    }

     if(activeAnimation == false)
    {
       totalPosition = vec4(position, 1.0f);
    } 
     
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * totalPosition;

    TexCoord = texCoord;
       
    objectColor = inColor;
   
    modelNormal = mat3(transpose(inverse(modelMatrix))) * normal;
    
    fragPos = vec3(modelMatrix * vec4(position, 1.0));
}

#endif

//--------------------

#ifdef __Fragment_Shader__

struct DirLight
{
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;

    vec3 direction;
};

struct PointLight 
{    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
#define MAX_LIGHTPOINTS 5

float specularStrength = 0.5;

uniform int numPointLights;
uniform DirLight dirLight;
uniform PointLight pointLight[MAX_LIGHTPOINTS];
uniform vec3 viewPos; 
uniform vec4 alternateColor;

uniform bool useDirLight;
uniform bool takeDamage;
uniform bool hasTexture;
uniform sampler2D ourTexture;

in vec4 objectColor;
in vec2 TexCoord;
in vec3 modelNormal;
in vec3 fragPos;
out vec4 color;

vec4 CalculateDirectional(DirLight light, vec3 normal, vec3 viewDir, float specularStrength, vec4 objectColor);
vec4 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float specularStrength, vec4 objectColor);

void main()
{

    vec3 norm = normalize(modelNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec4 outputColor = objectColor;
    if(useDirLight)
    {
       outputColor =  CalculateDirectional(dirLight, norm, viewDir, specularStrength, objectColor);
    }
   
   if(numPointLights > 0) 
   {
       for(int i = 0; i < numPointLights; i++)
       {
            outputColor += CalculatePointLight(pointLight[i], norm, fragPos, viewDir, specularStrength, objectColor);
       }
   }
    
     vec4 texColor = (hasTexture) ? texture(ourTexture, TexCoord) : vec4(1,1,1,1);

    if(takeDamage)
    {
        color = alternateColor * texColor;
    }
    else 
    {
        color = outputColor * texColor;
    }


}

vec4 CalculateDirectional(DirLight light, vec3 normal, vec3 viewDir, float specularStrength, vec4 objectColor)
{
    
    vec3 lightDir = normalize(-light.direction); 
    
    vec3 reflectDir = reflect(-lightDir, normal);
   
   //Specular
   
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    vec4 resultSpecular = specularStrength  * light.specular; 
    
    //Diffuse  
   
   float diff = max(dot(normal, lightDir), 0.0);
   
   vec4 resultDiffuse = diff * light.diffuse;
   

    //Cel Shading 

    float intensity = 0.8 * diff + 0.2 * spec;

 	if (intensity > 0.6) {
 		intensity = 0.7;
 	}
 	else if (intensity > 0.4) {
 		intensity = 0.5;
 	}
 	else {
 		intensity = 0.5;
    }

    //Resulting Color and Texture

    vec4 resultColor = (light.ambient + light.diffuse + resultSpecular) * intensity * objectColor;

   
   return (resultColor);
}

vec4 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float specularStrength, vec4 objectColor)
{
     vec3 lightDir = normalize(light.position - fragPos);

    // diffuse 
    float diff = max(dot(normal, lightDir), 0.0);

    vec4 resultDiffuse = diff * light.diffuse;
    // specular 
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

     vec4 resultSpecular = specularStrength * spec * light.specular; 
    // attenuation
    float distance    = length(light.position - fragPos);

    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    


     //Cel Shading 

    float intensity = 0.7 * diff + 0.6 * spec;

 	if (intensity > 0.9) {
 		intensity = 1.1;
 	}
 	else if (intensity > 0.5) {
 		intensity = 0.7;
 	}
 	else {
 		intensity = 0.5;
    }

    //Resulting Color and Texture

    vec4 resultColor = (light.ambient + resultDiffuse + resultSpecular) * intensity * objectColor;

   
   return (resultColor * attenuation);
}

#endif




















