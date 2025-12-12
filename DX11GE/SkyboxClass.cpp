#include "SkyboxClass.h"
#include <d3dcompiler.h>
#include <vector>

#include "DDSTextureLoader.h" 

static void MakeCubeInside(std::vector<XMFLOAT3>& pos, std::vector<UINT>& idx)
{
    // 단위 큐브(내부에서 보이도록 FRONT를 컬링할 예정 -> 역시계 정점 배열 유지)
    static const XMFLOAT3 P[] = {
        {-1,-1,-1}, { -1, 1,-1}, { 1, 1,-1}, { 1,-1,-1}, // back  (z-)
        {-1,-1, 1}, { -1, 1, 1}, { 1, 1, 1}, { 1,-1, 1}, // front (z+)
    };
    pos.assign(P, P + 8);
    // 12 삼각형(36 인덱스)
    static const UINT I[] = {
        // back z-  (0,1,2,3)
        0,1,2, 0,2,3,
        // front z+ (4,5,6,7)
        4,6,5, 4,7,6,
        // left x-  (0,4,5,1)
        0,4,5, 0,5,1,
        // right x+ (3,2,6,7)
        3,2,6, 3,6,7,
        // top y+   (1,5,6,2)
        1,5,6, 1,6,2,
        // bottom y-(0,3,7,4)
        0,3,7, 0,7,4,
    };
    idx.assign(I, I + 36);
}

bool SkyboxClass::Initialize(ID3D11Device* dev, HWND hwnd, const wchar_t* cubemapPath,
    const void* vsByteCode, size_t vsByteCodeSize,
    ID3D11VertexShader* vs, ID3D11PixelShader* ps)
{
    m_vs = vs; m_ps = ps;

    // 입력 레이아웃
    D3D11_INPUT_ELEMENT_DESC il[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    HRESULT hr = dev->CreateInputLayout(il, 1, vsByteCode, vsByteCodeSize, &m_layout);
    if (FAILED(hr)) return false;

    // 큐브 VB/IB
    std::vector<XMFLOAT3> pos; std::vector<UINT> idx;
    MakeCubeInside(pos, idx);
    m_indexCount = (UINT)idx.size();

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = (UINT)(pos.size() * sizeof(XMFLOAT3));
    D3D11_SUBRESOURCE_DATA srd = { pos.data(), 0, 0 };
    hr = dev->CreateBuffer(&bd, &srd, &m_vb);
    if (FAILED(hr)) return false;

    bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = (UINT)(idx.size() * sizeof(UINT));
    srd = { idx.data(), 0, 0 };
    hr = dev->CreateBuffer(&bd, &srd, &m_ib);
    if (FAILED(hr)) return false;

    // 상수버퍼
    bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.ByteWidth = sizeof(CB);
    hr = dev->CreateBuffer(&bd, nullptr, &m_cb);
    if (FAILED(hr)) return false;

    // 큐브맵 SRV
    hr = CreateDDSTextureFromFile(dev, cubemapPath, nullptr, &m_srv);
    if (FAILED(hr)) return false;

    // 샘플러 (Clamp)
    D3D11_SAMPLER_DESC samp = {};
    samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samp.AddressU = samp.AddressV = samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samp.MaxLOD = D3D11_FLOAT32_MAX;
    hr = dev->CreateSamplerState(&samp, &m_samp);
    if (FAILED(hr)) return false;

    // 깊이 스텐실: DepthEnable = TRUE, WriteDisable, Func = LESS_EQUAL
    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = TRUE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = dev->CreateDepthStencilState(&dsd, &m_dssLessEq);
    if (FAILED(hr)) return false;

    // 래스터라이저: FRONT 컬링(큐브 내부에서 그리므로)
    D3D11_RASTERIZER_DESC rsd = {};
    rsd.FillMode = D3D11_FILL_SOLID;
    rsd.CullMode = D3D11_CULL_FRONT;
    rsd.DepthClipEnable = TRUE;
    hr = dev->CreateRasterizerState(&rsd, &m_rsCullFront);
    if (FAILED(hr)) return false;

    return true;
}

void SkyboxClass::Shutdown()
{
    if (m_rsCullFront) { m_rsCullFront->Release();  m_rsCullFront = nullptr; }
    if (m_dssLessEq) { m_dssLessEq->Release();    m_dssLessEq = nullptr; }
    if (m_samp) { m_samp->Release();         m_samp = nullptr; }
    if (m_srv) { m_srv->Release();          m_srv = nullptr; }
    if (m_cb) { m_cb->Release();           m_cb = nullptr; }
    if (m_layout) { m_layout->Release();       m_layout = nullptr; }
    if (m_ib) { m_ib->Release();           m_ib = nullptr; }
    if (m_vb) { m_vb->Release();           m_vb = nullptr; }
}

void SkyboxClass::Render(ID3D11DeviceContext* ctx, const XMMATRIX& view, const XMMATRIX& proj)
{
    // 뷰의 평행이동 제거(카메라 위치 무시)
    XMMATRIX v = view;
    v.r[3] = XMVectorSet(0, 0, 0, 1); // _41,_42,_43 = 0

    CB cb;
    cb.viewProj = XMMatrixTranspose(v * proj); // row-major 상수버퍼

    D3D11_MAPPED_SUBRESOURCE mapped;
    ctx->Map(m_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &cb, sizeof(CB));
    ctx->Unmap(m_cb, 0);

    // 파이프라인 바인드
    UINT stride = sizeof(XMFLOAT3), offset = 0;
    ctx->IASetVertexBuffers(0, 1, &m_vb, &stride, &offset);
    ctx->IASetIndexBuffer(m_ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetInputLayout(m_layout);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11DepthStencilState* prevDSS = nullptr;
    UINT prevStencilRef = 0;
    ID3D11RasterizerState* prevRS = nullptr;
    ctx->OMGetDepthStencilState(&prevDSS, &prevStencilRef);
    ctx->RSGetState(&prevRS);

    ctx->OMSetDepthStencilState(m_dssLessEq, 0);
    ctx->RSSetState(m_rsCullFront);

    ctx->VSSetShader(m_vs, nullptr, 0);
    ctx->VSSetConstantBuffers(0, 1, &m_cb);
    ctx->PSSetShader(m_ps, nullptr, 0);
    ctx->PSSetShaderResources(0, 1, &m_srv);
    ctx->PSSetSamplers(0, 1, &m_samp);

    ctx->DrawIndexed(m_indexCount, 0, 0);

    // 상태 원복
    ctx->OMSetDepthStencilState(prevDSS, prevStencilRef);
    ctx->RSSetState(prevRS);
    if (prevDSS) prevDSS->Release();
    if (prevRS)  prevRS->Release();
}
