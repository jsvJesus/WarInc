#include "r3dPCH.h"
#include "r3d.h"
#include "GameCommon.h"

#include "BulletShellManager.h"

BulletShell::BulletShell()
{
	m_Type = BST_Pistol;
	m_physObj = 0;
	m_Pos.Assign(0,0,0);
	D3DXMatrixIdentity(&m_RenderMatrix);
}

BulletShell::~BulletShell()
{
	Destroy();
}

const float BULLET_LIFETIME = 15.0f;

void BulletShell::Init(const r3dPoint3D& pos, const r3dPoint3D& vel, const PhysicsObjectConfig& config, r3dMesh* mesh, BulletShellType type, const D3DXMATRIX& rotation)
{
	Destroy();

	if(!mesh)
	{
		r3dOutToLog("BulletShell::Init skipped: null shell mesh, type=%d\n", (int)type);
		return;
	}

	m_physObj = BasePhysicsObject::CreateDynamicObject(config, this, &pos, &mesh->localBBox.Size, mesh, &rotation);

	if(!m_physObj)
	{
		r3dOutToLog("BulletShell::Init skipped: failed to create physics object, type=%d, mesh=%s\n", (int)type, mesh->FileName.c_str());
		return;
	}

	m_physObj->addImpulse(vel);

	D3DXMatrixTranslation(&m_RenderMatrix, pos.x, pos.y, pos.z);

	m_Type = type;
	m_StartTime = r3dGetTime();
}

void BulletShell::Destroy()
{
	SAFE_DELETE(m_physObj);
}

void BulletShell::Update()
{
	if(m_physObj)
	{
		if(!m_physObj->IsSleeping())
		{
			r3dVector physPos = m_physObj->GetPosition();

			if(r3d_float_isFinite(physPos.x) && r3d_float_isFinite(physPos.y) && r3d_float_isFinite(physPos.z))
			{
				m_Pos = physPos;
				D3DXMatrixTranslation(&m_RenderMatrix, m_Pos.x, m_Pos.y, m_Pos.z);

				D3DXMATRIX mat = m_physObj->GetRotation();
				mat.m[3][0] = 0.0f;
				mat.m[3][1] = 0.0f;
				mat.m[3][2] = 0.0f;
				mat.m[3][3] = 1.0f;

				m_RenderMatrix = mat * m_RenderMatrix;
			}
		}

		if((r3dGetTime() - m_StartTime) > BULLET_LIFETIME)
			Destroy();
	}
}

void BulletShell::OnCollide(PhysicsCallbackObject *obj, CollisionInfo &trace)
{
	static int ejectBrassSoundID = -1;

	if(ejectBrassSoundID == -1)
		ejectBrassSoundID = SoundSys.GetEventIDByPath("Sounds/Misc/EjectBrass");

	if(ejectBrassSoundID != -1)
		snd_PlaySound(ejectBrassSoundID, m_Pos);
}

BulletShellMngr::BulletShellMngr()
{
	m_numActiveShells = 0;

	for(int i = 0; i < BST_NumElements; ++i)
	{
		m_shellMeshes[i] = NULL;
	}

	m_shellMeshes[BST_Pistol] = r3dGOBAddMesh("Data/ObjectsDepot/Weapons/Shell_Pistol.sco", true, false, false, true);
	m_shellMeshes[BST_Rifle] = r3dGOBAddMesh("Data/ObjectsDepot/Weapons/Shell_Rifle.sco", true, false, false, true);
	m_shellMeshes[BST_Shotgun] = r3dGOBAddMesh("Data/ObjectsDepot/Weapons/Shell_Shotgun.sco", true, false, false, true);

	for(int i = 0; i < BST_NumElements; ++i)
	{
		if(!m_shellMeshes[i])
		{
			r3dOutToLog("BulletShellMngr: shell mesh missing, shell type %d disabled\n", i);

			m_shellPhysConfigs[i].group = PHYSCOLL_TINY_GEOMETRY;
			m_shellPhysConfigs[i].type = PHYSICS_TYPE_BOX;
			m_shellPhysConfigs[i].mass = 0.02f;
			m_shellPhysConfigs[i].offset = r3dPoint3D(0, 0, 0);
			m_shellPhysConfigs[i].isDynamic = true;
			m_shellPhysConfigs[i].isKinematic = false;
			m_shellPhysConfigs[i].isTrigger = false;
			m_shellPhysConfigs[i].needBoxCollision = false;
			m_shellPhysConfigs[i].needExplicitCollisionMesh = false;
			m_shellPhysConfigs[i].ready = true;
			m_shellPhysConfigs[i].isFastMoving = true;

			continue;
		}

		m_shellPhysConfigs[i] = GameObject::LoadPhysicsConfig(m_shellMeshes[i]);

		if(!m_shellPhysConfigs[i].ready)
		{
			m_shellPhysConfigs[i].group = PHYSCOLL_TINY_GEOMETRY;
			m_shellPhysConfigs[i].type = PHYSICS_TYPE_BOX;
			m_shellPhysConfigs[i].mass = 0.02f;
			m_shellPhysConfigs[i].offset = r3dPoint3D(0, 0, 0);
			m_shellPhysConfigs[i].isDynamic = true;
			m_shellPhysConfigs[i].isKinematic = false;
			m_shellPhysConfigs[i].isTrigger = false;
			m_shellPhysConfigs[i].needBoxCollision = false;
			m_shellPhysConfigs[i].needExplicitCollisionMesh = false;
			m_shellPhysConfigs[i].ready = true;

			r3dOutToLog("BulletShellMngr: missing .phx for shell mesh %s, using default BOX physics\n", m_shellMeshes[i]->FileName.c_str());
		}

		m_shellPhysConfigs[i].group = PHYSCOLL_TINY_GEOMETRY;
		m_shellPhysConfigs[i].isFastMoving = true;
	}

	m_shellPhysConfigs[BST_Pistol].mass = 0.01f;
	m_shellPhysConfigs[BST_Rifle].mass = 0.015f;
	m_shellPhysConfigs[BST_Shotgun].mass = 0.03f;
}

BulletShellMngr::~BulletShellMngr()
{
	for(int i = 0; i < MAX_SHELLS; ++i)
		m_Shells[i].Destroy();
}

void BulletShellMngr::AddShell(const r3dPoint3D& pos, const r3dPoint3D& vel, const D3DXMATRIX& rotation, BulletShellType shellType)
{
	if(shellType < 0 || shellType >= BST_NumElements)
		return;

	if(!m_shellMeshes[(int)shellType])
		return;

	if(!(r3d_float_isFinite(vel.x) && r3d_float_isFinite(vel.y) && r3d_float_isFinite(vel.z)))
		return;

	m_Shells[m_numActiveShells].Init(pos, vel, m_shellPhysConfigs[(int)shellType], m_shellMeshes[(int)shellType], shellType, rotation);

	++m_numActiveShells;

	if(m_numActiveShells == MAX_SHELLS)
		m_numActiveShells = 0;
}

void BulletShellMngr::Update()
{
	for(int i = 0; i < MAX_SHELLS; ++i)
	{
		if(m_Shells[i].Active())
			m_Shells[i].Update();
	}
}

struct BulletShellDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw(Renderable* RThis, const r3dCamera& Cam)
	{
		R3DPROFILE_FUNCTION("BulletShellDeferredRenderable");

		BulletShellDeferredRenderable* This = static_cast<BulletShellDeferredRenderable*>(RThis);

		if(!This || !This->Mesh || !This->Parent)
			return;

		This->Mesh->SetVSConsts(This->Parent->getDrawMatrix());
		MeshDeferredRenderable::Draw(RThis, Cam);
	}

	BulletShell* Parent;
};

void BulletShellMngr::AppendRenderables(RenderArray(&render_arrays)[rsCount], const r3dCamera& Cam)
{
	R3DPROFILE_FUNCTION("BulletShellMngr::AppendRenderables");

	TL_STATIC_ASSERT(sizeof(BulletShellDeferredRenderable) <= MAX_RENDERABLE_SIZE);

	for(int i = 0; i < MAX_SHELLS; ++i)
	{
		if(!m_Shells[i].Active())
			continue;

		int shellType = (int)m_Shells[i].m_Type;

		if(shellType < 0 || shellType >= BST_NumElements)
			continue;

		if(!m_shellMeshes[shellType])
			continue;

		float distSq = (Cam - m_Shells[i].getPosition()).LengthSq();
		float dist = sqrtf(distSq);

		int idist = R3D_MIN((int)dist, 0xffff);

		uint32_t prevCount = render_arrays[rsFillGBuffer].Count();

		m_shellMeshes[shellType]->AppendRenderablesDeferred(render_arrays[rsFillGBuffer], r3dColor::white);

		for(uint32_t j = prevCount, e = render_arrays[rsFillGBuffer].Count(); j < e; j++)
		{
			BulletShellDeferredRenderable& rend = static_cast<BulletShellDeferredRenderable&>(render_arrays[rsFillGBuffer][j]);
			rend.SortValue |= idist;
			rend.Init();
			rend.Parent = &m_Shells[i];
		}
	}
}