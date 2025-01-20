#include "GraphicsDirectX.h"
#include "GraphicHandler.h"
#include "GraphicsDirect3D12.h"

IGraphics*	pGraphics	{ NULL };


namespace GRAPHICS
{

	//Must be uninitialized after
	// 	   
	//Use UninitializeGraphics()
	HRESULT		InitializeGraphics		(HINSTANCE hInstance, int nCmdShow, LRESULT (*WindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam))
{
	if (pGraphics)
	{
		DebugLog(L"\n***ERROR GRAPHIC INTERFACE ALLREADY INITIALIZED***\n");
		return E_FAIL;
	}

	HWND hwnd= CreateDefaultWindow(hInstance, nCmdShow, WindowProc);

	HRESULT hr = pGraphics->Initialize(hwnd);

	if FAILED(hr)
	{
		SafeRelease(&pGraphics);
		return hr;
	}

	return hr;
}
	void		UninitializeGraphics	()
	{
		SafeRelease(&pGraphics);
	}
	
	
	void		Render()
	{
		pGraphics->Render();
	}
	//Adds a reference release before Uninitializing!
	HRESULT		GetInterface(IGraphics** ppGraphics)
	{
		if(*ppGraphics)
			return E_INVALIDARG;

		if(!pGraphics)
			return E_NOINTERFACE;

		*ppGraphics = pGraphics;
		(*ppGraphics)->AddRef();

		return S_OK;
	}
	
	
	HRESULT		CreateRawSprite			(LPCWSTR FileName, RawSprite** ppRawSprite)
{
	return pGraphics->CreateRawSprite(FileName, ppRawSprite);
}
	HRESULT		CreateTextLayout		(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout)
{
	return pGraphics->CreateTextLayout(pDesc, ppTextLayout);
}
	
	
	void		DrawSprite				(const SpriteID sprID, const float x, const float y)
{

#ifdef SETTING_DEBUG

	if (!pGraphics)
	{
		DebugLog(L"DrawSprite Error : Graphics interface not initialized\n");
		return;
	}

#endif

	pGraphics->DrawSprite(sprID, x, y);

}
	void		DrawSpriteEx			(const SpriteID spriteID, const float x, const float y, const float scaleX, const float scaleY)
{
	pGraphics->DrawSpriteEx(spriteID, x, y, scaleX, scaleY);
}
	void		DrawLine				(float x1, float y1, float x2, float y2, const float color[4])
{
#ifdef SETTING_DEBUG

	if (!pGraphics)
	{
		DebugLog(L"DrawSprite Error : Graphics interface not initialized\n");
		return;
	}

#endif

	pGraphics->DrawLine(x1, y1, x2, y2, color);
}
	void		DrawRect				(const float x, const float y, const float width, const float height, const float color[4])
{
	pGraphics->DrawRect(x, y, width, height, color);
}
	void		DrawString				(LPCWSTR string, float x, float y)
{
	pGraphics->DrawDynamicText(string, x, y);
}
	void		DrawStaticText			(IDWriteTextLayout*	pTextLayout, const float x, const float y)
{
	pGraphics->DrawStaticText(pTextLayout, x, y);
}
	void		DrawRawSprite			(RawSprite* pRawSprite, const float x, const float y)
{
	pGraphics->DrawRawSprite(pRawSprite, x, y);
}
	void		DrawRawSpriteEx			(RawSprite* pRawSprite, const float x, const float y, const float scaleX, const float scaleY)
{
	pGraphics->DrawRawSpriteEx(pRawSprite, x, y, scaleX, scaleY);
}


}