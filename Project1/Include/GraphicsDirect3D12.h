#pragma once
#ifndef GRAPHICS_DIRECT_3D_12
#define GRAPHICS_DIRECT_3D_12

#include <CommonDirectX.h>
#include <D3D12.h>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")

#define IGD12_FLAG_INITIALIZED 1

class IGD12;
class IWICImageProcessor;

struct GD12_SHAPE_TEXTURE
{
	float x;
	float y;
	float depth;
};
struct GD12_SHAPE_RECT
{
	float	x;
	float	y;
	float	Width;
	float	Height;
	float	Color[4];
};
struct GD12_TEXTURE_DESC
{
	UINT Width;
	UINT Height;
	D3D12_RESOURCE_FLAGS FLAG;
};

class IGD12Texture : public Reference
{
protected:

	ID3D12DescriptorHeap*	pDescriptor { NULL };
	ID3D12Resource*			pBuffer		{ NULL };
	ID3D12Resource*			pDataBuffer	{ NULL };

	IGD12Texture() {};

private:

	~IGD12Texture()
	{
		SafeRelease(&pDescriptor);
		SafeRelease(&pBuffer);
		SafeRelease(&pDataBuffer);
	}

	friend class IGD12;
	friend class IGD12Surface;
};

class IGD12Surface : public IGD12Texture
{
protected:

	ID3D12DescriptorHeap*	pDescriptorRTV { NULL };

	IGD12Surface() {};

private:

	~IGD12Surface()
	{
		SafeRelease(&pDescriptorRTV);
	}

	friend class IGD12;
};

HRESULT CreateWICImageProcessor		(IWICImageProcessor** ppWICImageProcessor);
HRESULT CreateIGD12					(HWND Handle, IGD12** ppGraphics);

class IGD12VertexBufferQueue : public Reference
{
protected:

	//Constant size should change to dynamic option

	IGD12VertexBufferQueue() {};

	UINT						InstanceCount	{ 0 };
	UINT						Size			{ 0 };

	BYTE*						pQueueData		{ NULL };

	ID3D12Resource*				pBuffer			{ NULL };
	D3D12_VERTEX_BUFFER_VIEW	SRV				{ 0, 0, 0 };

	void Reset()
	{
		InstanceCount = 0;
	};

	HRESULT AddInstance(void* pData, UINT InstanceCount)
	{
		if (this->InstanceCount + InstanceCount > Size)
		{
			DebugLog(L"\nERROR:: IGD12VertexBufferQueue :: Buffer you are trying to uppend is too small");
			return E_FAIL;
		};

		memcpy(pQueueData + SRV.StrideInBytes * this->InstanceCount, pData, SRV.StrideInBytes * InstanceCount);
		this->InstanceCount += InstanceCount;

		return S_OK;
	};
	HRESULT Update()
	{
		void* ptr { NULL };

		if FAILED(pBuffer->Map(0, NULL, &ptr))
			return E_FAIL;

		memcpy(ptr, pQueueData, InstanceCount * SRV.StrideInBytes);

		pBuffer->Unmap(0, NULL);

		return S_OK;
	}

private:

	~IGD12VertexBufferQueue()
	{
		if (pQueueData)
			delete[] pQueueData;

		SafeRelease(&pBuffer);
	};

	friend class IGD12;
};

class IGD12RawImage : public Reference
{
public:
	UINT		width		{ 0 };
	UINT		height		{ 0 };
	UINT		bytewidth	{ 0 };
	UINT		RowPitch	{ 0 };
	UINT		Alignment	{ 0 };
	DXGI_FORMAT	pixelFormat { DXGI_FORMAT_UNKNOWN };
	BYTE*		pixelData	{ NULL };

protected:

	IGD12RawImage() {};

private:

	~IGD12RawImage()
	{
		if (pixelData)
		{
			delete[] pixelData;
		}
	}

	friend class IWICImageProcessor;
};
class IWICImageProcessor : public Reference
{
public:

	HRESULT		DecodeImageToPixData(LPCWSTR FileName, DXGI_FORMAT DXGIFormat, UINT Alignment, IGD12RawImage** ppGD12RawImage);

protected:

	IWICImagingFactory* pFactory{ NULL };

	IWICImageProcessor()
	{
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	}

	HRESULT Initialize();

private:
	~IWICImageProcessor()
	{
		Clear();
		CoUninitialize();
	}

	void Clear()
	{
		SafeRelease(&pFactory);
	}

	friend HRESULT CreateWICImageProcessor(IWICImageProcessor** ppWICImageProcessor);
};

class IGD12: public Reference
{
public:
	
	enum GD12_BUFFER_TYPE
	{
		GD12_BUFFER_TYPE_UPLOAD			= 0,
		GD12_BUFFER_TYPE_CONSTANT		= 1
	};
	enum GD12_DESCRIPTOR
	{
		GD12_DESCRIPTOR_RTV				= 0,
		GD12_DESCRIPTOR_SAMPLER			= 1,
		GD12_DESCRIPTOR_SETTINGS		= 2
	};
	enum GD12_COMMAND_LIST
	{
		GD12_COMMAND_LIST_DIRECT		= 0,
		GD12_COMMAND_LIST_COPY			= 1
	};
	enum GD12_COMMAND_ALLOCATOR
	{
		GD12_COMMAND_ALLOCATOR_DIRECT	= 0,
		GD12_COMMAND_ALLOCATOR_COPY		= 1
	};
	enum GD12_PIPELINE_STATE
	{
		GD12_PIPELINE_STATE_TEXTURE = 0,
		GD12_PIPELINE_STATE_RECT = 1
	};
	enum GD12_ROOT_SIGNATURE
	{
		GD12_ROOT_SIGNATURE_TEXTURE = 0,
		GD12_ROOT_SIGNATURE_RECT = 1
	};
	enum GD12_CONSTANT_BUFFER
	{
		GD12_CONSTANT_BUFFER_CAMERA = 0
	};

	const DXGI_FORMAT			DXGIFormat			{ DXGI_FORMAT_R8G8B8A8_UNORM };
	float						ClearColor[4]		{ 0.0f, 0.0f, 0.0f, 1.0f };

	//		USER FUNCTIONS
	HRESULT Render			();
	void	DrawRect		(GD12_SHAPE_RECT* pRect);
	int		CreateTexture	(LPCWCH FileName);
	void	DrawTexture		(GD12_SHAPE_TEXTURE* pShape, int id);

protected:
	
	IWICImageProcessor*			pWICImageProcessor			{ NULL };

	IDXGIFactory*				pDXGIFactory				{ NULL };
	IDXGISwapChain*				pSwapChain					{ NULL };

	//Surface test

	IGD12Surface*				pSurface					{ NULL };

	//		TEXTURE
	IGD12Texture**				apTexture					{ NULL };
	UINT						TextureCount				{ 0 };


	const UINT					CommandAllocatorCount		{ 2 };
	const UINT					CommandListCount			{ 2 };
	const UINT					DescriptorHeapCount			{ 3 };
	const UINT					RenderTargetCount			{ 2 };
	const UINT					PipelineStateCount			{ 2 };
	const UINT					RootSignatureCount			{ 2 };
	const UINT					ConstantBufferCount			{ 1 };


	ID3D12Debug*				pDebug						{ NULL };
	ID3D12Device*				pDevice						{ NULL };
	ID3D12CommandAllocator*		pCommandAllocator[2]		{ NULL, NULL };
	ID3D12CommandQueue*			pCommandQueue				{ NULL };
	ID3D12GraphicsCommandList*	pCommandList[2]				{ NULL, NULL };
	ID3D12DescriptorHeap*		pDescriptor[3]				{ NULL, NULL, NULL };
	ID3D12RootSignature*		pRootSignature[2]			{ NULL, NULL };
	ID3D12PipelineState*		pPipelineState[2]			{ NULL, NULL };
	ID3D12Resource*				pRTV[2]						{ NULL, NULL };
	ID3D12Resource*				pConstantBuffer[1]			{ NULL };
	ID3D12Resource*				pUploadBuffer				{ NULL };


	IGD12VertexBufferQueue*		pVertexBufferQueueRect		{ NULL };
	IGD12VertexBufferQueue*		pVertexBufferQueueTexture	{ NULL };
	
	ID3D12Fence*				pFence						{ NULL };

	D3D12_CPU_DESCRIPTOR_HANDLE	RTVHandle[2]				{};
	D3D12_VERTEX_BUFFER_VIEW	VertexBufferView[2]			{};
	D3D12_INDEX_BUFFER_VIEW		IndexBufferView				{};
	D3D12_VIEWPORT				ViewPort					{};
	D3D12_RECT					ScissorRect					{};

	HWND						hwnd						{ NULL };
	HANDLE						FenceEvent					{ NULL };
	UINT						FenceValue					{ 0 };
	UINT						FrameIndex					{ 0 };

	UINT64						FLAG						{ 0 };

	HRESULT Initialize				(HWND Handle);
	HRESULT CreateBuffer			(UINT BufferSize, ID3D12Resource** ppBuffer, GD12_BUFFER_TYPE type);
	HRESULT CreateCommandList		(ID3D12GraphicsCommandList** ppCommandList, D3D12_COMMAND_LIST_TYPE Type, ID3D12CommandAllocator** ppCommandAllocator, ID3D12PipelineState* pState = NULL);
	HRESULT UpdateBuffer			(ID3D12Resource* pBuffer, void* pData, const UINT Bytewidth);
	HRESULT UpdateConstantBuffer	(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pDestResource, ID3D12Resource* pSrcResource, const UINT Bytewidth);
	HRESULT UpdateTexture			(ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pDestResource, ID3D12Resource* pSrcResource, const UINT ByteWidth, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* FootPrint);
	HRESULT CreateTexture			(GD12_TEXTURE_DESC* pDesc, IGD12Texture** ppTexture);
	HRESULT CreateTextureFromFile	(LPCWCH FileName, IGD12Texture** ppTexture);
	HRESULT CreateSurface			(GD12_TEXTURE_DESC* pDesc, IGD12Surface** ppSurface);
	HRESULT CreateVertexBufferQueue (IGD12VertexBufferQueue** ppBuffer, UINT Size, UINT Stride);
	HRESULT ResizeVertexBufferQueue (IGD12VertexBufferQueue* pVBQ, UINT Size);

	IGD12()
	{

	}
	~IGD12()
	{
		Clear();
	}

private:

	struct
	{
		float Width = 2.f/SETTING_RESOLUTION_WIDTH;
		float Height = 2.f/SETTING_RESOLUTION_HEIGHT;
		float pad[62];
	} SettingsBuffer;

	void Clear()
	{
		if (FLAG && IGD12_FLAG_INITIALIZED)
			WaitForPreviousFrame();

		SafeRelease(&pDXGIFactory);
		SafeRelease(&pDevice);
		SafeRelease(&pCommandQueue);
		SafeRelease(&pDebug);
		SafeRelease(&pSwapChain);
		SafeRelease(&pFence);
		SafeRelease(&pUploadBuffer);
		SafeRelease(&pWICImageProcessor);
		SafeRelease(&pVertexBufferQueueRect);

		SafeRelease(&pSurface);

		for (int i = 0; i < ConstantBufferCount; i++)
			SafeRelease(&pConstantBuffer[i]);

		for (int i = 0; i < CommandListCount; i++)
			SafeRelease(&pCommandList[i]);

		for (int i = 0; i < CommandAllocatorCount; i++)
			SafeRelease(&pCommandAllocator[i]);

		for (int i = 0; i < DescriptorHeapCount; i++)
			SafeRelease(&pDescriptor[i]);

		for (int i = 0; i < RenderTargetCount; i++)
			SafeRelease(&(pRTV[i]));

		for (int i = 0; i < PipelineStateCount; i++)
			SafeRelease(&pPipelineState[i]);

		for (int i = 0; i < RootSignatureCount; i++)
			SafeRelease(&pRootSignature[i]);

		for (UINT i = 0; i < TextureCount; i++)
			SafeRelease(&apTexture[i]);

		if (apTexture)
			delete[] apTexture;

		FLAG = 0;
	}

	void WaitForPreviousFrame();
	void ExecuteList(ID3D12GraphicsCommandList* pList);

	friend HRESULT CreateIGD12(HWND Handle, IGD12** ppGraphics);
};
#endif