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
#include "bitmapclass.h"
#include "textureshaderclass.h"

class TitleScene : public GameNode
{
public:
	TitleScene();
	~TitleScene();

private:
	BitmapClass* m_Bitmap;
	TextureShaderClass* m_TextureShader;
public:
	HRESULT Init(void);
	void Update(void);
	void Render(ID3D11DeviceContext* dc);

};

