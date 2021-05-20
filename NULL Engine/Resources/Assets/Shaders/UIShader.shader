
#ifdef __Vertex_Shader__

layout (location = 0) in vec2 UIpos;
layout (location = 1) in vec2 UICoords;


out vec2 TexCoords;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(UIpos, 0.0, 1.0); 
    TexCoords = UICoords;
}  

#endif

//--------------------

#ifdef __Fragment_Shader__

out vec4 FragColor;
  
in vec2 TexCoords;

uniform bool useColor;
uniform vec4 inColor;
uniform sampler2D UITexture;

void main()
{ 
   vec4 texColor = texture(UITexture, TexCoords);
   
    if(useColor)
    {
        FragColor = inColor * texColor;
    }
    else {
        FragColor = texColor;
    }
}

#endif




