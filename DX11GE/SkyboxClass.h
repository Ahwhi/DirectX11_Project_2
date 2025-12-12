#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

class SkyboxClass {
public:
    bool Initialize(ID3D11Device* dev, HWND hwnd, const wchar_t* cubemapPath,
        const void* vsByteCode, size_t vsByteCodeSize,
        ID3D11VertexShader* vs, ID3D11PixelShader* ps);
    void Shutdown();
    void Render(ID3D11DeviceContext* ctx, const XMMATRIX& view, const XMMATRIX& proj);

private:
    struct CB { XMMATRIX viewProj; };

    ID3D11Buffer* m_vb = nullptr;
    ID3D11Buffer* m_ib = nullptr;
    UINT                   m_indexCount = 0;
    ID3D11InputLayout* m_layout = nullptr;
    ID3D11Buffer* m_cb = nullptr;
    ID3D11ShaderResourceView* m_srv = nullptr;
    ID3D11SamplerState* m_samp = nullptr;
    ID3D11DepthStencilState* m_dssLessEq = nullptr;
    ID3D11RasterizerState* m_rsCullFront = nullptr;

    // 셰이더는 외부에서 생성/공유
    ID3D11VertexShader* m_vs = nullptr; // not owned
    ID3D11PixelShader* m_ps = nullptr; // not owned
};
