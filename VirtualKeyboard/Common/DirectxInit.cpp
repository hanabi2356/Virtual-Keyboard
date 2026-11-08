#include "DirectxInit.h"
#include "KeyboardImage.h"
#include<fstream>
#include<sstream>
#include<string>
DirectxInit::DirectxInit(WindowInit* windowInit)
    :m_windowInit(windowInit)
{
}


void DirectxInit::OnResizeThunk(int width, int height, void* user)
{
    static_cast<DirectxInit*>(user)->OnResize(width, height);
}

bool DirectxInit::CreateDevice()
{
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxFactory)))) return false;

    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
    if (FAILED(hr))
    {
        ComPtr<IDXGIAdapter>warp;
        if (FAILED(m_dxFactory->EnumWarpAdapter(IID_PPV_ARGS(&warp)))) return false;

        if (FAILED(D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) return false;

    }
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return SUCCEEDED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
}

bool DirectxInit::CreateCommandObject()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cmdQueue)))) return false;
    if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator)))) return false;
    if(FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_cmdList)))) return false;

    return SUCCEEDED(m_cmdList->Close());
}

bool DirectxInit::CreateSwapChain(HWND hwnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = static_cast<UINT>(width);
    desc.Height = static_cast<UINT>(height);
    desc.Format = m_backBufferFormat;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = m_swapChainCount;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    ComPtr<IDXGISwapChain1>swapChain;
    if (FAILED(m_dxFactory->CreateSwapChainForHwnd(m_cmdQueue.Get(), hwnd, &desc, nullptr, nullptr, &swapChain))) return false;

    if (FAILED(swapChain.As(&m_swapChain))) return false;

    return SUCCEEDED(m_dxFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
}

bool DirectxInit::CreateDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.NumDescriptors = m_swapChainCount;
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    if (FAILED(m_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_rtvHeap)))) return false;

    D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
    dsvDesc.NumDescriptors = 1;
    dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    return SUCCEEDED(m_device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

bool DirectxInit::CreateRenderTargetView()
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < m_swapChainCount; i++)
    {
        if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i])))) return false;
        m_device->CreateRenderTargetView(m_swapChainBuffer[i].Get(), nullptr, rtv);
        rtv.ptr += m_rtvDescriptorSize;
    }
    return true;
}

bool DirectxInit::CreateDepthStencilBuffer(int width, int height)
{
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = m_depthStencilFormat;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clear = {};
    clear.Format = m_depthStencilFormat;
    clear.DepthStencil.Depth = 1.0f;
    clear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
    if (FAILED(m_device->CreateCommittedResource(
        &heap, 
        D3D12_HEAP_FLAG_NONE, 
        &depthDesc, 
        D3D12_RESOURCE_STATE_DEPTH_WRITE, 
        &clear, 
        IID_PPV_ARGS(&m_depthStencilBuffer)))) return false;

    m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr,
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    return true;
}

void DirectxInit::SetViewportAndScissor(int width, int height)
{
    m_viewPort.TopLeftX = 0.0f;
    m_viewPort.TopLeftY = 0.0f;
    m_viewPort.Width = static_cast<float>(width);
    m_viewPort.Height = static_cast<float>(height);
    m_viewPort.MinDepth = 0.0f;
    m_viewPort.MaxDepth = 1.0f;
    m_scissorRect = { 0, 0, width, height };
}

bool DirectxInit::DXInit()
{
    if (!m_windowInit || !m_windowInit->GetHwnd()) return false;
    if (CreateDevice() == false) return false;
    if (CreateCommandObject() == false) return false;

    RECT r = {};
    GetClientRect(m_windowInit->GetHwnd(), &r);
    int width = r.right - r.left;
    int height = r.bottom - r.top;
    if (width <= 0 || height <= 0) return false;

    if (CreateSwapChain(m_windowInit->GetHwnd(), width, height) == false) return false;
    if (CreateDescriptorHeap() == false) return false;

    m_windowInit->SetResizeCallback(&DirectxInit::OnResizeThunk, this);

    return OnResize(width, height);
}

void DirectxInit::FlushCommandQueue()
{
    ++m_currentFence;
    if (FAILED(m_cmdQueue->Signal(m_fence.Get(), m_currentFence))) return;
    
    if (m_fence->GetCompletedValue() < m_currentFence)
    {
        HANDLE fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        if (!fenceEvent) return;

        if (SUCCEEDED(m_fence->SetEventOnCompletion(m_currentFence, fenceEvent)))
        {
            WaitForSingleObject(fenceEvent, INFINITE);
        }
        CloseHandle(fenceEvent);
    }
}

bool DirectxInit::Render()
{
    if (!m_cmdAllocator || !m_cmdList || !m_swapChain) return false;
    if (FAILED(m_cmdAllocator->Reset())) return false;
    if (FAILED(m_cmdList->Reset(m_cmdAllocator.Get(), nullptr))) return false;

    m_cmdList->RSSetViewports(1, &m_viewPort);
    m_cmdList->RSSetScissorRects(1, &m_scissorRect);

    CD3DX12_RESOURCE_BARRIER target = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_cmdList->ResourceBarrier(1, &target);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = 
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    m_cmdList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    const float clearColor[] = { 0.93f, 0.93f, 0.94f, 1.0f };
    m_cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    m_cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    if (m_pipeline && m_texture && m_quad)
    {
        ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
        m_cmdList->SetDescriptorHeaps(1, heaps);
        m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
        m_cmdList->SetPipelineState(m_pipeline.Get());
        m_cmdList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
        m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_cmdList->IASetVertexBuffers(0, 1, &m_quadView);
        m_cmdList->DrawInstanced(6, 1, 0, 0);
    }

    CD3DX12_RESOURCE_BARRIER present = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_cmdList->ResourceBarrier(1, &present);

    if (FAILED(m_cmdList->Close())) return false;

    ID3D12CommandList* lists[] = { m_cmdList.Get() };
    m_cmdQueue->ExecuteCommandLists(1, lists);

    if (FAILED(m_swapChain->Present(1, 0))) return false;

    m_currentSwapChain = m_swapChain->GetCurrentBackBufferIndex();
    FlushCommandQueue();
    return true;
}

bool DirectxInit::CreatePSO(const wchar_t* path)
{
    if (!m_device || !path) return false;
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::ostringstream text;
    text << file.rdbuf();
    std::string source = text.str();
    if (source.empty()) return false;
    const char* hlsl = source.c_str();

    D3D12_DESCRIPTOR_RANGE range = {};
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 1;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges = 1;
    param.DescriptorTable.pDescriptorRanges = &range;
    param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;

    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 1;
    rootDesc.pParameters = &param;
    rootDesc.NumStaticSamplers = 1;
    rootDesc.pStaticSamplers = &sampler;
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serialized;
    ComPtr<ID3DBlob> error;
    if (FAILED(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error))) return false;
    if (FAILED(m_device->CreateRootSignature(
        0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)))) return false;

    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;
    if (FAILED(D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vs, &error)))
    {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return false;
    }
    if (FAILED(D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &ps, &error)))
    {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return false;
    }

    D3D12_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = m_rootSignature.Get();
    pso.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    pso.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    pso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pso.DepthStencilState.DepthEnable = FALSE;
    pso.InputLayout = { layout, _countof(layout) };
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = m_backBufferFormat;
    pso.SampleDesc.Count = 1;

    return SUCCEEDED(m_device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&m_pipeline)));
}

bool DirectxInit::OnResize(int width, int height)
{
    if (!m_device || width <= 0 || height <= 0) return false;

    FlushCommandQueue();

    for (int i = 0; i < m_swapChainCount; i++)
    {
        m_swapChainBuffer[i].Reset();
    }
    m_depthStencilBuffer.Reset();

    if (FAILED(m_swapChain->ResizeBuffers(
        m_swapChainCount, 
        static_cast<UINT>(width), 
        static_cast<UINT>(height), 
        m_backBufferFormat, 0))) return false;

    m_currentSwapChain = m_swapChain->GetCurrentBackBufferIndex();
    if (CreateRenderTargetView()==false) return false;
    if (CreateDepthStencilBuffer(width, height) == false) return false;

    SetViewportAndScissor(width, height);
    return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectxInit::CurrentRenderTargetView() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtv =
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += static_cast<SIZE_T>(m_currentSwapChain) * m_rtvDescriptorSize;

    return rtv;
}

/// <summary>
/// 천지인 자판 비트맵을 텍스처로 올리고 화면 전체 사각형을 만든다.
/// shortcut: 텍스처는 처음 클라이언트 크기이다. 창 크기가 바뀌면 늘려 그린다.
/// </summary>
bool DirectxInit::CreateKeyboard()
{
    if (!m_device || !m_cmdAllocator || !m_cmdList || !m_windowInit) return false;

    RECT rc = {};
    GetClientRect(m_windowInit->GetHwnd(), &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    if (width <= 0 || height <= 0) return false;

    KeyboardBitmap image = {};
    if (!CreateCheonjiinBitmap(width, height, image)) return false;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = static_cast<UINT64>(width);
    texDesc.Height = static_cast<UINT>(height);
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
    if (FAILED(m_device->CreateCommittedResource(
        &defaultHeap, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_texture))))
    {
        DestroyCheonjiinBitmap(image);
        return false;
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    UINT rows = 0;
    UINT64 rowSize = 0;
    UINT64 total = 0;
    m_device->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &rows, &rowSize, &total);

    CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(total);
    ComPtr<ID3D12Resource> upload;
    if (FAILED(m_device->CreateCommittedResource(
        &uploadHeap, D3D12_HEAP_FLAG_NONE, &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload))))
    {
        DestroyCheonjiinBitmap(image);
        return false;
    }

    BYTE* mapped = nullptr;
    if (FAILED(upload->Map(0, nullptr, reinterpret_cast<void**>(&mapped))))
    {
        DestroyCheonjiinBitmap(image);
        return false;
    }
    const BYTE* src = static_cast<const BYTE*>(image.bits);
    for (UINT row = 0; row < rows; ++row)
    {
        const BYTE* from = src + static_cast<size_t>(row) * image.pitch;
        BYTE* to = mapped + footprint.Offset + static_cast<size_t>(row) * footprint.Footprint.RowPitch;
        memcpy(to, from, static_cast<size_t>(width) * 4);
        for (int x = 0; x < width; ++x)
            reinterpret_cast<UINT*>(to)[x] |= 0xFF000000;
    }
    upload->Unmap(0, nullptr);
    DestroyCheonjiinBitmap(image);

    D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
    srvDesc.NumDescriptors = 1;
    srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&m_srvHeap)))) return false;

    D3D12_SHADER_RESOURCE_VIEW_DESC view = {};
    view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    view.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    view.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(m_texture.Get(), &view, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

    struct Vertex
    {
        float x, y, z, u, v;
    };
    Vertex verts[] = {
        { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
        { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
        {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
    };
    D3D12_RESOURCE_DESC quadDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(verts));
    if (FAILED(m_device->CreateCommittedResource(
        &uploadHeap, D3D12_HEAP_FLAG_NONE, &quadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_quad))))
        return false;
    void* quadData = nullptr;
    if (FAILED(m_quad->Map(0, nullptr, &quadData))) return false;
    memcpy(quadData, verts, sizeof(verts));
    m_quad->Unmap(0, nullptr);
    m_quadView.BufferLocation = m_quad->GetGPUVirtualAddress();
    m_quadView.SizeInBytes = sizeof(verts);
    m_quadView.StrideInBytes = sizeof(Vertex);

    if (FAILED(m_cmdAllocator->Reset())) return false;
    if (FAILED(m_cmdList->Reset(m_cmdAllocator.Get(), nullptr))) return false;
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = m_texture.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;
    D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
    srcLoc.pResource = upload.Get();
    srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLoc.PlacedFootprint = footprint;
    m_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &srcLoc, nullptr);
    CD3DX12_RESOURCE_BARRIER toShader = CD3DX12_RESOURCE_BARRIER::Transition(
        m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    m_cmdList->ResourceBarrier(1, &toShader);
    if (FAILED(m_cmdList->Close())) return false;
    ID3D12CommandList* lists[] = { m_cmdList.Get() };
    m_cmdQueue->ExecuteCommandLists(1, lists);
    FlushCommandQueue();
    return true;
}
