#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    if (!InitEffect())
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)//dt是计时器计算的帧数
{
    static float phi = 0.0f, theta = 0.0f;
    phi += 0.3f * dt, theta += 0.37f * dt;
    constantBuffer.g_World = XMMatrixTranspose(/*XMMatrixRotationX(phi)**/ XMMatrixRotationY(theta));
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

    memcpy_s(mappedData.pData, sizeof(constantBuffer), &constantBuffer, sizeof(constantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);



}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// RGBA = (0,0,0,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 绘制三角形
    m_pd3dImmediateContext->DrawIndexed(72, 0, 0);
    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()//获取着色器
{
    ComPtr<ID3DBlob>blob;
    HR(CreateShaderFromFile(L"Shaders\\Triangle_VS.cso", L"Shaders\\Triangle_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));//获取hlsl顶点着色器
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));


    HR(CreateShaderFromFile(L"Shaders\\Triangle_PS.cso", L"Shaders\\Triangle_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));
    return true;
}


bool GameApp::InitResource()
{
    // 4个顶点
    VertexPosColor vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.8165f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(0.0f, 0.8165f, -0.2722f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-0.7071f, -0.4082f,-0.2722f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(0.7071f, -0.4082f, -0.2722f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    };
    // 设置顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof vertices;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));


    DWORD indices[] = {
       0,1,2,//
       0,2,1,
       1,0,2,
       1,2,0,
       2,0,1,
       2,1,0,


       1,2,3,
       1,3,2,
       2,3,1,
       2,1,3,
       3,1,2,
       3,2,1,

       0,3,1,//
       0,1,3,
       1,0,3,
       1,3,0,
       3,0,1,
       3,1,0,

       0,2,3,//
       0,3,2,
       3,2,0,
       3,0,2,
       2,0,3,
       2,3,0,
    };

    D3D11_BUFFER_DESC ind;
    ZeroMemory(&ind, sizeof(ind));
    ind.ByteWidth = sizeof(indices);
    ind.Usage = D3D11_USAGE_IMMUTABLE;
    ind.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ind.CPUAccessFlags = 0;

    InitData.pSysMem = indices;
    HR(m_pd3dDevice->CreateBuffer(&ind, &InitData, m_pIndexBuffer.GetAddressOf()));//创建了索引缓冲区
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr,m_pConstantBuffer.GetAddressOf()));

    constantBuffer.g_World = XMMatrixIdentity();
    constantBuffer.g_View = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)

    ));
    constantBuffer.g_Proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, AspectRatio(), 1.0f, 1000.0f));



    // ******************
    // 给渲染管线各个阶段绑定好所需资源
    //

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(VertexPosColor);	// 跨越字节数
    UINT offset = 0;						// 起始偏移量

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    // 设置图元类型，设定输入布局
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->VSSetConstantBuffers(0,1,m_pConstantBuffer.GetAddressOf());

    // ******************
    // 设置调试对象名
    //
    //D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    //D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    //D3D11SetDebugObjectName(m_pVertexShader.Get(), "Trangle_VS");
    //D3D11SetDebugObjectName(m_pPixelShader.Get(), "Trangle_PS");
    //D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
    //D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");



    return true;
}
