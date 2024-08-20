#include "stdafx.h"
#include "AgentController.h"
#include "GameInstance.h"
#include "Agent.h"
#include "Obstacle.h"
#include "MainCamera.h"
#include "MainCameraController.h"
#include "NSHelper.h"
#include "Terrain.h"
#include "StaticBase.h"
#include "DebugDraw.h"

CAgentController::CAgentController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:Super(pDevice, pContext)
	, m_vLinearSpeed(Vec3(100.0f, 100.0f, 100.0f))
	, m_fAgentRadius(3.4f)
	//, m_fAgentRadius(5.0f)
{
}

CAgentController::CAgentController(const CAgentController& rhs)
	:Super(rhs)
	, m_vLinearSpeed(rhs.m_vLinearSpeed)
	, m_fAgentRadius(rhs.m_fAgentRadius)
{
}

HRESULT CAgentController::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAgentController::Initialize(void* pArg)
{
	m_pTransform = GetTransform();
	m_pTransform->SetScale(m_fAgentRadius * 2.0f * Vec3::One);

#pragma region DebugDraw
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);

	m_pEffect = new BasicEffect(m_pDevice);
	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCodes = nullptr;
	size_t		iLength = 0;
	m_pEffect->GetVertexShaderBytecode(&pShaderByteCodes, &iLength);

	if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCodes, iLength, &m_pInputLayout)))
	{
		Safe_Delete(m_pBatch);
		Safe_Delete(m_pEffect);
		Safe_Release(m_pInputLayout);
		return E_FAIL;
	}
#pragma endregion DebugDraw

	m_pGameObject->GetNavMeshAgent()->SetRadius(m_fAgentRadius);
	m_pGameObject->GetNavMeshAgent()->SetLinearSpeed(m_vLinearSpeed);

	CStaticBase* pHoldingObst = nullptr;

	pHoldingObst = CStaticBase::Create(m_pDevice, m_pContext);
	pHoldingObst->SetObjectTag(TEXT("Bus"));
	pHoldingObst->Initialize(nullptr);
	m_HoldingObstacles.emplace_back(TEXT("Bus"), pHoldingObst);

	pHoldingObst = CStaticBase::Create(m_pDevice, m_pContext);
	pHoldingObst->SetObjectTag(TEXT("Dumpster"));
	pHoldingObst->Initialize(nullptr);
	m_HoldingObstacles.emplace_back(TEXT("Dumpster"), pHoldingObst);

	pHoldingObst = CStaticBase::Create(m_pDevice, m_pContext);
	pHoldingObst->SetObjectTag(TEXT("Picnic_Table"));
	pHoldingObst->Initialize(nullptr);
	m_HoldingObstacles.emplace_back(TEXT("Picnic_Table"), pHoldingObst);

	pHoldingObst = CStaticBase::Create(m_pDevice, m_pContext);
	pHoldingObst->SetObjectTag(TEXT("Receptacle_Recycling"));
	pHoldingObst->Initialize(nullptr);
	m_HoldingObstacles.emplace_back(TEXT("Receptacle_Recycling"), pHoldingObst);

#pragma region AStarPerformance
	/*if (FAILED(m_pGameInstance->Add_Timer(TEXT("Timer_AStar"))))
		return E_FAIL;*/
#pragma endregion AStarPerformance

	return S_OK;
}

void CAgentController::Tick(_float fTimeDelta)
{
	Input(fTimeDelta);
}

void CAgentController::LateTick(_float fTimeDelta)
{
	
}

_bool CAgentController::IsIdle()
{
	return !m_isMoving;
}

_bool CAgentController::IsMoving()
{
	return m_isMoving;
}

_bool CAgentController::Pick(CTerrain* pTerrain, _uint screenX, _uint screenY)
{
	_float fDistance = 0.0f;
	Vec3 vPickedPos = Vec3::Zero;

	if (true == pTerrain->Pick(screenX, screenY, vPickedPos, fDistance, pTerrain->GetTransform()->WorldMatrix()))
	{
		m_pGameObject->GetNavMeshAgent()->SetMoveDirectly(false);
		return m_pGameObject->GetNavMeshAgent()->SetPath(vPickedPos);
	}

	return false;
}

void CAgentController::SetRadius(const _float fRadius)
{
	m_fAgentRadius = fRadius;
	m_pGameObject->GetNavMeshAgent()->SetRadius(m_fAgentRadius);
}

void CAgentController::SetLinearSpeed(const Vec3& vLinearSpeed)
{
	m_vLinearSpeed = vLinearSpeed;
	m_pGameObject->GetNavMeshAgent()->SetLinearSpeed(m_vLinearSpeed);
}

void CAgentController::Input(_float fTimeDelta)
{
	if (KEY_DOWN(KEY::V))
	{
		m_eViewMode = VIEWMODE(((uint8)m_eViewMode + 1U) % (uint8)VIEWMODE::MODE_END);
		m_pGameInstance->ChangeCamera();

		if (VIEWMODE::THIRD == m_eViewMode)
		{
			dynamic_cast<CMainCamera*>(m_pGameInstance->GetCurrentCamera())->GetController()->SetTarget(m_pTransform);
			//::ShowCursor(false);
		}
		else
		{
			//::ShowCursor(true);
		}
	}

	if (VIEWMODE::THIRD == m_eViewMode)
	{
		_long		dwMouseMove = 0;

		if (dwMouseMove = m_pGameInstance->Get_DIMouseMove(DIMS_X))
		{
			m_pTransform->RotateYAxisFixed(Vec3(0.0f, 12 * dwMouseMove * fTimeDelta, 0.0f));
		}

		if (dwMouseMove = m_pGameInstance->Get_DIMouseMove(DIMS_Y))
		{
			m_pTransform->RotateYAxisFixed(Vec3(12 * dwMouseMove * fTimeDelta, 0.0f, 0.0f));
		}

		MoveDirectly(fTimeDelta);
		PlaceObstacle();
		
		::SetCursorPos(g_iWinSizeX / 2, g_iWinSizeY / 2);
	}
}

_bool CAgentController::MoveDirectly(_float fTimeDelta)
{
	if (KEY_PRESSING(KEY::UP_ARROW) || KEY_DOWN(KEY::UP_ARROW))
		m_vNetMove += m_pTransform->GetForward();

	if (KEY_PRESSING(KEY::DOWN_ARROW) || KEY_DOWN(KEY::DOWN_ARROW))
		m_vNetMove -= m_pTransform->GetForward();
	
	if (KEY_PRESSING(KEY::LEFT_ARROW) || KEY_DOWN(KEY::LEFT_ARROW))
		m_vNetMove -= m_pTransform->GetRight();	

	if (KEY_PRESSING(KEY::RIGHT_ARROW) || KEY_DOWN(KEY::RIGHT_ARROW))
		m_vNetMove += m_pTransform->GetRight();

	if (Vec3::Zero != m_vNetMove)
	{
		m_pGameObject->GetNavMeshAgent()->SetMoveDirectly(true);
		m_pGameObject->GetNavMeshAgent()->SetState(true);

		m_vNetMove.Normalize();
		m_vNetMove *= (fTimeDelta * m_vLinearSpeed);

		m_pTransform->Translate(m_vNetMove);
		m_vNetMove = Vec3::Zero;

		return true;
	}

	return false;
}

void CAgentController::PlaceObstacle()
{
	if (KEY_DOWN(KEY::F1))
	{
		(0 != m_iObstacleIndex) ? m_iObstacleIndex = 0 : m_iObstacleIndex = -1;
	}
	else if (KEY_DOWN(KEY::F2))
	{
		(1 != m_iObstacleIndex) ? m_iObstacleIndex = 1 : m_iObstacleIndex = -1;
	}

	if (0 <= m_iObstacleIndex)
	{
		auto& [Name, Object] = m_HoldingObstacles[m_iObstacleIndex];
		Vec3 vPlacePosition = m_pTransform->GetPosition();

		Vec3 vLook = -m_pTransform->GetForward();
		vLook.y = 0.0f;
		vLook.Normalize();

		Vec3 vRight = Vec3::Up.Cross(vLook);
		vRight.Normalize();

		Vec3 vUp = vLook.Cross(vRight);
		vUp.Normalize();;

		Object->GetTransform()->SetRight(vRight);
		Object->GetTransform()->SetUp(vUp);
		Object->GetTransform()->SetForward(vLook);

		vPlacePosition += 25.0f * -vLook;

		Object->GetTransform()->SetPosition(vPlacePosition);

		Object->GetRenderer()->Add_RenderGroup(CRenderer::RG_NONBLEND_INSTANCE, Object);

		if (m_pGameInstance->Mouse_Down(DIM_LB))
		{
			Matrix& matObst = Object->GetTransform()->WorldMatrix();

			CB_PlaceObstacle(Name, vPlacePosition, matObst);
		}
	}
}

void CAgentController::DebugRender()
{
	/*m_pBatch->Begin();

	m_pBatch->End();*/
}

CAgentController* CAgentController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAgentController* pInstance = new CAgentController(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAgentController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CAgentController::Clone(CGameObject* pGameObject, void* pArg)
{
	CAgentController* pInstance = new CAgentController(*this);
	pInstance->m_pGameObject = pGameObject;

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CAgentController");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAgentController::Free()
{
	// DebugDraw
	Safe_Delete(m_pBatch);
	Safe_Delete(m_pEffect);
	Safe_Release(m_pInputLayout);

	Super::Free();
}
