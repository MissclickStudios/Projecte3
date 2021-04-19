#ifdef __Vertex_Shader__

layout (location = 0) in vec3 position; 
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 modelMatrix; 
uniform mat4 viewMatrix; 
uniform mat4 projectionMatrix; 

uniform float outlineThickness;

void main()
{ 
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position + normal * outlineThickness, 1.0f); 
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