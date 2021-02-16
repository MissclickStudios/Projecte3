// ----------------------------------------------------
// Struct abstraction to deal with files and folders.
// Employed by M_FileSystem.
// ----------------------------------------------------

#include "VariableTypedefs.h"
#include "Log.h"

#include "PathNode.h"

PathNode::PathNode() : path("")
{

}

bool PathNode::IsLastFolder() const
{
	for (uint i = 0; i < children.size(); ++i)
	{
		if (!children[i].is_file)
		{
			return false;
		}
	}

	return true;
}

bool PathNode::FindChild(const char* path, PathNode& child) const
{
	bool ret = false;
	
	if (path == nullptr)
	{
		return false;
	}

	for (uint i = 0; i < children.size(); ++i)
	{	
		if (children[i].path == path)
		{
			child = children[i];
			return true;
		}

		if (children[i].FindChild(path, child))
		{
			return true;
		}
	}

	return ret;
}

bool PathNode::operator ==(const PathNode node) const
{
	return path == node.path;
}