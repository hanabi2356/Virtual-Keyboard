#include "DirectxInit.h"

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

    const float clearColor[] = { 0.12f, 0.12f, 0.14f, 1.0f };
    m_cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    m_cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

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
