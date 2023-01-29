#include "Core/Core.h"

Core::Core(HWND& hwnd) {
	this->hwnd = hwnd;

	this->nNumBackBuffers = 2;

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

	/* Creation of our swap chain */
	{
		ComPtr<IDXGISwapChain1> sc;

		DXGI_SWAP_CHAIN_DESC1 scDesc = { };
		scDesc.BufferCount = this->nNumBackBuffers;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.SampleDesc.Count = 1;
		scDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
		ThrowIfFailed(this->factory->CreateSwapChainForHwnd(this->queue.Get(), this->hwnd, &scDesc, nullptr, nullptr, sc.GetAddressOf()));
		sc.As(&this->sc);
	}
	/* End:Creation of our swap chain */

	ThrowIfFailed(this->dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(this->allocator.GetAddressOf()))); // We create our command allocator

	/* Creation of our backbuffer */
	D3D12_DESCRIPTOR_HEAP_DESC backBufferDescriptorHeapDesc = { };
	backBufferDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	backBufferDescriptorHeapDesc.NumDescriptors = this->nNumBackBuffers;
	backBufferDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	
	ThrowIfFailed(this->dev->CreateDescriptorHeap(&backBufferDescriptorHeapDesc, IID_PPV_ARGS(this->backBufferDescriptorHeap.GetAddressOf())));

	this->backBufferOffset = this->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE backBufferHandle(this->backBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	
	for (int i = 0; i < this->nNumBackBuffers; i++) {
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(this->sc->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf())));
		this->backBuffers.push_back(backBuffer);

		this->dev->CreateRenderTargetView(this->backBuffers[i].Get(), nullptr, backBufferHandle);
		backBufferHandle.Offset(1, this->backBufferOffset);
	}
	/* End:Creation of our backbuffer */


	this->InitBuffer();
	this->InitPipeline();
	
	// Creation of our command list. Our command list requires our pipeline state, so, we create after pipeline initialization
	ThrowIfFailed(this->dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->allocator.Get(), this->plState.Get(), IID_PPV_ARGS(this->list.GetAddressOf())));
	ThrowIfFailed(this->list->Close());

}

/*
	In this method we are going to initialize our vertex buffer.
	On this vertex buffer we are only going to enter 3 vertices.
	These vertices are going to be a triangle vertices.
*/
void Core::InitBuffer() {
	/* 
		These vertices need to go clockwise because we're gonna enable Backface culling on our rasterizer. 
		Read more about rasterizer state at https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc  
	*/
	vertex vertices[] = {
		{0.f, .5f, 0.f, { 1.f, 0.f, 0.f, 1.f }},
		{-.5f, -.5f, 0.f, { 1.f, 0.f, 0.f, 1.f }},
		{.5f, -.5f, 0.f, { 1.f, 0.f, 0.f, 1.f }},
	};

	UINT verticesSize = sizeof(vertices); // Size of our vertex array

	/* Creation of our vertex buffer */
	D3D12_HEAP_PROPERTIES vertexBufferProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesSize);

	ThrowIfFailed(this->dev->CreateCommittedResource(
		&vertexBufferProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(this->vertexBuffer.GetAddressOf())
	));

	this->vertexBufferView.BufferLocation = this->vertexBuffer->GetGPUVirtualAddress();
	this->vertexBufferView.SizeInBytes = verticesSize;
	this->vertexBufferView.StrideInBytes = sizeof(vertex);
	/* End:Creation of our vertex buffer */

	return;
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
	In this method we will initialize our pipeline state
		Note: Additionally we are going to create an input layout and a root signature.
*/
void Core::InitPipeline() {
	/* Creation of our root signature */
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = { };
	rootSigDesc.pParameters = nullptr;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.NumParameters = 0;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSigBlob;
	ComPtr<ID3DBlob> rootSigErrorBlob;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, rootSigBlob.GetAddressOf(), rootSigErrorBlob.GetAddressOf()));
	
	if (rootSigErrorBlob) {
		MessageBox(this->hwnd, (char*)rootSigErrorBlob->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
		return;
	}
	this->dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(this->rootSig.GetAddressOf()));
	/* End:Creation of our root signature */

	/* Creation of our input layout */
	D3D12_INPUT_ELEMENT_DESC inputElements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL}
	};

	D3D12_INPUT_LAYOUT_DESC iLayoutDesc = { };
	iLayoutDesc.NumElements = _countof(inputElements);
	iLayoutDesc.pInputElementDescs = inputElements;
	/* End:Creation of our input layout */

	/* Compile our shader */
	WCHAR fullShaderPath[MAX_PATH];
	GetFullPathNameW(L"shader.fx", MAX_PATH, fullShaderPath, nullptr);

	ComPtr<ID3DBlob> VertexShader, PixelShader;
	ComPtr<ID3DBlob> VertexShaderError, PixelShaderError;

	ThrowIfFailed(D3DCompileFromFile(fullShaderPath, nullptr, nullptr, "VertexMain", "vs_5_1", NULL, NULL, VertexShader.GetAddressOf(), VertexShaderError.GetAddressOf()));
	ThrowIfFailed(D3DCompileFromFile(fullShaderPath, nullptr, nullptr, "PixelMain", "ps_5_1", NULL, NULL, PixelShader.GetAddressOf(), PixelShaderError.GetAddressOf()));
	
	if (VertexShaderError) {
		MessageBox(this->hwnd, (char*)VertexShaderError->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
		return;
	}
	
	if (PixelShaderError) {
		MessageBox(this->hwnd, (char*)PixelShaderError->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
		return;
	}
	/* End:Compile our shader */

	/* Creation of our pipeline state */
	D3D12_GRAPHICS_PIPELINE_STATE_DESC plDesc = { };
	plDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get());
	plDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get());
	plDesc.InputLayout = iLayoutDesc;
	plDesc.pRootSignature = this->rootSig.Get();
	plDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	plDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
	plDesc.NumRenderTargets = 1;
	plDesc.DepthStencilState.DepthEnable = FALSE; // Enable this 2 options if you're going to use Depth Stencil.s
	plDesc.DepthStencilState.StencilEnable = FALSE;
	plDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	plDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	plDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	plDesc.SampleMask = UINT32_MAX;
	plDesc.SampleDesc.Count = 1;

	ThrowIfFailed(this->dev->CreateGraphicsPipelineState(&plDesc, IID_PPV_ARGS(this->plState.GetAddressOf())));
	/* End:Creation of our pipeline state*/
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