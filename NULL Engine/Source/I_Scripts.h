#ifndef __I_SCRIPT_H__
#define __I_SCRIPT_H__

class R_Script;

namespace Parser
{
	enum class ParsingState : unsigned int
	{
		ERROR,
		CONTINUE,
		ENDFILE
	};

	bool CheckNullterminatedBuffer(char* buffer, int size);
	bool LanguageSymbol(char symbol);
	bool PreaviousSymbolIs(char*& symbol); //Careful reading outside buffer boundaries
	ParsingState HandlePossibleComment(char*& cursor);
	ParsingState GoStartSymbol(char*& cursor, char* symbol);
	ParsingState GoEndSymbol(char*& cursor, char* symbol);
	//Read the symbol and leave the cursor pointing to the next character after the readed symbol
	ParsingState ReadNextSymbol(char*& cursor, char*& startSymbol, unsigned int& symbolSize);
	//Leave the cursor on the start of the next symbol
	ParsingState GoNextSymbol(char*& cursor);
}

namespace Importer
{
	namespace Scripts																	// Importing, saving and loading aiMaterials with Assimp.
	{
		bool Import(const char* assetsPath, char* buffer, uint size, R_Script* rScript);			//Processes an aiMaterial into a ready-to-use R_Material. ERROR = nullptr.
		uint Save(const R_Script* rScript, char** buffer);						// Processes R_Material data into a ready-to-save buffer. Returns buffer size (0 = ERROR).
		bool Load(const char* buffer, R_Script* rMaterial);						// Processes buffer data into a ready-to-use R_Material. Returns nullptr on error.
	}
}

#endif // !__I_SCRIPT_H__