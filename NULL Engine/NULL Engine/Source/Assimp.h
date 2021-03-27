#ifndef __ASSIMP_H__
#define __ASSIMP_H__

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#include "Assimp/include/mesh.h"
#include "Assimp/include/material.h"
#include "Assimp/include/texture.h"
#include "Assimp/include/matrix4x4.h"
#include "Assimp/include/vector3.h"
#include "Assimp/include/quaternion.h"

struct aiTransform
{
	aiVector3D		position;
	aiQuaternion	rotation;
	aiVector3D		scale;
};

#endif // !__ASSIMP_H__