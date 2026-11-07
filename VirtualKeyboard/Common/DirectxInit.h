#pragma once
#include "../Common/Common.h"
#include "../Common/WindowInit.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class DirectxInit
{
private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGIFactory4> m_dxFactory;

	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_currentFence = 0;

	ComPtr<ID3D12CommandQueue> m_cmdQueue;
	ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_cmdList;

	ComPtr<IDXGISwapChain3> m_swapChain;
	static const int m_swapChainCount = 2;
	int m_currentSwapChain = 0;
	ComPtr<ID3D12Resource>m_swapChainBuffer[m_swapChainCount];
	ComPtr<ID3D12Resource>m_depthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	D3D12_VIEWPORT m_viewPort;
	D3D12_RECT m_scissorRect;

	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	UINT m_cbvSrvUavDescriptorSize = 0;

	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	WindowInit* m_windowInit = nullptr;
public:

	/// <summary>
	/// 창 객체를 저장함
	/// </summary>
	/// <param name="windowInit"></param>
	explicit DirectxInit(WindowInit* windowInit);

	/// <summary>
	/// Device와 Swap Chain을 만든 뒤 Resize Callback을 등록하고 첫 버퍼를 만든다
	/// </summary>
	/// <returns></returns>
	bool DXInit();

	/// <summary>
	/// Queue에 펜스를 시그널하고 완료될 때까지 기다린다
	/// </summary>
	void FlushCommandQueue();

	

private:
	/// <summary>
	/// 창 콜백의 user를 DirectxInit으로 바꿔 OnReisze로 넘김
	/// </summary>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <param name="user"></param>
	static void OnResizeThunk(int width, int height, void* user);

	/// <summary>
	/// 팩토리, 디바이스, 서술자 크기, 펜스를 생성
	/// 하드웨어 디바이스가 실패하면 WRAP 어댑터로 다시 생성
	/// </summary>
	/// <returns></returns>
	bool CreateDevice();

	/// <summary>
	/// swap chain을 생성
	/// </summary>
	/// <param name="hwnd"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <returns></returns>
	bool CreateSwapChain(HWND hwnd, int width, int height);

	/// <summary>
	/// CommandQueue, Allocator, List 생성 후 Close 
	/// Close해야 Flush가 안전하게 됨
	/// </summary>
	/// <returns></returns>
	bool CreateCommandObject();
	
	/// <summary>
	/// RTV 힙은 버퍼 수만큼, DSV 힙은 하나 생성
	/// Resize시 힙은 유지
	/// </summary>
	/// <returns></returns>
	bool CreateDescriptorHeap();

	/// <summary>
	/// Bakc Buffer마다 RTV를 생성해 힙에 순서대로 기록
	/// </summary>
	/// <returns></returns>
	bool CreateRenderTargetView();

	/// <summary>
	/// 깊이 버퍼를 DEPTH_WRITE 상태로 만들고 DSV를 기록함
	/// </summary>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <returns></returns>
	bool CreateDepthStencilBuffer(int width, int height);

	/// <summary>
	/// 뷰포트와 Scissor Rect를 클라이언트 전체에 맞춘다
	/// </summary>
	/// <param name="width"></param>
	/// <param name="height"></param>
	void SetViewportAndScissor(int width, int height);

	/// <summary>
	/// GPU를 기다린 다음 백버퍼, RTV, 깊이 버퍼, 뷰포트를 새 크기로 만든다
	/// </summary>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <returns></returns>
	bool OnResize(int width, int height);


/// <summary>
/// get/set
/// </summary>
public:
	ID3D12Resource* GetCurrentBackBuffer()const { return m_swapChainBuffer[m_currentSwapChain].Get(); }
};

