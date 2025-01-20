#include "GraphicsDirectX.h"
#include "CommonDirectX.h"
#include "Resource.h"

#pragma warning(disable:26451; disable:6001; disable:6386)

//*******************************GraphicsBase*****************************************************
HRESULT		GraphicsBase::Initialize						()
{
	HRESULT hr {S_OK};

	//Device, DeviceContext and SwapChain
	{
		DXGI_SWAP_CHAIN_DESC	SCDesc;
		DXGI_MODE_DESC			ModeDesc;

		ModeDesc.Width				= Settings.width;
		ModeDesc.Height				= Settings.height;
		ModeDesc.RefreshRate		= { 60, 1 };
		ModeDesc.Format				= DxgiFormat;
		ModeDesc.ScanlineOrdering	= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		ModeDesc.Scaling			= DXGI_MODE_SCALING_UNSPECIFIED;

		SCDesc.BufferDesc			= ModeDesc;
		SCDesc.SampleDesc.Count		= 1;
		SCDesc.SampleDesc.Quality	= 0;
		SCDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SCDesc.BufferCount			= 1;
		SCDesc.OutputWindow			= hwnd;
		SCDesc.Windowed				= true;
		SCDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;
		SCDesc.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		UINT flags{ D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT };

#ifdef SETTING_DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		hr = D3D11CreateDeviceAndSwapChain
		(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			NULL,
			NULL,
			D3D11_SDK_VERSION,
			&SCDesc,
			&pSwapChain,
			&pDevice,
			pFeatureLevel,
			&pDeviceContext
		);
		if FAILED(hr)
		{
			DebugLog(L"\nFAILED to initialize GraphicsBase\nError at CreateDeviceAndSwapChain\n");
			return hr;
		}
	}

	//Get Back buffer
	{
		hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		if FAILED(hr)
		{
			DebugLog(L"\nFailed to create Back Buffer");

			return hr;
		}
	}

	//View and View port
	{
		//Create View
		hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRTView);
		if FAILED(hr)
		{
			DebugLog(L"\nFailed to create Render TArget View");
			return hr;
		}



		//Create View port
		D3D11_TEXTURE2D_DESC bbd;
		pBackBuffer->GetDesc(&bbd);

		ZeroMemory(&ViewPort, sizeof(D3D11_VIEWPORT));

		ViewPort.Height = (float) bbd.Height;
		ViewPort.Width	= (float) bbd.Width;
		ViewPort.MinDepth = 0.f;
		ViewPort.MaxDepth = 1.f;
	}

	return hr;
}
HRESULT		GraphicsBase::CreateVertexShaderAndInputLayout	(LPCWSTR CSOFile, D3D11_INPUT_ELEMENT_DESC* pIEDesc, UINT ElementNum, ID3D11InputLayout** ppInputLayout, ID3D11VertexShader** ppVShader)
{
	if(pDevice == NULL)
	{
		return E_NOINTERFACE;
	}
	if((CSOFile == NULL) || ((*ppVShader) != NULL) || (pIEDesc == NULL) || (*ppInputLayout != NULL))
	{
		return E_INVALIDARG;
	}

	ShaderBlob	SBlob;
	HRESULT		hr	{S_OK};

	hr = SBlob.ReadCSOFile(CSOFile);

	if FAILED(hr)
	{
		DebugLog(L"\nERROR::GraphicsBase:CreateVertexShaderAndInputLayout: FAILED TO READ CSO File");
		return E_FAIL;
	}

	hr = pDevice->CreateVertexShader(SBlob.GetBuffer(), SBlob.GetSize(), NULL, ppVShader);
	if FAILED(hr)
	{
		return hr;
	}

	hr = pDevice->CreateInputLayout(pIEDesc, ElementNum, SBlob.GetBuffer(), SBlob.GetSize(), ppInputLayout);
	if FAILED(hr)
	{
		SafeRelease(ppVShader);
		return hr;
	}

	return hr;
}
HRESULT		GraphicsBase::CreateVertexShader				(LPCWSTR CSOFile, ID3D11VertexShader** ppVShader)
{
	if(pDevice == NULL)
	{
		return E_NOINTERFACE;
	}
	if((CSOFile == NULL) || ((*ppVShader) != NULL))
	{
		return E_INVALIDARG;
	}

	ShaderBlob	SBlob;

	if FAILED(SBlob.ReadCSOFile(CSOFile))
	{
		DebugLog(L"\nERROR::GraphicsBase:CreateVertexShader: FAILED TO READ CSO File");
		return E_FAIL;
	}

	return pDevice->CreateVertexShader(SBlob.GetBuffer(), SBlob.GetSize(), NULL, ppVShader);
}
HRESULT		GraphicsBase::CreatePixelShader					(LPCWSTR CSOFile, ID3D11PixelShader** ppPShader)
{
	if(pDevice == NULL)
	{
		return E_NOINTERFACE;
	}
	if((CSOFile == NULL) || ((*ppPShader) != NULL))
	{
		return E_INVALIDARG;
	}

	ShaderBlob	SBlob;

	if FAILED(SBlob.ReadCSOFile(CSOFile))
	{
		DebugLog(L"\nERROR::GraphicsBase:CreatePixelShader: FAILED TO READ CSO File");
		return E_FAIL;
	}

	return pDevice->CreatePixelShader(SBlob.GetBuffer(), SBlob.GetSize(), NULL, ppPShader);
}
HRESULT		GraphicsBase::CreateHullShader					(LPCWSTR CSOFile, ID3D11HullShader** ppHShader)
{
	if(pDevice == NULL)
	{
		return E_NOINTERFACE;
	}
	if((CSOFile == NULL) || ((*ppHShader) != NULL))
	{
		return E_INVALIDARG;
	}

	ShaderBlob	SBlob;

	if FAILED(SBlob.ReadCSOFile(CSOFile))
	{
		DebugLog(L"\nERROR::GraphicsBase:CreateHullShader: FAILED TO READ CSO File");
		return E_FAIL;
	}

	return pDevice->CreateHullShader(SBlob.GetBuffer(), SBlob.GetSize(), NULL, ppHShader);
}
HRESULT		GraphicsBase::CreateDomainShader				(LPCWSTR CSOFile, ID3D11DomainShader** ppDShader)
{
	if(pDevice == NULL)
	{
		return E_NOINTERFACE;
	}
	if((CSOFile == NULL) || ((*ppDShader) != NULL))
	{
		return E_INVALIDARG;
	}

	ShaderBlob	SBlob;

	if FAILED(SBlob.ReadCSOFile(CSOFile))
	{
		DebugLog(L"\nERROR::GraphicsBase:CreateDomainShader: FAILED TO READ CSO File");
		return E_FAIL;
	}

	return pDevice->CreateDomainShader(SBlob.GetBuffer(), SBlob.GetSize(), NULL, ppDShader);
}
HRESULT		GraphicsBase::CreateGeometryShader				(LPCWSTR CSOFile, ID3D11GeometryShader** ppGShader)
{
	if(pDevice == NULL)
	{
		return E_NOINTERFACE;
	}
	if((CSOFile == NULL) || ((*ppGShader) != NULL))
	{
		return E_INVALIDARG;
	}

	ShaderBlob	SBlob;

	if FAILED(SBlob.ReadCSOFile(CSOFile))
	{
		DebugLog(L"\nERROR::GraphicsBase:CreateGeometryShader: FAILED TO READ CSO File");
		return E_FAIL;
	}

	return pDevice->CreateGeometryShader(SBlob.GetBuffer(), SBlob.GetSize(), NULL, ppGShader);
}
HWND		GraphicsBase::GetHWND							()
{
	return hwnd;
}
//************************************************************************************************


//*******************************GraphicsD3D******************************************************
HRESULT		IGraphics::Initialize							(HWND Handle)
{
	HRESULT hr {S_OK};

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	hwnd = Handle;

	hr = GraphicsBase::Initialize();
	if FAILED(hr)
	{
		return hr;
	}
	hr = GraphicsPSC::Initialize();
	if FAILED(hr)
	{
		return hr;
	}
	hr = TextureManager::Initialize();
	if FAILED(hr)
	{
		return hr;
	}

	//DrawQueue
	{
		hr = CreateDrawQueue(&pDrawQueue);
		if FAILED(hr)
		{
			Clear();
			return hr;
		}
	}

	//Direct2D
	{
		pDirect2D = new Direct2DComponent;
	
		hr = pDirect2D->Initialize(pSwapChain);
		if FAILED(hr)
		{
			return hr;
		}
	}

	pDeviceContext->RSSetViewports(1, &ViewPort);
	pDeviceContext->OMSetRenderTargets(1, &pRTView, pDepthStencilView);
	pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

	return hr;
}
HRESULT		IGraphics::Render								()
{
	

	if(pSetSurface)
	{
		pDeviceContext->ClearRenderTargetView(pSetSurface->pRTV, BackgroundColor);
		pDeviceContext->ClearDepthStencilView(pSetSurface->pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}
	else
	{
		pDeviceContext->ClearRenderTargetView(pRTView, BackgroundColor);
		pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

//******************************************************************

	DrawQueueBuffer<SpriteBuffer,	100>*	pQSpriteBuffer		= &pDrawQueue->QSpriteBuffer;
	DrawQueueBuffer<SpriteExBuffer, 50>*	pQSpriteExBuffer	= &pDrawQueue->QSpriteExBuffer;
	DrawQueueBuffer<RectBuffer,		50>*	pQRectBuffer		= &pDrawQueue->QRectBuffer;
	DrawQueueBuffer<LineBuffer,		50>*	pQLineBuffer		= &pDrawQueue->QLineBuffer;
	DrawQueueBuffer<SurfaceBuffer,	10>*	pQSurfaceBuffer		= &pDrawQueue->QSurfaceBuffer;

	if (pQSpriteBuffer->Count)
	{
		pDeviceContext->VSSetShader(pVSSprite, NULL, 0);
		pDeviceContext->PSSetShader(pPSTexture, NULL, 0);
		pDeviceContext->IASetInputLayout(pILStandard);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		pDeviceContext->UpdateSubresource(pVSSBuffer16->GetBuffer(), 0, NULL, pQSpriteBuffer->Buffer, 0, 0);
		pDeviceContext->DrawInstanced(4U, pQSpriteBuffer->Count, 0, 0);

		pQSpriteBuffer->Count = 0;
	}
	if (pQSurfaceBuffer->Count)
	{
		pDeviceContext->VSSetShader(pVSRawSpriteEx, NULL, 0);
		pDeviceContext->PSSetShader(pPSRawTexture, NULL, 0);
		pDeviceContext->IASetInputLayout(pILStandard);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		for(UINT i { 0 }; i < pQSurfaceBuffer->Count; i++)
		{
			pDeviceContext->PSSetShaderResources(6, 1, &pQSurfaceBuffer[i].Buffer->pSurface->pSRV);

			float data[8]
			{ 
				pQSurfaceBuffer[i].Buffer->x,
				pQSurfaceBuffer[i].Buffer->y,
				1.f,
				1.f,
				0.5f,
				(float)pQSurfaceBuffer[i].Buffer->pSurface->width,
				(float)pQSurfaceBuffer[i].Buffer->pSurface->height,
				0.f
			};

			pDeviceContext->UpdateSubresource(pVBRawSpriteData, 0, NULL, data, 32, 0);
			pDeviceContext->DrawInstanced(4U, 1, 0, 0);

			SafeRelease(&pQSurfaceBuffer[i].Buffer->pSurface);
		}

		pQSurfaceBuffer->Count = 0;
	}
	if (pQSpriteExBuffer->Count)
	{
		pDeviceContext->VSSetShader(pVSSpriteEx, NULL, 0);
		pDeviceContext->PSSetShader(pPSTexture, NULL, 0);
		pDeviceContext->IASetInputLayout(pILStandard);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		pDeviceContext->UpdateSubresource(pVSSBuffer32->GetBuffer(), 0, NULL, pQSpriteExBuffer->Buffer, 0, 0);
		pDeviceContext->DrawInstanced(4U, pQSpriteExBuffer->Count, 0, 0);

		pQSpriteExBuffer->Count = 0;
	}
	if (pQRectBuffer->Count)
	{
		pDeviceContext->VSSetShader(pVSRect, NULL, 0);
		pDeviceContext->PSSetShader(pPSColor, NULL, 0);
		pDeviceContext->IASetInputLayout(pILStandard);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		pDeviceContext->UpdateSubresource(pVSSBuffer32->GetBuffer(), 0, NULL, pQRectBuffer->Buffer, 0, 0);
		pDeviceContext->DrawInstanced(4U, pQRectBuffer->Count, 0, 0);

		pQRectBuffer->Count = 0;
	}
	if (pQLineBuffer->Count)
	{
		pDeviceContext->VSSetShader(pVSLine, NULL, 0);
		pDeviceContext->PSSetShader(pPSColor, NULL, 0);
		pDeviceContext->IASetInputLayout(pILStandard);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

		pDeviceContext->UpdateSubresource(pVSSBuffer32->GetBuffer(), 0, NULL, pQLineBuffer->Buffer, 0, 0);
		pDeviceContext->DrawInstanced(2U, pQLineBuffer->Count, 0, 0);

		pQLineBuffer->Count = 0;
	}
	
	if (pDrawQueue->QStringCount || pDrawQueue->QStaticStringCount)
	{
		pDirect2D->BeginDraw();

		for(UINT i { 0 }; i < pDrawQueue->QStringCount; i++)
		{
			pDirect2D->DrawDynamicText
			(
				pDrawQueue->QStringBuffer[i].pString,
				pDrawQueue->QStringBuffer[i].x,
				pDrawQueue->QStringBuffer[i].y
			);

			delete[] pDrawQueue->QStringBuffer[i].pString;
			pDrawQueue->QStringBuffer[i].pString = NULL;
		}

		pDrawQueue->QStringCount = 0;

		for(UINT i { 0 }; i < pDrawQueue->QStaticStringCount; i++)
		{
			pDirect2D->DrawStaticText
			(
				pDrawQueue->QStaticStringBuffer[i].pTextLayout,
				pDrawQueue->QStaticStringBuffer[i].x,
				pDrawQueue->QStaticStringBuffer[i].y
			);

			SafeRelease(&pDrawQueue->QStaticStringBuffer[i].pTextLayout);
		}

		pDrawQueue->QStaticStringCount = 0;

		pDirect2D->EndDraw();
	}

	if (pDrawQueue->QRawSpriteCount)
	{
		pDeviceContext->VSSetShader(pVSRawSpriteEx, NULL, 0);
		pDeviceContext->PSSetShader(pPSRawTexture, NULL, 0);
		pDeviceContext->IASetInputLayout(pILStandard);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		for(UINT i { 0 }; i < pDrawQueue->QRawSpriteCount; i++)
		{
			pDeviceContext->PSSetShaderResources(6, 1, &pDrawQueue->QRawSpriteBuffer[i].pSprite->pSRVTexture);

			float data[8]
			{ 
				pDrawQueue->QRawSpriteBuffer[i].x,
				pDrawQueue->QRawSpriteBuffer[i].y,
				1.f,
				1.f,
				0.5f,
				(float)pDrawQueue->QRawSpriteBuffer[i].pSprite->GetWidth(),
				(float)pDrawQueue->QRawSpriteBuffer[i].pSprite->GetHeight(),
				0.f
			};

			pDeviceContext->UpdateSubresource(pVBRawSpriteData, 0, NULL, data, 32, 0);
			pDeviceContext->DrawInstanced(4U, 1, 0, 0);
			

			SafeRelease(&pDrawQueue->QRawSpriteBuffer[i].pSprite);
		}

		pDrawQueue->QRawSpriteCount = 0;
	}

	if (pDrawQueue->QRawSpriteExCount)
	{
		pDeviceContext->VSSetShader(pVSRawSpriteEx, NULL, 0);
		pDeviceContext->PSSetShader(pPSRawTexture, NULL, 0);
		pDeviceContext->IASetInputLayout(pILStandard);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		for(UINT i {0}; i < pDrawQueue->QRawSpriteExCount; i++)
		{
			pDeviceContext->PSSetShaderResources(6, 1, &pDrawQueue->QRawSpriteExBuffer[i].pRawSprite->pSRVTexture);

			RawSpriteExBuffer*	pBuffer = &pDrawQueue->QRawSpriteExBuffer[i];

			float data[8]
			{
				pBuffer->x,
				pBuffer->y,
				pBuffer->scaleX,
				pBuffer->scaleY,
				pBuffer->depth,
				(float)pDrawQueue->QRawSpriteExBuffer[i].pRawSprite->GetWidth(),
				(float)pDrawQueue->QRawSpriteExBuffer[i].pRawSprite->GetHeight(),
				0.f
			};

			pDeviceContext->UpdateSubresource(pVBRawSpriteData, 0, NULL, data, 32, 0);
			pDeviceContext->DrawInstanced(4U, 1, 0, 0);

			SafeRelease(&pDrawQueue->QRawSpriteExBuffer[i].pRawSprite);
		}

		pDrawQueue->QRawSpriteExCount = 0;
	}

	pSwapChain->Present(0, 0);

	return S_OK;
}
void		IGraphics::DrawSprite							(const SpriteID sprID, const float x, const float y)
{
	pDrawQueue->QSpriteBuffer.Add( {x, y, sprID, 0} );
}
void		IGraphics::DrawSpriteEx							(const SpriteID spriteID, const float x, const float y, const float scaleX, const float scaleY)
{
	pDrawQueue->QSpriteExBuffer.Add( {x, y, scaleX, scaleY, spriteID, 0.5f, 0, 0} );
}
void		IGraphics::DrawLine								(const float x1, const float y1, const float x2, const float y2, const float color[4])
{
	pDrawQueue->QLineBuffer.Add( {x1, y1, x2, y2, color[0], color[1], color[2], color[3]} );
}
void		IGraphics::DrawRect								(const float x, const float y, const float width, const float height, const float color[4])
{
	pDrawQueue->QRectBuffer.Add( { x, y, width, height, color[0], color[1], color[2], color[3] });
}
void		IGraphics::DrawDynamicText						(LPCWSTR string, float x, float y)
{
	pDrawQueue->AddString(string, x, y);
}
void		IGraphics::DrawStaticText						(IDWriteTextLayout*	pTextLayout, const float x, const float y)
{
	pDrawQueue->AddStaticText(pTextLayout, x, y);
}
void		IGraphics::DrawRawSprite						(RawSprite* pRawSprite, const float x, const float y)
{
	pDrawQueue->AddRawSprite(pRawSprite, x, y);
}
void		IGraphics::DrawRawSpriteEx						(RawSprite* pRawSprite, const float x, const float y, const float scaleX, const float scaleY)
{
	pDrawQueue->AddRawSpriteEx(pRawSprite, x, y, scaleX, scaleY);
}
void		IGraphics::DrawSurface							(ISurface* pSurface, const float x, const float y)
{
	if (pSurface == NULL)
	{
		DebugLog(L"ERROR : IGraphics :: DrawSurface :: Invalid Surface\n");
		return;
	}

	pDrawQueue->QSurfaceBuffer.Add( {pSurface, x, y} );
	pSurface->AddRef();
}
HRESULT		IGraphics::CreateMesh							(Vertex* vArray, UINT* iArray, UINT vertexCount, UINT indexCount, Mesh** ppMesh)
{
	if( (vArray == NULL) || (iArray == NULL) || (indexCount == 0) || (vertexCount == 0) || (*ppMesh != NULL) )
	{
		return E_INVALIDARG;
	}

	*ppMesh = new Mesh;

	(*ppMesh)->vertexCount	= vertexCount;
	(*ppMesh)->indexCount	= indexCount;
	(*ppMesh)->index		= new UINT[indexCount];
	(*ppMesh)->vertex		= new Vertex[vertexCount];

	memcpy((*ppMesh)->vertex, vArray, sizeof(Vertex) * vertexCount);
	memcpy((*ppMesh)->index, iArray, sizeof(UINT) * indexCount);

	return S_OK;
}
HRESULT		IGraphics::CreateRenderInstance					(const Sprite* pSprite, RenderInstance** ppRenderInstance)
{
	if ( ((*ppRenderInstance) != NULL) || (pSprite == NULL) )
	{
		return E_INVALIDARG;
	}

	(*ppRenderInstance) = new RenderInstance;

	HRESULT hr = (*ppRenderInstance)->Initialize(pSprite, pDrawQueue);
	if FAILED(hr)
	{
		SafeRelease(ppRenderInstance);
		DebugLog(L"\nGraphicsD3D :: Failed to initialize Render instance");
		return hr;
	}

	return hr;
}
HRESULT		IGraphics::CreateDrawQueue						(DrawQueue** ppDrawQueue)
{
	(*ppDrawQueue) = new DrawQueue;

	(*ppDrawQueue)->Initialize();

	return S_OK;
}
HRESULT		IGraphics::CreateTextLayout						(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout)
{
	return pDirect2D->CreateTextLayout(pDesc, ppTextLayout);
}
HRESULT		IGraphics::CreateSurface						(ISurface** ppSurface, const UINT width, const UINT height)
{
	HRESULT hr;

	if(ppSurface == NULL || *ppSurface != NULL || width == 0 || height == 0)
		return E_INVALIDARG;

	ID3D11Texture2D*				pTexture	{ NULL };
	ID3D11Texture2D*				pDSTexture	{ NULL };
	ID3D11DepthStencilView*			pDSV		{ NULL };
	ID3D11ShaderResourceView*		pSRV		{ NULL };
	ID3D11RenderTargetView*			pRTV		{ NULL };

	D3D11_TEXTURE2D_DESC			T2DDesc;
	D3D11_TEXTURE2D_DESC			DST2DDesc;

	ZeroMemory(&T2DDesc, sizeof(D3D11_TEXTURE2D_DESC));

	T2DDesc.Width					= width;
	T2DDesc.Height					= height;
	T2DDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	T2DDesc.ArraySize				= 1;
	T2DDesc.MipLevels				= 1;
	T2DDesc.BindFlags				= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	T2DDesc.SampleDesc.Count		= 1;
	T2DDesc.SampleDesc.Quality		= 0;
	T2DDesc.Usage					= D3D11_USAGE_DEFAULT;
	T2DDesc.CPUAccessFlags			= 0;
	T2DDesc.MiscFlags				= 0;

	DST2DDesc.Width					= width;
	DST2DDesc.Height				= height;
	DST2DDesc.Format				= DXGI_FORMAT_D32_FLOAT;
	DST2DDesc.MipLevels				= 1;
	DST2DDesc.ArraySize				= 1;
	DST2DDesc.SampleDesc.Count		= 1;
	DST2DDesc.SampleDesc.Quality	= 0;
	DST2DDesc.BindFlags				= D3D11_BIND_DEPTH_STENCIL;
	DST2DDesc.CPUAccessFlags		= 0;
	DST2DDesc.MiscFlags				= 0;
	DST2DDesc.Usage					= D3D11_USAGE_DEFAULT;


	hr = pDevice->CreateTexture2D(&T2DDesc, NULL, &pTexture);
	if FAILED(hr)
	{
		return E_FAIL;
	}

	hr = pDevice->CreateShaderResourceView(pTexture, NULL, &pSRV);
	if FAILED(hr)
	{
		SafeRelease(&pTexture);
		return E_FAIL;
	}

	hr = pDevice->CreateRenderTargetView(pTexture, NULL, &pRTV);
	if FAILED(hr)
	{
		SafeRelease(&pTexture);
		SafeRelease(&pSRV);

		return E_FAIL;
	}

	hr = pDevice->CreateTexture2D(&DST2DDesc, NULL, &pDSTexture);
	if FAILED(hr)
	{
		SafeRelease(&pTexture);
		SafeRelease(&pSRV);
		SafeRelease(&pRTV);

		return E_FAIL;
	}

	hr = pDevice->CreateDepthStencilView(pDSTexture, NULL, &pDSV);
	if FAILED(hr)
	{
		SafeRelease(&pTexture);
		SafeRelease(&pSRV);
		SafeRelease(&pRTV);
		SafeRelease(&pDSTexture);

		return E_FAIL;
	}

	SafeRelease(&pDSTexture);


	(*ppSurface) = new ISurface;

	(*ppSurface)->pTexture	= pTexture;
	(*ppSurface)->pSRV		= pSRV;
	(*ppSurface)->pRTV		= pRTV;
	(*ppSurface)->pDSV		= pDSV;
	(*ppSurface)->height	= height;
	(*ppSurface)->width		= width;

	D3D11_VIEWPORT* pViewPort = &(*ppSurface)->ViewPort;

	pViewPort->Width	= (float)width;
	pViewPort->Height	= (float)height;
	pViewPort->MinDepth = 0.f;
	pViewPort->MaxDepth = 1.f;

	return hr;
}
HRESULT		IGraphics::SetSurface							(ISurface* pSurface)
{
	if(pSurface == NULL)
		return E_INVALIDARG;

	//Flush
	Render();

	pDeviceContext->RSSetViewports(1, &pSurface->ViewPort);
	pDeviceContext->OMSetRenderTargets(1, &pSurface->pRTV, pSurface->pDSV);

	float Data[4]
	{
		1.f/pSurface->ViewPort.Width,
		1.f/pSurface->ViewPort.Height,
		0.f,
		0.f
	};

	pDeviceContext->UpdateSubresource(pVSBSettings, 0U, NULL, Data, 16U, 0U);

	pSetSurface = pSurface;
	pSurface->AddRef();

	return S_OK;
}
void		IGraphics::ResetSurface							()
{
	if (pSetSurface == NULL)
		return;

	//Flash
	Render();

	pDeviceContext->RSSetViewports(1, &ViewPort);
	pDeviceContext->OMSetRenderTargets(1, &pRTView, pDepthStencilView);

	float Data[4] 
	{
		1.f/(float)Settings.width,
		1.f/(float)Settings.height,
		0.f,
		0.f
	};
	pDeviceContext->UpdateSubresource(pVSBSettings, 0U, NULL, Data, 16U, 0U);

	SafeRelease(&pSetSurface);
}
void		IGraphics::SetBGColor							(const float color[4])
{
	BackgroundColor[0] = color[0];
	BackgroundColor[1] = color[1];
	BackgroundColor[2] = color[2];
	BackgroundColor[3] = color[3];
}
//***********************************************************************************************


//*******************************GraphicsPSC*****************************************************
HRESULT		GraphicsPSC::Initialize							()
{
	HRESULT hr;

	//Shaders and create input layout
	{
		D3D11_INPUT_ELEMENT_DESC IEDesc[]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "SV_VertexID", 0, DXGI_FORMAT_R32_UINT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		//Vertex Shader and Input Layout
		hr = CreateVertexShaderAndInputLayout(L"Resource/Shaders/VSSprite.cso", IEDesc, ARRAYSIZE(IEDesc), &pILStandard, &pVSSprite);
		if FAILED(hr)
		{
			DebugLog(L"\nFailed to create Vertex Shader");
			return hr;
		}


		//Line VS
		hr = CreateVertexShader(L"Resource/Shaders/VSLine.cso", &pVSLine);
		if FAILED(hr)
		{
			return hr;
		}

		
		//Rect VS
		hr = CreateVertexShader(L"Resource/Shaders/VSRect.cso", &pVSRect);
		if FAILED(hr)
		{
			return hr;
		}

		//Raw Sprite Ex VS
		hr = CreateVertexShader(L"Resource/Shaders/VSRawSpriteEx.cso", &pVSRawSpriteEx);


		//SpriteEx VS and IL
		hr = CreateVertexShader(L"Resource/Shaders/VSSpriteEx.cso", &pVSSpriteEx);
		if FAILED(hr)
		{
			return hr;
		}


		//Pixel Shader
		hr = CreatePixelShader(L"Resource/Shaders/PSTexture.cso", &pPSTexture);
		if FAILED(hr)
		{
			DebugLog(L"\nFailed to create Pixel Shader");
			return hr;
		}

		//PSRawTexture
		hr = CreatePixelShader(L"Resource/Shaders/PSRawTexture.cso", &pPSRawTexture);
		if FAILED(hr)
		{
			DebugLog(L"\nERROR :: Failed to Create PSRawTexture");
			return hr;
		}

		//PSLine
		hr = CreatePixelShader(L"Resource/Shaders/PSColor.cso", &pPSColor);
		if FAILED(hr)
		{
			return hr;
		}


		//Hull Shader
		hr = CreateHullShader(L"Resource/Shaders/HullShader.cso", &pHullShader);
		if FAILED(hr)
		{
			return hr;
		}


		//DomainShader
		hr = CreateDomainShader(L"Resource/Shaders/DomainShader.cso", &pDomainShader);
		if FAILED(hr)
		{
			return hr;
		}

		//GeometryShader
		hr = CreateGeometryShader(L"Resource/Shaders/GeometryShader.cso", &pGeometryShader);
		if FAILED(hr)
		{
			return hr;
		}
	}

	//Buffers
	{
		hr = CreateStructuredBuffer(sizeof(float) * 4, 100, &pVSSBuffer16);
		if FAILED(hr)
		{
			return hr;
		}

		hr = CreateStructuredBuffer(sizeof(float) * 8, 100, &pVSSBuffer32);
		if FAILED(hr)
		{
			return hr;
		}

		pDeviceContext->VSSetShaderResources( 0, 1, &pVSSBuffer16->pSRV );
		pDeviceContext->VSSetShaderResources( 1, 1, &pVSSBuffer32->pSRV );
	}

	//Constant Buffers
	{
		D3D11_BUFFER_DESC BDesc;
		
		BDesc.Usage					= D3D11_USAGE_DEFAULT;
		BDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		BDesc.CPUAccessFlags		= 0;
		BDesc.MiscFlags				= 0;

		//Settings buffer
		DirectX::XMFLOAT4 cbuffer
		{
			1.f/Settings.width,
			1.f/Settings.height,
			0.f,
			0.f
		};

		D3D11_SUBRESOURCE_DATA SRDDesc;
		SRDDesc.SysMemPitch			= 0;
		SRDDesc.SysMemSlicePitch	= 0;

		BDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		BDesc.ByteWidth				= sizeof(DirectX::XMFLOAT4);
		SRDDesc.pSysMem				= &cbuffer;

		hr = pDevice->CreateBuffer(&BDesc, &SRDDesc, &pVSBSettings);
		if FAILED(hr)
		{
			return hr;
		}


		//Standard mesh
		Mesh*	pMesh	{NULL};
		HRESULT hr = CreateSimpleMesh(&pMesh);
		if FAILED(hr)
		{
			return hr;
		}


		//Vertex buffer
		BDesc.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
		BDesc.ByteWidth				= pMesh->vertexCount * sizeof(Vertex);
		SRDDesc.pSysMem				= pMesh->vertex;

		hr = pDevice->CreateBuffer(&BDesc, &SRDDesc, &pVBStandard);
		if FAILED(hr)
		{
			SafeRelease(&pMesh);
			return hr;
		}

		SafeRelease(&pMesh);

		//RawSpriteData
		BDesc.ByteWidth				= sizeof(float) * 8;
		
		hr = pDevice->CreateBuffer(&BDesc, NULL, &pVBRawSpriteData);
		if FAILED(hr)
		{
			return hr;
		}

		UINT			stride			= sizeof(Vertex);
		UINT			offset			= 0;

		pDeviceContext->IASetVertexBuffers(0, 1, &pVBStandard, &stride, &offset);
		pDeviceContext->VSSetConstantBuffers(1, 1, &pVSBSettings);
		pDeviceContext->VSSetConstantBuffers(2, 1, &pVBRawSpriteData);
	}

	//Depth Stencil State and View
	{
		//View
		ID3D11Texture2D* pTexture {NULL};
		D3D11_TEXTURE2D_DESC T2DDesc;
		D3D11_TEXTURE2D_DESC bbd;
		D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;


		pBackBuffer->GetDesc(&bbd);

		T2DDesc.Width				= bbd.Width;
		T2DDesc.Height				= bbd.Height;
		T2DDesc.Format				= DXGI_FORMAT_D32_FLOAT;
		T2DDesc.MipLevels			= 1;
		T2DDesc.ArraySize			= 1;
		T2DDesc.SampleDesc.Count	= 1;
		T2DDesc.SampleDesc.Quality	= 0;
		T2DDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
		T2DDesc.CPUAccessFlags		= 0;
		T2DDesc.MiscFlags			= 0;
		T2DDesc.Usage				= D3D11_USAGE_DEFAULT;

		hr = pDevice->CreateTexture2D(&T2DDesc, NULL, &pTexture);
		if FAILED(hr)
		{
			return hr;
		}

		DSVDesc.Format				= DXGI_FORMAT_D32_FLOAT;
		DSVDesc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
		DSVDesc.Texture2D.MipSlice	= 0;
		DSVDesc.Flags				= 0;

		hr = pDevice->CreateDepthStencilView(pTexture, &DSVDesc, &pDepthStencilView);
		SafeRelease(&pTexture);
		if FAILED(hr)
		{
			return hr;
		}

		//Stencil
		D3D11_DEPTH_STENCIL_DESC DSDesc;

		DSDesc.DepthEnable			= TRUE;
		DSDesc.DepthWriteMask		= D3D11_DEPTH_WRITE_MASK_ALL;
		DSDesc.DepthFunc			= D3D11_COMPARISON_LESS;
		DSDesc.StencilEnable		= FALSE;
		DSDesc.StencilReadMask		= 0xFF;
		DSDesc.StencilWriteMask		= 0xFF;

		// Stencil operations if pixel is front-facing
		DSDesc.FrontFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
		DSDesc.FrontFace.StencilDepthFailOp		= D3D11_STENCIL_OP_INCR;
		DSDesc.FrontFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
		DSDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		DSDesc.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
		DSDesc.BackFace.StencilDepthFailOp		= D3D11_STENCIL_OP_DECR;
		DSDesc.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
		DSDesc.BackFace.StencilFunc				= D3D11_COMPARISON_ALWAYS;

		hr = pDevice->CreateDepthStencilState(&DSDesc, &pDepthStencilState);
		if FAILED(hr)
		{
			return hr;
		}
	}

	//Blend State
	{
		D3D11_BLEND_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));

		bd.RenderTarget[0].BlendEnable				= TRUE;
		bd.RenderTarget[0].SrcBlend					= D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;

		hr = pDevice->CreateBlendState(&bd, &pBlendState);
		if FAILED(hr)
		{
			return hr;
		}

		pDeviceContext->OMSetBlendState(pBlendState, NULL, 0xFFFFFFFF);
	}

	//Rasterizer State
	{
		D3D11_RASTERIZER_DESC rsd;

		rsd.FillMode				= D3D11_FILL_SOLID;
		rsd.CullMode				= D3D11_CULL_BACK;
		rsd.FrontCounterClockwise	= FALSE;
		rsd.DepthBias				= D3D11_DEFAULT_DEPTH_BIAS;
		rsd.DepthBiasClamp			= D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		rsd.SlopeScaledDepthBias	= D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rsd.DepthClipEnable			= FALSE;
		rsd.ScissorEnable			= FALSE;
		rsd.MultisampleEnable		= FALSE;
		rsd.AntialiasedLineEnable	= TRUE;

		hr = pDevice->CreateRasterizerState(&rsd, &pRasterizerState);
		if FAILED(hr)
		{
			return hr;
		}

		pDeviceContext->RSSetState(pRasterizerState);
	}

	//Sampler State
	{
		D3D11_SAMPLER_DESC sd;
		ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));

		sd.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
		sd.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
		sd.MaxLOD			= D3D11_FLOAT32_MAX;
		sd.MinLOD			= 0.f;
		sd.MipLODBias		= 0.f;
		sd.ComparisonFunc	= D3D11_COMPARISON_NEVER;
		sd.BorderColor[0]	= 0.f;
		sd.BorderColor[1]	= 0.f;
		sd.BorderColor[2]	= 0.f;
		sd.BorderColor[3]	= 0.f;

		hr = pDevice->CreateSamplerState(&sd, &pSamplerState);
		if FAILED(hr)
		{
			return hr;
		}

		pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);
	}

	return hr;
}
HRESULT		GraphicsPSC::ExtractBufferData					(ID3D11Buffer* pBuffer, RawBufferData** ppRawBufferData)
{
	if(pBuffer == NULL || ppRawBufferData == NULL || (*ppRawBufferData) != NULL)
		return E_INVALIDARG;

	ID3D11Buffer*				pSBuffer	{ NULL };
	D3D11_BUFFER_DESC			BDesc;
	D3D11_MAPPED_SUBRESOURCE	MSR;
	pBuffer->GetDesc(&BDesc);

	BDesc.CPUAccessFlags	= D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	BDesc.BindFlags			= 0;
	BDesc.Usage				= D3D11_USAGE_STAGING;

	(*ppRawBufferData) = new RawBufferData;
	(*ppRawBufferData)->desc = BDesc;


	HRESULT hr = pDevice->CreateBuffer(&BDesc, NULL, &pSBuffer);
	if FAILED(hr)
	{
		SafeRelease(ppRawBufferData);
		return hr;
	}

	pDeviceContext->CopyResource(pSBuffer, pBuffer);
	hr = pDeviceContext->Map(pSBuffer, 0, D3D11_MAP_READ, 0, &MSR);
	if FAILED(hr)
	{
		SafeRelease(ppRawBufferData);
		SafeRelease(&pSBuffer);
		return hr;
	}


	(*ppRawBufferData)->pData	= new BYTE[BDesc.ByteWidth];

	memcpy((*ppRawBufferData)->pData, MSR.pData, BDesc.ByteWidth);
	pDeviceContext->Unmap(pSBuffer, 0);
	SafeRelease(&pSBuffer);

	return hr;
}
HRESULT		GraphicsPSC::CreateStructuredBuffer				(UINT elementSize, UINT numElements, StructBuffer** ppStructBuffer)
{
	if (elementSize == 0 || numElements == 0 || (*ppStructBuffer) != NULL)
		return E_INVALIDARG;

	D3D11_BUFFER_DESC				BDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	StructBuffer* pStructBuffer = new StructBuffer;

	BDesc.ByteWidth					= elementSize * numElements;
	BDesc.Usage						= D3D11_USAGE_DEFAULT;
	BDesc.BindFlags					= D3D11_BIND_SHADER_RESOURCE;
	BDesc.CPUAccessFlags			= 0;
	BDesc.MiscFlags					= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BDesc.StructureByteStride		= elementSize;

	SRVDesc.Format					= DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.ElementOffset	= 0;
	SRVDesc.Buffer.ElementWidth		= elementSize;
	SRVDesc.Buffer.FirstElement		= 0;
	SRVDesc.Buffer.NumElements		= numElements;

	HRESULT hr = pDevice->CreateBuffer(&BDesc, NULL, &(pStructBuffer->pBuffer));
	if FAILED(hr)
	{
		SafeRelease(&pStructBuffer);
		return hr;
	}

	hr = pDevice->CreateShaderResourceView(pStructBuffer->pBuffer, &SRVDesc, &(pStructBuffer->pSRV));
	if FAILED(hr)
	{
		SafeRelease(&pStructBuffer);
		return hr;
	}

	(*ppStructBuffer) = pStructBuffer;

	return S_OK;
}
HRESULT		GraphicsPSC::StructureBufferAppend				(const UINT numOfElements, const void* srcData, StructBuffer* pStructBuffer)
{
	if(pStructBuffer == NULL || numOfElements == 0 || srcData == NULL)
		return E_INVALIDARG;

	StructBuffer* pSwapSBuffer { NULL };

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	D3D11_BUFFER_DESC BDesc;

	pStructBuffer->pBuffer->GetDesc(&BDesc);
	pStructBuffer->pSRV->GetDesc(&SRVDesc);

	CreateStructuredBuffer(BDesc.StructureByteStride, SRVDesc.Buffer.NumElements + numOfElements, &pSwapSBuffer);

	D3D11_BOX box {0, 0, 0, BDesc.StructureByteStride * SRVDesc.Buffer.NumElements, 1, 1};
	pDeviceContext->CopySubresourceRegion(pSwapSBuffer->pBuffer, 0, 0, 0, 0, pStructBuffer->pBuffer, 0, &box);
	box = { BDesc.StructureByteStride * SRVDesc.Buffer.NumElements, 0, 0, BDesc.StructureByteStride * (SRVDesc.Buffer.NumElements + numOfElements), 1, 1 };
	pDeviceContext->UpdateSubresource(pSwapSBuffer->pBuffer, 0, &box, srcData, BDesc.StructureByteStride * numOfElements, 0);

	*pStructBuffer = pSwapSBuffer;

	//SafeRelease(&pStructBuffer->pBuffer);
	//SafeRelease(&pStructBuffer->pSRV);
	//
	//pStructBuffer->pBuffer = pSwapSBuffer->pBuffer;
	//pSwapSBuffer->pBuffer->AddRef();
	//pStructBuffer->pSRV = pSwapSBuffer->pSRV;
	//pSwapSBuffer->pSRV->AddRef();

	SafeRelease(&pSwapSBuffer);

	return S_OK;
}
HRESULT		GraphicsPSC::StructuredBufferRemove				(const UINT element, StructBuffer* pStructBuffer)
{
	if(pStructBuffer == NULL)
		return E_INVALIDARG;

	StructBuffer*	pSwapSBuffer	{ NULL };

	D3D11_BUFFER_DESC BDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;

	pStructBuffer->pBuffer->GetDesc(&BDesc);
	pStructBuffer->pSRV->GetDesc(&SRVDesc);

	UINT NumElements = SRVDesc.Buffer.NumElements;

	if(NumElements >= element || (NumElements == 1)) //Kh kh...
		return E_INVALIDARG;

	HRESULT hr = CreateStructuredBuffer(BDesc.StructureByteStride, SRVDesc.Buffer.NumElements - 1U, &pSwapSBuffer);
	if FAILED(hr)
		return hr;

	D3D11_BOX box;

	if(element != 0)	//if element is not the first element
	{
		box = { 0, 0, 0, BDesc.StructureByteStride * (element), 1, 1 };
		pDeviceContext->CopySubresourceRegion(pSwapSBuffer->pBuffer, 0, 0, 0, 0, pStructBuffer->pBuffer, 0, &box);
	}
	
	if(element != (NumElements - 1)) //if element is not the last element
	{
		box = {BDesc.StructureByteStride * (element + 1U), 0, 0, BDesc.StructureByteStride * NumElements, 1, 1};
		pDeviceContext->CopySubresourceRegion(pSwapSBuffer->pBuffer, 0, BDesc.StructureByteStride * element, 0, 0, pStructBuffer->pBuffer, 0, &box);
	}

	*pStructBuffer = pSwapSBuffer;
	SafeRelease(&pSwapSBuffer);
	//untested
	return S_OK;
}
//***********************************************************************************************


//*******************************TextureManager**************************************************
HRESULT		TextureManager::AddSpriteToBatch				(RawImageData* pRawImageData, PointU point, BatchID batchID)
{
	if (pRawImageData == NULL || batchID >= BatchCount)
		return E_INVALIDARG;

	HRESULT hr;
	D3D11_TEXTURE2D_DESC		T2DDesc;
	ID3D11Texture2D*			pTexture		{ NULL };
	ID3D11ShaderResourceView*	pSRVTexture		{ NULL };
	SpriteBatch*				pSpriteBatch	{ apSpriteBatch[batchID] };
	RectU						CollisionRect	{ point.x, point.y, point.x + pRawImageData->width - 1, point.y + pRawImageData->height - 1 };

	if(pSpriteBatch->CheckCollision(&CollisionRect))
	{
		DebugLog(L"TextureManager : AddSpriteTiBatch : Invalid point");

		return E_INVALIDARG;
	}

	pSpriteBatch->pTexture->GetDesc(&T2DDesc);

	if (pRawImageData->pixelFormat != T2DDesc.Format)
	{
		DebugLog(L"TextureManager : AddSpriteToBatch : Invalid pixel format\n");

		return E_INVALIDARG;
	}


	hr = pDevice->CreateTexture2D(&T2DDesc, 0, &pTexture);
	if FAILED(hr)
	{
		return hr;
	}
	pDeviceContext->CopyResource(pTexture, pSpriteBatch->pTexture);

	float SpriteData[10]
	{
		(float)pRawImageData->width,
		(float)pRawImageData->height,

		(float)(point.x) / BATCH_WIDTH,
		(float)(point.y) / BATCH_HEIGHT,

		(float)(point.x + pRawImageData->width) / BATCH_WIDTH,
		(float)(point.y) / BATCH_HEIGHT,

		(float)(point.x) / BATCH_WIDTH,
		(float)(point.y + pRawImageData->height) / BATCH_HEIGHT,

		(float)(point.x + pRawImageData->width) / BATCH_WIDTH,
		(float)(point.y + pRawImageData->height) / BATCH_HEIGHT
	};

	D3D11_BOX box = {point.x, point.y, 0, point.x + pRawImageData->width, point.y + pRawImageData->height, 1};
	pDeviceContext->UpdateSubresource(pTexture, 0, &box, pRawImageData->pixelData, pRawImageData->RowPitch, 0);

	hr = pDevice->CreateShaderResourceView(pTexture, NULL, &pSRVTexture);
	if FAILED(hr)
	{
		SafeRelease(&pTexture);
		SafeRelease(&pSRVTexture);

		return hr;
	}

	hr = UninitializeBatch(batchID);

	hr = StructureBufferAppend(1, SpriteData, pSpriteBatch->pSBuffer);

	SafeRelease(&pSpriteBatch->pTexture);
	SafeRelease(&pSpriteBatch->pSRVTexture);

	pSpriteBatch->pTexture		= pTexture;
	pSpriteBatch->pSRVTexture	= pSRVTexture;

	Sprite* pSprite { NULL };
	CreateSprite(&pSprite, pRawImageData->width, pRawImageData->height);
	pSprite->batchID = pSpriteBatch->batchID;
	pSpriteBatch->AddSprite(pSprite, CollisionRect);
	SafeRelease(&pSprite);

	if SUCCEEDED(hr)
		InitializeBatch(batchID);

	return S_OK;
}
HRESULT		TextureManager::InitializeBatch					(BatchID batchID)
{
	if(batchID >= BatchCount)
		return E_INVALIDARG;

	if(batchID == InitializedBatch || batchID == 0xffffffff)
		return S_OK;

	pDeviceContext->PSSetShaderResources(5, 1, &apSpriteBatch[batchID]->pSRVTexture);
	pDeviceContext->VSSetShaderResources(4, 1, apSpriteBatch[batchID]->pSBuffer->GetPpSRV());

	InitializedBatch = batchID;

	return S_OK;
}
HRESULT		TextureManager::UninitializeBatch				(BatchID batchID)
{
	if(InitializedBatch != batchID || batchID == 0xffffffff)
		return E_FAIL;

	InitializedBatch = 0xffffffff;

	return S_OK;
}
HRESULT		TextureManager::CreateSprite					(Sprite** ppSprite, UINT width, UINT height)
{
	if (ppSprite == NULL || (*ppSprite) != NULL)
		return E_INVALIDARG;

	(*ppSprite) = new Sprite;
	(*ppSprite)->width = (float)width;
	(*ppSprite)->height = (float)height;
	(*ppSprite)->spriteID = pSpriteCollection->SpriteCount;

	pSpriteCollection->AddSprite((*ppSprite));

	return S_OK;
}
HRESULT		TextureManager::CreateSpriteBatch				()
{
	SpriteBatch* pSpriteBatch = new SpriteBatch;
	pSpriteBatch->pSpriteCollection = new SpriteCollection;
	pSpriteBatch->batchID = BatchCount;

	//Sprite Resource
	D3D11_TEXTURE2D_DESC T2DDesc;
	ZeroMemory(&T2DDesc, sizeof(D3D11_TEXTURE2D_DESC));

	T2DDesc.Width = BATCH_WIDTH;
	T2DDesc.Height = BATCH_HEIGHT;
	T2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	T2DDesc.ArraySize = 1;
	T2DDesc.MipLevels = 1;
	T2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	T2DDesc.SampleDesc.Count = 1;
	T2DDesc.SampleDesc.Quality = 0;
	T2DDesc.Usage = D3D11_USAGE_DEFAULT;
	T2DDesc.CPUAccessFlags = 0;
	T2DDesc.MiscFlags = 0;

	HRESULT hr = pDevice->CreateTexture2D(&T2DDesc, NULL, &pSpriteBatch->pTexture);
	if FAILED(hr)
	{
		SafeRelease(&pSpriteBatch);
		return hr;
	}

	hr = pDevice->CreateShaderResourceView(pSpriteBatch->pTexture, NULL, &pSpriteBatch->pSRVTexture);
	if FAILED(hr)
	{
		SafeRelease(&pSpriteBatch);
		return hr;
	}

	hr = CreateStructuredBuffer(sizeof(float) * 10, 1, &pSpriteBatch->pSBuffer);
	if FAILED(hr)
	{
		SafeRelease(&pSpriteBatch);
		return hr;
	}

	float Data[10]
	{
		(float)BATCH_WIDTH, (float)BATCH_HEIGHT,
		0, 0,
		1, 0,
		0, 1,
		1, 1
	};

	pDeviceContext->UpdateSubresource(pSpriteBatch->pSBuffer->GetBuffer(), 0, NULL, Data, sizeof(float) * 10, sizeof(float) * 10);

	ArrayPush(&apSpriteBatch, BatchCount, pSpriteBatch);

	BatchCount++;

	return S_OK;
}
HRESULT		TextureManager::LoadBatch						(LPCWSTR fileName)
{
	HRESULT hr { S_OK };

	HANDLE file = CreateFileW(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		DebugLog(L"ERROR TextureManager : LoadBatch : Could not open file\n");
		return E_INVALIDARG;
	}

	wchar_t extention[]{ L".batch" };

	if( wcscmp(extention, wcsrchr(fileName, L'.')) )
	{
		CloseHandle(file);
		DebugLog(L"ERROR TextureManager : LoadBatch : Wrong file type\n");
		return E_INVALIDARG;
	}

	BYTE*	pTextureData	{ NULL };
	BYTE*	pBufferData		{ NULL };
	UINT*	pSpriteData		{ NULL };
	RectU*	aCollisionCache	{ NULL };

	BatchHeader				Header;
	D3D11_SUBRESOURCE_DATA	SRDTexture;

	//Disassemble batch data
	{
		UINT	FileSize	{ GetFileSize(file, NULL) };
		BYTE*	DataBlob	{ new BYTE[FileSize] };
		DWORD	BytesRead	{ 0 };
		UINT	offset		{ 0 };


		if ( !ReadFile(file, DataBlob, FileSize, &BytesRead, NULL) )
		{
			CloseHandle(file);
			delete[] DataBlob;
			DebugLog(L"ERROR TextureManager : LoadBatch : Could not read file\n");
			return E_FAIL;
		}
		CloseHandle(file);

		//Batch texture
#pragma warning(disable:6385)
		memcpy(&Header, DataBlob, sizeof(BatchHeader));
#pragma warning(default:6385)
		offset += sizeof(BatchHeader);

		if(Header.SpriteCount == 0)
		{
			delete[] DataBlob;
			return CreateSpriteBatch();
		}

		pSpriteData = new UINT[Header.SpriteCount * 2];
		memcpy(pSpriteData, DataBlob + offset, sizeof(UINT) * 2 * Header.SpriteCount);
		offset += sizeof(UINT) * 2 * Header.SpriteCount;

		pTextureData = new BYTE[Header.TextureByteWidth];
		memcpy(pTextureData, DataBlob + offset, Header.TextureByteWidth);
		offset += Header.TextureByteWidth;

		pBufferData = new BYTE[Header.BDesc.ByteWidth];
		memcpy(pBufferData, DataBlob + offset, Header.BDesc.ByteWidth);
		offset += Header.BDesc.ByteWidth;

		aCollisionCache = new RectU[Header.SpriteCount];
		memcpy(aCollisionCache, DataBlob + offset, sizeof(RectU) * Header.SpriteCount);

		delete[] DataBlob;
	}


	SpriteBatch* pSpriteBatch = new SpriteBatch;
	pSpriteBatch->batchID = BatchCount;
	pSpriteBatch->pSpriteCollection = new SpriteCollection;
	pSpriteBatch->aCollisionCache = aCollisionCache;

	SRDTexture.pSysMem = pTextureData;
	SRDTexture.SysMemPitch = Header.TextureRowPitch;
	SRDTexture.SysMemSlicePitch = Header.TextureByteWidth;

	hr = pDevice->CreateTexture2D(&Header.T2DDesc, &SRDTexture, &pSpriteBatch->pTexture);
	if FAILED(hr)
	{
		delete[] pTextureData;
		delete[] pSpriteData;
		delete[] pBufferData;
		SafeRelease(&pSpriteBatch);
		return hr;
	}
	hr = pDevice->CreateShaderResourceView(pSpriteBatch->pTexture, NULL, &pSpriteBatch->pSRVTexture);
	if FAILED(hr)
	{
		delete[] pTextureData;
		delete[] pSpriteData;
		delete[] pBufferData;
		SafeRelease(&pSpriteBatch);
		return hr;
	}
	hr = CreateStructuredBuffer(Header.BDesc.StructureByteStride, Header.SpriteCount + 1U, &pSpriteBatch->pSBuffer);
	if FAILED(hr)
	{
		delete[] pTextureData;
		delete[] pSpriteData;
		delete[] pBufferData;
		SafeRelease(&pSpriteBatch);
		return hr;
	}
	pDeviceContext->UpdateSubresource(pSpriteBatch->pSBuffer->GetBuffer(), 0, NULL, pBufferData, Header.BDesc.ByteWidth, 0);

	for(UINT i { 0 }; i < Header.SpriteCount; i++)
	{
		Sprite* pSprite { NULL };
		CreateSprite(&pSprite, pSpriteData[i], pSpriteData[i + 1]);
		pSprite->batchID = BatchCount;
		pSpriteBatch->pSpriteCollection->AddSprite(pSprite);
		SafeRelease(&pSprite);
	}

	delete[] pTextureData;
	delete[] pSpriteData;
	delete[] pBufferData;


	//Add to sprite batch array
	ArrayPush(&apSpriteBatch, BatchCount, pSpriteBatch);

	BatchCount++;

	return S_OK;
}
HRESULT		TextureManager::UnloadBatch						(BatchID batchID)
{
	if(batchID >= BatchCount)
		return E_INVALIDARG;

	UninitializeBatch(batchID);

	SafeRelease(&apSpriteBatch[batchID]);
	ArrayRemove(&apSpriteBatch, BatchCount, batchID);
	BatchCount--;

	return S_OK;
}
HRESULT		TextureManager::SaveBatch						(BatchID batchID, const wchar_t* fileName)
{
	/*
	* Saved Data format
	* 
	* BatchHeader	Header
	* BYTE			Sprite Data
	* BYTE			Batch texture data
	* BYTE			Buffer data
	*/
	
	//OP Add option for empty batches

	if (batchID >= BatchCount || fileName == NULL)
		return E_INVALIDARG;

	HRESULT						hr					{ S_OK };
	DWORD						BytesWriten			{ 0 };
	UINT						bytewidth			{ 0 };

	BYTE*						pData				{ NULL };

	RawImageData*				pRawImageData		{ NULL };
	RawBufferData*				pRawBufferData		{ NULL };
	D3D11_TEXTURE2D_DESC		T2DDesc;

	SpriteBatch*				pSpriteBatch		{ apSpriteBatch[batchID] };
	UINT						sprCount			{ pSpriteBatch->pSpriteCollection->SpriteCount };
	UINT*						SpriteData			{ new UINT[sprCount*2] };

	hr = ExtractImageData(pSpriteBatch->pTexture, &pRawImageData);
	if FAILED(hr)
	{
		return hr;
	}

	hr = ExtractBufferData(pSpriteBatch->pSBuffer->GetBuffer(), &pRawBufferData);
	if FAILED(hr)
	{
		SafeRelease(&pRawImageData);
	}

	pSpriteBatch->pTexture->GetDesc(&T2DDesc);

	for(UINT i { 0 }; i < sprCount; i++)
	{
		SpriteData[i]		= (UINT)pSpriteBatch->pSpriteCollection->apSprites[i]->width;
		SpriteData[(i + 1)] = (UINT)pSpriteBatch->pSpriteCollection->apSprites[i]->height;
	}


	//Assemble batch data
	{
		UINT offset	{ 0 };
		bytewidth	= 
			sizeof(UINT) * sprCount * 2		+
			sizeof(BatchHeader)				+
			pRawImageData->bytewidth		+
			pRawBufferData->desc.ByteWidth	+ 
			sprCount * sizeof(RectU);

		pData		= new BYTE[bytewidth];

		BatchHeader header;

		header.BDesc			= pRawBufferData->desc;
		header.T2DDesc			= T2DDesc;
		header.SpriteCount		= sprCount;
		header.TextureRowPitch	= pRawImageData->RowPitch;
		header.TextureByteWidth = pRawImageData->bytewidth;

		memcpy(pData, &header, sizeof(BatchHeader));
		offset += sizeof(BatchHeader);
		memcpy(pData + offset, SpriteData, sizeof(UINT) * 2 * sprCount);
		offset += sizeof(UINT) * 2 * sprCount;
		memcpy(pData + offset, pRawImageData->pixelData, pRawImageData->bytewidth);
		offset += pRawImageData->bytewidth;
		memcpy(pData + offset, pRawBufferData->pData, pRawBufferData->desc.ByteWidth);
		offset += pRawBufferData->desc.ByteWidth;
		memcpy(pData + offset, pSpriteBatch->aCollisionCache, sizeof(RectU) * sprCount);
	}


	//Clean up used resources
	{
		SafeRelease(&pRawImageData);
		SafeRelease(&pRawBufferData);
	}


	//Save data to file
	{
		HANDLE file = CreateFileW(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			delete[] pData;

			DebugLog(L"ERROR: Texture manager : Save batch : Failed to create data file\n");
			return hr;
		}

		WriteFile(file, pData, bytewidth, &BytesWriten, NULL);
		delete[] pData;
		CloseHandle(file);

		if (BytesWriten == 0)
		{
			hr = E_FAIL;
		}
	}

	return hr;
}
HRESULT		TextureManager::CreateRawSprite					(LPCWSTR FileName, RawSprite** ppRawSprite)
{
	if ((*ppRawSprite) != NULL || FileName == NULL || ppRawSprite == NULL)
		return E_INVALIDARG;


	HRESULT hr;
	RawImageData* pRawImageData	{ NULL };
	D3D11_TEXTURE2D_DESC T2DDesc;
	D3D11_SUBRESOURCE_DATA SRDDesc;

	hr = DecodeImageToPixData(FileName, DxgiFormat, &pRawImageData);
	if FAILED(hr)
	{
		return hr;
	}

	(*ppRawSprite) = new RawSprite;

	T2DDesc.Format				= DxgiFormat;
	T2DDesc.ArraySize			= 1;
	T2DDesc.MipLevels			= 1;
	T2DDesc.Usage				= D3D11_USAGE_DEFAULT;
	T2DDesc.SampleDesc.Count	= 1;
	T2DDesc.SampleDesc.Quality	= 0;
	T2DDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	T2DDesc.CPUAccessFlags		= 0;
	T2DDesc.MiscFlags			= 0;
	T2DDesc.Width				= pRawImageData->width;
	T2DDesc.Height				= pRawImageData->height;
								  
	SRDDesc.pSysMem				= pRawImageData->pixelData;
	SRDDesc.SysMemPitch			= pRawImageData->RowPitch;
	SRDDesc.SysMemSlicePitch	= pRawImageData->bytewidth;

	hr = pDevice->CreateTexture2D(&T2DDesc, &SRDDesc, &(*ppRawSprite)->pTexture);
	if FAILED(hr)
	{
		SafeRelease(&pRawImageData);
		SafeRelease(ppRawSprite);
		return hr;
	}

	float Data[10]
	{
		(float)pRawImageData->width, (float)pRawImageData->height,
		0.f, 0.f,
		1.f, 0.f,
		0.f, 1.f,
		1.f, 1.f
	};

	hr = CreateStructuredBuffer(sizeof(float) * 10, 1, &(*ppRawSprite)->pSBuffer);
	if FAILED(hr)
	{
		SafeRelease(&pRawImageData);
		SafeRelease(ppRawSprite);
		return hr;
	}

	hr = pDevice->CreateShaderResourceView((*ppRawSprite)->pTexture, NULL, &(*ppRawSprite)->pSRVTexture);
	if FAILED(hr)
	{
		SafeRelease(&pRawImageData);
		SafeRelease(ppRawSprite);
		return hr;
	}

	pDeviceContext->UpdateSubresource((*ppRawSprite)->pSBuffer->GetBuffer(), 0, NULL, Data, sizeof(float) * 10, sizeof(float) * 10);
	SafeRelease(&pRawImageData);

	return hr;
}
HRESULT		TextureManager::ExtractImageData				(ID3D11Texture2D* pTexture2D, RawImageData** ppRawImageData)
{
	if(pTexture2D == NULL || (*ppRawImageData) != NULL)
		return E_INVALIDARG;


	ID3D11Texture2D*	pSTexture2D{ NULL };
	D3D11_TEXTURE2D_DESC T2DDesc;
	D3D11_MAPPED_SUBRESOURCE MSR;
	pTexture2D->GetDesc(&T2DDesc);

	T2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	T2DDesc.BindFlags = 0;
	T2DDesc.Usage = D3D11_USAGE_STAGING;


	HRESULT hr = pDevice->CreateTexture2D(&T2DDesc, NULL, &pSTexture2D);
	if FAILED(hr)
	{
		return hr;
	}

	pDeviceContext->CopyResource(pSTexture2D, pTexture2D);
	hr = pDeviceContext->Map(pSTexture2D, 0, D3D11_MAP_READ, 0, &MSR);
	if FAILED(hr)
	{
		SafeRelease(&pSTexture2D);
		return hr;
	}

	(*ppRawImageData)				= new RawImageData;
	(*ppRawImageData)->bytewidth	= MSR.DepthPitch;
	(*ppRawImageData)->height		= T2DDesc.Height;
	(*ppRawImageData)->width		= T2DDesc.Width;
	(*ppRawImageData)->RowPitch		= MSR.RowPitch;
	(*ppRawImageData)->pixelData	= new BYTE[MSR.DepthPitch];
	(*ppRawImageData)->pixelFormat	= T2DDesc.Format;

	memcpy((*ppRawImageData)->pixelData, MSR.pData, MSR.DepthPitch);
	pDeviceContext->Unmap(pSTexture2D, 0);
	SafeRelease(&pSTexture2D);

	return hr;
}
HRESULT		TextureManager::DecodeImageToPixData			(LPCWSTR FileName, DXGI_FORMAT DXGIFormat, RawImageData** ppRawImageData)
{

	if ( (ppRawImageData == NULL) || (FileName == NULL) || (pWICFactory == NULL) || ((*ppRawImageData) != NULL) )
	{
		if (pWICFactory == NULL)
			return E_NOINTERFACE;

		return E_INVALIDARG;
	}


	WICPixelFormatGUID	GUIDFormat	{ DXGIToGUID(DXGIFormat) };
	UINT				PixelSize	{ DXGIToPixelSize(DXGIFormat) };

	if ( GUIDFormat == GUID_WICPixelFormatUndefined || PixelSize == 0 )
	{
		DebugLog(L"ERROR: WICComponent : DecodeImageToPixData : Invalid format\n");
		return E_INVALIDARG;
	}

	HRESULT hr;

	IWICBitmapDecoder*		pDecoder		{ NULL };
	IWICBitmapFrameDecode*	pFrameDecode	{ NULL };

	hr = pWICFactory->CreateDecoderFromFilename(FileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
	if FAILED(hr)
	{
		DebugLog(L"\nDecodeImageToPixData :: Failed to create decoder for ");
		DebugLog(FileName);
		return hr;
	}

	hr = pDecoder->GetFrame(0, &pFrameDecode);
	if FAILED(hr)
	{
		SafeRelease(&pDecoder);
		return hr;
	}

	WICPixelFormatGUID DecodeFormat;
	hr = pFrameDecode->GetPixelFormat(&DecodeFormat);
	if FAILED(hr)
	{
		SafeRelease(&pFrameDecode);
		SafeRelease(&pDecoder);
		return hr;
	}


	UINT Width, Height, BufferSize;


	//Convert if needed
	if (DecodeFormat != GUIDFormat)
	{
		IWICFormatConverter* pFormatConverter { NULL };

		hr = pWICFactory->CreateFormatConverter(&pFormatConverter);
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
			DebugLog(L"DecodeImageToPixData :: Failed to convert ");
			DebugLog(FileName);

			return hr;
		}

		pFormatConverter->GetSize(&Width, &Height);
		BufferSize = Width*Height*PixelSize;

		(*ppRawImageData) = new RawImageData;

		(*ppRawImageData)->pixelData	= new BYTE[ BufferSize ];
		(*ppRawImageData)->height		= Height;
		(*ppRawImageData)->width		= Width;
		(*ppRawImageData)->pixelFormat	= DXGI_FORMAT_R8G8B8A8_UNORM;
		(*ppRawImageData)->RowPitch		= Width * PixelSize;
		(*ppRawImageData)->bytewidth	= BufferSize;

		hr = pFormatConverter->CopyPixels(NULL, Width*PixelSize, BufferSize, (*ppRawImageData)->pixelData);
		if FAILED(hr)
		{
			SafeRelease(ppRawImageData);
		}

		SafeRelease(&pFrameDecode);
		SafeRelease(&pDecoder);
		SafeRelease(&pFormatConverter);
		return hr;
	}


	pFrameDecode->GetSize(&Width, &Height);
	BufferSize = Width*Height*PixelSize;

	(*ppRawImageData) = new RawImageData;

	(*ppRawImageData)->pixelData	= new BYTE[ BufferSize ];
	(*ppRawImageData)->height		= Height;
	(*ppRawImageData)->width		= Width;
	(*ppRawImageData)->pixelFormat	= DXGIFormat;
	(*ppRawImageData)->RowPitch		= Width * PixelSize;
	(*ppRawImageData)->bytewidth	= BufferSize;

	hr = pFrameDecode->CopyPixels(NULL, Width*PixelSize, BufferSize, (*ppRawImageData)->pixelData);
	if FAILED(hr)
	{
		SafeRelease(&pFrameDecode);
		SafeRelease(&pDecoder);
		SafeRelease(ppRawImageData);
	}

	SafeRelease(&pFrameDecode);
	SafeRelease(&pDecoder);

	return hr;
}
HRESULT		TextureManager::Initialize						()
{
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if FAILED(hr)
	{
		DebugLog(L"ERROR: TextureManager : Initialize : Create WIC Factory Failed\n");
		Clear();
		return hr;
	}

	return hr;
}
void		TextureManager::Clear							()
{
	SafeRelease(&pWICFactory);
	SafeRelease(&pSpriteCollection);

	if (apSpriteBatch)
	{
		for(UINT i{0}; i < BatchCount; i++)
		{
			SafeRelease(&apSpriteBatch[i]);
		}
		delete[] apSpriteBatch;
	}
}
//***********************************************************************************************

#pragma warning(default:26451; default:6001; default:6386)