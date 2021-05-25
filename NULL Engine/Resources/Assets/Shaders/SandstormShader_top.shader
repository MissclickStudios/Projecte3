
#ifdef __Vertex_Shader__

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;


out vec2 TexCoord;
out vec3 modelNormal;
out vec3 fragPos; 
out vec4 objectColor;

uniform vec4 inColor;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform float deltaTime;

void main()
{    
    vec4 totalPosition = vec4(0.0f);
    vec4 localPosition = vec4(0.0f);


    totalPosition = vec4(position, 1.0f);
     
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
uniform bool hasTexture;
uniform sampler2D ourTexture;

uniform float Time;

in vec4 objectColor;
in vec2 TexCoord;
in vec3 modelNormal;
in vec3 fragPos;
out vec4 color;

uniform vec2 wind_direction = vec2(0.6, 0.7);

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
    
   	vec2 displaced_TexCoord = vec2(TexCoord.x + (Time/512)*0.02* wind_direction.x, TexCoord.y + (Time/512)*0.02* wind_direction.y);
   	vec4 texColor = vec4(0.93, 0.54, 0.38, texture(ourTexture, displaced_TexCoord).r*0.3);
		
  
  color = outputColor * texColor;


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































