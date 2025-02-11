#pragma once

#include "Component.h"

/* 1. 화면에 그려져야하는 객체들을 그리는 순서대로 모아서 보관한다. */
/* 2. 보관하고 있는 객체들의 렌더콜(드로우콜)을 수행한다. */

BEGIN(Engine)

class ENGINE_DLL CRenderer final : public CComponent
{
	using Super = CComponent;
public:
	/* RG_NONBLEND : 이후 그려지는 Blend오브젝트들의 섞는 연산을 위해 반드시 불투명한 애들을 먼저 그려야한다. */
	/* RG_BLEND : 반투명하게 그려지는 객체들도 반드시 멀리있는 놈부터 그린다. */
	enum RENDERGROUP { RG_PRIORITY, RG_NONLIGHT, RG_SHADOW_INSTANCE, RG_NONBLEND_INSTANCE, RG_NONBLEND, RG_BLEND, RG_BLEND_INSTANCE, RG_UI, RG_END };
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);	
	CRenderer(const CRenderer& rhs) = delete;
	virtual ~CRenderer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pGameObject);
	HRESULT Draw_RenderObjects();

#ifdef _DEBUG
public:
	HRESULT Add_Debug(class CComponent* pDebug)
	{
		m_RenderDebug.push_back(pDebug);
		Safe_AddRef(pDebug);
		return S_OK;
	}
#endif

private:
	vector<CGameObject*>						m_RenderObjects[RG_END];
	map<InstanceID, class CVIBuffer_Instance*>	m_InstanceBuffers;

	class CTargetManager*						m_pTargetManager = { nullptr };
	class CLightManager*						m_pLightManager = { nullptr };
	class CGraphicDevice*						m_pGraphicDevice = nullptr;

private:
	class CVIBuffer_Rect*						m_pVIBuffer = { nullptr };
	class CShader*								m_pShader = { nullptr };
	class CShader*								m_pShaderPostProcess = { nullptr };
	ID3D11DepthStencilView*						m_pShadowDSV = nullptr;

	_float4x4									m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;
	_int										m_iWaterCaptureCount = 6;
	_int										m_iMaxWaterCaptureCount = 1;

	_float4x4									m_LightView, m_LightProj;

#ifdef _DEBUG
private:
	vector<CComponent*>							m_RenderDebug;
	_bool										m_bTargetOnOff = false;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_NonLight();
	HRESULT Render_NonBlend();
	HRESULT Render_NonBlend_Instance();

	HRESULT Render_LightAcc();
	HRESULT Render_Deferred();

	HRESULT Render_Blend();
	HRESULT Render_Blend_Instance();
	HRESULT Render_UI();

#ifdef _DEBUG
	HRESULT Render_Debug();
#endif

private:
	void	AddInstanceData(InstanceID instanceId, InstancingData& data);
	void	ClearInstanceData();

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(CGameObject* pGameObject, void* pArg) override;
	virtual void Free() override;
};

END