#pragma once
#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

#include "GraphicsDirectX.h"
#include "GraphicHandler.h"
#include <windowsx.h>

#pragma comment(lib, "Graphics.lib")

float	G_WHEEL			= 1.f;
float	G_MOUSE_X_PREV	= 0.f;
float	G_MOUSE_Y_PREV	= 0.f;
float	G_MOUSE_SHIFT_X	= 0.f;
float	G_MOUSE_SHIFT_Y	= 0.f;
float	G_MOUSE_X		= 0;
float	G_MOUSE_Y		= 0;
bool	G_MOUSE_LCLICK	= false;

bool CollisionPointRect(PointF point, RectF rect)
{
	return ( rect.x < point.x && rect.x2 > point.x && rect.y < point.y && rect.y2 > point.y );
}

bool GetFile	(wchar_t* File, int len)
{
	File[0] = L'\0';

	OPENFILENAME OFN;

	ZeroMemory(&OFN, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);

	IGraphics* pGraphics { NULL };
	GRAPHICS::GetInterface(&pGraphics);

	OFN.hwndOwner = pGraphics->GetHWND();

	SafeRelease(&pGraphics);

	OFN.nFilterIndex = 1;
	OFN.lpstrFile = NULL;
	OFN.nMaxFile = len;
	OFN.lpstrFileTitle = NULL;
	OFN.nMaxFileTitle = 0;
	OFN.lpstrFile = File;
	OFN.lpstrInitialDir = NULL;
	OFN.lpstrTitle = NULL;
	OFN.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	OFN.FlagsEx = 0;
	OFN.nFileOffset = 0;
	OFN.lpstrDefExt = NULL;
	OFN.lCustData = NULL;
	OFN.lpTemplateName = NULL;

	bool result = GetOpenFileName(&OFN);

	POINT point;
	GetCursorPos(&point);
	ScreenToClient(OFN.hwndOwner, &point);

	

	G_MOUSE_X_PREV = (float)point.x;
	G_MOUSE_Y_PREV = (float)point.y;

	return result;
}
bool SaveAsFile	(wchar_t* File, int len)
{
	File[0] = L'\0';

	OPENFILENAME OFN;

	wchar_t title[256];
	title[0] = L'\0';

	ZeroMemory(&OFN, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);

	IGraphics* pGraphics { NULL };
	GRAPHICS::GetInterface(&pGraphics);

	OFN.hwndOwner = pGraphics->GetHWND();

	SafeRelease(&pGraphics);

	OFN.lpstrFileTitle = title;
	OFN.nMaxFileTitle = 256;
	OFN.nFilterIndex = 1;
	OFN.lpstrFilter = L".batch";
	OFN.nMaxFile = len;
	OFN.lpstrFile = File;

	bool result = GetSaveFileName(&OFN);

	POINT point;
	GetCursorPos(&point);
	ScreenToClient(OFN.hwndOwner, &point);

	G_MOUSE_X_PREV = (float)point.x;
	G_MOUSE_Y_PREV = (float)point.y;

	return result;
}

class UISprite
{
protected:
	float		x			{ 0 };
	float		y			{ 0 };
	float		width		{ 0 };
	float		height		{ 0 };
	RawSprite*	pRawSprite	{ NULL };
	int			selected	{ 0 };

	float scale { 1.f };

	void Draw()
	{
		if (pRawSprite == NULL)
			return;

		float color[] { float(selected), 0.f, 0.f, 1.f };

		float x2 { x + width * scale };
		float y2 { y + height * scale };

		GRAPHICS::DrawLine( x , y , x2, y , color );
		GRAPHICS::DrawLine( x2, y , x2, y2, color );
		GRAPHICS::DrawLine( x2, y2, x , y2, color );
		GRAPHICS::DrawLine( x , y2, x , y , color );
		GRAPHICS::DrawRawSpriteEx(pRawSprite, x, y, scale, scale);
	}

	RectF GetCollisionBox()
	{
		return { x, y, x + width * scale, y + height * scale };
	}

	UISprite(){};
	~UISprite()
	{
		SafeRelease(&pRawSprite);
	}

public:

	HRESULT SetSprite(RawSprite* pSprite)
	{
		if( pSprite == NULL )
		{
			return E_INVALIDARG;
		}
		if( pRawSprite != NULL )
		{
			SafeRelease(&pRawSprite);
		}

		pRawSprite = pSprite;
		pSprite->AddRef();
		pSprite->GetSize(&width, &height);

		return S_OK;
	}
	void FreeSprite()
	{
		SafeRelease(&pRawSprite);
	}

	friend class UIBatch;

};
class UIButton
{
	float x;
	float y;
	float width;
	float height;

	DWRITE_TEXTLAYOUT_DESC TLDesc;

	RectF collisionRect;

	IDWriteTextLayout* pTextLayout { NULL };

public:

	UIButton(float x, float y, float width, float height, void (*OnClick)()) : x{x}, y{y}, width{width}, height{height}, OnClick{OnClick} 
	{
		collisionRect.x = x;
		collisionRect.y = y;
		collisionRect.x2 = x + width;
		collisionRect.y2 = y + height;

		TLDesc.Text = L"No text";
		TLDesc.StringLength = (UINT)wcslen(TLDesc.Text);
		TLDesc.Width = (UINT)width;
		TLDesc.Height = (UINT)height;
		TLDesc.FontSize = 20;

		GRAPHICS::CreateTextLayout(&TLDesc, &pTextLayout);
	}

	~UIButton()
	{
		SafeRelease(&pTextLayout);
	}

	void (*OnClick)();

	void Draw()
	{
		static float color[4]{ 0.5, 0.5, 0.5, 1 };
		GRAPHICS::DrawStaticText(pTextLayout, x, y);
		GRAPHICS::DrawRect(x, y, width, height, color);
	}
	RectF GetRect()
	{
		return collisionRect;
	}
	void SetText(const wchar_t* string)
	{
		SafeRelease(&pTextLayout);

		TLDesc.Text = string;
		TLDesc.StringLength = (UINT)wcslen(string);

		GRAPHICS::CreateTextLayout(&TLDesc, &pTextLayout);
	}
};
class UIBatch
{
public:

	//flags
	// 0x1 = loaded

	float	x				{ 0 };
	float	y				{ 0 };
	float	width			{ 4096.f };
	float	height			{ 4096.f };
	float	spriteOffsetX	{ 0 };
	float	spriteOffsetY	{ 0 };
	UINT	flag			{ 0 };
	ISurface* pSurface		{ NULL };

	float scale { 1.f };
	UISprite sprite;

	void Draw()
	{
		if(!(flag && 0x1))
			return;

		float color[] { 1.f, 1.f, 1.f, 1.f };

		float x2 { x + width * scale };
		float y2 { y + height * scale };

		GRAPHICS::DrawLine(x,  y,  x2, y, color);
		GRAPHICS::DrawLine(x2, y,  x2, y2, color);
		GRAPHICS::DrawLine(x2, y2, x,  y2, color);
		GRAPHICS::DrawLine(x,  y2, x,  y, color);
		GRAPHICS::DrawSpriteEx(SPRITE::Batch, x, y, scale, scale);

		sprite.Draw();

		if(pSurface != NULL)
		{
			IGraphics* pGraphics { NULL };
			GRAPHICS::GetInterface(&pGraphics);

			pGraphics->DrawSurface(pSurface, 0, 0);

			SafeRelease(&pGraphics);
		}
	}

	void Action()
	{
		if(!(flag && 0x1))
			return;

		sprite.selected = CollisionPointRect( {G_MOUSE_X, G_MOUSE_Y}, sprite.GetCollisionBox() );

		if (!sprite.selected)
		{
			x += G_MOUSE_SHIFT_X;
			y += G_MOUSE_SHIFT_Y;
		}
		else
		{
			spriteOffsetX += G_MOUSE_SHIFT_X * (1/scale);
			spriteOffsetY += G_MOUSE_SHIFT_Y * (1/scale);
		}

		scale			= G_WHEEL;
		sprite.scale	= scale;
		sprite.x		= x + ((int)spriteOffsetX - ((int)spriteOffsetX % 16)) * scale;
		sprite.y		= y + ((int)spriteOffsetY - ((int)spriteOffsetY % 16)) * scale;
	}

	HRESULT AddSprite()
	{
		if(!(flag && 0x1) || (sprite.pRawSprite == NULL))
			return E_FAIL;

		int x { (int)spriteOffsetX };
		int y { (int)spriteOffsetY };
		x -= x % 16;
		y -= y % 16;
		
		if (x < 0 || y < 0 || (x + (int)sprite.width) >= 4096 || (y + sprite.height) >= 4096)
			return E_FAIL;

		RawImageData* pRawImageData{ NULL };
		ID3D11Texture2D* pTexture2D{ NULL };
		sprite.pRawSprite->GetTexture(&pTexture2D);

		IGraphics* pGraphics { NULL };
		GRAPHICS::GetInterface(&pGraphics);

		pGraphics->ExtractImageData(pTexture2D, &pRawImageData);
		HRESULT hr = pGraphics->AddSpriteToBatch(pRawImageData, {(UINT)x, (UINT)y}, 0);

		SafeRelease(&pGraphics);
		SafeRelease(&pTexture2D);
		SafeRelease(&pRawImageData);

		return hr;
	}

	~UIBatch()
	{
		SafeRelease(&pSurface);
	}
};


UIBatch	batch;

void ButtonCreateBatch	()
{
	IGraphics* pGraphics { NULL };
	GRAPHICS::GetInterface(&pGraphics);

	pGraphics->CreateSpriteBatch();
	pGraphics->InitializeBatch(0U);
	batch.flag |= 0x1;

	SafeRelease(&pGraphics);
}
void ButtonSaveBatch	()
{
	wchar_t file[256];
	if(!SaveAsFile(file, 256))
		return;

	wchar_t* extention = wcsrchr(file, L'.');
	if(extention)
	{
		if(wcscmp(extention, L".batch"))
		{
			int size = 256 - (extention - file);
			memcpy(extention, L".batch", sizeof(wchar_t) * 7);
		}
	}
	else
	{
		wcscat_s(file, 256, L".batch");
	}
	
	IGraphics* pGraphics { NULL };
	GRAPHICS::GetInterface(&pGraphics);

	pGraphics->SaveBatch(0, file);

	SafeRelease(&pGraphics);
}
void ButtonAddSprite	()
{
	batch.AddSprite();
}
void ButtonLoadBatch	()
{
	wchar_t file[256];
	IGraphics* pGraphics { NULL };

	GRAPHICS::GetInterface(&pGraphics);

	if(!GetFile(file, 256))
		return;

	if FAILED(pGraphics->LoadBatch(file))
		return;

	batch.flag |= 0x1;

	pGraphics->InitializeBatch(0);

	SafeRelease(&pGraphics);
}
void ButtonLoadSprite	()
{
	RawSprite* pRawSprite{ NULL };
	wchar_t File[256]{ 0 };
	IGraphics* pGraphics { NULL };

	GRAPHICS::GetInterface(&pGraphics);

	if (!GetFile(File, 256))
	{
		return;
	}

	if FAILED(pGraphics->CreateRawSprite(File, &pRawSprite))
	{
		return;
	}

	batch.sprite.SetSprite(pRawSprite);
	SafeRelease(&pRawSprite);
	SafeRelease(&pGraphics);
}
void ButtonRunScript	()
{
	IGraphics* pGraphics { NULL };
	GRAPHICS::GetInterface(&pGraphics);

	pGraphics->CreateSurface(&batch.pSurface, 50, 50);
	pGraphics->SetSurface(batch.pSurface);
	GRAPHICS::DrawSprite(SPRITE::Twitter, 30, 30);
	pGraphics->ResetSurface();


	float color[4] = {0.f, 0.f, 0.f, 1.f};

	pGraphics->SetBGColor(color);
	SafeRelease(&pGraphics);
}

class UIPanel: public Reference
{
private:

	float height	{ 600 };
	float width		{ 100 };
	float x			{ SETTING_RESOLUTION_WIDTH - width };
	float y			{ SETTING_RESOLUTION_HEIGHT - height };
	

	~UIPanel()
	{
		for(UINT i{ 0 }; i < numButtons; i++)
			delete apButton[i];
	};

	UINT numButtons{ 6 };
	UIButton* apButton[ 6 ];

protected:

	

public:

	UIPanel()
	{
		apButton[0] = new UIButton(x, SETTING_RESOLUTION_HEIGHT - 100, 100, 100, &ButtonAddSprite);
		apButton[1] = new UIButton(x, SETTING_RESOLUTION_HEIGHT - 200, 100, 100, &ButtonSaveBatch);
		apButton[2] = new UIButton(x, SETTING_RESOLUTION_HEIGHT - 300, 100, 100, &ButtonLoadBatch);
		apButton[3] = new UIButton(x, SETTING_RESOLUTION_HEIGHT - 400, 100, 100, &ButtonLoadSprite);
		apButton[4] = new UIButton(x, SETTING_RESOLUTION_HEIGHT - 500, 100, 100, &ButtonCreateBatch);
		apButton[5] = new UIButton(x, SETTING_RESOLUTION_HEIGHT - 600, 100, 100, &ButtonRunScript);
		apButton[0]->SetText(L"Add Sprite");
		apButton[1]->SetText(L"Save Batch");
		apButton[2]->SetText(L"Load Batch");
		apButton[3]->SetText(L"Load Sprite");
		apButton[4]->SetText(L"Create New");
		apButton[5]->SetText(L"Run script");
	};

	void Draw()
	{
		for(UINT i{ 0 }; i < numButtons; i++)
			apButton[i]->Draw();
	};

	void OnClick()
	{
		for(UINT i { 0 }; i < numButtons; i++)
		{
			if( CollisionPointRect({G_MOUSE_X, G_MOUSE_Y}, apButton[i]->GetRect()))
			{
				apButton[i]->OnClick();
				break;
			}
		}
	};

	RectF GetCollisionBox()
	{
		return { x, y, x + width, y + height };
	}
};
#endif