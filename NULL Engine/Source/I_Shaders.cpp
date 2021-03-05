#include "Application.h"
#include "I_Shaders.h"
#include "R_Shader.h"
#include "M_FileSystem.h"
#include "OpenGL.h"
#include "JSONParser.h"

#include "MemoryManager.h"


bool Importer::Shaders::Import(const char* fullPath, R_Shader* shader)
{
	bool ret = true;
	std::string path;
	std::string name;
	std::string ext;
	App->fileSystem->SplitFilePath(fullPath, &path, &name, &ext);

	char* buffer;
	int size = App->fileSystem->Load(fullPath, &buffer);

	if (size <= 0)
	{
		delete[] buffer;
		LOG("Shader File: %s not found or can't be loaded.", fullPath);
		return false;
	}
	
	std::string file(buffer);
	if (file.find(VERTEX_SHADER) != std::string::npos)
	{
		shader->vertexID = ImportVertex(file, shader);
	}
	if (file.find(FRAGMENT_SHADER) != std::string::npos)
	{
		shader->fragmentID = ImportFragment(file, shader);
	}

	delete[] buffer;

	// Create the Shader Program and link it
	if (shader->vertexID != 0 && shader->fragmentID != 0)
	{
		GLint outcome;
		shader->vertexID = (GLuint)shader->vertexID;
		shader->fragmentID = (GLuint)shader->fragmentID;

		shader->shaderProgramID = glCreateProgram();
		glAttachShader(shader->shaderProgramID, shader->vertexID);
		glAttachShader(shader->shaderProgramID, shader->fragmentID);
		glLinkProgram(shader->shaderProgramID);

		glGetProgramiv(shader->shaderProgramID, GL_LINK_STATUS, &outcome);
		if (outcome == 0)
		{
			GLchar info[512];
			glGetProgramInfoLog(shader->shaderProgramID, 512, NULL, info);
			LOG("Shader compiling error: %s", info);
			ret = false;
		}
		else if(shader->uniforms.size() == 0)
		{
			GetShaderUniforms(shader);
		}

		glDeleteShader(shader->vertexID);
		glDeleteShader(shader->fragmentID);
		glDeleteShader(shader->shaderProgramID);
	}
	else
	{
		LOG("ERROR, Vertex shader: %d or Fragment shader: %d are not correctly compiled.", shader->vertexID, shader->fragmentID);
	}
	return ret;

}

int Importer::Shaders::ImportVertex(std::string shaderFile, R_Shader* shader)
{
	std::string vertexFile = std::string ("#version 330 core\r\n") + 
		std::string("#define ") +
		VERTEX_SHADER + "\r\n";
	vertexFile += shaderFile;

	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* source = (const GLchar*)vertexFile.c_str();
	glShaderSource(vertexShader, 1, &source, nullptr);
	glCompileShader(vertexShader);
	
	GLint outcome;
	GLchar info[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &outcome);
	if (outcome == 0)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, info);
		LOG("Vertex shader compilation error (%s)", info);
	}

	

	return (outcome != 0) ? vertexShader : -1;
}

int Importer::Shaders::ImportFragment(std::string shaderFile, R_Shader* shader)
{
	std::string fragmentFile = std::string("#version 330 core\r\n") +
		std::string("#define ") + 
		FRAGMENT_SHADER + "\r\n";
	fragmentFile += shaderFile;
	
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* source = (const GLchar*)fragmentFile.c_str();
	glShaderSource(fragmentShader, 1, &source, nullptr);
	glCompileShader(fragmentShader);

	GLint outcome;
	GLchar info[512];

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &outcome);
	if (outcome == 0)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, info);
		LOG("Vertex shader compilation error (%s)", info);
	}

	return (outcome != 0) ? fragmentShader : -1;
}

void Importer::Shaders::GetShaderUniforms(R_Shader* shader)
{
	GLint activeUniforms;

	glGetProgramiv(shader->shaderProgramID, GL_ACTIVE_UNIFORMS, &activeUniforms);

	for (uint i = 0; i < activeUniforms; i++)
	{
		Uniform uniform;
		GLint length;
		GLint size;
		GLchar name[32];
		glGetActiveUniform(shader->shaderProgramID, i, sizeof(name), &length, &size, &uniform.GLtype, name);
		uniform.name = name;
		if (uniform.name != "inColor" && uniform.name != "time" && uniform.name != "modelMatrix" && uniform.name != "viewMatrix" && uniform.name != "projectionMatrix" && uniform.name != "cameraPosition")
		{
			uint uinformLoc = glGetUniformLocation(shader->shaderProgramID, uniform.name.c_str());

			switch (uniform.GLtype)
			{
			case GL_INT:
				uniform.uniformType = UniformType::INT;
				glGetUniformiv(shader->shaderProgramID, uinformLoc, &uniform.integer);
				break;
			case GL_FLOAT:
				uniform.uniformType = UniformType::FLOAT;
				glGetUniformfv(shader->shaderProgramID, uinformLoc, &uniform.floatNumber);
				break;
			case GL_BOOL:
				uniform.uniformType = UniformType::BOOL;
				break;
			case GL_INT_VEC2:
				uniform.uniformType = UniformType::INT_VEC2;
				glGetUniformiv(shader->shaderProgramID, uinformLoc, (GLint*)&uniform.vec2);
				break;
			case GL_INT_VEC3:
				uniform.uniformType = UniformType::INT_VEC3;
				glGetUniformiv(shader->shaderProgramID, uinformLoc, (GLint*)&uniform.vec3);
				break;
			case GL_INT_VEC4:
				uniform.uniformType = UniformType::INT_VEC4;
				glGetUniformiv(shader->shaderProgramID, uinformLoc, (GLint*)&uniform.vec4);
				break;
			case GL_FLOAT_VEC2:
				uniform.uniformType = UniformType::FLOAT_VEC2;
				glGetUniformfv(shader->shaderProgramID, uinformLoc, (GLfloat*)&uniform.vec2);
				break;
			case GL_FLOAT_VEC3:
				uniform.uniformType = UniformType::FLOAT_VEC3;
				glGetUniformfv(shader->shaderProgramID, uinformLoc, (GLfloat*)&uniform.vec3);
				break;
			case GL_FLOAT_VEC4:
				uniform.uniformType = UniformType::FLOAT_VEC4;
				glGetUniformfv(shader->shaderProgramID, uinformLoc, (GLfloat*)&uniform.vec4);
				break;
			case GL_FLOAT_MAT4:
				uniform.uniformType = UniformType::MATRIX4;
				glGetnUniformfv(shader->shaderProgramID, uinformLoc, sizeof(uniform.matrix4), &uniform.matrix4.v[0][0]);
				break;
			default: 
				uniform.uniformType = UniformType::NONE; 
				break;
			}

			if(uniform.uniformType != UniformType::NONE) 
				shader->uniforms.push_back(uniform);
		}
	}
}

void Importer::Shaders::SetShaderUniforms(R_Shader* shader)
{
	if (shader->uniforms.empty())
	{
		return;
	}

	for (uint i = 0; i < shader->uniforms.size(); i++)
	{
		switch (shader->uniforms[i].uniformType)
		{
		//case UniformType::BOOL: shader->SetUniform1i(shader->uniforms[i].name.c_str(), shader->uniforms[i].boolean); break;
			case  UniformType::INT:			shader->SetUniform1i(shader->uniforms[i].name.c_str(),shader->uniforms[i].integer);										break;
			case  UniformType::FLOAT:		shader->SetUniform1f(shader->uniforms[i].name.c_str(), shader->uniforms[i].floatNumber);								break;
			case  UniformType::INT_VEC2:	shader->SetUniformVec2i(shader->uniforms[i].name.c_str(), (GLint*)shader->uniforms[i].vec2.ptr());						break;
			case  UniformType::INT_VEC3:	shader->SetUniformVec3i(shader->uniforms[i].name.c_str(), (GLint*)shader->uniforms[i].vec3.ptr());						break;
			case  UniformType::INT_VEC4:	shader->SetUniformVec4i(shader->uniforms[i].name.c_str(), (GLint*)shader->uniforms[i].vec4.ptr());						break;
			case  UniformType::FLOAT_VEC2:	shader->SetUniformVec2f(shader->uniforms[i].name.c_str(), (GLfloat*)shader->uniforms[i].vec2.ptr());					break;
			case  UniformType::FLOAT_VEC3:	shader->SetUniformVec3f(shader->uniforms[i].name.c_str(), (GLfloat*)shader->uniforms[i].vec3.ptr());					break;
			case  UniformType::FLOAT_VEC4:	shader->SetUniformVec4f(shader->uniforms[i].name.c_str(), (GLfloat*)shader->uniforms[i].vec4.ptr());					break;
			case UniformType::MATRIX4:		shader->SetUniformMatrix4(shader->uniforms[i].name.c_str(), (GLfloat*)shader->uniforms[i].matrix4.Transposed().ptr());	break;
			default: break;
		}
	}
}

uint Importer::Shaders::Save(const R_Shader* shader, char** buffer)
{
	uint written = 0;

	if (shader == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save R_Shader* in Library! Error: Given R_Shader* was nullptr.");
		return 0;
	}


	ParsonNode parsonFile = ParsonNode();
	ParsonArray parsonArrray = parsonFile.SetArray("Uniforms");

	for (uint i = 0; i < shader->uniforms.size(); i++)
	{
		ParsonNode& node = parsonArrray.SetNode(shader->uniforms[i].name.c_str());
		node.SetString("Name", shader->uniforms[i].name.c_str());
		node.SetInteger("Type", (int)shader->uniforms[i].uniformType);
		switch (shader->uniforms[i].uniformType)
		{
			case  UniformType::INT:			node.SetNumber("Value", shader->uniforms[i].integer);		break;
			case  UniformType::FLOAT:		node.SetNumber("Value", shader->uniforms[i].floatNumber);	break;
			case  UniformType::INT_VEC2:	node.SetFloat2("Value", shader->uniforms[i].vec2);			break;
			case  UniformType::INT_VEC3:	node.SetFloat3("Value", shader->uniforms[i].vec3);			break;
			case  UniformType::INT_VEC4:	node.SetFloat4("Value", shader->uniforms[i].vec4);			break;
			case  UniformType::FLOAT_VEC2:	node.SetFloat2("Value", shader->uniforms[i].vec2);			break;
			case  UniformType::FLOAT_VEC3:	node.SetFloat3("Value", shader->uniforms[i].vec3);			break;
			case  UniformType::FLOAT_VEC4:	node.SetFloat4("Value", shader->uniforms[i].vec4);			break;
		}
	}

	
	
	std::string path = SHADERS_PATH + std::to_string(shader->GetUID()) + SHADERS_EXTENSION;
	written = parsonFile.SerializeToFile(path.c_str(), buffer);

	return written;
}

bool Importer::Shaders::Load(const char* buffer, R_Shader* shader, uint size)
{
	bool ret = true;
	ParsonNode parsonFile(buffer);
	ParsonArray parsonArrray = parsonFile.GetArray("Uniforms");
	Uniform uniform;
	for (uint i = 0; i < parsonArrray.GetSize(); i++)
	{
		ParsonNode node = parsonArrray.GetNode(i);
		uniform.name = node.GetString("Name");
		uniform.uniformType = (UniformType)node.GetInteger("Type");
		switch (uniform.uniformType)
		{
			case  UniformType::INT:			uniform.integer = node.GetNumber("Value");		break;
			case  UniformType::FLOAT:		uniform.floatNumber = node.GetNumber("Value");	break;
			case  UniformType::INT_VEC2:	uniform.vec2 = node.GetFloat2("Value");			break;
			case  UniformType::INT_VEC3:	uniform.vec3 = node.GetFloat3("Value");			break;
			case  UniformType::INT_VEC4:	uniform.vec4 = node.GetFloat4("Value");			break;
			case  UniformType::FLOAT_VEC2:	uniform.vec2 = node.GetFloat2("Value");			break;
			case  UniformType::FLOAT_VEC3:	uniform.vec3 = node.GetFloat3("Value");			break;
			case  UniformType::FLOAT_VEC4:	uniform.vec4 = node.GetFloat4("Value");			break;
		}
		
		shader->uniforms.push_back(uniform);
	}

	Recompile(shader);

	return ret;
}

void Importer::Shaders::Recompile(R_Shader* shader)
{
	glDetachShader(shader->shaderProgramID, shader->vertexID);
	glDetachShader(shader->shaderProgramID, shader->fragmentID);
	glDeleteProgram(shader->shaderProgramID);

	Importer::Shaders::Import(shader->GetAssetsPath(), shader);
}