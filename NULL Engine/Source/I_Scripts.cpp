#include <string.h>
#include <map>

#include "Application.h"
#include "M_FileSystem.h"
#include "I_Scripts.h"
#include "JSONParser.h"
#include "FileSystemDefinitions.h"
#include "M_ResourceManager.h"
#include "Log.h"
#include "R_Script.h"
//#include "M_ScriptManager.h"

#include "MemoryManager.h"

bool Importer::Scripts::Import(const char* assetsPath, char* buffer, uint size, R_Script* rScript)
{
	std::map<std::string, uint32> forcedUIDs;
	App->resourceManager->GetForcedUIDsFromMeta(rScript->GetAssetsPath(), forcedUIDs);

	if (!forcedUIDs.empty())
	{
		rScript->ForceUID(forcedUIDs.begin()->second);
		rScript->SetLibraryPathAndFile();
	}
	//Parse the header file to find the scripts in it
	char* cursor = buffer;
	char api[] = "SCRIPTS_API";
	char scriptBaseClass[] = "Script";
	Parser::ParsingState currentState;

	if (!Parser::CheckNullterminatedBuffer(buffer, size))
	{
		LOG("[ERROR] Can't parse buffer from file %s because it isn't nullterminated", assetsPath);
		return false;
	}

	//TODO: Go to class/struct first and then checking if it is exportable
	while (Parser::GoEndSymbol(cursor, api) == Parser::ParsingState::CONTINUE) 
	{
		char* scriptFirstCharacter = nullptr;
		unsigned int nameSize;
		currentState = Parser::ReadNextSymbol(cursor, scriptFirstCharacter, nameSize);
		if (currentState == Parser::ParsingState::ENDFILE)
			break;
		if (currentState == Parser::ParsingState::ERROR)
		{
			LOG("[ERROR] Not imported scripts from file %s because it won't compile", assetsPath);
			return false;
		}

		currentState = Parser::GoNextSymbol(cursor);
		if (currentState == Parser::ParsingState::ENDFILE)
			break;
		if (currentState == Parser::ParsingState::ERROR)
		{
			LOG("[ERROR] Not imported scripts from file %s because it won't compile", assetsPath);
			return false;
		}
		//For now scriptFirstCharacter is unreachable as nullptr but if i change smth it could be null
		std::string scriptName(scriptFirstCharacter, nameSize);
		switch (*cursor) 
		{
		case '{': 
		{
			++cursor;
			//Found engine script
			//Check if a Create Function exists for that script
			if (strstr(buffer, std::string("Create" + scriptName).c_str()) == nullptr)
			{
				LOG("[WARNING] Can't use script %s in file %s because it doesn't have a Create%s() function", scriptName.c_str(), assetsPath, scriptName.c_str());
				break;
			}
			if (strstr(buffer, std::string("Delete" + scriptName).c_str()) == nullptr)
			{
				LOG("[WARNING] Can't use script %s in file %s because it doesn't have a Delete%s() function", scriptName.c_str(), assetsPath, scriptName.c_str());
				break;;
			}
			LOG("[STATUS] New script %s from file %s added successfully", scriptName.c_str(), assetsPath);
			rScript->dataStructures.push_back({ scriptName, false });
			break; 
		}
		case ':':
		{
			++cursor;
			currentState = Parser::GoNextSymbol(cursor);
			if (currentState != Parser::ParsingState::CONTINUE)
			{
				LOG("[ERROR] Not imported scripts from file %s because it won't compile: check %s class/struct declaration/definition after \":\" ", assetsPath, scriptName.c_str());
				return false;
			}
			if (std::string(cursor, 6) != "public")
			{
				if (std::string(cursor, 9) == "protected")
				{
					cursor += 9;
					LOG("[WARNING] Script %s with protected inheritance type in file %s not allowed", scriptName.c_str(), assetsPath);
					break;
				}
				if (std::string(cursor, 7) == "private")
				{
					cursor += 7;
					LOG("[WARNING] Script %s with private inheritance type in file %s not allowed", scriptName.c_str(), assetsPath);
					break;
				}

				LOG("[ERROR] Not Imported scripts in header %s because it will not compile: script %s class/struct declaration malformed", assetsPath, scriptName.c_str());
				return false;
			}
			cursor += 6;
			currentState = Parser::GoNextSymbol(cursor);
			if (currentState != Parser::ParsingState::CONTINUE)
			{
				LOG("[ERROR] Not imported scripts from file %s because it won't compile: check %s class/struct after \"public\" specification ", assetsPath, scriptName.c_str());
				return false;
			}
			unsigned int baseClassLength = strlen(scriptBaseClass);
			if (std::string(cursor, baseClassLength) != scriptBaseClass)
			{
				cursor += baseClassLength;
				LOG("[WARNING] Found class/struct %s in file %s marked to export that doesn't inherit from script class", scriptName.c_str(), assetsPath);
				break;
			}
			cursor += baseClassLength;
			currentState = Parser::GoNextSymbol(cursor);
			if (currentState != Parser::ParsingState::CONTINUE || *cursor != '{' )
			{
				LOG("[ERROR] Not imported scripts from file %s because it won't compile: check %s class/struct after \"%s\" base class specification ", assetsPath, scriptName.c_str(), scriptBaseClass);
				return false;
			}
			//Check if a Create Function exists for that script
			if (strstr(buffer, std::string("Create" + scriptName).c_str()) == nullptr)
			{
				LOG("[WARNING] Can't use script %s in file %s because it doesn't have a Create%s() function", scriptName.c_str(), assetsPath, scriptName.c_str());
				break;
			}
			//Engine script is valid
			LOG("[STATUS] New script %s from file %s added successfully", scriptName.c_str(), assetsPath);
			rScript->dataStructures.push_back({ scriptName, true });
			break;
		}
		case ';': 
		{
			++cursor;
			LOG("[WARNING] Found exporting script %s declaration of %s header file", scriptName.c_str(), assetsPath);
			break;
		}
		default:
			LOG("[ERROR] Not imported scripts from file %s because it won't compile: check after class/struct declaration name %s", assetsPath, scriptName.c_str());
			return false;
		}

	}

	if (rScript->dataStructures.empty()) 
		LOG("[WARNING] No scripts found on file %s", assetsPath);

	//TODO: Need to save the last modification????
	rScript->lastTimeMod = App->fileSystem->GetLastModTime(assetsPath);
	return true;
}

uint Importer::Scripts::Save(const R_Script* rScript, char** buffer)
{
	uint size = 0;
	if (rScript == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save R_Script* in Library! Error: Given R_Script* was nullptr.");
		return size;
	}
	//std::string errorString = "[ERROR] Importer: Could not Save Model { " + std::string(rModel->GetAssetsFile()) + " } from Library";
	ParsonNode rootNode = ParsonNode();
	if (!rScript->dataStructures.empty())
	{
		rootNode.SetBool("HasData", true);
		ParsonArray scriptsArray= rootNode.SetArray("FileScripts");
		for (int i = 0; i < rScript->dataStructures.size(); ++i) 
		{
			ParsonNode scriptData = scriptsArray.SetNode("ScriptData");
			scriptData.SetString("Name", rScript->dataStructures[i].first.c_str());
			scriptData.SetBool("EngineScript", rScript->dataStructures[i].second);
		}
	}
	else 
	{
		rootNode.SetBool("HasData", false);
		LOG("[WARNING] Saved Resource Script of file %s doesn't contain data", rScript->GetAssetsPath());
	}

	std::string path = SCRIPTS_PATH + std::to_string(rScript->GetUID()) + SCRIPTS_EXTENSION;
	size = rootNode.SerializeToFile(path.c_str(), buffer);
	if (size > 0)
	{
		LOG("[STATUS] Importer: Successfully saved Script { %s } in Library! Path: %s", rScript->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("%s! Error: File System could not write to script file");
	}
	return size;
}

bool Importer::Scripts::Load(const char* buffer, R_Script* rScript)
{
	if (rScript == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Script from Library! Error: R_Script was nullptr.");
		return false;
	}
	if (buffer == nullptr)
	{
		LOG("%s! Error: Given buffer to load Script was nullptr.");
		return false;
	}
	rScript->lastTimeMod = App->fileSystem->GetLastModTime(rScript->GetAssetsPath());
	ParsonNode rootNode = ParsonNode(buffer);
	if (rootNode.GetBool("HasData"))
	{
		ParsonArray scriptsArray = rootNode.GetArray("FileScripts");
		for (int i = 0; i < scriptsArray.size; ++i)
		{
			ParsonNode scriptData = scriptsArray.GetNode(i);
			rScript->dataStructures.push_back({ scriptData.GetString("Name") ,scriptData.GetBool("EngineScript") });
		}
	}
	else
	{
		//TODO: is it a problem if the resource doesn't contain data ??
		LOG("[WARNING] Load Resource Script of file %s doesn't contain data", rScript->GetAssetsPath());
	}
	//TODO: what do i do with the reloadCompleated variable from the resource Script
	//rScript->reloadCompleted = rootNode.GetBool("EngineScripts");
	return true;
}

bool Parser::CheckNullterminatedBuffer(char* buffer, int size)
{
	return buffer[size] == '\0';
}

bool Parser::LanguageSymbol(char symbol)
{
	return (symbol == '{' || symbol == '}' || symbol == ':' || symbol == ';' || symbol == '+' || symbol == '-' || symbol == '/' || symbol == '\\' || symbol == '*' || symbol == '['
		|| symbol == ']' || symbol == '(' || symbol == ')' || symbol == ',' || symbol == '.' || symbol == '\"' || symbol == '\'' || symbol == '!' || symbol == '?' || symbol == '='
		|| symbol == '&' || symbol == '%' || symbol == '~' || symbol == '#' || symbol == '<' || symbol == '>' || symbol == '|' || symbol == '^');
}

Parser::ParsingState Parser::HandlePossibleComment(char*& cursor)
{
	if (*cursor == '/')
	{
		++cursor;
		switch (*cursor)
		{
		case '/':
			cursor = strstr(cursor, "\n");
			if (cursor == nullptr)
				return Parser::ParsingState::ENDFILE;
			++cursor;
			return Parser::ParsingState::CONTINUE;

		case '*':
			++cursor;
			cursor = strstr(cursor, "*/");
			if (cursor == nullptr) 
				return Parser::ParsingState::ENDFILE;
			cursor += 2;
			return Parser::ParsingState::CONTINUE;
		case '\0':
			return Parser::ParsingState::ERROR;
		default://TODO: /(OPERATOR MAY BE VALID)  example a /-3; But if we are parsing a header file we normally don't perform division operations
			return Parser::ParsingState::ERROR;
		}
	}
	return Parser::ParsingState::CONTINUE;
}

Parser::ParsingState Parser::GoStartSymbol(char*& cursor, char* symbol)
{
	cursor = strstr(cursor, symbol);
	if (cursor == nullptr)
		return Parser::ParsingState::ENDFILE;
	return Parser::ParsingState::CONTINUE;
}

Parser::ParsingState Parser::GoEndSymbol(char*& cursor, char* symbol)
{
	cursor = strstr(cursor, symbol);
	if (cursor == nullptr)
		return Parser::ParsingState::ENDFILE;
	cursor += strlen(symbol);
	return Parser::ParsingState::CONTINUE;
}

Parser::ParsingState Parser::ReadNextSymbol(char*& cursor, char*& startSymbol, unsigned int& symbolSize)
{
	symbolSize = 0;

	while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n')
	{
		if (cursor == '\0')
			return Parser::ParsingState::ENDFILE;
		
		Parser::ParsingState stateAfterComment = HandlePossibleComment(cursor);
		if (stateAfterComment == Parser::ParsingState::ENDFILE)
			return Parser::ParsingState::ENDFILE;
		if (stateAfterComment == Parser::ParsingState::ERROR)
			return Parser::ParsingState::ERROR;

		if (LanguageSymbol(*cursor))
		{
			symbolSize = 1;
			return Parser::ParsingState::CONTINUE;
		}

		++cursor;
	}

	Parser::ParsingState stateAfterComment = HandlePossibleComment(cursor);
	if (stateAfterComment == Parser::ParsingState::ENDFILE)
		return Parser::ParsingState::ENDFILE;
	if (stateAfterComment == Parser::ParsingState::ERROR)
		return Parser::ParsingState::ERROR;

	startSymbol = cursor;
	while (*cursor != ' ' && *cursor != '\t' && *cursor != '\n')
	{
		if (cursor == '\0')
			return Parser::ParsingState::ENDFILE;
		if (*cursor == '#')
			return Parser::ParsingState::ERROR;
		if (LanguageSymbol(*cursor))
			return Parser::ParsingState::CONTINUE;

		++symbolSize;
		++cursor;
	}
	return Parser::ParsingState::CONTINUE;
}

Parser::ParsingState Parser::GoNextSymbol(char*& cursor)
{
	Parser::ParsingState stateAfterComment = HandlePossibleComment(cursor);
	if (stateAfterComment == Parser::ParsingState::ENDFILE)
		return Parser::ParsingState::ENDFILE;
	if (stateAfterComment == Parser::ParsingState::ERROR)
		return Parser::ParsingState::ERROR;

	while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n')
	{
		if (cursor == '\0')
			return Parser::ParsingState::ENDFILE;

		Parser::ParsingState stateAfterComment = HandlePossibleComment(cursor);
		if (stateAfterComment == Parser::ParsingState::ENDFILE)
			return Parser::ParsingState::ENDFILE;
		if (stateAfterComment == Parser::ParsingState::ERROR)
			return Parser::ParsingState::ERROR;

		if (LanguageSymbol(*cursor))
			return Parser::ParsingState::CONTINUE;

		++cursor;
	}
	return Parser::ParsingState::CONTINUE;
}
