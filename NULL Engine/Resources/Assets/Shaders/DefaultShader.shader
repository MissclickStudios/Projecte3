
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
    modelNormal = normal;
}

#endif

//--------------------

#ifdef __Fragment_Shader__
uniform vec4 lightColor;

uniform vec3 lightPos; 

in vec4 objectColor;
in vec2 TexCoord;
in vec3 modelNormal;
in vec3 fragPos;
out vec4 color;

uniform bool hasTexture;
uniform sampler2D ourTexture;
void main()
{
   vec3 norm = normalize(modelNormal);
   
   vec3 lightDir = normalize(lightPos - fragPos);  
   
   float diff = max(dot(norm, lightDir), 0.0);
   
   vec4 diffuse = diff * lightColor;
   
   vec4 ambient = 0.1 * lightColor;
   
   vec4 resultColor = (ambient + diffuse) * objectColor;
   
   
   vec4 texColor = (hasTexture) ? texture(ourTexture, TexCoord) : vec4(1,1,1,1);
   
   color = texColor  * resultColor;

}

#endif












