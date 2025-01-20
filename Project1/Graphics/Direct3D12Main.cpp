#include "GraphicsDirect3D12.h"
#include "Resource.h"

#define SHADER_CSO_FILE_VS L"../Resource/Shaders/VSD12.cso"
#define SHADER_CSO_FILE_PS L"../Resource/Shaders/PSD12.cso"
#define SHADER_CSO_FILE_GSTexture L"../Resource/Shaders/GSD12Texture.cso"
#define SHADER_CSO_FILE_VSRect L"../Resource/Shaders/VSD12Rect.cso"
#define SHADER_CSO_FILE_PSRect L"../Resource/Shaders/PSD12Rect.cso"
#define SHADER_CSO_FILE_GSRect L"../Resource/Shaders/GSD12Rect.cso"

HRESULT		CreateIGD12									(HWND Handle, IGD12** ppGraphics)
{
	(*ppGraphics) = new IGD12;

	if FAILED((*ppGraphics)->Initialize(Handle))
	{
		SafeRelease(ppGraphics);
		return E_FAIL;
	}

	return S_OK;
}
HRESULT		CreateWICImageProcessor						(IWICImageProcessor** ppWICImageProcessor)
{
	if (*ppWICImageProcessor)
		return E_INVALIDARG;

	(*ppWICImageProcessor) = new IWICImageProcessor;
	if FAILED((*ppWICImageProcessor)->Initialize())
	{
		SafeRelease(ppWICImageProcessor);
		DebugLog(L"\nERROR::CreateWICImageProcessor: FAILED TO INITIALIZE");
		return E_FAIL;
	}

	return S_OK;
};

HRESULT		IGD12::Initialize							(HWND Handle)
{
	HRESULT hr;

	if (!IsWindow(Handle))
	{
		DebugLog(L"ERROR:: INVALID HANDLE");
		return E_FAIL;
	}
	hwnd = Handle;

	//		WIC IMAGE PROCESSOR
	{
		hr = CreateWICImageProcessor(&pWICImageProcessor);
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE WIC IMAGE PROCESSOR");
			Clear();
			return hr;
		}
	}

	//		Initial Initialization
	{

		//		DEBUG INTERFACE
		{
			hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO GET DEBUG INTERFACE");

				return hr;
			}

			pDebug->EnableDebugLayer();
		}

		//		DXGI FACTORY
		{
			hr = CreateDXGIFactory(IID_PPV_ARGS(&pDXGIFactory));
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE DXGI FACTORY");
				Clear();
				return hr;
			}
		}

		//		DEVICE
		{
			hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE DEVICE");
				Clear();
				return hr;
			}
		}

		//		COMMAND QUEUE
		{
			D3D12_COMMAND_QUEUE_DESC	CommandQueueDesc{};
			CommandQueueDesc.Flags	= D3D12_COMMAND_QUEUE_FLAG_NONE;
			CommandQueueDesc.Type	= D3D12_COMMAND_LIST_TYPE_DIRECT;

			hr = pDevice->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&pCommandQueue));
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE COMMAND QUEUE");
				Clear();
				return hr;
			}
		}

		//		SWAP CHAIN
		{
			DXGI_SWAP_CHAIN_DESC		SwapChainDesc{};
			SwapChainDesc.BufferCount		= 2;
			SwapChainDesc.BufferDesc.Width	= SETTING_RESOLUTION_WIDTH;
			SwapChainDesc.BufferDesc.Height = SETTING_RESOLUTION_HEIGHT;
			SwapChainDesc.BufferDesc.Format = DXGIFormat;
			SwapChainDesc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			SwapChainDesc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
			SwapChainDesc.OutputWindow		= hwnd;
			SwapChainDesc.SampleDesc.Count	= 1;
			SwapChainDesc.Windowed			= TRUE;

			hr = pDXGIFactory->CreateSwapChain(pCommandQueue, &SwapChainDesc, &pSwapChain);
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE SWAP CHAIN");
				Clear();
				return hr;
			}
		}

	}

	//		FENCE
	{
		hr = pDevice->CreateFence(NULL, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE FENCE");
			Clear();
			return hr;
		}

		FenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!FenceEvent)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE FENCE EVENT");
			Clear();
			return HRESULT_FROM_WIN32(GetLastError());
		}

		FenceValue = 1;

		WaitForPreviousFrame();
	}

	//		VIEW PORT  &  SCISSOR RECT
	{
		ViewPort.Height		= SETTING_RESOLUTION_HEIGHT;
		ViewPort.Width		= SETTING_RESOLUTION_WIDTH;
		ViewPort.TopLeftX	= 0;
		ViewPort.TopLeftY	= 0;
		ViewPort.MaxDepth	= 1;
		ViewPort.MinDepth	= 0;

		ScissorRect.left	= 0;
		ScissorRect.right	= SETTING_RESOLUTION_WIDTH;
		ScissorRect.top		= 0;
		ScissorRect.bottom	= SETTING_RESOLUTION_HEIGHT;
	}

	//		DESCRIPTOR HEAP
	{
		D3D12_DESCRIPTOR_HEAP_DESC	DHDesc{};
		DHDesc.NumDescriptors = 2;
		DHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		DHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		hr = pDevice->CreateDescriptorHeap(&DHDesc, IID_PPV_ARGS(&pDescriptor[GD12_DESCRIPTOR_RTV]));
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE RTV DESCRIPTOR HEAP");
			Clear();
			return hr;
		}

		//		SETTINGS CBV
		DHDesc.NumDescriptors	= 1;
		DHDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		DHDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		hr = pDevice->CreateDescriptorHeap(&DHDesc, IID_PPV_ARGS(&pDescriptor[GD12_DESCRIPTOR_SETTINGS]));
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE RTV DESCRIPTOR HEAP");
			Clear();
			return hr;
		}


		//		SAMPLER
		DHDesc.NumDescriptors	= 1;
		DHDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		DHDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		hr = pDevice->CreateDescriptorHeap(&DHDesc, IID_PPV_ARGS(&pDescriptor[GD12_DESCRIPTOR_SAMPLER]));
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE SAMPLER DESCRIPTOR HEAP");
			Clear();
			return hr;
		}
	}

	//		RENDER TARGET VIEW
	{
		D3D12_CPU_DESCRIPTOR_HANDLE		Handle{ pDescriptor[GD12_DESCRIPTOR_RTV]->GetCPUDescriptorHandleForHeapStart() };

		for (int i = 0; i < 2; i++)
		{
			hr = pSwapChain->GetBuffer(i, IID_PPV_ARGS(&(pRTV[i])));
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO GET SWAP CHAIN BUFFER");
				Clear();
				return hr;
			}
			pDevice->CreateRenderTargetView(pRTV[i], NULL, Handle);
			Handle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
	}

	//		BUFFERS
	{
		//		UPLOAD BUFFER
		hr = CreateBuffer(10000000, &pUploadBuffer, GD12_BUFFER_TYPE_UPLOAD);
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE UPLOAD BUFFER");
			Clear();
			return hr;
		}


		//		SETTINGS BUFFER
		hr = CreateBuffer(sizeof(SettingsBuffer), &pConstantBuffer[GD12_CONSTANT_BUFFER_CAMERA], GD12_BUFFER_TYPE_CONSTANT);
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE SETTING BUFFER");
			Clear();
			return hr;
		}
		
		D3D12_CONSTANT_BUFFER_VIEW_DESC			ConstantBufferViewDesc	{};
		
		ConstantBufferViewDesc.BufferLocation	= pConstantBuffer[GD12_CONSTANT_BUFFER_CAMERA]->GetGPUVirtualAddress();
		ConstantBufferViewDesc.SizeInBytes		= sizeof(SettingsBuffer);
		
		pDevice->CreateConstantBufferView(&ConstantBufferViewDesc, pDescriptor[GD12_DESCRIPTOR_SETTINGS]->GetCPUDescriptorHandleForHeapStart());


		//		VERTEX BUFFER TEXTURE
		if FAILED(CreateVertexBufferQueue(&pVertexBufferQueueTexture, 10, sizeof(GD12_SHAPE_TEXTURE)))
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO Create BUFFER");
			Clear();
			return E_FAIL;
		}


		//		VERTEX BUFFER RECT
		if FAILED(CreateVertexBufferQueue(&pVertexBufferQueueRect, 10, sizeof(GD12_SHAPE_RECT)))
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE BUFFER");
			Clear();
			return E_FAIL;
		}
	}

	//		SAMPLER
	{
		D3D12_SAMPLER_DESC SamplerDesc{};
		SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.BorderColor[0] = 0.f;
		SamplerDesc.BorderColor[1] = 0.f;
		SamplerDesc.BorderColor[2] = 0.f;
		SamplerDesc.BorderColor[3] = 0.f;
		SamplerDesc.MinLOD = 0.f;
		SamplerDesc.MipLODBias = 0.f;
		SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

		pDevice->CreateSampler(&SamplerDesc, pDescriptor[GD12_DESCRIPTOR_SAMPLER]->GetCPUDescriptorHandleForHeapStart());
	}

	//		ROOT SIGNATURE
	{
		//		TEXTURE
		{
			D3D12_ROOT_SIGNATURE_DESC	RootSignatureDescTexture	{};
			D3D12_ROOT_PARAMETER		ParameterDesc[4]			{};
			ID3DBlob*					pSignature					{ NULL };

			D3D12_DESCRIPTOR_RANGE Range[2]{};

			//Settings buffer
			ParameterDesc[0].ShaderVisibility						= D3D12_SHADER_VISIBILITY_GEOMETRY;
			ParameterDesc[0].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_CBV;
			ParameterDesc[0].Descriptor.ShaderRegister				= 0;
			ParameterDesc[0].Descriptor.RegisterSpace				= 0;

			//Texture data
			ParameterDesc[2].ShaderVisibility						= D3D12_SHADER_VISIBILITY_GEOMETRY;
			ParameterDesc[2].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_CBV;
			ParameterDesc[2].Descriptor.ShaderRegister				= 1;
			ParameterDesc[2].Descriptor.RegisterSpace				= 0;

			//Sampler
			Range[0].BaseShaderRegister								= 0;
			Range[0].NumDescriptors									= 1;
			Range[0].OffsetInDescriptorsFromTableStart				= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			Range[0].RegisterSpace									= 0;
			Range[0].RangeType										= D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

			ParameterDesc[1].ShaderVisibility						= D3D12_SHADER_VISIBILITY_PIXEL;
			ParameterDesc[1].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			ParameterDesc[1].DescriptorTable.NumDescriptorRanges	= 1;
			ParameterDesc[1].DescriptorTable.pDescriptorRanges		= &Range[0];

			//Texture
			Range[1].BaseShaderRegister								= 0;
			Range[1].NumDescriptors									= 1;
			Range[1].OffsetInDescriptorsFromTableStart				= 1;
			Range[1].RegisterSpace									= 0;
			Range[1].RangeType										= D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

			ParameterDesc[3].ShaderVisibility						= D3D12_SHADER_VISIBILITY_PIXEL;
			ParameterDesc[3].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			ParameterDesc[3].DescriptorTable.NumDescriptorRanges	= 1;
			ParameterDesc[3].DescriptorTable.pDescriptorRanges		= &Range[1];

			

			RootSignatureDescTexture.NumParameters					= 4;
			RootSignatureDescTexture.pParameters					= ParameterDesc;
			RootSignatureDescTexture.Flags							= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			RootSignatureDescTexture.NumStaticSamplers				= 0;

			hr = D3D12SerializeRootSignature(&RootSignatureDescTexture, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, NULL);
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO SERIALIZE ROOT SIGNATURE TEXTURE");
				SafeRelease(&pSignature);
				Clear();
				return hr;
			}

			hr = pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature[GD12_ROOT_SIGNATURE_TEXTURE]));
			SafeRelease(&pSignature);

			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE ROOT SIGNATURE TEXTURE");
				Clear();
				return hr;
			}
		}

		//		RECT
		{
			D3D12_ROOT_SIGNATURE_DESC	RootSignatureDescTexture	{};
			D3D12_ROOT_PARAMETER		ParameterDesc				{};
			ID3DBlob*					pSignature					{ NULL };


			ParameterDesc.ShaderVisibility				= D3D12_SHADER_VISIBILITY_GEOMETRY;
			ParameterDesc.ParameterType					= D3D12_ROOT_PARAMETER_TYPE_CBV;
			ParameterDesc.Descriptor.ShaderRegister		= 0;
			ParameterDesc.Descriptor.RegisterSpace		= 0;

			RootSignatureDescTexture.NumParameters		= 1;
			RootSignatureDescTexture.pParameters		= &ParameterDesc;
			RootSignatureDescTexture.Flags				= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			RootSignatureDescTexture.NumStaticSamplers	= 0;

			hr = D3D12SerializeRootSignature(&RootSignatureDescTexture, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, NULL);
			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO SERIALIZE ROOT SIGNATURE RECT");
				SafeRelease(&pSignature);
				Clear();
				return hr;
			}

			hr = pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature[GD12_ROOT_SIGNATURE_RECT]));
			SafeRelease(&pSignature);

			if FAILED(hr)
			{
				DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE ROOT SIGNATURE RECT");
				Clear();
				return hr;
			}
		}
	}

	//		GRAPHICS PIPELINE STATE
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC	GraphicsPipelineStateDesc{};
		D3D12_GRAPHICS_PIPELINE_STATE_DESC	GraphicsPipelineStateDescRect{};
		D3D12_RASTERIZER_DESC				RasterizerDesc;
		D3D12_BLEND_DESC					BlendDesc;
		D3D12_DEPTH_STENCIL_DESC			DepthStencilDesc;
		ShaderBlob							PSBlob;
		ShaderBlob							VSBlob;
		ShaderBlob							GSTextureBlob;
		ShaderBlob							VSRectBlob;
		ShaderBlob							PSRectBlob;
		ShaderBlob							GSRectBlob;
		D3D12_INPUT_ELEMENT_DESC			InputElementDescTexture[]
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		D3D12_INPUT_ELEMENT_DESC			InputElementDescRect[]
		{
			{"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		//		LOAD SHADERS
		if FAILED(VSBlob.ReadCSOFile(SHADER_CSO_FILE_VS))
	{
		DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO READ VS FILE");
		Clear();
		return E_FAIL;
	}
		if FAILED(PSBlob.ReadCSOFile(SHADER_CSO_FILE_PS))
	{
		DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO READ PS FILE");
		Clear();
		return E_FAIL;
	}
		if FAILED(GSTextureBlob.ReadCSOFile(SHADER_CSO_FILE_GSTexture))
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO READ GSTexture FILE");
			Clear();
			return E_FAIL;
		}
		if FAILED(VSRectBlob.ReadCSOFile(SHADER_CSO_FILE_VSRect))
	{
		DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO READ VSRect FILE");
		Clear();
		return E_FAIL;
	}
		if FAILED(PSRectBlob.ReadCSOFile(SHADER_CSO_FILE_PSRect))
	{
		DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO READ PSRect FILE");
		Clear();
		return E_FAIL;
	}
		if FAILED(GSRectBlob.ReadCSOFile(SHADER_CSO_FILE_GSRect))
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO READ GSRect FILE");
			Clear();
			return E_FAIL;
		}



		RasterizerDesc.FillMode								= D3D12_FILL_MODE_SOLID;
		RasterizerDesc.CullMode								= D3D12_CULL_MODE_NONE;
		RasterizerDesc.FrontCounterClockwise				= FALSE;
		RasterizerDesc.DepthBias							= D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDesc.DepthBiasClamp						= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDesc.SlopeScaledDepthBias					= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDesc.DepthClipEnable						= FALSE;
		RasterizerDesc.MultisampleEnable					= FALSE;
		RasterizerDesc.AntialiasedLineEnable				= TRUE;
		RasterizerDesc.ForcedSampleCount					= 0;
		RasterizerDesc.ConservativeRaster					= D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		
		BlendDesc.AlphaToCoverageEnable						= FALSE;
		BlendDesc.IndependentBlendEnable					= FALSE;
		BlendDesc.RenderTarget[0].BlendEnable				= FALSE;
		BlendDesc.RenderTarget[0].LogicOpEnable				= FALSE;
		BlendDesc.RenderTarget[0].SrcBlend					= D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlend					= D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].BlendOp					= D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha				= D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlendAlpha			= D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].BlendOpAlpha				= D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].LogicOp					= D3D12_LOGIC_OP_NOOP;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask		= D3D12_COLOR_WRITE_ENABLE_ALL;

		DepthStencilDesc.DepthEnable						= FALSE;
		DepthStencilDesc.DepthWriteMask						= D3D12_DEPTH_WRITE_MASK_ALL;
		DepthStencilDesc.DepthFunc							= D3D12_COMPARISON_FUNC_LESS;
		DepthStencilDesc.StencilEnable						= FALSE;
		DepthStencilDesc.StencilReadMask					= D3D12_DEFAULT_STENCIL_READ_MASK;
		DepthStencilDesc.StencilWriteMask					= D3D12_DEFAULT_STENCIL_WRITE_MASK;
		DepthStencilDesc.FrontFace.StencilFailOp			= D3D12_STENCIL_OP_KEEP;
		DepthStencilDesc.BackFace.StencilFailOp				= D3D12_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilDepthFailOp		= D3D12_STENCIL_OP_KEEP;
		DepthStencilDesc.BackFace.StencilDepthFailOp		= D3D12_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilPassOp			= D3D12_STENCIL_OP_KEEP;
		DepthStencilDesc.BackFace.StencilPassOp				= D3D12_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilFunc				= D3D12_COMPARISON_FUNC_ALWAYS;
		DepthStencilDesc.BackFace.StencilFunc				= D3D12_COMPARISON_FUNC_ALWAYS;	
		
		GraphicsPipelineStateDesc.InputLayout				= { InputElementDescTexture, _countof(InputElementDescTexture) };
		GraphicsPipelineStateDesc.pRootSignature			= pRootSignature[GD12_ROOT_SIGNATURE_TEXTURE];
		GraphicsPipelineStateDesc.VS						= { VSBlob.GetBuffer(), VSBlob.GetSize() };
		GraphicsPipelineStateDesc.PS						= { PSBlob.GetBuffer(), PSBlob.GetSize() };
		GraphicsPipelineStateDesc.GS						= { GSTextureBlob.GetBuffer(), GSTextureBlob.GetSize() };
		GraphicsPipelineStateDesc.RasterizerState			= RasterizerDesc;
		GraphicsPipelineStateDesc.BlendState				= BlendDesc;
		GraphicsPipelineStateDesc.DepthStencilState			= DepthStencilDesc;
		GraphicsPipelineStateDesc.SampleMask				= UINT_MAX;
		GraphicsPipelineStateDesc.PrimitiveTopologyType		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		GraphicsPipelineStateDesc.NumRenderTargets			= 1;
		GraphicsPipelineStateDesc.RTVFormats[0]				= DXGIFormat;
		GraphicsPipelineStateDesc.SampleDesc.Count			= 1;
		GraphicsPipelineStateDesc.SampleDesc.Quality		= 0;

		GraphicsPipelineStateDescRect.InputLayout			= { InputElementDescRect, _countof(InputElementDescRect) };
		GraphicsPipelineStateDescRect.pRootSignature		= pRootSignature[GD12_ROOT_SIGNATURE_RECT];
		GraphicsPipelineStateDescRect.VS					= { VSRectBlob.GetBuffer(), VSRectBlob.GetSize() };
		GraphicsPipelineStateDescRect.PS					= { PSRectBlob.GetBuffer(), PSRectBlob.GetSize() };
		GraphicsPipelineStateDescRect.GS					= { GSRectBlob.GetBuffer(), GSRectBlob.GetSize() };
		GraphicsPipelineStateDescRect.RasterizerState		= RasterizerDesc;
		GraphicsPipelineStateDescRect.BlendState			= BlendDesc;
		GraphicsPipelineStateDescRect.DepthStencilState		= DepthStencilDesc;
		GraphicsPipelineStateDescRect.SampleMask			= UINT_MAX;
		GraphicsPipelineStateDescRect.PrimitiveTopologyType	= D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		GraphicsPipelineStateDescRect.NumRenderTargets		= 1;
		GraphicsPipelineStateDescRect.RTVFormats[0]			= DXGIFormat;
		GraphicsPipelineStateDescRect.SampleDesc.Count		= 1;
		GraphicsPipelineStateDescRect.SampleDesc.Quality	= 0;
		

		if FAILED(pDevice->CreateGraphicsPipelineState(&GraphicsPipelineStateDesc, IID_PPV_ARGS(&pPipelineState[GD12_PIPELINE_STATE_TEXTURE])))
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE GRAPHICS PIPELINE STATE");
			Clear();
			return E_FAIL;
		}
		if FAILED(pDevice->CreateGraphicsPipelineState(&GraphicsPipelineStateDescRect, IID_PPV_ARGS(&pPipelineState[GD12_PIPELINE_STATE_RECT])))
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE GRAPHICS PIPELINE STATE RECT");
			Clear();
			return E_FAIL;
		}
	}

	//		COMMAND ALLOCATOR AND COMMAND LIST
	{
		hr = CreateCommandList(&pCommandList[GD12_COMMAND_LIST_DIRECT], D3D12_COMMAND_LIST_TYPE_DIRECT, &pCommandAllocator[GD12_COMMAND_ALLOCATOR_DIRECT], pPipelineState[GD12_PIPELINE_STATE_RECT]);
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE COMMAND ALLOCATOR & COMMAND LIST");
			Clear();
			return hr;
		}

		hr = CreateCommandList(&pCommandList[GD12_COMMAND_LIST_COPY], D3D12_COMMAND_LIST_TYPE_COPY, &pCommandAllocator[GD12_COMMAND_ALLOCATOR_COPY], NULL);
		if FAILED(hr)
		{
			DebugLog(L"\nERROR::IGD12::Initialize:  FAILED TO CREATE COPY ALLOCATOR & COPY LIST");
			Clear();
			return hr;
		}

		pCommandAllocator[GD12_COMMAND_ALLOCATOR_DIRECT]->Reset();
		pCommandList[GD12_COMMAND_LIST_DIRECT]->Reset(pCommandAllocator[GD12_COMMAND_ALLOCATOR_DIRECT], pPipelineState[GD12_PIPELINE_STATE_TEXTURE]);
	}

	//		UPDATE BUFFERS
	{
		//		SETTINGS BUFFER
		hr = UpdateBuffer(pUploadBuffer, &SettingsBuffer, sizeof(SettingsBuffer));
		hr = UpdateConstantBuffer(pCommandList[GD12_COMMAND_LIST_DIRECT], pConstantBuffer[GD12_CONSTANT_BUFFER_CAMERA], pUploadBuffer, sizeof(SettingsBuffer));	
	}

	//		THE END

	FLAG |= IGD12_FLAG_INITIALIZED;

	//
	GD12_TEXTURE_DESC SurfaceDesc;
	SurfaceDesc.Height = 1000;
	SurfaceDesc.Width = 1000;
	SurfaceDesc.FLAG = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	if FAILED(CreateSurface(&SurfaceDesc, &pSurface))
	{
		Clear();
		return hr;
	}

	ExecuteList(pCommandList[GD12_COMMAND_LIST_DIRECT]);

	return hr;
};
void		IGD12::WaitForPreviousFrame					()
{
	pCommandQueue->Signal(pFence, FenceValue);

	if (pFence->GetCompletedValue() < FenceValue)
	{
		pFence->SetEventOnCompletion(FenceValue, FenceEvent);
		WaitForSingleObject(FenceEvent, INFINITE);
	}

	FenceValue++;
}
void		IGD12::ExecuteList							(ID3D12GraphicsCommandList* pList)
{
	pList->Close();
	pCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&pCommandList));

	WaitForPreviousFrame();

	pCommandAllocator[GD12_COMMAND_ALLOCATOR_DIRECT]->Reset();
	pList->Reset(pCommandAllocator[GD12_COMMAND_ALLOCATOR_DIRECT], NULL);
}
HRESULT		IGD12::Render								()
{
	//		PREPARE

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource	= pRTV[FrameIndex];
	Barrier.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	Barrier.Transition.StateBefore	= D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle{ pDescriptor[GD12_DESCRIPTOR_RTV]->GetCPUDescriptorHandleForHeapStart()};
	RTVHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * FrameIndex;

	ExecuteList(pCommandList[GD12_COMMAND_LIST_DIRECT]);
	
	
	pCommandList[GD12_COMMAND_LIST_DIRECT]->RSSetViewports(1, &ViewPort);
	pCommandList[GD12_COMMAND_LIST_DIRECT]->RSSetScissorRects(1, &ScissorRect);
	


	// SURFACE TEST

	float CC[4]{ 0.4,0,0,1 };

	D3D12_RESOURCE_BARRIER SurfaceBarrier;
	SurfaceBarrier.Type						= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	SurfaceBarrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
	SurfaceBarrier.Transition.pResource		= pSurface->pBuffer;
	SurfaceBarrier.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	SurfaceBarrier.Transition.StateBefore	= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	SurfaceBarrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;

	D3D12_CPU_DESCRIPTOR_HANDLE SurfaceHandle { pSurface->pDescriptorRTV->GetCPUDescriptorHandleForHeapStart() };

	pCommandList[GD12_COMMAND_LIST_DIRECT]->ResourceBarrier(1, &SurfaceBarrier);
	pCommandList[GD12_COMMAND_LIST_DIRECT]->OMSetRenderTargets(1, &SurfaceHandle, FALSE, NULL);
	pCommandList[GD12_COMMAND_LIST_DIRECT]->ClearRenderTargetView(SurfaceHandle, CC, 0, NULL);

	SurfaceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	SurfaceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	pCommandList[GD12_COMMAND_LIST_DIRECT]->ResourceBarrier(1, &SurfaceBarrier);

	//		RENDER	

	pCommandList[GD12_COMMAND_LIST_DIRECT]->ResourceBarrier(1, &Barrier);
	pCommandList[GD12_COMMAND_LIST_DIRECT]->OMSetRenderTargets(1, &RTVHandle, FALSE, NULL);
	pCommandList[GD12_COMMAND_LIST_DIRECT]->ClearRenderTargetView(RTVHandle, ClearColor, 0, NULL);



	//		RECT
	if (pVertexBufferQueueRect->InstanceCount)
	{
		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetPipelineState(pPipelineState[GD12_PIPELINE_STATE_RECT]);
		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetGraphicsRootSignature(pRootSignature[GD12_ROOT_SIGNATURE_RECT]);
		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetGraphicsRootConstantBufferView(0, pConstantBuffer[GD12_CONSTANT_BUFFER_CAMERA]->GetGPUVirtualAddress());

		pVertexBufferQueueRect->Update();

		pCommandList[GD12_COMMAND_LIST_DIRECT]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		pCommandList[GD12_COMMAND_LIST_DIRECT]->IASetVertexBuffers(0, 1, &pVertexBufferQueueRect->SRV);

		pCommandList[GD12_COMMAND_LIST_DIRECT]->DrawInstanced(pVertexBufferQueueRect->InstanceCount, 1, 0, 0);

		pVertexBufferQueueRect->Reset();
	}

	//		TEXTURE
	if (pVertexBufferQueueTexture->InstanceCount)
	{
		IGD12Texture* pTexture = apTexture[0];

		pVertexBufferQueueTexture->Update();

		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetPipelineState(pPipelineState[GD12_PIPELINE_STATE_TEXTURE]);
		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetGraphicsRootSignature(pRootSignature[GD12_ROOT_SIGNATURE_TEXTURE]);
		pCommandList[GD12_COMMAND_LIST_DIRECT]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		ID3D12DescriptorHeap* pp[2]{ pTexture->pDescriptor, pDescriptor[GD12_DESCRIPTOR_SAMPLER] };
		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetDescriptorHeaps(2, pp);

		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetGraphicsRootDescriptorTable(1, pDescriptor[GD12_DESCRIPTOR_SAMPLER]->GetGPUDescriptorHandleForHeapStart());

		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetGraphicsRootConstantBufferView(0, pConstantBuffer[GD12_CONSTANT_BUFFER_CAMERA]->GetGPUVirtualAddress());

		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetGraphicsRootConstantBufferView(2, pTexture->pDataBuffer->GetGPUVirtualAddress());
		pCommandList[GD12_COMMAND_LIST_DIRECT]->SetGraphicsRootDescriptorTable(3, pTexture->pDescriptor->GetGPUDescriptorHandleForHeapStart());


		pCommandList[GD12_COMMAND_LIST_DIRECT]->IASetVertexBuffers(0, 1, &pVertexBufferQueueTexture->SRV);

		pCommandList[GD12_COMMAND_LIST_DIRECT]->DrawInstanced(pVertexBufferQueueTexture->InstanceCount, 1, 0, 0);

		pVertexBufferQueueTexture->Reset();
	}

	//		FINISH

	Barrier.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;

	pCommandList[GD12_COMMAND_LIST_DIRECT]->ResourceBarrier(1, &Barrier);

	ExecuteList(pCommandList[GD12_COMMAND_LIST_DIRECT]);

	HRESULT hr = pSwapChain->Present(1, 0);
	FrameIndex = FrameIndex xor 1;

	if FAILED(hr)
	{
		DebugLog(L"\nERROR::IGD12:Render:  PRESENT FAILED FAILED");
		return hr;
	}

	return S_OK;
}
HRESULT		IGD12::CreateBuffer							(UINT BufferSize, ID3D12Resource** ppBuffer, GD12_BUFFER_TYPE Type)
{
	if (BufferSize == 0 || (*ppBuffer) || !ppBuffer)
		return E_INVALIDARG;

	D3D12_HEAP_PROPERTIES HeapProperties{};
	D3D12_RESOURCE_DESC ResourceDesc{};

	HeapProperties.CreationNodeMask = 0;
	HeapProperties.VisibleNodeMask = 0;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;


	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = BufferSize;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	switch (Type)
	{
	case GD12_BUFFER_TYPE_UPLOAD:

		HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		return pDevice->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(ppBuffer));

	case GD12_BUFFER_TYPE_CONSTANT:

		HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		return pDevice->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL, IID_PPV_ARGS(ppBuffer));

	default:

		return E_INVALIDARG;
	};
}
HRESULT		IGD12::CreateCommandList					(ID3D12GraphicsCommandList** ppCommandList, D3D12_COMMAND_LIST_TYPE Type, ID3D12CommandAllocator** ppCommandAllocator, ID3D12PipelineState* pState)
{
	if (!pDevice)
		E_NOINTERFACE;

	if (*ppCommandList)
		return E_INVALIDARG;

	if (!*ppCommandAllocator)
	{
		if FAILED(pDevice->CreateCommandAllocator(Type, IID_PPV_ARGS(ppCommandAllocator)))
		{
			DebugLog(L"\nERROR::IGD12::CreateCommandList:  FAILED TO CREATE COMMAND ALLOCATOR");
			return E_FAIL;
		}
	}

	if FAILED(pDevice->CreateCommandList(0, Type, *ppCommandAllocator, pState, IID_PPV_ARGS(ppCommandList)))
	{
		DebugLog(L"\nERROR::IGD12::CreateCommandList:  FAILED TO CREATE COMMAND LIST");
		SafeRelease(ppCommandAllocator);
		return E_FAIL;
	}

	if FAILED((*ppCommandList)->Close())
	{
		DebugLog(L"\nERROR::IGD12::CreateCommandList:  FAILED TO CLOSE COMMAND LIST");
		SafeRelease(ppCommandAllocator);
		SafeRelease(ppCommandList);
		return E_FAIL;
	}

	return S_OK;
}
HRESULT		IGD12::UpdateBuffer							(ID3D12Resource* pBuffer, void* pData, const UINT Bytewidth)
{
	if (!pBuffer || !pData || !Bytewidth)
		return E_INVALIDARG;
		

	D3D12_RESOURCE_DESC Desc{ pBuffer->GetDesc() };

	if (Desc.Width < Bytewidth)
		return E_INVALIDARG;

	void* pMapPointer	{ NULL };

	if FAILED(pBuffer->Map(0, NULL, &pMapPointer))
	{
		DebugLog(L"\nERROR::IGD12::UpdateBuffer:  FAILED TO MAP BUFFER");
		return E_FAIL;
	}

	memcpy(pMapPointer, pData, Bytewidth);
	pBuffer->Unmap(0, NULL);

	return S_OK;
}
HRESULT		IGD12::UpdateConstantBuffer					(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pDestResource, ID3D12Resource* pSrcResource, const UINT Bytewidth)
{																							 
	D3D12_RESOURCE_BARRIER Barrier;															 
	Barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;				 
	Barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;						 
	Barrier.Transition.pResource	= pDestResource;										 
	Barrier.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;				 
	Barrier.Transition.StateBefore	= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;		 
	Barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_COPY_DEST;						 
																							 																						
	pCommandList->ResourceBarrier(1, &Barrier);					 
	pCommandList->CopyBufferRegion(pDestResource, 0, pSrcResource, 0, Bytewidth);
																							 
	Barrier.Transition.StateBefore	= D3D12_RESOURCE_STATE_COPY_DEST;						 
	Barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;		 
																							 
	pCommandList->ResourceBarrier(1, &Barrier);					 
	
	ExecuteList(pCommandList);

	return S_OK;																			 
}																							 
HRESULT		IGD12::UpdateTexture						(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pDestResource, ID3D12Resource* pSrcResource, const UINT ByteWidth, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* FootPrint)
{
	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource	= pDestResource;
	Barrier.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	Barrier.Transition.StateBefore	= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	Barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_TEXTURE_COPY_LOCATION Dest;

	Dest.pResource					= pDestResource;
	Dest.Type						= D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	Dest.SubresourceIndex			= 0;

	D3D12_TEXTURE_COPY_LOCATION Src;

	Src.pResource					= pSrcResource;
	Src.Type						= D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	Src.PlacedFootprint				= *FootPrint;

	pCommandList->ResourceBarrier(1, &Barrier);
	pCommandList->CopyTextureRegion(&Dest, 0, 0, 0, &Src, NULL);

	Barrier.Transition.StateBefore	= D3D12_RESOURCE_STATE_COPY_DEST;
	Barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;;

	pCommandList->ResourceBarrier(1, &Barrier);
	
	ExecuteList(pCommandList);

	return S_OK;
}
HRESULT		IGD12::CreateTextureFromFile				(LPCWCH FileName, IGD12Texture** ppTexture)
{
	IGD12RawImage*						pImage			{ NULL };
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT	FootPrint;
	D3D12_HEAP_PROPERTIES				HeapProperties;
	D3D12_RESOURCE_DESC					ResourceDesc;
	
	if (FileName == NULL || ppTexture == NULL || (*ppTexture != NULL))
		return E_INVALIDARG;

	if FAILED(pWICImageProcessor->DecodeImageToPixData(FileName, DXGIFormat, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, &pImage))
	{
		DebugLog(L"\nERROR::IGD12::CreateTextureFromFile:  FAILED TO DECOD");
		return E_FAIL;
	}

	(*ppTexture) = new IGD12Texture;

	HeapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.CreationNodeMask		= 0;
	HeapProperties.VisibleNodeMask		= 0;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.Type					= D3D12_HEAP_TYPE_DEFAULT;

	ResourceDesc.Format					= DXGIFormat;
	ResourceDesc.Dimension				= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.MipLevels				= 1;
	ResourceDesc.Alignment				= 0;
	ResourceDesc.DepthOrArraySize		= 1;
	ResourceDesc.Width					= pImage->width;
	ResourceDesc.Height					= pImage->height;
	ResourceDesc.Flags					= D3D12_RESOURCE_FLAG_NONE;
	ResourceDesc.Layout					= D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count		= 1;
	ResourceDesc.SampleDesc.Quality		= 0;

	FootPrint.Offset					= 0;
	FootPrint.Footprint.Depth			= 1;
	FootPrint.Footprint.Format			= DXGIFormat;
	FootPrint.Footprint.Height			= pImage->height;
	FootPrint.Footprint.Width			= pImage->width;
	FootPrint.Footprint.RowPitch		= pImage->RowPitch;

	float Data[6]{ 0, 0, 1, 1, (float)pImage->width, (float)pImage->height };


	//		CREATE DATA BUFFER AND TEXTURE
	if FAILED(pDevice->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, NULL, IID_PPV_ARGS(&(*ppTexture)->pBuffer)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTextureFromFile:  FAILED TO CREATE TEXTURE");
		SafeRelease(ppTexture);
		SafeRelease(&pImage);
		return E_FAIL;
	}
	if FAILED(CreateBuffer(256, &(*ppTexture)->pDataBuffer, GD12_BUFFER_TYPE_CONSTANT))
	{
		DebugLog(L"\nERROR::IGD12::CREATE TEXTURE FROM FILE:  FAILED TO CREATE SETTING BUFFER");
		SafeRelease(ppTexture);
		SafeRelease(&pImage);
		return E_FAIL;
	}


	//		UPDATE DATA BUFFER AND TEXTURE
	if FAILED(UpdateBuffer(pUploadBuffer, pImage->pixelData, pImage->bytewidth))
	{
		DebugLog(L"\nERROR::IGD12::CreateTextureFromFile:  FAILED TO CREATE TEXTURE");
		SafeRelease(ppTexture);
		SafeRelease(&pImage);
		return E_FAIL;
	}
	if FAILED(UpdateTexture(pCommandList[GD12_COMMAND_LIST_DIRECT], (*ppTexture)->pBuffer, pUploadBuffer, pImage->bytewidth, &FootPrint))
	{
		DebugLog(L"\nERROR::IGD12::CreateTextureFromFile:  FAILED TO CREATE TEXTURE");
		SafeRelease(ppTexture);
		SafeRelease(&pImage);
		return E_FAIL;
	}
	if FAILED(UpdateBuffer(pUploadBuffer, Data, sizeof(Data)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTextureFromFile:  FAILED TO UPDATE BUFFER");
		SafeRelease(ppTexture);
		SafeRelease(&pImage);
		return E_FAIL;
	}
	if FAILED(UpdateConstantBuffer(pCommandList[GD12_COMMAND_LIST_DIRECT], (*ppTexture)->pDataBuffer, pUploadBuffer, sizeof(Data)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTextureFromFile:  FAILED TO UPDATE BUFFER");
		SafeRelease(ppTexture);
		SafeRelease(&pImage);
		return E_FAIL;
	}

	SafeRelease(&pImage);


	//		DESCRIPTOR
	D3D12_DESCRIPTOR_HEAP_DESC DHDesc;
	DHDesc.NumDescriptors	= 2;
	DHDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DHDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DHDesc.NodeMask			= 0;

	if FAILED(pDevice->CreateDescriptorHeap(&DHDesc, IID_PPV_ARGS(&(*ppTexture)->pDescriptor)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTextureFromFile:  FAILED TO CREATE DESCRIPTOR HEAP");
		SafeRelease(ppTexture);
		return E_FAIL;
	}


	//		BUFFER VIEW
	D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = (*ppTexture)->pDescriptor->GetCPUDescriptorHandleForHeapStart();

	//		TEXTURE DATA VIEW
	D3D12_CONSTANT_BUFFER_VIEW_DESC			ConstantBufferViewDesc{};

	ConstantBufferViewDesc.BufferLocation	= (*ppTexture)->pDataBuffer->GetGPUVirtualAddress();
	ConstantBufferViewDesc.SizeInBytes		= 256;

	pDevice->CreateConstantBufferView(&ConstantBufferViewDesc, CPUDescriptorHandle);


	//		TEXTURE VIEW
	CPUDescriptorHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	pDevice->CreateShaderResourceView((*ppTexture)->pBuffer, NULL, CPUDescriptorHandle);

	return S_OK;
}
HRESULT		IGD12::CreateTexture						(GD12_TEXTURE_DESC* pDesc, IGD12Texture** ppTexture)
{
	D3D12_HEAP_PROPERTIES				HeapProperties;
	D3D12_RESOURCE_DESC					ResourceDesc;

	if (pDesc == NULL || ppTexture == NULL || (*ppTexture != NULL))
		return E_INVALIDARG;

	(*ppTexture) = new IGD12Texture;

	HeapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.CreationNodeMask		= 0;
	HeapProperties.VisibleNodeMask		= 0;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.Type					= D3D12_HEAP_TYPE_DEFAULT;

	ResourceDesc.Format					= DXGIFormat;
	ResourceDesc.Dimension				= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.MipLevels				= 1;
	ResourceDesc.Alignment				= 0;
	ResourceDesc.DepthOrArraySize		= 1;
	ResourceDesc.Width					= pDesc->Width;
	ResourceDesc.Height					= pDesc->Height;
	ResourceDesc.Flags					= pDesc->FLAG;
	ResourceDesc.Layout					= D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count		= 1;
	ResourceDesc.SampleDesc.Quality		= 0;

	float Data[6]{ 0, 0, 1, 1, (float)pDesc->Width, (float)pDesc->Height };


	//		CREATE DATA BUFFER AND TEXTURE
	if FAILED(pDevice->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, NULL, IID_PPV_ARGS(&(*ppTexture)->pBuffer)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO CREATE TEXTURE");
		SafeRelease(ppTexture);
		return E_FAIL;
	}
	if FAILED(CreateBuffer(256, &(*ppTexture)->pDataBuffer, GD12_BUFFER_TYPE_CONSTANT))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO CREATE SETTING BUFFER");
		SafeRelease(ppTexture);
		return E_FAIL;
	}


	//		UPDATE DATA BUFFER AND TEXTURE

	if FAILED(UpdateBuffer(pUploadBuffer, Data, sizeof(Data)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO UPDATE BUFFER");
		SafeRelease(ppTexture);
		return E_FAIL;
	}
	if FAILED(UpdateConstantBuffer(pCommandList[GD12_COMMAND_LIST_DIRECT], (*ppTexture)->pDataBuffer, pUploadBuffer, sizeof(Data)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO UPDATE BUFFER");
		SafeRelease(ppTexture);
		return E_FAIL;
	}


	//		DESCRIPTOR
	D3D12_DESCRIPTOR_HEAP_DESC DHDesc;
	DHDesc.NumDescriptors = 2;
	DHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DHDesc.NodeMask = 0;

	if FAILED(pDevice->CreateDescriptorHeap(&DHDesc, IID_PPV_ARGS(&(*ppTexture)->pDescriptor)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO CREATE DESCRIPTOR HEAP");
		SafeRelease(ppTexture);
		return E_FAIL;
	}


	//		BUFFER VIEW
	D3D12_CPU_DESCRIPTOR_HANDLE		CPUDescriptorHandle = (*ppTexture)->pDescriptor->GetCPUDescriptorHandleForHeapStart();

	//		TEXTURE DATA VIEW
	D3D12_CONSTANT_BUFFER_VIEW_DESC	ConstantBufferViewDesc{};

	ConstantBufferViewDesc.BufferLocation	= (*ppTexture)->pDataBuffer->GetGPUVirtualAddress();
	ConstantBufferViewDesc.SizeInBytes		= 256;

	pDevice->CreateConstantBufferView(&ConstantBufferViewDesc, CPUDescriptorHandle);


	//		TEXTURE VIEW
	CPUDescriptorHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	pDevice->CreateShaderResourceView((*ppTexture)->pBuffer, NULL, CPUDescriptorHandle);

	return S_OK;
}
HRESULT		IGD12::CreateSurface(GD12_TEXTURE_DESC* pDesc, IGD12Surface** ppSurface)
{
	D3D12_HEAP_PROPERTIES				HeapProperties;
	D3D12_RESOURCE_DESC					ResourceDesc;

	if (pDesc == NULL || ppSurface == NULL || (*ppSurface != NULL))
		return E_INVALIDARG;

	(*ppSurface) = new IGD12Surface;

	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.CreationNodeMask = 0;
	HeapProperties.VisibleNodeMask = 0;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ResourceDesc.Format = DXGIFormat;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Alignment = 0;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.Width = pDesc->Width;
	ResourceDesc.Height = pDesc->Height;
	ResourceDesc.Flags = pDesc->FLAG;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;

	float Data[6]{ 0, 0, 1, 1, (float)pDesc->Width, (float)pDesc->Height };


	//		CREATE DATA BUFFER AND TEXTURE
	//Probably should add cleare value
	if FAILED(pDevice->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, NULL, IID_PPV_ARGS(&(*ppSurface)->pBuffer)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO CREATE TEXTURE");
		SafeRelease(ppSurface);
		return E_FAIL;
	}
	if FAILED(CreateBuffer(256, &(*ppSurface)->pDataBuffer, GD12_BUFFER_TYPE_CONSTANT))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO CREATE SETTING BUFFER");
		SafeRelease(ppSurface);
		return E_FAIL;
	}


	//		UPDATE DATA BUFFER AND TEXTURE

	if FAILED(UpdateBuffer(pUploadBuffer, Data, sizeof(Data)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO UPDATE BUFFER");
		SafeRelease(ppSurface);
		return E_FAIL;
	}
	if FAILED(UpdateConstantBuffer(pCommandList[GD12_COMMAND_LIST_DIRECT], (*ppSurface)->pDataBuffer, pUploadBuffer, sizeof(Data)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO UPDATE BUFFER");
		SafeRelease(ppSurface);
		return E_FAIL;
	}


	//		SRV DESCRIPTOR
	D3D12_DESCRIPTOR_HEAP_DESC DHDesc;
	DHDesc.NumDescriptors = 2;
	DHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DHDesc.NodeMask = 0;

	if FAILED(pDevice->CreateDescriptorHeap(&DHDesc, IID_PPV_ARGS(&(*ppSurface)->pDescriptor)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO CREATE DESCRIPTOR HEAP");
		SafeRelease(ppSurface);
		return E_FAIL;
	}


	//		RTV DESCRIPTOR

	DHDesc.NumDescriptors = 1;
	DHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DHDesc.NodeMask = 0;

	if FAILED(pDevice->CreateDescriptorHeap(&DHDesc, IID_PPV_ARGS(&(*ppSurface)->pDescriptorRTV)))
	{
		DebugLog(L"\nERROR::IGD12::CreateTexture:  FAILED TO CREATE RTV DESCRIPTOR HEAP");
		SafeRelease(ppSurface);
		return E_FAIL;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHandle { (*ppSurface)->pDescriptorRTV->GetCPUDescriptorHandleForHeapStart()};
	pDevice->CreateRenderTargetView((*ppSurface)->pBuffer, NULL, RTVDescriptorHandle);


	//		BUFFER VIEW
	D3D12_CPU_DESCRIPTOR_HANDLE		CPUDescriptorHandle = (*ppSurface)->pDescriptor->GetCPUDescriptorHandleForHeapStart();

	//		TEXTURE DATA VIEW
	D3D12_CONSTANT_BUFFER_VIEW_DESC	ConstantBufferViewDesc{};

	ConstantBufferViewDesc.BufferLocation = (*ppSurface)->pDataBuffer->GetGPUVirtualAddress();
	ConstantBufferViewDesc.SizeInBytes = 256;

	pDevice->CreateConstantBufferView(&ConstantBufferViewDesc, CPUDescriptorHandle);


	//		TEXTURE VIEW
	CPUDescriptorHandle.ptr += pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	pDevice->CreateShaderResourceView((*ppSurface)->pBuffer, NULL, CPUDescriptorHandle);

	return S_OK;
}
HRESULT		IGD12::CreateVertexBufferQueue				(IGD12VertexBufferQueue** ppBuffer, UINT Size, UINT Stride)
{
	if (*ppBuffer != NULL || Size == 0 || Stride == 0 || Size * Stride < 64)
		return E_INVALIDARG;

	(*ppBuffer) = new IGD12VertexBufferQueue;

	if FAILED(CreateBuffer(Size * Stride, &(*ppBuffer)->pBuffer, GD12_BUFFER_TYPE_UPLOAD))
	{
		SafeRelease(ppBuffer);
		return E_FAIL;
	}

	(*ppBuffer)->pQueueData			= new BYTE[Size * Stride];
	(*ppBuffer)->SRV.BufferLocation = (*ppBuffer)->pBuffer->GetGPUVirtualAddress();
	(*ppBuffer)->SRV.SizeInBytes	= Size * Stride;
	(*ppBuffer)->SRV.StrideInBytes	= Stride;
	(*ppBuffer)->Size				= Size;

	return S_OK;
};
HRESULT		IGD12::ResizeVertexBufferQueue				(IGD12VertexBufferQueue* pVBQ, UINT Size)
{
	if (pVBQ == NULL || Size == 0)
		return E_INVALIDARG;

	UINT OldSize = pVBQ->Size;
	UINT Stride = pVBQ->SRV.StrideInBytes;

	if (OldSize == Size)
		return S_OK;

	ID3D12Resource*	pNewBuffer	{ NULL };

	if FAILED(CreateBuffer(Size * Stride, &pNewBuffer, GD12_BUFFER_TYPE_UPLOAD))
		return E_FAIL;

	BYTE* pNewQueueData = new BYTE[Size * Stride];

	memcpy(pNewQueueData, pVBQ, (Size > OldSize ? OldSize : Size) * Stride);

	delete[] pVBQ->pQueueData;
	SafeRelease(&pVBQ->pBuffer);

	pVBQ->pQueueData			= pNewQueueData;
	pVBQ->pBuffer				= pNewBuffer;
	pVBQ->SRV.SizeInBytes		= Size * Stride;
	pVBQ->SRV.BufferLocation	= pNewBuffer->GetGPUVirtualAddress();
	pVBQ->Size					= Size;
	
	return S_OK;
}

//		USER FUNCTIONS
void		IGD12::DrawRect								(GD12_SHAPE_RECT* pRect)
{
	if (pVertexBufferQueueRect->Size <= pVertexBufferQueueRect->InstanceCount)
		ResizeVertexBufferQueue(pVertexBufferQueueRect, pVertexBufferQueueRect->Size + 10);

	pVertexBufferQueueRect->AddInstance(pRect, 1);
}
int			IGD12::CreateTexture						(LPCWCH FileName)
{
	if (FileName == NULL)
		return -1;

	IGD12Texture* pTexture { NULL };

	if FAILED(CreateTextureFromFile(FileName, &pTexture))
	{
		return -1;
	}

	if (apTexture)
	{
		IGD12Texture** apTextureNew = new IGD12Texture*[TextureCount + 1];

		memcpy(apTextureNew, apTexture, sizeof(IGD12Texture*) * TextureCount);
		delete[] apTexture;
		apTextureNew[TextureCount] = pTexture;
		
		apTexture = apTextureNew;
	}
	else
	{
		apTexture = new IGD12Texture*[1];
		apTexture[0] = pTexture;
	}

	TextureCount++;

	return TextureCount - 1;
}
void		IGD12::DrawTexture							(GD12_SHAPE_TEXTURE* pShape, int id)
{
	if (pVertexBufferQueueTexture->Size <= pVertexBufferQueueTexture->InstanceCount)
		ResizeVertexBufferQueue(pVertexBufferQueueTexture, pVertexBufferQueueTexture->Size + 10);

	pVertexBufferQueueTexture->AddInstance(pShape, 1);

	return;
}

HRESULT		IWICImageProcessor::Initialize				()
{
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
	if FAILED(hr)
	{
		DebugLog(L"\nERROR:: IWICImageProcessor:Initialize:	FALIED TO CREATE WIC FACTORY");
		Clear();
		return hr;
	}

	return hr;
}
HRESULT		IWICImageProcessor::DecodeImageToPixData	(LPCWSTR FileName, DXGI_FORMAT DXGIFormat, UINT Alignment, IGD12RawImage** ppGD12RawImage)
{
	HRESULT					hr;
	IWICBitmapDecoder*		pDecoder		{ NULL };
	IWICBitmapFrameDecode*	pFrameDecode	{ NULL };
	WICPixelFormatGUID		GUIDFormat		{ DXGIToGUID(DXGIFormat) };
	UINT					PixelSize		{ DXGIToPixelSize(DXGIFormat) };

	if ((ppGD12RawImage == NULL) || (FileName == NULL) || (pFactory == NULL) || ((*ppGD12RawImage) != NULL))
	{
		if (pFactory == NULL)
			return E_NOINTERFACE;

		return E_INVALIDARG;
	}
	if (GUIDFormat == GUID_WICPixelFormatUndefined || PixelSize == 0)
	{
		DebugLog(L"\nERROR::IWICImageProcessor:DecodeImageToPixData:		INVALID FORMAT\n");
		return E_INVALIDARG;
	}


	hr = pFactory->CreateDecoderFromFilename(FileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
	if FAILED(hr)
	{
		DebugLog(L"\nERROR::IWICImageProcessor:DecodeImageToPixData:		FAILED TO CREATE DOCODER FOR ");
		DebugLog(FileName);
		return hr;
	}

	hr = pDecoder->GetFrame(0, &pFrameDecode);
	if FAILED(hr)
	{
		DebugLog(L"\nERROR::IWICImageProcessor:DecodeImageToPixData: FAILED TO CREATE FRAME");
		SafeRelease(&pDecoder);
		return hr;
	}

	WICPixelFormatGUID DecodeFormat;
	hr = pFrameDecode->GetPixelFormat(&DecodeFormat);
	if FAILED(hr)
	{
		DebugLog(L"\nERROR::IWICImageProcessor:DecodeImageToPixData: FAILED TO GET PIXEL FORMAT");
		SafeRelease(&pFrameDecode);
		SafeRelease(&pDecoder);
		return hr;
	}


	UINT Width, Height, BufferSize;


	//Convert if needed
	if (DecodeFormat != GUIDFormat)
	{
		IWICFormatConverter* pFormatConverter{ NULL };

		hr = pFactory->CreateFormatConverter(&pFormatConverter);
		if FAILED(hr)
		{
			SafeRelease(&pFrameDecode);
			SafeRelease(&pDecoder);
			return hr;
		}

		hr = pFormatConverter->Initialize(pFrameDecode, GUIDFormat, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom);
		if FAILED(hr)
		{
			SafeRelease(&pFrameDecode);
			SafeRelease(&pDecoder);
			SafeRelease(&pFormatConverter);
			DebugLog(L"\nERROR::IWICImageProcessor:DecodeImageToPixData: FAILED TO CONVERT ");
			DebugLog(FileName);

			return hr;
		}

		
		pFormatConverter->GetSize(&Width, &Height);
		UINT RowPitch = Width * PixelSize + (Alignment - ((Width * PixelSize) % Alignment));
		BufferSize = RowPitch * Height;
		

		(*ppGD12RawImage) = new IGD12RawImage;

		(*ppGD12RawImage)->pixelData	= new BYTE[BufferSize];
		(*ppGD12RawImage)->height		= Height;
		(*ppGD12RawImage)->width		= Width;
		(*ppGD12RawImage)->pixelFormat	= DXGIFormat;
		(*ppGD12RawImage)->RowPitch		= RowPitch;
		(*ppGD12RawImage)->Alignment	= Alignment;
		(*ppGD12RawImage)->bytewidth	= BufferSize;

		hr = pFormatConverter->CopyPixels(NULL, RowPitch, BufferSize, (*ppGD12RawImage)->pixelData);
		if FAILED(hr)
		{
			SafeRelease(ppGD12RawImage);
		}

		SafeRelease(&pFrameDecode);
		SafeRelease(&pDecoder);
		SafeRelease(&pFormatConverter);
		return hr;
	}

	pFrameDecode->GetSize(&Width, &Height);
	UINT RowPitch = Width * PixelSize + (Alignment - ((Width * PixelSize) % Alignment));
	BufferSize = RowPitch * Height;
	

	(*ppGD12RawImage) = new IGD12RawImage;

	(*ppGD12RawImage)->pixelData	= new BYTE[BufferSize];
	(*ppGD12RawImage)->height		= Height;
	(*ppGD12RawImage)->width		= Width;
	(*ppGD12RawImage)->pixelFormat	= DXGIFormat;
	(*ppGD12RawImage)->RowPitch		= RowPitch;
	(*ppGD12RawImage)->bytewidth	= BufferSize;

	hr = pFrameDecode->CopyPixels(NULL, RowPitch, BufferSize, (*ppGD12RawImage)->pixelData);
	if FAILED(hr)
	{
		SafeRelease(&pFrameDecode);
		SafeRelease(&pDecoder);
		SafeRelease(ppGD12RawImage);
	}

	SafeRelease(&pFrameDecode);
	SafeRelease(&pDecoder);

	return hr;
}