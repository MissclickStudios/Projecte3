
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

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    
    TexCoord = texCoord;
    
    objectColor = inColor;
    
    fragPos = vec3(modelMatrix * vec4(position, 1.0));
    
    modelNormal = mat3(transpose(inverse(modelMatrix))) * normal;  
}

#endif

//--------------------

#ifdef __Fragment_Shader__

struct Light
{
vec4 diffuse;
vec4 ambient;
vec4 specular;

vec3 direction;
};

uniform Light light;
uniform vec3 lightPos;
uniform vec3 viewPos; 

in vec4 objectColor;
in vec2 TexCoord;
in vec3 modelNormal;
in vec3 fragPos;
out vec4 color;

uniform bool hasTexture;
uniform sampler2D ourTexture;
void main()
{
	//Direction and normals calculations
	
   float specularStrength = 0.5; 
   
   vec3 norm = normalize(modelNormal);
   
   //vec3 lightDir = normalize(lightPos - fragPos); 
   
    vec3 lightDir = normalize(-light.direction);

   vec3 viewDir = normalize(viewPos - fragPos);
   
   vec3 reflectDir = reflect(-lightDir, norm);
   
   //Specular
   
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    vec4 resultSpecular = specularStrength * spec * light.diffuse; 
    
    //Diffuse  
   
   float diff = max(dot(norm, lightDir), 0.0);
   
   vec4 resultDiffuse = diff * light.diffuse;
   
   //Resulting Color and Texture
   
   vec4 resultColor = (light.ambient + resultDiffuse + resultSpecular) * objectColor;
   
   vec4 texColor = (hasTexture) ? texture(ourTexture, TexCoord) : vec4(1,1,1,1);
   
   color = texColor  * resultColor;
  

}

#endif










