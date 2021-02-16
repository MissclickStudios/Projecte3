#ifndef __I_FOLDERS_H__
#define __I_FOLDERS_H__

class R_Folder;

typedef unsigned int uint;

namespace Importer
{
	namespace Folders
	{
		uint Save(const R_Folder* r_folder, char** buffer);
		bool Load(const char* buffer, R_Folder* r_folder);
	}
}

#endif // !__I_FOLDERS_H__