#pragma once
#ifndef GRAPHIC_HANDLER_H
#define GRAPHIC_HANDLER_H

#include "CommonDirectX.h"
#include "Resource.h"


namespace GRAPHICS
{


	HRESULT		InitializeGraphics		(HINSTANCE hInstance, int nCmdShow, LRESULT (*WindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam));
	void		UninitializeGraphics	();
	

	void		Render();
	HRESULT		GetInterface(IGraphics** ppGraphics);

	
	HRESULT		CreateRawSprite			(LPCWSTR FileName, RawSprite** ppRawSprite);
	HRESULT		CreateTextLayout		(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout);
	
	
	void		DrawSprite				(const SpriteID sprID, const float x, const float y);
	void		DrawSpriteEx			(const SpriteID spriteID, const float x, const float y, const float scaleX, const float scaleY);
	void		DrawLine				(float x1, float y1, float x2, float y2, const float color[4]);
	void		DrawRect				(const float x, const float y, const float width, const float height, const float color[4]);
	void		DrawString				(LPCWSTR string, float x, float y);
	void		DrawStaticText			(IDWriteTextLayout*	pTextLayout, const float x, const float y);
	void		DrawRawSprite			(RawSprite* pRawSprite, const float x, const float y);
	void		DrawRawSpriteEx			(RawSprite* pRawSprite, const float x, const float y, const float scaleX, const float scaleY);


}
#endif