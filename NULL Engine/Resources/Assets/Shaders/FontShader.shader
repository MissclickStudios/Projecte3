
#ifdef __Vertex_Shader__

layout (location = 0) in vec4 vertex;
out vec2 texCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    texCoords = vertex.zw;
}  

#endif

//--------------------

#ifdef __Fragment_Shader__

in vec2 texCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texCoords).r);
    color = textColor * sampled;
}  

#endif











