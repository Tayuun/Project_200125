#include "GraphicsDirectX.h"
#include "CommonDirectX.h"
#include "Resource.h"



//*******************************Direct2DComponent***********************************************
HRESULT		Direct2DComponent::Initialize		(IDXGISwapChain* pSwapChain)
{
	if(pSwapChain == NULL)
	{
		return E_INVALIDARG;
	}

	this->pSwapChain = pSwapChain;
	pSwapChain->AddRef();

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2D1Factory);
	if FAILED(hr)
	{
		Clear();
		return hr;
	}

	//Render Target and DXGISurface
	{
		hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDXGISurface));
		if FAILED(hr)
		{
			Clear();
			return hr;
		}

		D2D1_RENDER_TARGET_PROPERTIES rtp;

		rtp = D2D1::RenderTargetProperties
		(
			D2D1_RENDER_TARGET_TYPE_DEFAULT, 
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), 
			0, 
			0
		);

		hr = pD2D1Factory->CreateDxgiSurfaceRenderTarget(pDXGISurface, &rtp, &pRenderTarget);
		if FAILED(hr)
		{
			Clear();
			return hr;
		}

		pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
	}

	D2D1_COLOR_F color{ 0.f, 0.f, 0.f, 1.f};

	hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
	if FAILED(hr)
	{
		Clear();
		return hr;
	}

	//DWrite and Text Format
	{
		pDWrite = new DWriteComponent;

		hr = pDWrite->Initialize();
		if FAILED(hr)
		{
			Clear();
			return hr;
		}

		hr = pDWrite->CreateTextFormat(&pTextFormat, L"Arial", 40.f);
		if FAILED(hr)
		{	
			Clear();
			return hr;
		}
	}

	return hr;
}
HRESULT		Direct2DComponent::Render			()
{
#ifdef SETTING_DEBUG
	if(pRenderTarget == NULL)
	{
		return E_NOINTERFACE;
	}
#endif

	pRenderTarget->BeginDraw();

	D2D1_RECT_F rect{0.f, 0.f, 500.f, 500.f};

	pRenderTarget->DrawTextW(L"DWrite works!", 13, pTextFormat, rect, pBrush);

	return pRenderTarget->EndDraw();
}
void		Direct2DComponent::BeginDraw		()
{
	pRenderTarget->BeginDraw();
}
HRESULT		Direct2DComponent::EndDraw			()
{
	return pRenderTarget->EndDraw();
}
void		Direct2DComponent::DrawDynamicText	(LPCWSTR string, float x, float y)
{
	pRenderTarget->DrawTextW(string, (UINT)wcslen(string), pTextFormat, {x, y, x + 500.f, y + 500.f}, pBrush);
}
void		Direct2DComponent::DrawStaticText	(IDWriteTextLayout* pTextLayout, const float x, const float y)
{
	pRenderTarget->DrawTextLayout( {x, y}, pTextLayout, pBrush );
}
HRESULT		Direct2DComponent::CreateTextLayout	(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout)
{
	return pDWrite->CreateTextLayout(pDesc, ppTextLayout);
}
void		Direct2DComponent::Clear			()
{
	SafeRelease(&pTextFormat);
	SafeRelease(&pBrush);
	SafeRelease(&pDWrite);
	SafeRelease(&pRenderTarget);
	SafeRelease(&pDXGISurface);
	SafeRelease(&pSwapChain);
	SafeRelease(&pD2D1Factory);
}
//***********************************************************************************************


//*******************************DWriteComponent*************************************************
HRESULT		DWriteComponent::Initialize			()
{
	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if FAILED(hr)
	{
		Clear();
		return hr;
	}

	return S_OK;
}
HRESULT		DWriteComponent::CreateTextLayout	(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout)
{
	if (pDesc == NULL || ppTextLayout == NULL || (*ppTextLayout) != NULL || pDesc->Text == NULL)
		return E_INVALIDARG;


	IDWriteTextFormat*	pTextFormat	{ NULL };
	IDWriteTextLayout*	pTextLayout { NULL };

	HRESULT hr = pDWriteFactory->CreateTextFormat
	(
		pDesc->Font,
		pDesc->FontCollection,
		pDesc->FontWeight,
		pDesc->FontStyle,
		pDesc->FontStretch,
		pDesc->FontSize,
		L"en-us",
		&pTextFormat
	);
	if FAILED(hr)
	{
		return E_FAIL;
	}

	hr = pDWriteFactory->CreateTextLayout(pDesc->Text, pDesc->StringLength, pTextFormat, (FLOAT)pDesc->Width, (FLOAT)pDesc->Height, &pTextLayout);
	if FAILED(hr)
	{
		SafeRelease(&pTextFormat);
		return E_FAIL;
	}

	(*ppTextLayout) = pTextLayout;
	SafeRelease(&pTextFormat);

	return S_OK;
}
HRESULT		DWriteComponent::CreateTextFormat	(IDWriteTextFormat** ppTextFormat, LPCWSTR FontName, const float FontSize)
{
	if(FontSize <= 0)
	{
		return E_INVALIDARG;
	}
	HRESULT hr = pDWriteFactory->CreateTextFormat(FontName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FontSize, L"en-us", ppTextFormat);
	if FAILED(hr)
	{
		return hr;
	}

	return hr;
}
void		DWriteComponent::Clear				()
{
	SafeRelease(&pDWriteFactory);
}
//***********************************************************************************************