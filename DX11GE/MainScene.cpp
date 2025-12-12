#include "pch.h"
#include "MainScene.h"

MainScene::MainScene()
{
	m_modelLoader = NULL;
	m_Model_Pitcher = NULL;
	m_Model_Batter = NULL;
	m_Model_Penguin = NULL;
	m_Model_Rabbit = NULL;
	m_Model_Boxman = NULL;
	m_Model_Man = NULL;

	m_groundModel = NULL;
	m_baseballModel = NULL;
	m_TreeModel = NULL;

	m_player = NULL;

	m_LightShader = NULL;
	m_Light = NULL;
	m_Skybox = NULL;

	m_Text.clear();
	m_ActiveModels.clear();
	m_ActiveSkinModels.clear();

	m_Cpu = 0;
	m_Sound = 0;
}

MainScene::~MainScene()
{
	if (m_modelLoader)		delete m_modelLoader;
	if (m_Model_Pitcher)	delete m_Model_Pitcher;
	if (m_Model_Batter)		delete m_Model_Batter;
	if (m_Model_Penguin)	delete m_Model_Penguin;
	if (m_Model_Rabbit)		delete m_Model_Rabbit;
	if (m_Model_Boxman)		delete m_Model_Boxman;
	if (m_Model_Man)		delete m_Model_Man;

	if (m_player)			delete m_player;

	if (m_Light)			delete m_Light;
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	if (m_groundModel)
	{
		delete m_groundModel;
		m_groundModel = 0;
	}

	if (m_baseballModel)
	{
		delete m_baseballModel;
		m_baseballModel = 0;
	}

	// Release the cpu object.
	if (m_Cpu)
	{
		m_Cpu->Shutdown();
		delete m_Cpu;
		m_Cpu = 0;
	}

	m_Text.clear();
	m_ActiveModels.clear();
	m_ActiveSkinModels.clear();


}

HRESULT MainScene::Init(void)
{
	ID3D11Device* device = DEVICEMANAGER.GetDevice();

	m_Cpu = new CpuClass;
	if (!m_Cpu)
	{
		return false;
	}

	// Initialize the cpu object.
	m_Cpu->Initialize();

	// Create the sound object.
	m_Sound = new SoundClass;
	if (!m_Sound)
	{
		return false;
	}

	// Initialize the sound object.
	m_Sound->Initialize(_hWnd);

	// === Directional Light ===
	{
		m_LightShader = new LightShaderClass;
		m_LightShader->Initialize(device, _hWnd);

		m_Light = new LightClass;
		const float LightIntensity = 0.5f;
		m_Light->SetAmbientColor(LightIntensity, LightIntensity, LightIntensity, 1.0f);
		m_Light->SetDiffuseColor(0.5f, 0.5f, 0.5f, 1.0f);
		m_Light->SetDirection(0.0f, -1.0f, 0.0f);
		m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light->SetSpecularPower(32.0f);
	}

	// === Point Lights ===
	{
		// Red
		m_PointLight1.position = XMFLOAT3(30.f, 6.f, -15.f);
		m_PointLight1.range = 10.0f;
		m_PointLight1.color = XMFLOAT4(1.f, 0.f, 0.f, 1.f);

		// Green
		m_PointLight2.position = XMFLOAT3(30.f, 6.f, 0.f);
		m_PointLight2.range = 10.0f;
		m_PointLight2.color = XMFLOAT4(0.f, 1.f, 0.f, 1.f);

		// Blue
		m_PointLight3.position = XMFLOAT3(30.f, 6.f, 15.f);
		m_PointLight3.range = 10.0f;
		m_PointLight3.color = XMFLOAT4(0.f, 0.f, 1.f, 1.f);
	}
	
	m_modelLoader = new ModelLoader;

	//model - not animated
	{
		// 1. 땅
		m_groundModel = new ModelClass;
		m_groundModel->Initialize(device, L"./asset/Floor.obj", L"./asset/Floor_d.dds");
		m_groundModel->SetNormalTexture(device, L"./asset/Floor_n.dds");
		m_ActiveModels.push_back(m_groundModel);

		// 2. 야구공
		m_baseballModel = new ModelClass;
		if (!m_baseballModel->Initialize(device, L"./asset/Ball.obj", L"./asset/Ball_d.dds")) {
			MessageBox(_hWnd, L"Ball.obj 로드 실패", L"Error", MB_OK); 
			return E_FAIL;
		}
		m_baseballModel->SetNormalTexture(device, L"./asset/Ball_n.dds");
		m_ActiveModels.push_back(m_baseballModel);

		// 3. 나무
		m_TreeModel = new ModelClass;
		if (!m_TreeModel->Initialize(device, L"./asset/Tree.obj", L"./asset/Tree.dds")) {
			MessageBox(_hWnd, L"Tree.obj 로드 실패", L"Error", MB_OK);
			return E_FAIL;
		}
		m_ActiveModels.push_back(m_TreeModel);

		// 4. 과녁
		m_TargetModel = new ModelClass;
		if (!m_TargetModel->Initialize(device, L"./asset/Target.obj", L"./asset/Target.dds")) {
			MessageBox(_hWnd, L"Target.obj 로드 실패", L"Error", MB_OK);
			return E_FAIL;
		}
		m_ActiveModels.push_back(m_TargetModel);

		// 5. 방망이
		m_BatModel = new ModelClass;
		if (!m_BatModel->Initialize(device, L"./asset/Bat.obj", L"./asset/Bat.dds")) {
			MessageBox(_hWnd, L"Bat.obj 로드 실패", L"Error", MB_OK);
			return E_FAIL;
		}
		m_ActiveModels.push_back(m_BatModel);
	}
	
	//model - animated
	{
		// 1. 투수
		m_Model_Pitcher = m_modelLoader->LoadModel(L"./asset/mane.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		if (!m_Model_Pitcher) { MessageBox(_hWnd, L"mane.fbx 로드 실패", L"Error", MB_OK); return E_FAIL; }
		m_modelLoader->LoadAnimation(L"./asset/mane_idle.fbx", m_Model_Pitcher, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		m_modelLoader->LoadAnimation(L"./asset/mane_pitching.fbx", m_Model_Pitcher, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		m_modelLoader->LoadAnimation(L"./asset/mane_jump.fbx", m_Model_Pitcher, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		m_Model_Pitcher->LoadTexture(L"./asset/mane_d.dds");
		m_Model_Pitcher->SetNormalize(true);
		m_Model_Pitcher->SetSize(XMFLOAT3(5.f, 5.f, 5.f));
		m_Model_Pitcher->SetPosition(0.f, 0.f, 0.f);
		m_Model_Pitcher->SetRotationDeg(0.f, 90.f, 0.f);
		m_ActiveSkinModels.push_back(m_Model_Pitcher);
		

		// 2. 타자
		m_Model_Batter = m_modelLoader->LoadModel(L"./asset/blackman.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		if (!m_Model_Batter) { MessageBox(_hWnd, L"blackman.fbx 로드 실패", L"Error", MB_OK); return E_FAIL; }
		m_modelLoader->LoadAnimation(L"./asset/blackman_idle.fbx", m_Model_Batter, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		m_Model_Batter->LoadTexture(L"./asset/blackman_d.dds");
		m_Model_Batter->SetNormalize(true);
		m_Model_Batter->SetSize(XMFLOAT3(5.f, 5.f, 5.f));
		m_Model_Batter->SetPosition(30.f, 0.f, -3.f);
		m_Model_Batter->SetRotationDeg(0.f, 90.f, 0.f);
		m_ActiveSkinModels.push_back(m_Model_Batter);

		// 3. 펭귄
		m_Model_Penguin = m_modelLoader->LoadModel(L"./asset/Penguin.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		if (!m_Model_Penguin) { MessageBox(_hWnd, L"Penguin.fbx 로드 실패", L"Error", MB_OK); return E_FAIL; }
		m_Model_Penguin->LoadTexture(L"./asset/Penguin.dds");
		m_Model_Penguin->SetNormalize(true);
		m_Model_Penguin->SetSize(XMFLOAT3(3.f, 3.f, 3.f));
		m_Model_Penguin->SetPosition(30.f, 0.f, -10.f);
		m_Model_Penguin->SetRotationDeg(0.f, 180.f, 0.f);
		m_ActiveSkinModels.push_back(m_Model_Penguin);

		// 4. 토끼
		m_Model_Rabbit = m_modelLoader->LoadModel(L"./asset/Rabbit.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		if (!m_Model_Rabbit) { MessageBox(_hWnd, L"Rabbit.fbx 로드 실패", L"Error", MB_OK); return E_FAIL; }
		m_Model_Rabbit->LoadTexture(L"./asset/Rabbit.dds");
		m_Model_Rabbit->SetNormalize(true);
		m_Model_Rabbit->SetSize(XMFLOAT3(3.f, 3.f, 3.f));
		m_Model_Rabbit->SetPosition(35.f, 0.f, -14.f);
		m_Model_Rabbit->SetRotationDeg(0.f, 180.f, 0.f);
		m_ActiveSkinModels.push_back(m_Model_Rabbit);

		// 5. 박스맨
		m_Model_Boxman = m_modelLoader->LoadModel(L"./asset/Boxman.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		if (!m_Model_Boxman) { MessageBox(_hWnd, L"Boxman.fbx 로드 실패", L"Error", MB_OK); return E_FAIL; }
		m_Model_Boxman->LoadTexture(L"./asset/Boxman.dds");
		m_Model_Boxman->SetNormalize(true);
		m_Model_Boxman->SetSize(XMFLOAT3(3.f, 3.f, 3.f));
		m_Model_Boxman->SetPosition(40.f, 0.f, 20.f);
		m_Model_Boxman->SetRotationDeg(0.f, 0.f, 0.f);
		m_ActiveSkinModels.push_back(m_Model_Boxman);

		// 6. 남자
		m_Model_Man = m_modelLoader->LoadModel(L"./asset/Man.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
		if (!m_Model_Man) { MessageBox(_hWnd, L"Man.fbx 로드 실패", L"Error", MB_OK); return E_FAIL; }
		m_Model_Man->LoadTexture(L"./asset/Man.dds");
		m_Model_Man->SetNormalize(true);
		m_Model_Man->SetSize(XMFLOAT3(8.f, 8.f, 8.f));
		m_Model_Man->SetPosition(50.f, 0.f, 0.f);
		m_Model_Man->SetRotationDeg(0.f, 90.f, 0.f);
		m_ActiveSkinModels.push_back(m_Model_Man);

		m_player = new Player;
	}

	// Skybox 셰이더 컴파일 ===
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* err = nullptr;

	HRESULT hr = D3DCompileFromFile(L"./data/SkyboxShader.hlsl", nullptr, nullptr,
		"VSMain", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsBlob, &err);
	if (FAILED(hr)) { if (err) { MessageBoxA(_hWnd, (char*)err->GetBufferPointer(), "VS Compile", MB_OK); err->Release(); } return false; }

	hr = D3DCompileFromFile(L"./data/SkyboxShader.hlsl", nullptr, nullptr,
		"PSMain", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &psBlob, &err);
	if (FAILED(hr)) { if (err) { MessageBoxA(_hWnd, (char*)err->GetBufferPointer(), "PS Compile", MB_OK); err->Release(); } vsBlob->Release(); return false; }

	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_SkyVS);
	if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }
	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_SkyPS);
	if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

	m_SkyVSBytecode.assign((uint8_t*)vsBlob->GetBufferPointer(), (uint8_t*)vsBlob->GetBufferPointer() + vsBlob->GetBufferSize());
	vsBlob->Release();
	psBlob->Release();

	m_Skybox = new SkyboxClass();
	if (!m_Skybox) return false;

	if (!m_Skybox->Initialize(device, _hWnd, L"./asset/skybox.dds",
		m_SkyVSBytecode.data(), m_SkyVSBytecode.size(), m_SkyVS, m_SkyPS))
	{
		MessageBox(_hWnd, L"Could not initialize the skybox.", L"Error", MB_OK);
		return false;
	}

	_mainCam.SetPosition(-10.0f, 0.0f, -10.0f);
	float startYaw = XMConvertToRadians(60.0f);
	_mainCam.SetRotation(0.0f, startYaw, 0.0f);

	// 마우스 시스템에 초기 yaw/pitch도 맞춰주기
	INPUTMANAGER.SetYaw(startYaw);
	INPUTMANAGER.SetPitch(0.0f);
	_sceneInfo->viewMatrix = _mainCam.Update(TIMEMANAGER.GetElapedTime());
	// Create the text object.

	for (int i = 0; i < 6; i++) {
		TextClass* text = new TextClass();
		bool result = text->Initialize(device, DEVICEMANAGER.GetDeviceContext(), _hWnd, WINSIZEX, WINSIZEY, XMMatrixIdentity());
		if (!result)
		{
			MessageBox(_hWnd, L"Could not initialize the text object.", L"Error", MB_OK);
			return false;
		}
		m_Text.push_back(text);
	}

	return S_OK;
}

void MainScene::Update(void)
{
	float dt = TIMEMANAGER.GetElapedTime();
	
	m_player->Update(dt);
	m_Cpu->Frame();

	// 1) SHIFT 눌리면 피칭 시작 (한 번만)
	if (!m_IsPitching && !m_BallFlying && INPUTMANAGER.GetShift() == 1.0f) {
		m_IsPitching = true;
		m_PitchTimer = 0.0f;

		// 피칭 애니 시작 (이미 Render에서도 PlayAni(2) 호출하지만,
		// 안전하게 여기서도 한 번 걸어줘도 됨)
		m_Model_Pitcher->PlayAni(2);
	}

	// 2) 피칭 중이면 타이머 증가
	if (m_IsPitching && !m_BallFlying) {
		m_PitchTimer += dt;

		// 1.5초 지나면 공 발사
		if (m_PitchTimer >= 1.99f) {
			m_IsPitching = false;
			m_BallFlying = true;

			// 현재 손 위치에서 공 시작
			XMMATRIX handWorld = m_Model_Pitcher->GetNodeWorldTM(L"mixamorig1:RightHand");
			XMFLOAT4X4 handMat;
			XMStoreFloat4x4(&handMat, handWorld);
			m_BallPos = XMFLOAT3(handMat._41, handMat._42, handMat._43);

			// x축 방향 속도 (값은 적당히 조정)
			m_BallVel = XMFLOAT3(40.0f, 0.0f, 0.0f);
		}
	}

	// 3) 공이 날아가는 중이면 위치 업데이트
	if (m_BallFlying) {
		m_BallPos.x += m_BallVel.x * dt;
		m_BallPos.y += m_BallVel.y * dt;
		m_BallPos.z += m_BallVel.z * dt;

		{
			XMFLOAT3 manCenter = XMFLOAT3(50.f, 0.f, 0.f);

			const float ballRadius = 0.5f;
			const float manRadius = 5.0f;

			float dx = m_BallPos.x - manCenter.x;
			float dy = m_BallPos.y - manCenter.y;
			float dz = m_BallPos.z - manCenter.z;

			float distSq = dx * dx + dy * dy + dz * dz;
			float sumR = ballRadius + manRadius;

			if (distSq <= sumR * sumR) {
				m_collisionCount++;
				m_BallFlying = false;
			}
		}

		// 너무 멀리 가면 리셋 (원하면 값 조정 or 삭제)
		if (m_BallPos.x > 100.0f) {
			m_BallFlying = false;
		}
	}

	_sceneInfo->viewMatrix = _mainCam.Update(dt);
}

void MainScene::Render(ID3D11DeviceContext* dc)
{
	bool result = true;

	XMFLOAT2 uvTilingBall(1.0f, 1.0f);
	XMFLOAT2 uvTilingChar(1.f, 1.f);
	XMFLOAT2 uvTilingGround(50.f, 50.f);

	PointLightParam skinnedPointLights[NUM_POINT_LIGHTS];

	skinnedPointLights[0].position = m_PointLight1.position;
	skinnedPointLights[0].range = m_PointLight1.range;
	skinnedPointLights[0].color = m_PointLight1.color;

	skinnedPointLights[1].position = m_PointLight2.position;
	skinnedPointLights[1].range = m_PointLight2.range;
	skinnedPointLights[1].color = m_PointLight2.color;

	skinnedPointLights[2].position = m_PointLight3.position;
	skinnedPointLights[2].range = m_PointLight3.range;
	skinnedPointLights[2].color = m_PointLight3.color;


	// ==== [1] 스킨 셰이더용 공통 상수 버퍼 셋팅 ====
	RM_SHADER.SetShaderParameters(
		dc,
		m_Light,
		_mainCam.GetPositionF(),
		skinnedPointLights,
		NUM_POINT_LIGHTS);

	RM_SHADER.SetShaderParameters(
		dc,
		_sceneInfo->viewMatrix,
		_sceneInfo->projectionMatrix);


	XMFLOAT3 mov(m_Model_Pitcher->GetCenter());
	if (!m_Model_Pitcher->GetAnimation(2).isPlaying() && !m_Model_Pitcher->GetAnimation(3).isPlaying()) {
		m_Model_Pitcher->PlayAni(1);
	}

	if (INPUTMANAGER.GetShift() == 1.0f) {
			m_Model_Pitcher->PlayAni(2);
			
	}

	if (INPUTMANAGER.GetControl() == 1.0f) {
		m_Model_Pitcher->PlayAni(3);
	}

	if (m_Model_Pitcher->GetAnimation(2).isPlaying()) {
		m_Model_Pitcher->SetRotationDeg(0.f, -90.f, 0.f);
	}
	else {
		m_Model_Pitcher->SetRotationDeg(0.f, 0.f, 0.f);
	}
	m_Model_Batter->PlayAni(1);



	DEVICEMANAGER.TurnZBufferOn(dc);
	DEVICEMANAGER.TurnOffAlphaBlending(dc);


	// ==== [3] 스킨 캐릭터 렌더 (텍스쳐 직접 바인딩) ====
	{
		ID3D11ShaderResourceView* tex = m_Model_Pitcher->GetTexture();
		dc->PSSetShaderResources(0, 1, &tex);

		m_Model_Pitcher->Render(dc);
	}

	{
		ID3D11ShaderResourceView* tex = m_Model_Batter->GetTexture();
		dc->PSSetShaderResources(0, 1, &tex);

		m_Model_Batter->Render(dc);
	}

	{
		ID3D11ShaderResourceView* tex = m_Model_Penguin->GetTexture();
		dc->PSSetShaderResources(0, 1, &tex);

		m_Model_Penguin->Render(dc);
	}

	{
		ID3D11ShaderResourceView* tex = m_Model_Rabbit->GetTexture();
		dc->PSSetShaderResources(0, 1, &tex);

		m_Model_Rabbit->Render(dc);
	}

	{
		ID3D11ShaderResourceView* tex = m_Model_Boxman->GetTexture();
		dc->PSSetShaderResources(0, 1, &tex);

		m_Model_Boxman->Render(dc);
	}

	{
		ID3D11ShaderResourceView* tex = m_Model_Man->GetTexture();
		dc->PSSetShaderResources(0, 1, &tex);

		m_Model_Man->Render(dc);
	}




	// 카메라 이동 로직
	float dt = TIMEMANAGER.GetElapedTime();
	float moveSpeed = 10.0f * dt; // 프레임 독립 속도

	// 현재 카메라 위치
	XMFLOAT3 camPosF = _mainCam.GetPositionF();
	XMVECTOR camPos = XMLoadFloat3(&camPosF);

	// 카메라가 바라보는 방향 (정규화)
	XMVECTOR forward = XMVector3Normalize(_mainCam.GetDirection());

	// 오른쪽 방향 = up(0,1,0) × forward
	XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));

	float v = INPUTMANAGER.GetVertical();   // W/S
	float h = INPUTMANAGER.GetHorizontal(); // A/D

	if (v > 0.0f)      camPos = XMVectorAdd(camPos, XMVectorScale(forward, moveSpeed));
	else if (v < 0.0f) camPos = XMVectorSubtract(camPos, XMVectorScale(forward, moveSpeed));

	if (h > 0.0f)      camPos = XMVectorAdd(camPos, XMVectorScale(right, moveSpeed));
	else if (h < 0.0f) camPos = XMVectorSubtract(camPos, XMVectorScale(right, moveSpeed));

	XMStoreFloat3(&camPosF, camPos);

	camPosF.y = 5.0f;
	_mainCam.SetPosition(camPosF.x, camPosF.y, camPosF.z);


	if (m_Skybox) {
		m_Skybox->Render(dc, _sceneInfo->viewMatrix, _sceneInfo->projectionMatrix);
	}


	m_groundModel->Render(dc);
	XMMATRIX groundWorld =
		XMMatrixScaling(1000.f, 1.f, 1000.f) * XMMatrixTranslation(0.f, 0.f, 0.f);

	LightShaderClass::PointLightData pointLights[NUM_LIGHTS];


	pointLights[0].position = m_PointLight1.position;
	pointLights[0].range = m_PointLight1.range;
	pointLights[0].color = m_PointLight1.color;

	pointLights[1].position = m_PointLight2.position;
	pointLights[1].range = m_PointLight2.range;
	pointLights[1].color = m_PointLight2.color;

	pointLights[2].position = m_PointLight3.position;
	pointLights[2].range = m_PointLight3.range;
	pointLights[2].color = m_PointLight3.color;
	

	result = m_LightShader->Render(dc, m_groundModel->GetIndexCount(),
		groundWorld, _sceneInfo->viewMatrix, _sceneInfo->projectionMatrix,
		m_groundModel->GetTexture(),
		m_groundModel->GetNormalTexture(),
		m_Light->GetDirection(),
		m_Light->GetAmbientColor(),
		m_Light->GetDiffuseColor(),
		_mainCam.GetPositionF(),
		m_Light->GetSpecularColor(),
		m_Light->GetSpecularPower(),
		pointLights, NUM_LIGHTS,
		uvTilingGround);


	{
		XMMATRIX ballWorld;

		if (m_BallFlying) {
			// 이미 던진 상태: 위치 + 스케일만 사용
			ballWorld =
				XMMatrixScaling(0.5f, 0.5f, 0.5f) *
				XMMatrixTranslation(m_BallPos.x, m_BallPos.y, m_BallPos.z);
		}
		else {
			// 아직 던지기 전: 손 뼈에 붙이기
			XMMATRIX pitcherHandWorld = m_Model_Pitcher->GetNodeWorldTM(L"mixamorig1:RightHand");

			XMMATRIX ballLocal =
				XMMatrixScaling(10.5f, 10.5f, 10.5f) *
				XMMatrixRotationRollPitchYaw(0.f, 0.f, 0.f) *
				XMMatrixTranslation(0.f, 13.f, -5.f);

			ballWorld = ballLocal * pitcherHandWorld;
		}

		m_baseballModel->Render(dc);
		result = m_LightShader->Render(dc, m_baseballModel->GetIndexCount(),
			ballWorld, _sceneInfo->viewMatrix, _sceneInfo->projectionMatrix,
			m_baseballModel->GetTexture(),
			m_baseballModel->GetNormalTexture(),
			m_Light->GetDirection(),
			m_Light->GetAmbientColor(),
			m_Light->GetDiffuseColor(),
			_mainCam.GetPositionF(),
			m_Light->GetSpecularColor(),
			m_Light->GetSpecularPower(),
			pointLights, NUM_LIGHTS,
			uvTilingBall);
	}


	m_TreeModel->Render(dc);
	XMMATRIX treeWorld = XMMatrixScaling(5.f, 5.f, 5.f) *
		XMMatrixTranslation(120.f, 0.f, 30.f);

	result = m_LightShader->Render(dc, m_TreeModel->GetIndexCount(),
		treeWorld, _sceneInfo->viewMatrix, _sceneInfo->projectionMatrix,
		m_TreeModel->GetTexture(),
		m_TreeModel->GetNormalTexture(),
		m_Light->GetDirection(),
		m_Light->GetAmbientColor(),
		m_Light->GetDiffuseColor(),
		_mainCam.GetPositionF(),
		m_Light->GetSpecularColor(),
		m_Light->GetSpecularPower(),
		pointLights, NUM_LIGHTS,
		uvTilingChar);

	m_TargetModel->Render(dc);
	XMMATRIX targetWorld = XMMatrixScaling(5.f, 5.f, 5.f) *
		XMMatrixTranslation(-30.f, 0.f, 30.f) * XMMatrixRotationY(140.f);

	result = m_LightShader->Render(dc, m_TargetModel->GetIndexCount(),
		targetWorld, _sceneInfo->viewMatrix, _sceneInfo->projectionMatrix,
		m_TargetModel->GetTexture(),
		m_TargetModel->GetNormalTexture(),
		m_Light->GetDirection(),
		m_Light->GetAmbientColor(),
		m_Light->GetDiffuseColor(),
		_mainCam.GetPositionF(),
		m_Light->GetSpecularColor(),
		m_Light->GetSpecularPower(),
		pointLights, NUM_LIGHTS,
		uvTilingChar);

	{
		XMMATRIX batterHandWorld = m_Model_Batter->GetNodeWorldTM(L"mixamorig10:RightHand");

		XMMATRIX batLocal =
			XMMatrixScaling(1.5f, 1.5f, 1.5f) *
			XMMatrixRotationRollPitchYaw(90.f, XMConvertToRadians(90.f), 0.f) *
			XMMatrixTranslation(-15.f, 15.f, -10.f);

		XMMATRIX batWorld = batLocal * batterHandWorld;

		m_BatModel->Render(dc);
		result = m_LightShader->Render(dc, m_BatModel->GetIndexCount(),
			batWorld, _sceneInfo->viewMatrix, _sceneInfo->projectionMatrix,
			m_BatModel->GetTexture(),
			m_BatModel->GetNormalTexture(),
			m_Light->GetDirection(),
			m_Light->GetAmbientColor(),
			m_Light->GetDiffuseColor(),
			_mainCam.GetPositionF(),
			m_Light->GetSpecularColor(),
			m_Light->GetSpecularPower(),
			pointLights, NUM_LIGHTS,
			uvTilingChar);
	}



	
	// 2D 렌더링 - Z버퍼 OFF, 알파 블렌딩 ON
	DEVICEMANAGER.TurnZBufferOff(dc);
	DEVICEMANAGER.TurnOnAlphaBlending(dc);
	
	XMMATRIX identityMatrix = XMMatrixIdentity();
	XMMATRIX orthoMatrix = _sceneInfo->orthoMatrix;
	result = m_Text[0]->SetFPS(TIMEMANAGER.GetFrameRate(), dc);
	result = m_Text[1]->SetCPU(m_Cpu->GetCpuPercentage(), dc);
	result = m_Text[2]->SetNumOfObjects(m_ActiveModels.size() + m_ActiveSkinModels.size(), dc);
	int totalFaceCount = 0;
	for (int i = 0; i < m_ActiveModels.size(); i++) {
		totalFaceCount += m_ActiveModels[i]->m_faceCount;
	}
	for (int i = 0; i < m_ActiveSkinModels.size(); i++) {
		totalFaceCount += m_ActiveSkinModels[i]->GetFaceCount();
	}
	result = m_Text[3]->SetNumOfPolygons(totalFaceCount, dc);
	result = m_Text[4]->SetScreenResolution(dc);
	result = m_Text[5]->SetCollisionCount(m_collisionCount, dc);
	
	// 텍스트 렌더
	result = m_Text[0]->Render(dc, identityMatrix, orthoMatrix);
	if (!result) return;
	
	result = m_Text[1]->Render(dc, identityMatrix, orthoMatrix);
	if (!result) return;
	
	result = m_Text[2]->Render(dc, identityMatrix, orthoMatrix);
	if (!result) return;
	
	result = m_Text[3]->Render(dc, identityMatrix, orthoMatrix);
	if (!result) return;
	
	result = m_Text[4]->Render(dc, identityMatrix, orthoMatrix);
	if (!result) return;

	result = m_Text[5]->Render(dc, identityMatrix, orthoMatrix);
	if (!result) return;
	
	DEVICEMANAGER.TurnOffAlphaBlending(dc);
	DEVICEMANAGER.TurnZBufferOn(dc);

}