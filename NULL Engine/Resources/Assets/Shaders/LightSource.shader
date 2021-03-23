#ifdef __Vertex_Shader__

layout(location = 0) in vec3 position;

out vec4 ourColor;

uniform vec4 inColor;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    ourColor = inColor;
}

#endif

//--------------------

#ifdef __Fragment_Shader__

in vec4 ourColor;
uniform vec4 lightSource;

void main()
{

   color = lightSource  * ourColor;

}

#endif