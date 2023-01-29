#include "Core/Core.h"

Core::Core(HWND& hwnd) {
	this->hwnd = hwnd;

	UINT dxgiFactoryCreationFlags = 0;

#ifndef NDEBUG
	dxgiFactoryCreationFlags |= DXGI_CREATE_FACTORY_DEBUG;

	{
		ComPtr<ID3D12Debug> debug;
		D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
		debug->EnableDebugLayer();
	}
#endif // NDEBUG

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryCreationFlags, IID_PPV_ARGS(this->factory.GetAddressOf())));

	GetMostCapableAdapter(this->adapter, this->factory);
	D3D_FEATURE_LEVEL maxFeatureLevel = this->GetAdapterMaxFeatureLevel(this->adapter)
;
	ThrowIfFailed(D3D12CreateDevice(this->adapter.Get(), maxFeatureLevel, IID_PPV_ARGS(this->dev.GetAddressOf()))); // Create our device

	/* Creation of our command queue */
	D3D12_COMMAND_QUEUE_DESC queueDesc = { };
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(this->dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(this->queue.GetAddressOf())));
	/* End:Creation of our command queue */


}

/*
	In this method, we'll retrieve the most capable adapter for the minimum feature level (D3D_FEATURE_LEVEL_11_0)
		Note: if there is no capable device, it will display a MessageBox with an error
*/
void Core::GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory) {
	std::vector<ComPtr<IDXGIAdapter>> adapters;
	D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	{
		ComPtr<IDXGIAdapter> tempAdapter;
		for (int i = 0; factory->EnumAdapters(i, tempAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; i++)
			adapters.push_back(tempAdapter);
	}

	for (ComPtr<IDXGIAdapter> tempAdapter : adapters) {
		if (SUCCEEDED(D3D12CreateDevice(tempAdapter.Get(), minimumFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
			tempAdapter.CopyTo(adapter.GetAddressOf());
			break;
		}
	}

	if (!adapter)
		MessageBox(this->hwnd, "Your adapter must be at least D3D_FEATURE_LEVEL_11_0", "Error", MB_OK | MB_ICONERROR);

	return;
}

/*
	In this method, we get the max feature level of the provided adapter
		Note: the adapter must be capable with the minimum feature level.
		Call Core::GetMostCapableAdapter for getting a capable adapter.
*/
D3D_FEATURE_LEVEL Core::GetAdapterMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter) {
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_2,
	};

	D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	D3D_FEATURE_LEVEL maxFeatureLevel;

	{
		D3D12_FEATURE_DATA_FEATURE_LEVELS featureData = { };
		featureData.NumFeatureLevels = _countof(featureLevels);
		featureData.pFeatureLevelsRequested = featureLevels;

		ComPtr<ID3D12Device> dev;
		ThrowIfFailed(D3D12CreateDevice(adapter.Get(), minimumFeatureLevel, IID_PPV_ARGS(dev.GetAddressOf())));
		ThrowIfFailed(dev->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureData, sizeof(featureData)));

		maxFeatureLevel = featureData.MaxSupportedFeatureLevel;
	}

	return maxFeatureLevel;
}