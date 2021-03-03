#ifndef __PATH_NODE_H__
#define __PATH_NODE_H__

#include <string>
#include <vector>
#include "Macros.h"

struct NULL_API PathNode
{
	PathNode();

	bool IsLastFolder	() const;									// Will return true if this pathnode is the last of the children inside the list.
	bool FindChild		(const char* path, PathNode& child) const;

	bool operator ==(const PathNode node) const;

	std::string path;												// Path that refers to the first PathNode
	std::string local_path;											// File/folder itself.

	std::vector<PathNode> children;									// List with all the children of a given PathNode. i.e All the sub-folders inside a given folder.

	bool isLeaf = true;
	bool isFile = true;
	bool isLastDirectory = true;
};

#endif // !__PATH_NODE_H__