#ifdef __Vertex_Shader__

layout (location = 0) in vec3 position; 
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 boneIds; 
layout(location = 4) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 modelMatrix; 
uniform mat4 viewMatrix; 
uniform mat4 projectionMatrix;

uniform bool activeAnimation;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform float outlineThickness;

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
                totalPosition = vec4(position + normal * outlineThickness, 1.0f);
                break;
            }

        localPosition = finalBonesMatrices[int(boneIds[i])] * vec4(position + normal * outlineThickness, 1.0f);
        totalPosition += localPosition * weights[i];
        }   

    }

     if(activeAnimation == false)
    {
       totalPosition = vec4(position + normal * outlineThickness, 1.0f);

    } 
   
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * totalPosition; 
   
}
#endif

//--------------------

#ifdef __Fragment_Shader__

uniform vec4 outlineColor; 
out vec4 color; 

void main()
{ 
    color = outlineColor; 
} 

#endif