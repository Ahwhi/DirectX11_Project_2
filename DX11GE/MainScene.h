#pragma once
#include "GameNode.h"
#include "DeviceManager.h"
#include "RMModel.h"
#include "ModelLoader.h"
#include "lightclass.h"
#include "lightshaderclass.h"
#include "modelclass.h"
#include "Player.h"
#include "SkyboxClass.h"
#include "textclass.h"
#include "cpuclass.h"
#include "soundclass.h"

struct PointLight {
	XMFLOAT3 position;
	float    range;
	XMFLOAT4 color;
};

class MainScene : public GameNode
{
public:
	MainScene();
	~MainScene();

private:
	ModelLoader *m_modelLoader;

	ModelClass* m_groundModel;
	ModelClass* m_baseballModel;
	ModelClass* m_TreeModel;
	ModelClass* m_TargetModel;
	ModelClass* m_BatModel;

	SkinModel *m_Model_Pitcher;
	SkinModel *m_Model_Batter;
	SkinModel* m_Model_Penguin;
	SkinModel* m_Model_Rabbit;
	SkinModel* m_Model_Boxman;
	SkinModel* m_Model_Man;

	Player *m_player;

	LightShaderClass* m_LightShader;
	LightClass* m_Light;
	PointLight m_PointLight1;
	PointLight m_PointLight2;
	PointLight m_PointLight3;
	SkyboxClass* m_Skybox;
	ID3D11VertexShader* m_SkyVS = nullptr;
	ID3D11PixelShader* m_SkyPS = nullptr;
	vector<uint8_t> m_SkyVSBytecode; // 입력 레이아웃 생성을 위해 보관

	vector<TextClass*> m_Text;
	CpuClass* m_Cpu;
	SoundClass* m_Sound;

	vector<SkinModel*> m_ActiveSkinModels;
	vector<ModelClass*> m_ActiveModels;

	bool     m_IsPitching = false;   // 피칭 준비(애니 중, 아직 발사 전)
	bool     m_BallFlying = false;   // 공이 날아가는 중인지
	float    m_PitchTimer = 0.0f;    // 피칭 시작 후 경과 시간
	XMFLOAT3 m_BallPos = XMFLOAT3(0, 0, 0); // 공 월드 위치
	XMFLOAT3 m_BallVel = XMFLOAT3(0, 0, 0); // 공 속도

	int		m_collisionCount = 0;

public:
	HRESULT Init(void);
	void Update(void);
	void Render(ID3D11DeviceContext* dc);


};

