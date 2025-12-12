//=============================================================
//	## RMShader ## (로드된 셰이더 정보를 관리한다.)
//=============================================================
#pragma once
#include "ResourceManager.h"
#include "Shader.h"
#include "lightclass.h"

static const int NUM_POINT_LIGHTS = 3;

struct CameraBuffer
{
	XMMATRIX matView;
	XMMATRIX matProj;
};

struct TransBuffer
{
	XMMATRIX matWorld;
};

struct ColorBuffer
{
	XMFLOAT4 fColor;
};

struct BoneBuffer
{
	XMMATRIX matBones[128];
};

struct CameraPosBuffer
{
	XMFLOAT3 cameraPosition;
	float    pad1;
};

struct PointLightParam
{
	XMFLOAT3 position;
	float    range;
	XMFLOAT4 color;
};

struct LightBuffer
{
	XMFLOAT4 ambientColor;
	XMFLOAT4 dirDiffuseColor;
	XMFLOAT3 lightDirection;
	float    specularPower;
	XMFLOAT4 specularColor;

	PointLightParam pointLights[NUM_POINT_LIGHTS];
};

class RMShader : public ResourceManager<Shader*, RMShader>
{
	
public:
	RMShader() {};
	~RMShader() {};

	HRESULT Init(ID3D11Device* device);

	bool SetShaderParameters(ID3D11DeviceContext * dc, vector<XMMATRIX>& matBoneList);
	bool SetShaderParameters(ID3D11DeviceContext * dc, XMMATRIX matWorld);
	bool SetShaderParameters(ID3D11DeviceContext * dc, XMMATRIX matView, XMMATRIX matProj);
	bool SetShaderParameters(ID3D11DeviceContext * dc, XMFLOAT4 color);
	bool SetShaderParameters(
		ID3D11DeviceContext* dc,
		LightClass* light,
		const XMFLOAT3& cameraPos,
		const PointLightParam* pointLights,
		int numPointLights);


protected:
	Shader* loadResource(wstring fileName, void* param=nullptr);
	void releaseResource(Shader* data);
private:
	ID3D11Buffer*			_transBuffer	= nullptr;
	ID3D11Buffer*			_cameraBuffer	= nullptr;
	ID3D11Buffer*			_colorBuffer	= nullptr;
	ID3D11Buffer*			_boneBuffer		= nullptr;
	ID3D11Buffer* _cameraPosBuffer = nullptr;
	ID3D11Buffer* _lightBuffer = nullptr;
	
};
