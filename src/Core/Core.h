#pragma once
#include <iostream>
#include <Windows.h>
#include <wrl.h>
#include <DirectX/d3d12.h>
#include <DirectX/d3dx12.h>
#include <dxgi1_4.h>
#include <vector>
#include <d3dcompiler.h>

using namespace Microsoft::WRL;
typedef float RGB[3];
typedef float RGBA[4];

struct vertex {
	RGB position;
	RGBA color;
};

/*
	This method will throw an exception if the provided HRESULT parameter failed.
*/
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr))
		throw std::exception();
}

class Core {
private:
	HWND hwnd;

	ComPtr<IDXGISwapChain3> sc;
	ComPtr<IDXGIFactory2> factory;
	ComPtr<IDXGIAdapter> adapter;

	ComPtr<ID3D12Device> dev;
	ComPtr<ID3D12GraphicsCommandList> list;
	ComPtr<ID3D12CommandQueue> queue;
	ComPtr<ID3D12CommandAllocator> allocator;

	ComPtr<ID3D12PipelineState> plState;
	
	ComPtr<ID3D12DescriptorHeap> backBufferDescriptorHeap;
	
	ComPtr<ID3D12RootSignature> rootSig;

	ComPtr<ID3D12Fence> fence;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	std::vector<ComPtr<ID3D12Resource>> backBuffers;

	UINT backBufferOffset;

	int nNumBackBuffers;
	
	void GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory);

	D3D_FEATURE_LEVEL GetAdapterMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter);

	void InitBuffer();
	void InitPipeline();
public:
	Core(HWND& hwnd);
};