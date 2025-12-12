////////////////////////////////////////////////////////////////////////////////
// Filename: lightshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

/////////////
// GLOBALS //
/////////////
const int NUM_LIGHTS = 3;

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>

#include <fstream>

using namespace std;
using namespace DirectX;


////////////////////////////////////////////////////////////////////////////////
// Class name: LightShaderClass
////////////////////////////////////////////////////////////////////////////////
class LightShaderClass
{
public:
	struct PointLightData
	{
		XMFLOAT3 position;
		float    range;
		XMFLOAT4 color;
	};

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

	

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 dirDiffuseColor;
		XMFLOAT3 lightDirection;
		float    specularPower;

		XMFLOAT4 specularColor;

		// ★ 포인트라이트 배열
		PointLightData pointLights[NUM_LIGHTS];

		XMFLOAT2 uvTiling;
		XMFLOAT2 padding;    // 16바이트 정렬용
	};

public:
	LightShaderClass();
	LightShaderClass(const LightShaderClass&);
	~LightShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
		XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView* texture,
		ID3D11ShaderResourceView* normalTexture,
		XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 dirDiffuseColor,
		XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower,
		const PointLightData* pointLights, int numPointLights,
		XMFLOAT2 uvTiling);

	void SetFilter(int num);

	void SetPhong(bool flag) { isPhong = flag; }
	bool GetPhong() { return isPhong; }

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
		XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView* texture,
		ID3D11ShaderResourceView* normalTexture,
		XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 dirDiffuseColor,
		XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower,
		const PointLightData* pointLights, int numPointLights,
		XMFLOAT2 uvTiling);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11PixelShader* m_toonShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_cameraBuffer;

	ID3D11Device* m_device;
	bool isPhong;
};

#endif