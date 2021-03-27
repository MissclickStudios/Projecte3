#ifndef __I_FOLDERS_H__
#define __I_FOLDERS_H__

class R_Folder;

typedef unsigned int uint;

namespace Importer
{
	namespace Folders
	{
		uint Save(const R_Folder* rFolder, char** buffer);
		bool Load(const char* buffer, R_Folder* rFolder);
	}
}

#endif // !__I_FOLDERS_H__