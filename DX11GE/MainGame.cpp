#include "pch.h"
#include "MainGame.h"
#include "MainScene.h"
#include "TitleScene.h"

PWindowRender GameNode::_sceneInfo = NULL;

HRESULT MainGame::Init(ID3D11DeviceContext* dc)
{
	_sceneInfo = DEVICEMANAGER.InitRenderScreen(_hWnd, dc, WINSIZEX, WINSIZEY, 0.1f, 1000.f);
	m_deviceContext = dc;
	InitScene();

	return S_OK;
}

HRESULT MainGame::InitScene()
{
	SCENEMANAGER.AddScene("Title", new TitleScene);
	SCENEMANAGER.AddScene("Main", new MainScene);
	SCENEMANAGER.ChangeScene("Title");

	return S_OK;
}

void MainGame::Update(void)
{
	TIMEMANAGER.Update(0.0f);
	SCENEMANAGER.Update();

	_sceneInfo->viewMatrix = _mainCam.Update(TIMEMANAGER.GetElapedTime());
}

void MainGame::Render(ID3D11DeviceContext* dc)
{	
	//Set Main DC State
	DEVICEMANAGER.SetRasterState(dc);
	DEVICEMANAGER.SetSamplerState(dc);

	DEVICEMANAGER.BeginScene(_sceneInfo, dc, 0, 0, 0, 1);
	
	//투영 및 뷰 행렬 설정
	RM_SHADER.SetShaderParameters(dc, _sceneInfo->viewMatrix, _sceneInfo->projectionMatrix);
	SCENEMANAGER.Render(dc);

	//투영행렬 : 직교행렬로 설정
	RM_SHADER.SetShaderParameters(dc, XMMatrixTranslation(0, 0, 1), _sceneInfo->orthoMatrix);

	//command Excute
	DEVICEMANAGER.ExcuteCommand(dc);

	//SwapChain
	DEVICEMANAGER.EndScene(_sceneInfo);
}

