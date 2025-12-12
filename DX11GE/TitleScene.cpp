#include "pch.h"
#include "TitleScene.h"

TitleScene::TitleScene()
{
	m_Bitmap = 0;
	m_TextureShader = 0;
}

TitleScene::~TitleScene()
{
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}
}

HRESULT TitleScene::Init(void)
{
	ID3D11Device* device = DEVICEMANAGER.GetDevice();

	m_Bitmap = new BitmapClass;
	if (!m_Bitmap)
	{
		return false;
	}

	// Initialize the bitmap object.
	bool result = m_Bitmap->Initialize(device, WINSIZEX, WINSIZEY, L"./data/Title.dds", 1600, 900);
	if (!result)
	{
		MessageBox(_hWnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}

	// Create the texture shader object.
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_TextureShader->Initialize(device, _hWnd);
	if (!result)
	{
		MessageBox(_hWnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	_sceneInfo->viewMatrix = _mainCam.Update(TIMEMANAGER.GetElapedTime());
	_mainCam.SetPosition(0.0f, 0.0f, 0.0f);

	return S_OK;
}

void TitleScene::Update(void)
{
	if (INPUTMANAGER.GetNum1() == 1.0f) {
		SCENEMANAGER.ChangeScene("Main");
	}
}

void TitleScene::Render(ID3D11DeviceContext* dc)
{
	// ºñÆ®¸Ê ·»´õ
	DEVICEMANAGER.TurnZBufferOff(dc);
	DEVICEMANAGER.TurnOnAlphaBlending(dc);
	_sceneInfo->worldMatrix *= XMMatrixTranslation(0, 0, 000); // ºñÆ®¸Ê »ç¶óÁü ¹æÁö
	bool result = m_Bitmap->Render(dc, 0, 0);
	if (!result) return;

	// ¹è°æ ÀÌ¹ÌÁö °íÁ¤
	XMMATRIX identityMatrix = XMMatrixIdentity();
	result = m_TextureShader->Render(dc, m_Bitmap->GetIndexCount(),
		_sceneInfo->worldMatrix, identityMatrix, _sceneInfo->orthoMatrix, m_Bitmap->GetTexture());
	if (!result) return;

	DEVICEMANAGER.TurnOffAlphaBlending(dc);
	DEVICEMANAGER.TurnZBufferOn(dc);
}