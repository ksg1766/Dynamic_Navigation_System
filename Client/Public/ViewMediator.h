#pragma once
#include "Base.h"
#include "Client_Defines.h"


BEGIN(Engine)

class CGameInstance;
class CGameObject;
class CTerrain;

END

BEGIN(Client)

class CView;
class CPrefabsView;
class CNavMeshView;
class CViewMediator : public CBase
{
	using Super = CBase;

public:
	CViewMediator();
	virtual ~CViewMediator();

public:
	void	OnNotifiedSelected(CGameObject* pGameObject);
	void	OnNotifiedPickingOn(CView* pSender);
	void	OnNotifiedPlaceObject(const wstring& strObjectTag, const Matrix& matWorld, OUT CGameObject*& pGameObject);
	void	OnNotifiedPlaceObstacle(CGameObject* const pGameObject);
	void	OnNotifiedPlaceObstacle(const wstring& strObjectTag, const Vec3& vPickPos);
	void	OnNotifiedTransformChanged(CGameObject* const pGameObject);
	void	OnNotifiedTerrainChanged(CTerrain* const pTerrainBuffer);

	void	SetPrefabsView(CPrefabsView* pPrefabsView);
	void	SetNavMeshView(CNavMeshView* pNavMeshView);

protected:
	CGameInstance*	m_pGameInstance = nullptr;

	_bool			m_IsPickingActivated = false;

	CPrefabsView*	m_pPrefabsView = nullptr;
	CNavMeshView*	m_pNavMeshView = nullptr;

public:
	virtual void Free() override;
};

END