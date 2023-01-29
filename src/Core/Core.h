#pragma once
#include <iostream>
#include <Windows.h>
#include <wrl.h>
#include <DirectX/d3d12.h>
#include <dxgi1_4.h>
#include <vector>

using namespace Microsoft::WRL;

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

	void GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory);

	D3D_FEATURE_LEVEL GetAdapterMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter);
public:
	Core(HWND& hwnd);
};