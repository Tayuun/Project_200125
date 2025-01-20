#pragma once
#ifndef PROJECT_RESOURCE_H
#define PROJECT_RESOURCE_H

#include "CommonDirectX.h"

#pragma warning(disable:26451; disable:26495)

//***********FORWARD DEC***********
class IGraphics;
class Mesh;
class SpriteCollection;
class ISurface;

struct StructBuffer;

struct RectF;
struct RectU;

inline WICPixelFormatGUID	DXGIToGUID(DXGI_FORMAT DXGIFormat)
{
	switch (DXGIFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return GUID_WICPixelFormat32bppRGBA;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return GUID_WICPixelFormat32bppBGRA;
	default:
		return GUID_WICPixelFormatUndefined;
	}
}
inline UINT					DXGIToPixelSize(DXGI_FORMAT DXGIFormat)
{
	switch (DXGIFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return 4;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return 4;
	default:
		return 0;
	}
}


bool CollisionRectU(RectU rect1, RectU rect2);

HRESULT CreateSimpleMesh(Mesh** ppMesh);
HRESULT	CreateCircleMesh(Mesh**	ppMesh);
int		Noise(float x, float y);
template<class T>
void ArrayPush(T** Array, UINT ArraySize, T Element)
{
	if(ArraySize == 0)
	{
		(*Array) = new T[1];
		(*Array)[0] = Element;
		return;
	}

	T* SwapArray = new T[ArraySize + 1];
	memcpy(SwapArray, (*Array), sizeof(T) * ArraySize);
	SwapArray[ArraySize] = Element;
	delete[] (*Array);
	(*Array) = SwapArray;
}
template<class T>
void ArrayRemove(T** Array, UINT ArraySize, UINT ArrayPosition)
{
	if(ArraySize == 0 || ArrayPosition >= ArraySize || Array == NULL)
		return;

	if(ArraySize == 1)
	{
		delete[] (*Array);
		(*Array) = NULL;
		return;
	}

	T* SwapArray = new T[ArraySize - 1];

	memcpy(SwapArray, (*Array), sizeof(T) * ArrayPosition);
	memcpy(&SwapArray[ArrayPosition], &(*Array)[ArrayPosition + 1], sizeof(T) * (ArraySize - ArrayPosition - 1));

	delete[] (*Array);
	(*Array) = SwapArray;
}

HWND		CreateDefaultWindow(HINSTANCE hInstance, int nCmdShow, LRESULT(*WindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam));
//*********************************


//************TYPES****************
typedef UINT SpriteID;
typedef UINT BatchID;
//*********************************


//************STRUCTS**************
struct	SETTINGS_DESC
{
	UINT width;
	UINT height;
};

struct	RectU
{
	UINT x;
	UINT y;
	UINT x2;
	UINT y2;
};
struct	RectF
{
	FLOAT x;
	FLOAT y;
	FLOAT x2;
	FLOAT y2;

	operator RectU()
	{
		return { (UINT)x, (UINT)y, (UINT)x2, (UINT)y2 };
	}
};
struct	PointU
{
	UINT x;
	UINT y;
};
struct	PointF
{
	float x;
	float y;
};

struct	Sprite: public Reference
{
public:

	SpriteID	spriteID	{ 0 };
	BatchID		batchID		{ 0 };
	float		width		{ 0 };
	float		height		{ 0 };

protected:

	Sprite(){};

private:

	~Sprite(){};

	friend class TextureManager;
};

struct	DWRITE_TEXTLAYOUT_DESC
{
	const wchar_t*			Text			{ NULL };
	UINT					StringLength	{ 8 };
	UINT					Width			{ 100 };
	UINT					Height			{ 100 };
	LPCWSTR					Font			{ L"Arial" };
	IDWriteFontCollection*	FontCollection	{ NULL };
	DWRITE_FONT_WEIGHT		FontWeight		{ DWRITE_FONT_WEIGHT_NORMAL };
	DWRITE_FONT_STYLE		FontStyle		{ DWRITE_FONT_STYLE_NORMAL };
	DWRITE_FONT_STRETCH		FontStretch		{ DWRITE_FONT_STRETCH_NORMAL };
	FLOAT					FontSize		{ 10.f };
};

struct	RawSprite: public Reference
{
public:

	UINT GetWidth()
	{
		D3D11_TEXTURE2D_DESC T2DDesc;
		pTexture->GetDesc(&T2DDesc);

		return T2DDesc.Width;
	}
	UINT GetHeight()
	{
		D3D11_TEXTURE2D_DESC T2DDesc;
		pTexture->GetDesc(&T2DDesc);

		return T2DDesc.Height;
	}
	void GetSize(UINT* width, UINT* height)
	{
		D3D11_TEXTURE2D_DESC T2DDesc;
		pTexture->GetDesc(&T2DDesc);

		*width = T2DDesc.Width;
		*height = T2DDesc.Height;
	}
	void GetSize(float* width, float* height)
	{
		D3D11_TEXTURE2D_DESC T2DDesc;
		pTexture->GetDesc(&T2DDesc);

		*width	= (float)T2DDesc.Width;
		*height = (float)T2DDesc.Height;
	}
	HRESULT GetTexture(ID3D11Texture2D** ppTexture2D)
	{
		if((*ppTexture2D) != NULL)
			return E_INVALIDARG;

		(*ppTexture2D) = pTexture;
		pTexture->AddRef();

		return S_OK;
	}

protected:

	ID3D11Texture2D*			pTexture	{ NULL };
	StructBuffer*				pSBuffer	{ NULL };
	ID3D11ShaderResourceView*	pSRVTexture	{ NULL };

	RawSprite(){};
	
private:

	~RawSprite()
	{
		SafeRelease(&pTexture);
		SafeRelease(&pSBuffer);
		SafeRelease(&pSRVTexture);
	}

	friend class TextureManager;
	friend class IGraphics;

};
struct	RawImageData: public Reference
{
	UINT		width		{ 0 };
	UINT		height		{ 0 };
	UINT		bytewidth	{ 0 };
	UINT		RowPitch	{ 0 };
	DXGI_FORMAT	pixelFormat { DXGI_FORMAT_R8G8B8A8_TYPELESS };
	BYTE*		pixelData	{ NULL };

protected:

	RawImageData(){};

private:

	~RawImageData()
	{
		if (pixelData)
		{
			delete[] pixelData;
		}
	}

	friend class TextureManager;
};
struct	RawBufferData: Reference
{
	D3D11_BUFFER_DESC	desc;
	BYTE*	pData		{ NULL };

private:

	~RawBufferData()
	{
		delete[] pData;
	}

	friend class GraphicsPSC;
};

struct	StructBuffer: public Reference
{
public:

	ID3D11Buffer*				GetBuffer	()
	{
		return pBuffer;
	};
	ID3D11ShaderResourceView*	GetSRV		()
	{
		return pSRV;
	};
	ID3D11ShaderResourceView**	GetPpSRV	()
	{
		return &pSRV;
	}

private:

	~StructBuffer()
	{
		SafeRelease(&pSRV);
		SafeRelease(&pBuffer);
	};

protected:

	ID3D11Buffer*				pBuffer	{ NULL };
	ID3D11ShaderResourceView*	pSRV	{ NULL };

	StructBuffer(){};

	friend class GraphicsPSC;

	void operator=(StructBuffer* pSBuffer)
	{
		SafeRelease(&pBuffer);
		SafeRelease(&pSRV);

		pBuffer = pSBuffer->pBuffer;
		pSRV	= pSBuffer->pSRV;

		pSRV->AddRef();
		pBuffer->AddRef();
	}
};

struct	GRAPHICS_SURFACE_DESC
{
	UINT width;
	UINT height;
};

struct	Vertex
{
	DirectX::XMFLOAT3 pos;
	UINT VertexID;
};
class	Mesh: public Reference
{
protected:

	Mesh(){};
	
public:

	UINT	indexCount	{0};
	UINT	vertexCount	{0};
	Vertex* vertex		{NULL};
	UINT*	index		{NULL};

	friend class IGraphics;
	friend HRESULT CreateSimpleMesh(Mesh** ppMesh);
	friend HRESULT CreateCircleMesh(Mesh** ppMesh);

private:

	~Mesh()
	{
		if (vertex)
			delete[] vertex;
		if (index)
			delete[] index;
	}
};

class	ShaderBlob
{
protected:

	UINT		Size		{0};
	BYTE*		Buffer		{ NULL };

public:

				~ShaderBlob	()
	{
		if (Buffer)
			delete[] Buffer;
	}

	const BYTE*	GetBuffer	()
	{
		return Buffer;
	}
	UINT		GetSize		()
	{
		return Size;
	}
	HRESULT		ReadCSOFile	(const wchar_t* CSOFileName)
	{
		if (Buffer)
		{
			DebugLog(L"\nERROR::ShaderBlop:ReadCSOFile: BUFFER ALREADY IN USE");
			return E_FAIL;
		}

		HANDLE CSOFile = CreateFile(CSOFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (CSOFile == INVALID_HANDLE_VALUE)
		{
			DebugLog(L"\nERROR::ShaderBlob:ReadCSOFile: FAILED TO OPEN FILE");
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				DebugLog(L"\nFile not found ");

			return E_FAIL;
		}

		Size = GetFileSize(CSOFile, NULL);
		Buffer = new BYTE[Size];

		DWORD	BytesRead{ 0 };

		BOOL Result = ReadFile(CSOFile, Buffer, Size, &BytesRead, NULL);
		CloseHandle(CSOFile);

		if (!Result)
		{
			DebugLog(L"\nERROR::ShaderBlob:ReadCSOFile: FAILED TO READ VS FILE");
			Clear();
			return E_FAIL;
		}

		return S_OK;
	}

	void		Clear		()
	{
		Size = 0;
		if (Buffer)
			delete[] Buffer;
	};
};

class	SpriteCollection: public Reference
{
public:

	HRESULT AddSprite(Sprite* pSprite);
	HRESULT RemoveSprite(SpriteID sprID);

protected:

	UINT		SpriteCount { 0 };
	Sprite**	apSprites	{ NULL };

	SpriteCollection(){};

private:

	~SpriteCollection()
	{
		if(apSprites)
		{
			for(UINT i { 0 }; i < SpriteCount; i++)
			{
				SafeRelease(&apSprites[i]);
			}

			delete[] apSprites;
		}
	};

	friend class TextureManager;
	friend class Batch;
	friend class SpriteBatch;
};
class	SpriteBatch:public Reference
{
public:

	bool CheckCollision(RectU* pRect)
	{
		if(pRect->x < 0 || pRect->x >= SETTING_BATCH_WIDTH || pRect->y < 0 || pRect->y >= SETTING_BATCH_HEIGHT)
			return true;

		for(UINT i { 0 }; i < pSpriteCollection->SpriteCount; i++)
		{
			if(CollisionRectU(*pRect, aCollisionCache[i]))
				return true;
		}

		return false;
	};

protected:

	BatchID						batchID				{ 0 };

	ID3D11ShaderResourceView*	pSRVTexture			{ NULL };
	ID3D11Texture2D*			pTexture			{ NULL };
	StructBuffer*				pSBuffer			{ NULL };

	SpriteCollection*			pSpriteCollection	{ NULL };
	RectU*						aCollisionCache		{ NULL };

	SpriteBatch(){};
	HRESULT AddSprite(Sprite* pSprite, RectU CollisionRect)
	{
		if FAILED(pSpriteCollection->AddSprite(pSprite))
			return E_FAIL;

		ArrayPush(&aCollisionCache, pSpriteCollection->SpriteCount - 1U, CollisionRect);

		return S_OK;
	};

private:

	~SpriteBatch()
	{
		SafeRelease(&pSRVTexture);
		SafeRelease(&pTexture);
		SafeRelease(&pSBuffer);
		SafeRelease(&pSpriteCollection);
	};

	friend class TextureManager;
};
struct BatchHeader
{
	UINT					SpriteCount			{ 0 };
	UINT					TextureRowPitch		{ 0 };
	UINT					TextureByteWidth	{ 0 };
	D3D11_BUFFER_DESC		BDesc;
	D3D11_TEXTURE2D_DESC	T2DDesc;
};

namespace Graphics
{
	struct LINE
	{
		float position[4];
		float color[4];
	};
};

struct SpriteBuffer
{
	float x;
	float y;
	UINT sprID;
	UINT pad;
};
struct LineBuffer
{
	float x1;
	float y1;
	float x2;
	float y2;
	float color[4];
};
struct SpriteExBuffer
{
	float x;
	float y;
	float scaleX;
	float scaleY;
	UINT sprID;
	float depth;
	UINT pad[2];
};
struct RectBuffer
{
	float x;
	float y;
	float width;
	float height;
	float color[4];
};
struct StringBuffer
{
	wchar_t*	pString;
	float		x;
	float		y;
};
struct RawSpriteBuffer
{
	float x;
	float y;
	RawSprite* pSprite;
};
struct RawSpriteExBuffer
{
	float x;
	float y;
	float scaleX;
	float scaleY;
	SpriteID sprID;
	float	depth;
	RawSprite* pRawSprite;
};
struct StaticStringBuffer
{
	float x;
	float y;
	IDWriteTextLayout* pTextLayout;
};
struct SurfaceBuffer{
	ISurface* pSurface { NULL };
	float x;
	float y;
};

//*********************************
template<class T, UINT BufferSize>
class DrawQueueBuffer
{
protected:

	DrawQueueBuffer()
	{
		ZeroMemory(Buffer, sizeof(T) * BufferSize);
	};

	UINT Count{ 0 };
	T Buffer[BufferSize];

	HRESULT Add(T inst)
	{
		if(Count == BufferSize)
			return E_FAIL;

		Buffer[Count] = inst;
		Count++;

		return S_OK;
	};

	friend class TextureManager;
	friend class DrawQueue;
	friend class IGraphics;
};

class DrawQueue: public Reference
{
private:

	UINT QMaxRawSprite		{50};
	UINT QMaxRawSpriteEx	{50};
	UINT QMaxString			{50};
	UINT QMaxStaticString	{50};

	~DrawQueue()
	{
		delete[] QRawSpriteBuffer;
		delete[] QRawSpriteExBuffer;
		delete[] QStringBuffer;
		delete[] QStaticStringBuffer;
	};

protected:

	DrawQueueBuffer<SpriteBuffer, 100>	QSpriteBuffer;
	DrawQueueBuffer<SpriteExBuffer, 50> QSpriteExBuffer;
	DrawQueueBuffer<LineBuffer, 50>		QLineBuffer;
	DrawQueueBuffer<RectBuffer, 50>		QRectBuffer;
	DrawQueueBuffer<SurfaceBuffer, 10>	QSurfaceBuffer;

	UINT					QStringCount		{ 0 };
	StringBuffer*			QStringBuffer		{ NULL };

	UINT					QStaticStringCount	{ 0 };
	StaticStringBuffer*		QStaticStringBuffer	{ NULL };

	UINT					QRawSpriteCount		{ 0 };
	RawSpriteBuffer* 		QRawSpriteBuffer	{ NULL };

	UINT					QRawSpriteExCount	{ 0 };
	RawSpriteExBuffer*		QRawSpriteExBuffer	{ NULL };

	DrawQueue(){};

	HRESULT Initialize();

public:

	HRESULT AddString		(const wchar_t* str, const float x, const float y);
	HRESULT AddStaticText	(IDWriteTextLayout* pTextLayout, const float x, const float y);
	HRESULT AddRawSprite	(RawSprite* pRawSprite, const float x, const float y);
	HRESULT AddRawSpriteEx	(RawSprite* pRawSprite, const float x, const float y, const float scaleX, const float scaleY);

	friend class IGraphics;

};

class RenderInstance: public Reference
{
protected:
	DrawQueue*					pDrawQueue			{ NULL };

	RenderInstance(){};
	~RenderInstance()
	{
		Clear();
	}

	HRESULT Initialize(const Sprite* pSprite, DrawQueue* pDrawQueue);

public:

	HRESULT Render();
	void	Clear();

	friend class IGraphics;
};
class ISurface: public Reference
{
private:

	~ISurface()
	{
		SafeRelease(&pTexture);
		SafeRelease(&pSRV);
		SafeRelease(&pRTV);
		SafeRelease(&pDSV);
	};

protected:

	ID3D11Texture2D*			pTexture		{ NULL };
	ID3D11ShaderResourceView*	pSRV			{ NULL };
	ID3D11RenderTargetView*		pRTV			{ NULL };
	ID3D11DepthStencilView*		pDSV			{ NULL };

	D3D11_VIEWPORT				ViewPort;

	UINT height {0};
	UINT width  {0};

	ISurface()
	{
		ZeroMemory(&ViewPort, sizeof(D3D11_VIEWPORT));
	};

public:

	friend class IGraphics;
};

#pragma warning(default:26451; default:26495)
#endif