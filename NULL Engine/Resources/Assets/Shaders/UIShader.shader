
#ifdef __Vertex_Shader__

layout (location = 0) in vec2 UIpos;

out vec2 TexCoords;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(UIpos, 0.0, 1.0); 
    TexCoords = vec2((UIpos.x + 1.0) / 2,
                    (UIpos.y + 1.0) /2);
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




