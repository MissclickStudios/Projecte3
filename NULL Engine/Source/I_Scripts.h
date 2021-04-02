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
	void ReadPreaviousSymbol(char* cursor, char*& startSymbol, unsigned int& symbolSize); //Careful reading outside buffer boundaries or reading from a comment and interpreting it as a symbol!!!
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
	namespace Scripts																	
	{
		bool Import(const char* assetsPath, char* buffer, uint size, R_Script* rScript);
		uint Save(const R_Script* rScript, char** buffer);						
		bool Load(const char* buffer, R_Script* rMaterial);						
	}
}

#endif // !__I_SCRIPT_H__