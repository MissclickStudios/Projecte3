
#ifdef __Vertex_Shader__

layout(location = 0) in vec2 pos;

out vec2 TexCoords;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

void main()
{
    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4(pos, 0.0, 1.0); 
    TexCoords = vec2((pos.x + 1.0) / 2,
                    (pos.y + 1.0) /2);
}  

#endif

//--------------------

#ifdef __Fragment_Shader__

out vec4 FragColor;
  
in vec2 TexCoords;

uniform vec4 color;
uniform sampler2D UITexture;

void main()
{ 
    FragColor = texture(UITexture, TexCoords) * color;
}

#endif


