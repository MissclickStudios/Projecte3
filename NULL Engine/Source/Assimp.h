#ifndef __ASSIMP_H__
#define __ASSIMP_H__

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "assimp/mesh.h"
#include "assimp/material.h"
#include "assimp/texture.h"
#include "assimp/matrix4x4.h"
#include "assimp/vector3.h"
#include "assimp/quaternion.h"

struct aiTransform
{
	aiVector3D		position;
	aiQuaternion	rotation;
	aiVector3D		scale;
};

#endif // !__ASSIMP_H__