#pragma once
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;
#include "textureclass.h"
#include "AlignedAllocationPolicy.h"
#include <vector>
using namespace std;
#include <d3dcompiler.h>
#include "DDSTextureLoader.h"


class Skybox : public AlignedAllocationPolicy<16>
{
private:
	struct cbPerObject
	{
		XMMATRIX  WVP;
		XMMATRIX World;
	};
	cbPerObject cbPerObj;

	struct Vertex	//Overloaded Vertex Structure
	{
		Vertex() {}
		Vertex(float x, float y, float z,
			float u, float v,
			float nx, float ny, float nz)
			: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) {}

		XMFLOAT3 pos;
		XMFLOAT2 texCoord;
		XMFLOAT3 normal;
	};
	ID3D11Buffer* cbPerObjectBuffer;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;

	ID3D11Buffer* sphereIndexBuffer;
	ID3D11Buffer* sphereVertBuffer;
	ID3D11VertexShader* SKYMAP_VS;
	ID3D11PixelShader* SKYMAP_PS;
	ID3D10Blob* SKYMAP_VS_Buffer;
	ID3D10Blob* SKYMAP_PS_Buffer;
	ID3D11ShaderResourceView* smrv;
	ID3D11DepthStencilState* DSLessEqual;
	ID3D11RasterizerState* RSCullNone;
	int NumSphereVertices;
	int NumSphereFaces;
	XMMATRIX sphereWorld;

	void CreateSphere(ID3D11Device* device, int LatLines, int LongLines);
	void SetShader(ID3D11Device* device);
	void LoadCubeMap(ID3D11Device* device);
public:
	bool initialize(ID3D11DeviceContext* d3d11DevCon, ID3D11Device* device, HWND hwnd);

	void SetRenderState(ID3D11Device* device);
	void Render(ID3D11DeviceContext* d3d11DevCon, XMMATRIX , XMMATRIX , XMVECTOR );
	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
private:
	ID3D10Blob* VS_Buffer;
	ID3D10Blob* PS_Buffer;
	ID3D11SamplerState* CubesTexSamplerState;

	ID3D11ShaderResourceView* d2dTexture;
};