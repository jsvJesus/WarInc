#ifndef __PHYSXWORLD_H__
#define __PHYSXWORLD_H__

#include <extensions/PxDefaultCpuDispatcher.h>

#include "WarIncPhysX34Compat.h"

#ifndef WO_SERVER
#include "VehicleManager.h"
#endif

class VehicleManager;

class PhysXWorld
{
	bool m_needFetchResults;

public:
	PxFoundation* Foundation;
	PxPvd* Pvd;
	PxPvdTransport* PvdTransport;

	PxPhysics* PhysXSDK;
	PxScene* PhysXScene;
	PxCooking* Cooking;
	PxControllerManager* CharacterManager;
	PxDefaultCpuDispatcher* CpuDispatcher;

	PxMaterial* defaultMaterial;

	VehicleManager* m_VehicleManager;

	CRITICAL_SECTION concurrencyGuard;

public:
	PhysXWorld();
	~PhysXWorld();

	void Init();
	void Destroy();

	bool CookMesh(const r3dMesh* mesh, const char* save_as = NULL);
	PxTriangleMesh* getCookedMesh(const char* filename);

	bool CookConvexMesh(const r3dMesh* mesh, const char* save_as = NULL);
	PxConvexMesh* getConvexMesh(const char* filename);

	bool HasCookedMesh(const r3dMesh* mesh);
	bool HasConvexMesh(const r3dMesh* mesh);

	bool raycastSingle(
		const PxVec3& origin,
		const PxVec3& unitDir,
		const PxReal distance,
		PxSceneQueryFlags outputFlags,
		PxRaycastHit& hit,
		const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData()
	);

	void StartSimulation();
	void EndSimulation();
	void DrawDebug();

	CRITICAL_SECTION& GetConcurrencyGuard()
	{
		return concurrencyGuard;
	}

	void AddActor(PxActor& actor);
	void RemoveActor(PxActor& actor);

#ifndef FINAL_BUILD
	bool ExportWholeScene(const char* filename) const;
#endif
};

extern PhysXWorld* g_pPhysicsWorld;

class MyPhysXAllocator : public PxAllocatorCallback
{
public:
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) OVERRIDE;
	virtual void deallocate(void* ptr) OVERRIDE;
};

#endif