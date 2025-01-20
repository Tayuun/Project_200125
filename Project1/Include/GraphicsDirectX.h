#pragma once
#ifndef PROJECT_DIRECTX_GRAPHICS_H
#define PROJECT_DIRECTX_GRAPHICS_H

#include "CommonDirectX.h"
#include "Resource.h"
#include "GraphicHandler.h"

class	IGraphics;



class DWriteComponent: public Reference
{
private:
	IDWriteFactory*		pDWriteFactory	{NULL};

	~DWriteComponent()
	{
		Clear();
	}
	DWriteComponent(){};

protected:
	HRESULT Initialize();
	void Clear();

public:

	HRESULT		CreateTextLayout(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout);
	HRESULT		CreateTextFormat(IDWriteTextFormat** ppTextFormat, LPCWSTR FontName, const float FontSize);

	friend class Direct2DComponent;
};
class Direct2DComponent: public Reference
{
private:

	DWriteComponent*		pDWrite			{NULL};
	ID2D1Factory*			pD2D1Factory	{NULL};
	ID2D1RenderTarget*		pRenderTarget	{NULL};
	IDXGISwapChain*			pSwapChain		{NULL};
	IDXGISurface*			pDXGISurface	{NULL};
	
	ID2D1SolidColorBrush*	pBrush			{NULL};
	IDWriteTextFormat*		pTextFormat		{NULL};

protected:

	~Direct2DComponent()
	{
		Clear();
	};
	Direct2DComponent(){};

	void		Clear			();
	HRESULT		Render			();
	HRESULT		Initialize		(IDXGISwapChain* pSwapChain);

	void		BeginDraw		();
	HRESULT		EndDraw			();
	void		DrawDynamicText	(LPCWSTR string, float x, float y);
	void		DrawStaticText	(IDWriteTextLayout* pTextLayout, const float x, const float y);
	HRESULT		CreateTextLayout(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout);

public:

	friend class IGraphics;

};


class GraphicsBase: public Reference
{
private:

protected:

	SETTINGS_DESC	Settings	{ SETTING_RESOLUTION_WIDTH, SETTING_RESOLUTION_HEIGHT };
	DXGI_FORMAT		DxgiFormat	{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D11_VIEWPORT	ViewPort	{0};


	D3D_FEATURE_LEVEL*		pFeatureLevel		{NULL};
	HWND					hwnd				{NULL};
	

	ID3D11DeviceContext*	pDeviceContext		{NULL};
	ID3D11Device*			pDevice				{NULL};
	IDXGISwapChain*			pSwapChain			{NULL};


	ID3D11Texture2D*		pBackBuffer			{NULL};
	ID3D11RenderTargetView*	pRTView				{NULL};


	HRESULT		Initialize();


	HRESULT		CreateVertexShaderAndInputLayout	(LPCWSTR CSOFile, D3D11_INPUT_ELEMENT_DESC* pIEDesc, UINT ElementNum, ID3D11InputLayout** ppInputLayout, ID3D11VertexShader** ppVShader);
	HRESULT		CreateVertexShader					(LPCWSTR CSOFile, ID3D11VertexShader** ppVShader);
	HRESULT		CreatePixelShader					(LPCWSTR CSOFile, ID3D11PixelShader** ppPShader);
	HRESULT		CreateHullShader					(LPCWSTR CSOFile, ID3D11HullShader** ppHShader);
	HRESULT		CreateDomainShader					(LPCWSTR CSOFile, ID3D11DomainShader** ppDShader);
	HRESULT		CreateGeometryShader				(LPCWSTR CSOFile, ID3D11GeometryShader** ppGShader);

	
	GraphicsBase(){};
	~GraphicsBase()
	{
		SafeRelease(&pRTView);
		SafeRelease(&pBackBuffer);


		SafeRelease(&pSwapChain);
		SafeRelease(&pDeviceContext);
		SafeRelease(&pDevice);


		CoUninitialize();
	};

public:
	
	HWND		GetHWND								();

};
class GraphicsPSC: public GraphicsBase
{
protected:

	//***********************Shaders***********************
	ID3D11PixelShader*			pPSColor			{ NULL };
	ID3D11PixelShader*			pPSTexture			{ NULL };
	ID3D11PixelShader*			pPSRawTexture		{ NULL };

	ID3D11InputLayout*			pILStandard			{ NULL };

	ID3D11VertexShader*			pVSRawSpriteEx		{ NULL };
	ID3D11VertexShader*			pVSLine				{ NULL };
	ID3D11VertexShader*			pVSSpriteEx			{ NULL };
	ID3D11VertexShader*			pVSSprite			{ NULL };
	ID3D11VertexShader*			pVSRect				{ NULL };

	ID3D11HullShader*			pHullShader			{ NULL };
	ID3D11DomainShader*			pDomainShader		{ NULL };
	ID3D11GeometryShader*		pGeometryShader		{ NULL };
	//*****************************************************


	//*********************Buffers*************************
	ID3D11Buffer*				pVSBSettings		{ NULL };
	ID3D11Buffer*				pVBStandard			{ NULL };
	ID3D11Buffer*				pVBRawSpriteData	{ NULL };

	StructBuffer*				pVSSBuffer16		{ NULL };
	StructBuffer*				pVSSBuffer32		{ NULL };
	//*****************************************************

	//******************Pipeline state*********************
	ID3D11DepthStencilState*	pDepthStencilState	{NULL};
	ID3D11DepthStencilView*		pDepthStencilView	{NULL};
	ID3D11SamplerState*			pSamplerState		{NULL};
	ID3D11RasterizerState*		pRasterizerState	{NULL};
	ID3D11BlendState*			pBlendState			{NULL};
	//*****************************************************


	GraphicsPSC(){};
	~GraphicsPSC()
	{
		SafeRelease(&pBlendState);
		SafeRelease(&pRasterizerState);
		SafeRelease(&pSamplerState);
		SafeRelease(&pDepthStencilState);
		SafeRelease(&pDepthStencilView);

		//**************Shaders**************
		SafeRelease(&pVBStandard);
		SafeRelease(&pVSBSettings);
		SafeRelease(&pVBRawSpriteData);
		SafeRelease(&pVSSBuffer16);	
		SafeRelease(&pVSSBuffer32);

		SafeRelease(&pPSColor);
		SafeRelease(&pPSTexture);
		SafeRelease(&pPSRawTexture);

		SafeRelease(&pILStandard);

		SafeRelease(&pVSRawSpriteEx);
		SafeRelease(&pVSLine);
		SafeRelease(&pVSSprite);
		SafeRelease(&pVSSpriteEx);
		SafeRelease(&pVSRect);

		SafeRelease(&pHullShader);
		SafeRelease(&pDomainShader);
		SafeRelease(&pGeometryShader);
		//***********************************
	};

	HRESULT Initialize();

	HRESULT ExtractBufferData(ID3D11Buffer* pBuffer, RawBufferData** ppRawBufferData);
	HRESULT CreateStructuredBuffer(UINT elementSize, UINT numElements, StructBuffer** ppStructBuffer);
	HRESULT StructureBufferAppend(const UINT numOfElements, const void* srcData, StructBuffer* pStructBuffer);
	HRESULT StructuredBufferRemove(const UINT numOfElement, StructBuffer* pStructBuffer);
};
class TextureManager: public GraphicsPSC
{
private:

	const UINT					BATCH_WIDTH			{ 4096U };
	const UINT					BATCH_HEIGHT		{ 4096U };

	UINT						BatchCount			{0};
	UINT						SpriteCount			{0};

	SpriteBatch**				apSpriteBatch		{ NULL };
	SpriteCollection*			pSpriteCollection	{ new SpriteCollection };

	HRESULT CreateSprite		(Sprite** pSprite, UINT width, UINT height);

protected:

	BatchID						InitializedBatch	{0xffffffff};

	TextureManager(){};
	~TextureManager()
	{
		Clear();
	}

	IWICImagingFactory*	pWICFactory	{NULL};
	
	HRESULT DecodeImageToPixData(LPCWSTR FileName, DXGI_FORMAT DXGIFormat, RawImageData** ppRawImageData);

	HRESULT AddTexture			(RawImageData* pRawImageData, UINT x, UINT y, SpriteBatch* pSpriteBatch);
	
	HRESULT Initialize();
	void Clear();

public:

	HRESULT CreateSpriteBatch	();
	HRESULT SaveBatch			(BatchID batchID, const wchar_t* fileName);
	HRESULT LoadBatch			(LPCWSTR fileName);
	HRESULT UnloadBatch			(BatchID batchID);
	HRESULT InitializeBatch		(BatchID batchID);
	HRESULT UninitializeBatch	(BatchID batchID);
	HRESULT CreateRawSprite		(LPCWSTR FileName, RawSprite** ppRawSprite);
	HRESULT ExtractImageData	(ID3D11Texture2D* pRawSprite, RawImageData** ppRawImageData);
	HRESULT AddSpriteToBatch	(RawImageData* pRawImageData, PointU point, BatchID batchID);
	
};
class IGraphics: public TextureManager
{
	friend HRESULT GRAPHICS::InitializeGraphics(HINSTANCE hInstance, int nCmdShow, LRESULT (*WindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam));
private:

	float		BackgroundColor[4]	{ 0.8f, 0.8f, 0.8f, 1.f};
	ISurface*	pSetSurface			{ NULL };

protected:

	DrawQueue*					pDrawQueue			{NULL};
	Direct2DComponent*			pDirect2D			{NULL};

	HRESULT		Initialize(HWND Handle);

	IGraphics(){};
	~IGraphics()
	{
		SafeRelease(&pSetSurface);

		SafeRelease(&pDrawQueue);
		SafeRelease(&pDirect2D);

		ZeroMemory(&ViewPort, sizeof(D3D11_VIEWPORT));
	}

public:

	HRESULT		Render								();

	HRESULT		CreateMesh							(Vertex* vArray, UINT* iArray, UINT vertexCount, UINT indexCount, Mesh** ppMesh);
	HRESULT		CreateRenderInstance				(const Sprite* pSprite, RenderInstance** ppRenderInstance);
	HRESULT		CreateDrawQueue						(DrawQueue** ppDrawQueue);
	HRESULT		CreateTextLayout					(DWRITE_TEXTLAYOUT_DESC* pDesc, IDWriteTextLayout** ppTextLayout);
	HRESULT		CreateSurface						(ISurface** ppSurface, const UINT width, const UINT height);

	HRESULT		SetSurface							(ISurface* pSurface);
	void		ResetSurface						();

	void		SetBGColor							(const float color[4]);

	void		DrawSprite							(const SpriteID sprID, const float x, const float y);
	void		DrawSpriteEx						(const SpriteID spriteID, const float x, const float y, const float scaleX, const float scaleY);
	void		DrawLine							(const float x1, const float y1, const float x2, const float y2, const float color[4]);
	void		DrawRect							(const float x, const float y, const float width, const float height, const float color[4]);
	void		DrawDynamicText						(LPCWSTR string, float x, float y);
	void		DrawStaticText						(IDWriteTextLayout*	pTextLayout, const float x, const float y);
	void		DrawRawSprite						(RawSprite* pRawSprite, const float x, const float y);
	void		DrawRawSpriteEx						(RawSprite* pRawSprite, const float x, const float y, const float scaleX, const float scaleY);
	void		DrawSurface							(ISurface* pSurface, const float x, const float y);
};
#endif