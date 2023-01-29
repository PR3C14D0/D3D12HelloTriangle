#pragma once
#include <iostream>
#include <Windows.h>
#include <wrl.h>
#include <DirectX/d3d12.h>
#include <DirectX/d3dx12.h>
#include <dxgi1_4.h>
#include <vector>

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
	
	ComPtr<ID3D12Fence> fence;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	void GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory);

	D3D_FEATURE_LEVEL GetAdapterMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter);

	void InitBuffer();

public:
	Core(HWND& hwnd);
};