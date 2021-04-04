
module;
#pragma once
#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include <directxmath.h>
#include <stack>
#include "d3dx12.h"

export module rayzor_main;
import cube;
import utilities;
import scene_graph;
import render_visitor;

using Microsoft::WRL::ComPtr;


export class RayzorMain
{
public:
	RayzorMain(HWND hwnd);
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
	bool Frame(scene_graph& scene);

	ComPtr<ID3D12Device> get_device() {
		return m_device;
	}
private:
	bool Render(ID3D12GraphicsCommandList* cmds, scene_graph& scene);

	bool m_shouldQuit = false;
	HWND m_hwnd;

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_directCommandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
	ComPtr<ID3D12Resource> m_backBufferRenderTarget[2];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_directCommandList;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	unsigned long long m_fenceValue;
	int m_videoCardMemory;
	UINT m_bufferIndex;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissor;
	UINT m_rtvDescriptorSize;
	bool Initialize(int, int);
};





#define CHECK_FAIL(x,y) if(FAILED(x)) {MessageBox(m_hwnd, y, y, MB_OK); throw y; }
RayzorMain::RayzorMain(HWND hwnd) : m_hwnd(hwnd)
{
	if (!Initialize(800, 600)) {
		throw std::exception("Nope");
	}
}
bool RayzorMain::Initialize(int screenWidth, int screenHeight)
{
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT result;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ComPtr<IDXGIFactory4> factory;
	ComPtr<IDXGIAdapter> adapter;
	ComPtr<IDXGIOutput> adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	std::vector<DXGI_MODE_DESC> displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ComPtr<IDXGISwapChain> swapChain;
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;


	ComPtr<ID3D12Debug> debug_controller;

	result = D3D12GetDebugInterface(IID_PPV_ARGS(debug_controller.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"D3D12GetDebugInterface");
	debug_controller->EnableDebugLayer();
	UINT dxgi_flags = DXGI_CREATE_FACTORY_DEBUG;


	featureLevel = D3D_FEATURE_LEVEL_12_1;
	result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"D3D12CreateDevice Error");
	ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	result = m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_directCommandQueue.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"D3D12CommandQueue Error");
	result = CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"CreateDXGIFactory1");
	result = factory->EnumAdapters(0, &adapter);
	CHECK_FAIL(result, L"EnumAdapters");
	result = adapter->EnumOutputs(0, &adapterOutput);
	CHECK_FAIL(result, L"EnumOutputs");
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	CHECK_FAIL(result, L"GetDisplayModeList");
	displayModeList.resize(numModes);
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data());
	CHECK_FAIL(result, L"GetDisplayModeList");
	// Now go through all the display modes and find the one that matches the screen height and width.
// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Height == (unsigned int)screenHeight)
		{
			if (displayModeList[i].Width == (unsigned int)screenWidth)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}
	result = adapter->GetDesc(&adapterDesc);
	CHECK_FAIL(result, L"adapter->GetDesc");
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Set the swap chain to use double buffering.
	swapChainDesc.BufferCount = 2;

	// Set the height and width of the back buffers in the swap chain.
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Width = screenWidth;

	// Set a regular 32-bit surface for the back buffers.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the usage of the back buffers to be render target outputs.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the swap effect to discard the previous buffer contents after swapping.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = m_hwnd;

	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Finally create the swap chain using the swap chain description.	
	result = factory->CreateSwapChain(m_directCommandQueue.Get(), &swapChainDesc, swapChain.ReleaseAndGetAddressOf());
	CHECK_FAIL(result, L"CreateSwapChain");

	result = swapChain->QueryInterface(IID_PPV_ARGS(m_swapChain.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"QueryInterface")

	ZeroMemory(&renderTargetViewHeapDesc, sizeof(renderTargetViewHeapDesc));
	renderTargetViewHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		2,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};
	result = m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, IID_PPV_ARGS(m_rtvDescriptorHeap.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"CreateDescriptorHeap")

	CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < 2; i++) 
	{
		CHECK_FAIL(m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_backBufferRenderTarget[i].ReleaseAndGetAddressOf())), L"GetBuffer");
		m_device->CreateRenderTargetView(m_backBufferRenderTarget[i].Get(), nullptr, renderTargetViewHandle);
		renderTargetViewHandle.Offset(1, m_rtvDescriptorSize);

	}
	

	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create a command allocator.
	result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"CreateCommandAllocator");
	
	result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), NULL, IID_PPV_ARGS(m_directCommandList.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"CreateCommandList");
	CHECK_FAIL(m_directCommandList->Close(), L"Close");
	result = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()));
	CHECK_FAIL(result, L"CreateFence")
	m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
	m_fenceValue = 1;


	//rendering_cube = cube->create_cube(m_device.Get(), { 0.0f, 0.0f, 0.0f, 0.0f });

	m_viewport = { 0.0f,0.0f,(float)screenWidth, (float)screenHeight, 0.0f, 1.0f };
	m_scissor = { 0, 0, screenWidth, screenHeight };

	return true;
	
}

LRESULT RayzorMain::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		m_shouldQuit = true;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool RayzorMain::Frame(scene_graph& scene)
{
	if (m_shouldQuit) {
		return false;
	}
	HRESULT result;



	
	result = m_commandAllocator->Reset();
	if (FAILED(result))
		return false;
	result = m_directCommandList->Reset(m_commandAllocator.Get(), NULL);
	if (FAILED(result))
		return false;
	if (!Render(m_directCommandList.Get(), scene))
		return false;
	ID3D12CommandList* lists[1];
	lists[0] = m_directCommandList.Get();
	m_directCommandQueue->ExecuteCommandLists(1, lists);
	if (FAILED(m_swapChain->Present(1, 0)))
		return false;
	unsigned long long fenceToWaitFor = m_fenceValue;
	if (FAILED(m_directCommandQueue->Signal(m_fence.Get(), fenceToWaitFor)))
		return false;
	m_fenceValue++;
	if (m_fence->GetCompletedValue() < fenceToWaitFor)
	{
		result = m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent);
		if (FAILED(result))
			return false;
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	return true;
}
bool RayzorMain::Render(ID3D12GraphicsCommandList* cmds, scene_graph& scene)
{
	D3D12_RESOURCE_BARRIER barrier = {
		D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		D3D12_RESOURCE_BARRIER_FLAG_NONE
	};
	barrier.Transition = {
		m_backBufferRenderTarget[m_bufferIndex].Get(),
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	};
	cmds->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_bufferIndex, m_rtvDescriptorSize);

	cmds->RSSetViewports(1, &m_viewport);
	cmds->RSSetScissorRects(1, &m_scissor);
	// Set the back buffer as the render target.
	cmds->OMSetRenderTargets(1, &rtv_handle, true, NULL);
	float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	cmds->ClearRenderTargetView(rtv_handle, color, 0, NULL);

	using namespace DirectX;
	XMMATRIX view = XMMatrixLookToLH({ 1.0f, 1.0f, 4.0f }, { -1.0f, -1.0f, -4.0f }, { 0.0f, 1.0f, 0.0f });
	XMMATRIX projection = XMMatrixPerspectiveFovLH(5.0f, 4.0f / 3.0f, 0.01f, 100.0f);
	std::stack<XMMATRIX> world_matrix_stack;
	world_matrix_stack.push(XMMatrixIdentity());
	
	auto root = scene.get_root();
	auto visitor = render_visitor(cmds);
	visitor.render(root.get());



	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	
	cmds->ResourceBarrier(1, &barrier);
	if (FAILED(cmds->Close()))
		return false;

	return true;
}


