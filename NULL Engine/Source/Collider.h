#ifndef __COLLIDER_H__
#define __COLLIDER_H__

namespace physx
{
	class PxRigidActor;
	class PxRigidStatic;
	class PxTransform;
	class PxShape;
	class PxConvexMesh;
	class PxTriangleMesh;
	class PxVec3;
}

enum class ColliderUpdateType
{
	NONE = 0,
	STATE = 1,
	SHAPE = 2,
	ALL = 3
};

#endif // !__COLLIDER_H__
