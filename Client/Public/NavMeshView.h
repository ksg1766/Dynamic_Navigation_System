#pragma once
#include "View.h"
#include "Client_Defines.h"

BEGIN(Engine)

class CGameObject;
class CShader;
struct Obst;
struct Cell;

END

BEGIN(Client)

struct iVec3
{
	_int x, y, z;

	_bool operator<(const iVec3& other) const { return (x == other.x) ? z < other.z : x < other.x; }
};

class CNavMeshView final : public CView
{
    using Super = CView;

private:
	CNavMeshView(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CNavMeshView() = default;

public:
	virtual HRESULT Initialize(void* pArg)	override;
	virtual HRESULT Tick()					override;
	virtual HRESULT LateTick()				override;
	virtual HRESULT	DebugRender()			override;

public:
	HRESULT		DynamicCreate(CGameObject* const pGameObject);
	HRESULT		DynamicCreate(const wstring& strObjectTag, const Vec3& vPickPos, Matrix matWorld = Matrix::Identity);
	HRESULT		UpdateObstacleTransform(CGameObject* const pGameObject);

private:
	void		ClearNeighbors(vector<Cell*>& vecCells);
	void		SetUpNeighbors(vector<Cell*>& vecCells);
	void		SetUpCells2Grids(vector<Cell*>& vecCells, OUT unordered_multimap<_int, Cell*>& umapCellGrids, const _int iGridCX = 64U, const _int iGridCZ = 64U);
	void		SetUpObsts2Grids(vector<Obst*>& vecObstacles, OUT unordered_multimap<_int, Obst*>& umapObstGrids, const _int iGridCX = 64U, const _int iGridCZ = 64U);

	HRESULT		BakeNavMesh();
	HRESULT		BakeSingleObstacleData();
	HRESULT		BakeObstacles();
	HRESULT		BakeHeightMap3D();

	HRESULT		UpdatePointList(triangulateio& tIn, const vector<Vec3>& vecPoints, const Obst* pObst = nullptr);
	HRESULT		UpdateSegmentList(triangulateio& tIn, const vector<Vec3>& vecPoints, const Obst* pObst = nullptr);
	HRESULT		UpdateHoleList(triangulateio& tIn, const Obst* pObst = nullptr);
	HRESULT		UpdateRegionList(triangulateio& tIn, const Obst* pObst = nullptr);

	HRESULT		DynamicCreate(Obst& tObst);
	HRESULT		DynamicDelete(Obst& tObst);

	HRESULT		CreateAgent(Vec3 vSpawnPosition);
	HRESULT		CreateAgent(_int iSpawnIndex);
	HRESULT		StressTest();

	HRESULT		CreateAI();
	HRESULT		StartAIStressTest();

private:
	void		SetPolygonHoleCenter(Obst& tObst);
	HRESULT		GetIntersectedCells(Obst& tObst, OUT set<Cell*>& setIntersected, _bool bPop = false, _bool bDelete = false);

private:
	HRESULT		CalculateObstacleOutline(CGameObject* const pGameObject, OUT vector<Vec3>& vecOutline);
	HRESULT		CalculateObstacleOutlines(OUT vector<vector<Vec3>>& vecOutlines, CGameObject* const pGameObject = nullptr);
	HRESULT		CalculateObstacleOutlinesTopDown(OUT vector<vector<Vec3>>& vecOutlines, CGameObject* const pGameObject = nullptr);
	HRESULT		CalculateHillOutline(OUT vector<vector<Vec3>>& vecOutlines);
	void		Dfs(const iVec3& vStart, const set<iVec3>& setPoints, OUT vector<iVec3>& vecLongest);
	void		DfsTerrain(vector<vector<_int>>& vecPoints, OUT vector<vector<iVec3>>& vecOutlines);
	Vec3		CalculateNormal(const iVec3& vPrev, const iVec3& vCurrent, const iVec3& vNext);
	_bool		IsClockwise(const vector<iVec3>& vecPoints);
	vector<Vec3> ExpandOutline(const vector<iVec3>& vecOutline, _float fDistance);
	_int		IntersectSegments(const Vec3& vP1, const Vec3& vP2, const Vec3& vQ1, const Vec3& vQ2, Vec3* pIntersection);
	vector<Vec3> ProcessIntersections(vector<Vec3>& vecExpandedOutline);

	_float		PerpendicularDistance(const Vec3& vPt, const Vec3& vLineStart, const Vec3& vLineEnd);
	void		RamerDouglasPeucker(const vector<Vec3>& vecPointList, _float fEpsilon, OUT vector<Vec3>& vecOut);

private:
	void		Input();
	Cell*		FindCellByPosition(const Vec3& vPosition);

	HRESULT		SaveNvFile();
	HRESULT		Save3DNvFile();
	HRESULT		LoadNvFile();
	HRESULT		Load3DNvFile();
	HRESULT		DeleteNvFile();
	HRESULT		RefreshNvFile();

	HRESULT		LoadMainScene();
	HRESULT		LoadMazeTestScene();

	HRESULT		SaveObstacleLocalOutline(const Obst* const pObst, string strName);
	HRESULT		LoadObstacleOutlineData();

private:
	void		InfoView();
	void		ObstaclesGroup();

private:
	HRESULT		InitialSetting();
	HRESULT		Reset();

	HRESULT		SafeReleaseTriangle(triangulateio& tTriangle);

private:
	// Path Finding (A*)
	class CAgent*			m_pAgent = nullptr;
	vector<class CAIAgent*>	m_vecAIAgents;

	// Cell Data
	_int					m_iStaticPointCount = 0;

	vector<Vec3>			m_vecPoints;
	vector<vector<Vec3>>	m_vecPointsMultiLevel;
	vector<const _char*>	m_strPoints;
	unordered_multimap<_float, pair<_float, _float>>	m_umapPointHeights;

	vector<_int>			m_vecSegments;

	vector<Obst*>			m_vecObstacles;
	unordered_map<CGameObject*, _short>	m_hmapObstacleIndex;
	vector<const _char*>	m_strObstacles;

	vector<Vec3>			m_vecRegions;
	vector<const _char*>	m_strRegions;

	unordered_multimap<_int, Cell*> m_umapCellGrids;
	unordered_multimap<_int, Obst*> m_umapObstGrids;
	vector<Cell*>		m_vecCells;
	vector<const _char*>	m_strCells;

	// Polygon (stress test)
	_bool					m_bStressTest = false;
	Obst*					m_pStressObst = nullptr;
	Matrix					m_matStressOffset = Matrix::Identity;

	// triangulate
	triangulateio			m_tIn, m_tOut;
	vector<triangulateio>	m_vecIn, m_vecOut;
	_char					m_szTriswitches[3] = "pz";

	// Default
	CTerrain*				m_pTerrainBuffer = nullptr;

	// DebugDraw
	_bool					m_bRenderDebug = false;
	_bool					m_bRenderCells = true;
	_bool					m_bRenderObstacleOutlines = true;

	_bool					m_bRenderPathCells = true;
	_bool					m_bRenderEntries = true;
	_bool					m_bRenderWayPoints = true;

	PrimitiveBatch<VertexPositionColor>* m_pBatch = nullptr;
	BasicEffect*			m_pEffect = nullptr;
	ID3D11InputLayout*		m_pInputLayout = nullptr;

	// Datafile
	_int					m_file_Current = 0;
	_int					m_item_Current = 0;
	string					m_strFilePath = "StaticObstacles";
	vector<const _char*>	m_vecDataFiles;

	map<wstring, Obst>		m_mapObstaclePrefabs;

public:
	static class CNavMeshView* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual void Free() override;
};

END