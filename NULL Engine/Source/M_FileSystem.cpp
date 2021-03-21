#include <fstream>
#include <filesystem>

#include "PhysFS/include/physfs.h"

#include "Application.h"
#include "Log.h"
#include "FileSystemDefinitions.h"
#include "PathNode.h"

#include "M_FileSystem.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/PhysFS/libx86/physfs.lib")

M_FileSystem::M_FileSystem(bool is_active) : Module("FileSystem", is_active)
{
	// Needs to be created before Init() so other modules are able to use it.
	char* basePath = SDL_GetBasePath();													// SDL_GetBasePath() returns the path where the application resides (Dir where it was run from).
	PHYSFS_init(nullptr);																	// Method that initializes the PhysFS library. Must be called before any other PhysFS method.
	SDL_free(basePath);																	// Frees the base path.

	// Setting the working directory as the writing directory.
	PhysfsResult result = (PhysfsResult)PHYSFS_setWriteDir(".");							// Method that tells where PhysFS can write files. Sets a new write dir with the given one.
	if (result == PhysfsResult::FAILURE)
	{
		LOG("[ERROR] File System:  Could not create Write Directory! Error: %s\n", PHYSFS_getLastError());
	}

	AddPath(".");																			// Adding the ProjectFolder path (working directory path). 
	AddPath("Assets");																		// Adding the Assets Folder path.
	CreateLibraryDirectories();																// Creates all the library directories.
}

M_FileSystem::~M_FileSystem()
{
	PHYSFS_deinit();																		// Deinitializes the PhysFS library. Cleans everything related to the library.
}

bool M_FileSystem::Init(ParsonNode& config)
{
	LOG("[STATUS] File System: Initializing File System");
	
	bool ret = true;

	char* writePath = SDL_GetPrefPath(App->GetOrganizationName(), App->GetEngineName());	// SDL_GetPrefPath() returns the user-and-app-specific path where files can be written.

	// Turn this on while in game mode
	if (App->play)
	{
		PhysfsResult result = (PhysfsResult)PHYSFS_setWriteDir(writePath);
		if (result == PhysfsResult::FAILURE)
		{
			LOG("[ERROR] File System: Could not Create the Write Directory! Error: %s\n", PHYSFS_getLastError());
		}
	}

	SDL_free(writePath);
	
	return ret;
}

bool M_FileSystem::CleanUp()
{
	bool ret = true;

	return ret;
}

bool M_FileSystem::LoadConfiguration(ParsonNode& root)
{
	bool ret = true;

	return true;
}

bool M_FileSystem::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	return true;
}

// ---------------- UTILITY METHODS ----------------
bool M_FileSystem::AddPath(const char* pathOrZip)
{
	bool ret = false;

	PhysfsResult result = (PhysfsResult)PHYSFS_mount(pathOrZip, nullptr, 1);						// Method that adds an archive or directory to the search path. Returns 0 on FAILURE.

	if (result == PhysfsResult::FAILURE)
	{
		LOG("[ERROR] File System: Could not add a path or zip! Error: %s\n", PHYSFS_getLastError());	// Method that returns the string of the last error message issued by PhysFS.
	}
	else
	{
		ret = true;
	}

	return ret;
}

bool M_FileSystem::Exists(const char* file) const
{
	// Checks whether or not a file is in the search path. Returns 0 upon FAILURE.

	return PHYSFS_exists(file);
}

void M_FileSystem::CreateLibraryDirectories()
{
	CreateDir(ASSETS_PATH);
	CreateDir(ASSETS_MODELS_PATH);
	CreateDir(ASSETS_SCENES_PATH);
	CreateDir(ASSETS_TEXTURES_PATH);
	CreateDir(ASSETS_SCRIPTS_PATH);

	CreateDir(LIBRARY_PATH);
	CreateDir(ANIMATIONS_PATH);
	CreateDir(BONES_PATH);
	CreateDir(FOLDERS_PATH);
	CreateDir(MATERIALS_PATH);
	CreateDir(MESHES_PATH);
	CreateDir(MODELS_PATH);
	CreateDir(PARTICLES_PATH);
	CreateDir(SCENES_PATH);
	CreateDir(SHADERS_PATH);
	CreateDir(TEXTURES_PATH);
	CreateDir(SCRIPTS_PATH);
}

bool M_FileSystem::CreateDir(const char* directory)
{
	if (!IsDirectory(directory))
	{
		PHYSFS_mkdir(directory);															// Method that creates a directory with the given directory path.
		return true;
	}

	return false;
}

bool M_FileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file);														// Method to check whether or not the given file is already a registered directory.
}

const char* M_FileSystem::GetBaseDirectory() const
{
	return PHYSFS_getBaseDir();																// Method that returns the directory from where the application was run from.
}

const char* M_FileSystem::GetWriteDirectory() const
{
	return PHYSFS_getWriteDir();															// Method that returns a path where PhysFS will allow file writing. Returns NULL by default.
}

const char* M_FileSystem::GetReadDirectories() const
{
	static char paths[512];

	paths[0] = '\0';

	char** path;

	for (path = PHYSFS_getSearchPath(); *path != nullptr; ++path)							// Method that returns the current search path (array of strings). Returns an empty list by df.
	{
		strcat_s(paths, 512, *path);
		strcat_s(paths, 512, "\n");
	}

	return paths;
}

void M_FileSystem::GetRealDirectory(const char* path, std::string& output) const
{
	output = PHYSFS_getBaseDir();															// Method that returns the directory from where the application was run from.

	std::string baseDirectory	= PHYSFS_getBaseDir();
	std::string searchPath		= *PHYSFS_getSearchPath();									// Method that returns the current search path (array of strings). Returns an empty list by df.
	std::string realDirectory	= PHYSFS_getRealDir(path);									// Method that searches where in the search path a file resides and returns where it was found.

	output.append(searchPath).append("/").append(realDirectory).append("/").append(path);	// Extends the output string with the search path, the real directory and the path strings.
}

std::string M_FileSystem::GetPathRelativeToAssets(const char* originalPath) const
{
	std::string relativePath;

	GetRealDirectory(originalPath, relativePath);											// Gets the complete path as the output.

	return relativePath;
}

void M_FileSystem::DiscoverFiles(const char* directory, std::vector<std::string>& files, std::vector<std::string>& directories, const char* filter) const
{
	if (directory == nullptr)
	{
		LOG("[ERROR] File System: Could not Discover Files inside the given directory! Error: Given Directory string was nullptr.");
		return;
	}
	if (!Exists(directory))
	{
		LOG("[ERROR] File System: Could not Discover Files inside the { %s } directory! Error: Given Directory did not exist.", directory);
		return;
	}
	
	char** fileListing		= PHYSFS_enumerateFiles(directory);								// Method that returns a listing with all the files in a given search path's directory.
	bool useFilter			= (filter != nullptr) ? true : false;

	for (char** file = fileListing; *file != nullptr; ++file)								// Will iterate the file listing file per file.
	{
		if (useFilter)
		{
			if (HasExtension(*file, std::string(filter)))									// Returns true if the given path has the given extension.
			{
				continue;																	// Goes to the next iteration as the current file had the filtered extension.
			}
		}
		
		std::string path = directory + std::string("/") + *file;							// Will put together a path with the given directory and the file being currently iterated.

		if (IsDirectory(path.c_str()))
		{
			directories.push_back(*file);													// If the path is a directory, the file will be added to the directory list.
		}
		else
		{
			files.push_back(*file);															// If the path is not a directory, the file will be added to the file list.
		}
	}

	PHYSFS_freeList(fileListing);															// Method that deallocates the resources (memory) of lists returned by PhysFS.
}

void M_FileSystem::DiscoverAllFiles(const char* directory, std::vector<std::string>& files, std::vector<std::string>& directories, const char* filter) const
{
	if (directory == nullptr)
	{
		LOG("[ERROR] File System: Could not Discover All Files inside the given directory! Error: Given Directory string was nullptr.");
		return;
	}
	if (!Exists(directory))
	{
		LOG("[ERROR] File System: Could not Discover All Files inside the { %s } directory! Error: Given Directory does not exist.", directory);
		return;
	}
	
	char** fileListing		= PHYSFS_enumerateFiles(directory);
	bool useFilter			= (filter != nullptr) ? true : false;

	for (char** file = fileListing; *file != nullptr; ++file)
	{
		if (useFilter)
		{
			if (HasExtension(*file, std::string(filter)))
			{
				continue;
			}
		}

		std::string path = directory + std::string("/") + *file;

		if (IsDirectory(path.c_str()))
		{
			directories.push_back(path);
			DiscoverAllFiles(path.c_str(), files, directories, filter);
		}
		else
		{
			files.push_back(path);
		}
	}

	PHYSFS_freeList(fileListing);
}

void M_FileSystem::GetAllFilesWithExtensionAndName(const char* directory, const char* extension, const char* name, std::vector<std::string>& fileList) const
{
	std::vector<std::string> files;
	std::vector<std::string> directories;

	//DiscoverFiles(directory, files, directories);											// Returns a file and directory lists in the given search path's directory.
	DiscoverAllFiles(directory, files, directories);										// Returns a file and directory lists in the given search path's directory.

	for (uint i = 0; i < files.size(); ++i)
	{
		std::string ext;
		std::string file;
		SplitFilePath(files[i].c_str(), nullptr, &file, &ext);							// Gets the extension string/path of the file being currently iterated, if it has any.

		std::size_t found = file.find(name);
		if (ext == extension && found != std::string::npos)																// If the extension of the file is the same as the one passed as argument
		{
			fileList.push_back(files[i]);													// The file currently being iterated will be added to the file list passed as argument.
		}
	}
}

void M_FileSystem::GetAllFilesWithExtension(const char* directory, const char* extension, std::vector<std::string>& fileList) const
{
	std::vector<std::string> files;
	std::vector<std::string> directories;

	//DiscoverFiles(directory, files, directories);											// Returns a file and directory lists in the given search path's directory.
	DiscoverAllFiles(directory, files, directories);										// Returns a file and directory lists in the given search path's directory.

	for (uint i = 0; i < files.size(); ++i)
	{
		std::string ext;
		SplitFilePath(files[i].c_str(), nullptr, nullptr, &ext);							// Gets the extension string/path of the file being currently iterated, if it has any.

		if (ext == extension)																// If the extension of the file is the same as the one passed as argument
		{
			fileList.push_back(files[i]);													// The file currently being iterated will be added to the file list passed as argument.
		}
	}
}

PathNode M_FileSystem::GetFiles(const char* directory, std::vector<std::string>* filterExt, std::vector<std::string>* ignoreExt) const
{
	PathNode root;

	if (directory == nullptr)
	{
		LOG("[ERROR] File System: Could not Get Files From Directory! Error: Given Directory string was nullptr.");
		return root;
	}
	if (!Exists(directory))
	{
		LOG("[ERROR] File System: Could not Get Files From { %s } Directory! Error: Given Directory does not exist.", directory);
		return root;
	}

	root.path = directory;
	SplitFilePath(directory, nullptr, &root.local_path);

	if (root.local_path == "")
	{
		root.local_path = directory;
	}

	std::vector<std::string> files;
	std::vector<std::string> directories;
	DiscoverFiles(directory, files, directories);

	for (uint i = 0; i < directories.size(); ++i)
	{
		PathNode dir;

		dir.isFile = false;
		dir.isLeaf = false;
		dir.isLastDirectory = IsLastDirectory(dir);

		root.children.push_back(dir);
	}

	for (uint i = 0; i < files.size(); ++i)
	{
		// THIS FUNCTION HAS BEEN PUT ON HOLD
	}

	files.clear();
	directories.clear();
}

PathNode M_FileSystem::GetAllFiles(const char* directory, std::vector<std::string>* filterExt, std::vector<std::string>* ignoreExt) const
{
	PathNode root;

	if (Exists(directory))
	{
		root.path = directory;
		SplitFilePath(directory, nullptr, &root.local_path);								// Keep this in mind. Originaly it was App->fileSystem->SplitFilePath().

		if (root.local_path == "")															// If there is no file path in the passed directory.
		{
			root.local_path = directory;													// Set the file path with the passed directory.
		}

		std::vector<std::string> file_list;
		std::vector<std::string> directory_list;

		DiscoverFiles(directory, file_list, directory_list);								// Get all the files and directories contained in the passed directory.

		for (uint i = 0; i < directory_list.size(); ++i)									// Adding all child directories to the PathNode's children list.
		{
			std::string path = directory;
			
			path.append("/").append(directory_list[i]);										// Extends the path string with "/" and the directory string in the directory_list being iterated.
			
			root.children.push_back(GetAllFiles(path.c_str(), filterExt, ignoreExt));		// Recursively gets all the children directories and adds the current as child of the PathNode.
		}

		for (uint i = 0; i < file_list.size(); ++i)											// Adding all child files to the PathNode's children list.
		{
			bool filter = true;
			bool discard = false;

			if (filterExt != nullptr)
			{
				filter = HasExtension(file_list[i].c_str(), *filterExt);					// Will check all the extensions in the list to check if the file being iterated has any of them.
			}

			if (ignoreExt != nullptr)
			{
				discard = HasExtension(file_list[i].c_str(), *ignoreExt);					// Will check all the extensions in the list to check if the file being iterated has any of them.
			}

			if (filter && !discard)															// If the file being iterated has filtered extensions but no discarded extensions
			{
				std::string path = directory;

				path.append("/").append(file_list[i]);										// Extends the path string with "/" and the file string in the file_list being iterated.

				root.children.push_back(GetAllFiles(path.c_str(), filterExt, ignoreExt));	// Recursively gets all the children files and adds the current as child of the root PathNode.
			}
		}

		root.isFile				= HasExtension(root.path.c_str());						// root.isFile will be true if its path has any extensions.
		root.isLeaf				= root.children.empty();								// root.isLeaf will be true if root has no child nodes.
		root.isLastDirectory		= IsLastDirectory(root);
	}

	return root;
}

bool M_FileSystem::ContainsDirectory(const char* directory) const
{
	bool ret = true;
	
	if (directory == nullptr)
	{
		LOG("[ERROR] File System: Could not check whether or not Directory Contained Directories! Error: Given Directory Path was nullptr.");
		return false;
	}

	if (!IsDirectory(directory))
	{
		return false;
	}

	std::vector<std::string> files;
	std::vector<std::string> directories;

	DiscoverFiles(directory, files, directories);

	ret = !directories.empty();

	files.clear();
	directories.clear();

	return ret;
}

bool M_FileSystem::IsLastDirectory(const PathNode& pathNode) const
{
	for (uint i = 0; i < pathNode.children.size(); ++i)
	{
		if (IsDirectory(pathNode.children[i].path.c_str()))
		{
			return false;
		}
	}

	return true;
}

bool M_FileSystem::HasExtension(const char* path) const
{
	std::string extension = "";
	
	SplitFilePath(path, nullptr, nullptr, &extension);										// Will fill the extension string with the extension string in the given path, if there is any.

	return extension.empty();																// Returns true if extension is not "".
}

bool M_FileSystem::HasExtension(const char* path, std::string extension) const
{
	std::string ext = "";

	SplitFilePath(path, nullptr, nullptr, &ext);											// Will fill the extension string with the extension string in the given path, if there is any.

	return ext == extension;																// Returns true if the given path has the given extension in it.
}

bool M_FileSystem::HasExtension(const char* path, std::vector<std::string> extensions) const
{
	std::string extension = "";

	SplitFilePath(path, nullptr, nullptr, &extension);

	if (extension.empty())
	{
		return false;
	}

	for (uint i = 0; i < extensions.size(); ++i)											// All extensions in the list will be checked to see if any matches the one given by the path.
	{
		if (extension == extensions[i])
		{
			return true;
		}
	}
	
	return false;
}

std::string M_FileSystem::NormalizePath(const char* fullPath) const
{
	std::string normalizedPath(fullPath);

	for (uint i = 0; i < normalizedPath.size(); ++i)
	{
		if (normalizedPath[i] == '\\')
		{
			normalizedPath[i] = '/';
		}
	}

	return normalizedPath;
}

bool M_FileSystem::PathIsNormalized(const char* fullPath) const
{
	std::string path(fullPath);

	for (uint i = 0; i < path.size(); ++i)
	{
		if (path[i] == '\\')
		{
			return false;
		}
	}

	return true;
}

void M_FileSystem::SplitFilePath(const char* fullPath, std::string* path, std::string* file, std::string* extension) const
{
	if (fullPath != nullptr)
	{
		std::string full(fullPath);
		size_t positionSeparator = full.find_last_of("\\/");								// find_last_of() returns the position of the specified character.
		size_t positionDot = full.find_last_of(".");										// It returns size_t. (size_t == uint but defined in vcruntime.h)

		if (path != nullptr)
		{
			if (positionSeparator < full.length())
			{
				*path = full.substr(0, positionSeparator + 1);								// The path will be the string from position 0 to the position of the separator.
			}
			else
			{
				path->clear();																// If the last separator's position is equal to fullPath's length then there is no path.
			}
		}

		if (file != nullptr)
		{
			if (positionSeparator < full.length())
			{
				*file = full.substr(positionSeparator + 1, positionDot - positionSeparator - 1);	// The file string will be from the position of the separator to (Complete later).
			}
			else
			{
				*file = full.substr(0, positionDot);										// The file string will be the string from postion 0 to the last dot's position.
			}
		}

		if (extension != nullptr)
		{
			if (positionDot < full.length())
			{
				*extension = full.substr(positionDot + 1);									// The extension string will span from the position of the last dot to the end of the string.
			}
			else
			{
				extension->clear();															// If the last dot's position is the same as full_path's length, then there is no extension.
			}
		}
	}
}

// ---------------- OPEN FOR READ/WRITE METHODS ----------------
uint M_FileSystem::Load(const char* path, const char* file, char** buffer) const
{
	std::string fullPath(path);

	fullPath += file;

	return Load(fullPath.c_str(), buffer);
}

uint M_FileSystem::Load(const char* file, char** buffer) const
{
	uint ret = 0;

	PHYSFS_file* fsFile = PHYSFS_openRead(file);											// Method that opens the file with the given path for reading.

	if (fsFile != nullptr)
	{
		PHYSFS_sint32 size = (PHYSFS_sint32)PHYSFS_fileLength(fsFile);						// Mehtod that returns the length of a file in bytes.

		if (size > 0)
		{
			*buffer = new char[size + 1];

			uint amountRead = (uint)PHYSFS_readBytes(fsFile, *buffer, size);				// Method that returns the amount of objects read and stores the data in the given buffer.
			//uint amount_read = (uint)PHYSFS_readBytes(fs_file, *buffer, 1, size);			// PHYSFS_read has been deprecated, now PHYSF_readBytes is the new method to read from a file.

			if (amountRead != size)
			{
				LOG("[ERROR] File System: Could not read from File %s! Error: %s\n", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
				RELEASE_ARRAY(buffer);
			}
			else
			{
				ret = amountRead;

				(*buffer)[size] = '\0';														// Adding end of file at the end of the buffer.
			}
		}

		if (PHYSFS_close(fsFile) == (int)PhysfsResult::FAILURE)							// Method that closes a given file previously opened by PhysFS.
		{
			LOG(" [ERROR] File System: Could not close File %s! Error: %s\n", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		}
	}
	else
	{
		LOG("[ERROR] File System: Could not open File %s! Error: %s\n", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}

	return ret;
}

uint M_FileSystem::Save(const char* file, const void* buffer, uint size, bool append) const
{
	uint ret = 0;

	bool overwrite = Exists(file);															// Checks whether or not the given file already exists in the search path.

	PHYSFS_file* fsFile;																	// PHYSFS_file is a PhysFS file handle.
	/**/																					// Whenever an op (r,w, etc) is done to a file, a ptr to one of these file handles is returned.

	if (append)
	{
		fsFile = PHYSFS_openAppend(file);													// Method that opens the given file for appending.
	}
	else
	{
		fsFile = PHYSFS_openWrite(file);													// Method that opens the given file for writing.
	}

	if (fsFile != nullptr)
	{
		uint written = (uint)PHYSFS_writeBytes(fsFile, buffer, size);						// Method that writes data from a PHYSFS_File to the given buffer. Returns the n of bytes written.
		//uint written = (uint)PHYSFS_writeBytes(fs_file, buffer, size);					// PHYSFS_write() has been deprecated. Now PHYSFS_writeBytes() is the standard.

		if (written != size)																// Checks whether or not all the data has been written.
		{
			LOG("[ERROR] File System: Could not write to File %s! Error: %s", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		}
		else
		{
			if (append)
			{
				LOG("[FILE_SYSTEM] File System: Successfully Appended/Added %u data to [%s%s]", size, GetWriteDirectory(), file);
			}
			else if (overwrite)
			{
				LOG("[FILE_SYSTEM] File System: Successfully overwrote File [%s%s] with %u bytes", GetWriteDirectory(), file, size);
			}
			else
			{
				LOG("[FILE_SYSTEM] File System: Successfully created New File [%s%s] of %u bytes", GetWriteDirectory(), file, size);
			}

			ret = written;
		}

		PhysfsResult result = (PhysfsResult)PHYSFS_close(fsFile);						// Method that closes a PhysFS file handle. Returns 1 on success.
		if (result == PhysfsResult::FAILURE)
		{
			LOG("[ERROR] File System: Could not close File %s! Error: %s", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		}
	}
	else
	{
		LOG("[ERROR] File System: Could not open File %s! Error: %s", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}

	return ret;
}

bool M_FileSystem::DuplicateFile(const char* file, const char* destinationFolder, std::string& relativePath)
{
	std::string fileString;
	std::string extensionString;

	SplitFilePath(file, nullptr, &fileString, &extensionString);														// Gets the file str and the extension str from the given path/file.

	relativePath = relativePath.append(destinationFolder).append("/") + fileString.append(".") + extensionString;	// Extends relative path with the dst_folder, file and extension str.
	std::string destinationFile = std::string(*PHYSFS_getSearchPath()).append("/") + relativePath;					// Gets a string with the complete path to where duplicate the file.

	return DuplicateFile(file, destinationFile.c_str());
}

bool M_FileSystem::DuplicateFile(const char* sourceFile, const char* destinationFile)
{
	std::ifstream source;																	// Input stream class to operate on files. Represents the source file that will act as the input.
	std::ofstream destination;																// Output stream class to operate on files. Represents the dst file that will act as the output.

	source.open(sourceFile, std::ios::binary);												// Opens the given file and associates it with the stream object.
	destination.open(destinationFile, std::ios::binary);									// The mode argument defines that the operations will be made in binary mode instead of text.

	bool sourceIsOpen = source.is_open();													// is_open() returns true if the stream object calling it is currently associated with a file.
	bool destinationIsOpen = destination.is_open();										// Read the line above.

	destination << source.rdbuf();															// rdbuf() returns the internal file buffer where the data is read from or written to.
	/**/																					// In this case the data from source's buffer is read and written to the destination stream obj.

	source.close();																			// close() closes the file currently associated with the object.
	destination.close();																	// This, in turn, disassociates the file from the stream.

	if (sourceIsOpen && destinationIsOpen)
	{
		LOG("[FILE_SYSTEM] File System: File Duplicated Correctly!");
		return true;
	}
	else
	{
		LOG("[ERROR] File System: File could not be Duplicated!");
		return false;
	}
}

bool M_FileSystem::Remove(const char* file)
{
	bool ret = false;

	if (file == nullptr)
	{
		LOG("[ERROR] File System: Could not delete File! Error: Given File Path was nullptr.");
		return false;
	}
	if (!Exists(file))
	{
		LOG("[ERROR] File System: Could not delete File [%s]! Error: %s\n", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}

	if (IsDirectory(file))																// If the given file is a directory, then all the files inside it need to be removed recursively.
	{
		std::vector<std::string> containedFiles;										// Will store all the contained files in the directory. THIS HERE
		std::vector<std::string> containedDirectories;									// Will store all the contained directories in the directory.

		PathNode root_directory = GetAllFiles(file);									// Gets all the files and dirs contained in the given directory and stores them as its children.

		for (uint i = 0; i < root_directory.children.size(); ++i)
		{
			Remove(root_directory.children[i].path.c_str());							// Recursively removes the children being iterated. This means that the file will be deleted.
		}
	}

	PhysfsResult result = (PhysfsResult)PHYSFS_delete(file);							// Method that deletes a file or directory. A directory must be empty before it can be deleted.
	if (result == PhysfsResult::SUCCESS)
	{
		LOG("[FILE_SYSTEM] File System: Successfully deleted File [%s]!", file);
		ret = true;
	}
	else
	{
		LOG("[ERROR] File System: Could not delete File [%s]! Error: %s\n", file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}
	
	return ret;
}

// Method that returns the last mod time of the given file. Returns -1 if it cannot be determined.
uint64 M_FileSystem::GetLastModTime(const char* fileName) const
{
	PHYSFS_Stat stat;
	if (!PHYSFS_stat(fileName, &stat))
		LOG("[ERROR] File System: Could not determine last modified time of File [%s]! Error: %s\n", fileName, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	return stat.modtime;
}

std::string M_FileSystem::GetUniqueName(const char* path, const char* name) const
{
	std::vector<std::string> files;
	std::vector<std::string> directories;

	DiscoverFiles(path, files, directories);												// Gets all the files and directories contained in the given path/directory.

	std::string finalName(name);
	bool unique = false;

	for (uint index = 0; index < 50 && !unique; ++index)									// Allows up to 50 unique names with the same base name. Ex: house_01, house_02, ..., house_50.
	{
		unique = true;

		if (index > 0)																		// Constructing the name to be compared. File name "index" will start at 01.
		{
			finalName = std::string(name).append("_");										// name: "house" + "_" -> final_name: "house_"

			if (index < 10)																	// If the file's index is below 10.
			{
				finalName.append("0");														// Adds a 0 before the index. final_name: "house_0"
			}

			finalName.append(std::to_string(index));										// Adds the index to the file name string. final_name: "house_01"
		}

		for (uint i = 0; i < files.size(); ++i)												// Iterates through all the files in the directory to find a match.
		{
			if (finalName == files[i])
			{
				unique = false;																// If there is a match, the name is not unique anymore and the loop jumps to the next iteration.
				break;
			}
		}
	}

	return finalName;
}

std::string M_FileSystem::GetDirectory(const char* path)
{
	std::string fullPath	= NormalizePath(path);
	std::string dirPath	= "";

	SplitFilePath(fullPath.c_str(), &dirPath, nullptr, nullptr);

	return dirPath;
}

std::string M_FileSystem::GetLastDirectory(const char* path)
{	
	std::string fullPath	= NormalizePath(path);											// Assets/Dir/LastDir/File.extension. Normalized to avoid errors regarding "\".
	std::string dirPath	= "";
	std::string lastDir	= "";

	SplitFilePath(fullPath.c_str(), &dirPath, nullptr, nullptr);							// Assets/Dir/LastDir/

	dirPath			= dirPath.substr(0, dirPath.size() - 1);							// Assets/Dir/LastDir
	uint last_dir_start	= dirPath.find_last_of("/") + 1;									// Getting the position of the "/" before the last directory.
	lastDir			= dirPath.substr(last_dir_start, dirPath.size()) + "/";			// LastDir/

	return lastDir;
}

std::string M_FileSystem::GetFileExtension(const char* path)
{
	std::string fullPath = path;
	std::string extension = "";

	size_t dotPosition = fullPath.find_last_of(".");

	extension = (dotPosition != fullPath.npos) ? fullPath.substr(dotPosition + 1) : "[NONE]";

	/*if (extension == "[NONE]")
	{
		//LOG("[WARNING] File System: Path %s does not have any file extension!", path);
	}*/

	return extension;
}

std::string M_FileSystem::GetFileAndExtension(const char* path)
{
	std::string fullPath	= path;
	std::string file		= "";
	std::string extension	= "";															// Just for safety check purposes.

	fullPath = NormalizePath(fullPath.c_str());											// Will swap all '\\' for '/'.														

	size_t fileStart		= fullPath.find_last_of("/");									// Gets the position of the last '/' of the string. Returns npos if none was found.
	size_t extensionStart	= fullPath.find_last_of(".");									// Gets the position of the last '.' of the string. Returns npos if none was found.

	if (fileStart != std::string::npos)
	{
		file = fullPath.substr(fileStart + 1, fullPath.size());							// Will get the string past the last slash
	}
	else
	{
		LOG("[WARNING] File System: Path %s does not have any file!", path);
	}

	if (extensionStart != std::string::npos)
	{
		extension = fullPath.substr(extensionStart + 1, fullPath.size());				// Will get the string past the last dot of the path string. Ex: File.ext --> ext

		if (extension == "")
		{
			LOG("[WARNING] File System: Path %s does not have any file extension!", path);
		}
	}

	return file;
}

std::string M_FileSystem::GetLastDirectoryAndFile(const char* path)
{
	std::string fullPath	= NormalizePath(path);
	std::string dirPath	= "";
	std::string lastDir	= "";
	std::string file		= "";

	SplitFilePath(path, &dirPath, &file, nullptr);

	dirPath			= dirPath.substr(0, dirPath.size() - 1);							// Assets/Dir/LastDir
	uint lastDirStart = dirPath.find_last_of("/") + 1;									// Getting the position of the "/" before the last directory.
	lastDir			= dirPath.substr(lastDirStart, dirPath.size()) + "/";			// LastDir/

	return (lastDir + file);
}