#include <Resource.h>
#include <CommonDirectX.h>

#pragma warning(disable:26451)

//*******************************Functions********************************************************
bool		CollisionRectU						(RectU rect1, RectU rect2)
{
	return (
		(rect1.x	<	rect2.x2)	&&
		(rect1.x2	>	rect2.x)	&&
		(rect1.y	<	rect2.y2)	&&
		(rect1.y2	>	rect2.y)
	);
}
HRESULT		CreateSimpleMesh					(Mesh** ppMesh)
{
	if((*ppMesh) != NULL)
	{
		return E_INVALIDARG;
	}

	(*ppMesh) = new Mesh;

	(*ppMesh)->vertexCount = 4;
	(*ppMesh)->indexCount  = 5;
	(*ppMesh)->index = new UINT[5]{ 0, 1, 2, 1, 3};
	(*ppMesh)->vertex = new Vertex[4]
	{
		{DirectX::XMFLOAT3( -1.f,   1.f, 0.0f),	0U},
		{DirectX::XMFLOAT3(  1.f,   1.f, 0.0f),	0U},
		{DirectX::XMFLOAT3( -1.f,  -1.f, 0.0f),	0U},
		{DirectX::XMFLOAT3(  1.f,  -1.f, 0.0f),	0U}
	};

	return S_OK;
}
HRESULT		CreateCircleMesh					(Mesh**	ppMesh)
{
	Vertex* verticies = new Vertex[359];
	UINT*	indicies = new UINT[1077];

	constexpr float rad = 0.0174532925f;

	verticies[0] = {DirectX::XMFLOAT3(0.f, 0.f, 0.f), 0U};

	for(UINT i = 0; i < 359U; i++)
	{
		float theta = (float)i * rad;

		verticies[i + 1U] = {DirectX::XMFLOAT3((float)cos(theta), (float)sin(theta), 0.f), 0U};

		if(i < 358)
		{
			indicies[i * 3U] = 0U;
			indicies[i * 3U + 1U] = i + 2U;
			indicies[i * 3U + 2U] = i + 1U;
		}
		else
		{
			indicies[358U * 3U]		 = 0U;
			indicies[358U * 3U+1U]	 = 1U;
			indicies[358U * 3U + 2U] = 359U;
		}
	}

	(*ppMesh) = new Mesh;
	(*ppMesh)->index = indicies;
	(*ppMesh)->indexCount = 1077;
	(*ppMesh)->vertex = verticies;
	(*ppMesh)->vertexCount = 359;

	return S_OK;
}
int			Noise								(float x, float y)
{
	return rand();
}
HWND		CreateDefaultWindow					(HINSTANCE hInstance, int nCmdShow, LRESULT(*WindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam))
{
	const wchar_t ClassName[] = SETTING_WINDOW_CLASS_NAME;

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = ClassName;

	RegisterClass(&wc);



	DWORD WndStyle = WS_OVERLAPPEDWINDOW;

	RECT Rect{ 0, 0, SETTING_RESOLUTION_WIDTH, SETTING_RESOLUTION_HEIGHT };
	AdjustWindowRectEx(&Rect, WndStyle, NULL, NULL);

	HWND hwnd = CreateWindowExW
	(
		0,
		ClassName,
		SETTING_WINDOW_NAME,
		WndStyle,

		CW_USEDEFAULT,
		CW_USEDEFAULT,
		Rect.right - Rect.left,
		Rect.bottom - Rect.top,

		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwnd == NULL)
	{
		return hwnd;
	}

	ShowWindow(hwnd, nCmdShow);

	return hwnd;
}
//************************************************************************************************


//*******************************DrawQueue********************************************************
HRESULT		DrawQueue::Initialize				()
{
	QRawSpriteBuffer	= new RawSpriteBuffer[QMaxRawSprite];
	QRawSpriteExBuffer	= new RawSpriteExBuffer[QMaxRawSpriteEx];
	QStringBuffer		= new StringBuffer[QMaxString];
	QStaticStringBuffer = new StaticStringBuffer[QMaxStaticString];

	return S_OK;
}
HRESULT		DrawQueue::AddString				(const wchar_t* str, const float x, const float y)
{
	if (QStringCount == 50U)
		return E_FAIL;

	UINT length = (UINT)wcslen(str) + 1U;
	QStringBuffer[QStringCount].pString = new wchar_t[length];
	wcscpy_s(QStringBuffer[QStringCount].pString, length, str);
	QStringBuffer[QStringCount].x = x;
	QStringBuffer[QStringCount].y = y;

	QStringCount++;

	return S_OK;
}
HRESULT		DrawQueue::AddStaticText			(IDWriteTextLayout* pTextLayout, const float x, const float y)
{
	if (QStaticStringCount == 50)
		return E_FAIL;

	QStaticStringBuffer[QStaticStringCount] = { x, y, pTextLayout };
	pTextLayout->AddRef();
	QStaticStringCount++;

	return S_OK;
}
HRESULT		DrawQueue::AddRawSprite				(RawSprite* pRawSprite, const float x, const float y)
{
	if (QRawSpriteCount == 100U || pRawSprite == NULL)
		return E_FAIL;

	QRawSpriteBuffer[QRawSpriteCount] = {x, y, pRawSprite};
	pRawSprite->AddRef();

	QRawSpriteCount++;

	return S_OK;
}
HRESULT		DrawQueue::AddRawSpriteEx			(RawSprite* pRawSprite, const float x, const float y, const float scaleX, const float scaleY)
{
	if (QRawSpriteExCount == 50U || pRawSprite == NULL)
		return E_FAIL;

	QRawSpriteExBuffer[QRawSpriteExCount] = {x, y, scaleX, scaleY, 0U, 0.4f, pRawSprite};
	pRawSprite->AddRef();

	QRawSpriteExCount++;

	return S_OK;
}
//************************************************************************************************


//*******************************SpriteCollection*************************************************
HRESULT		SpriteCollection::AddSprite			(Sprite* pSprite)
{
	if (pSprite == NULL)
		return E_INVALIDARG;

	if (SpriteCount == 0)
	{
		apSprites = new Sprite*[1];
		apSprites[0] = pSprite;
		pSprite->AddRef();

		SpriteCount++;
		return S_OK;
	}

	Sprite** apSSprites = new Sprite*[SpriteCount + 1U];
	memcpy(apSSprites, apSprites, sizeof(Sprite*) * SpriteCount);

	apSSprites[SpriteCount] = pSprite;
	pSprite->AddRef();

	delete[] apSprites;
	apSprites = apSSprites;

	SpriteCount++;

	return S_OK;
}
HRESULT		SpriteCollection::RemoveSprite		(SpriteID sprID)
{
	for(UINT i{0}; i < SpriteCount; i++)
	{
		if(apSprites[i]->spriteID == sprID)
		{
			if (SpriteCount == 1)
			{
				SafeRelease(&apSprites[i]);
				delete[] apSprites;
				apSprites = NULL;

				return S_OK;
			}

			Sprite** apSSprites = new Sprite*[SpriteCount - 1];
			memcpy(apSSprites, apSprites, sizeof(Sprite*) * i);
			memcpy(apSSprites + sizeof(Sprite*) * i, apSprites + sizeof(Sprite*) * (i + 1), sizeof(Sprite*) * (SpriteCount - (i + 1)));

			SafeRelease(&apSprites[i]);
			delete[] apSprites;
			apSprites = apSSprites;
			SpriteCount--;

			return S_OK;
		}
	}

	return E_INVALIDARG;
}
//************************************************************************************************


//*******************************RenderInstance***************************************************
HRESULT		RenderInstance::Initialize			(const Sprite* pSprite, DrawQueue* pDrawQueue)
{
	if ((pDrawQueue == NULL))
	{
		return E_INVALIDARG;
	}

	this->pDrawQueue = pDrawQueue;

	pDrawQueue->AddRef();



	return S_OK;
}
HRESULT		RenderInstance::Render				()
{

	return S_OK;
}
void		RenderInstance::Clear				()
{
	SafeRelease(&pDrawQueue);
}
//************************************************************************************************


//*******************************Reference********************************************************
#ifdef SETTING_DEBUG
void		Reference::AddRef					()
{
	InstanceCount++;
	RefCount++;
}
void		Reference::Release					()
{
	InstanceCount--;
	RefCount--;

	if (RefCount == 0)
	{
		delete this;
	}
}
void		Reference::printCount				()
{
	std::wstring str = std::to_wstring(InstanceCount);


	DebugLog(L"Unreleased instance count = ");
	DebugLog(str.c_str());
	DebugLog(L"\n");
}
#else
void		Reference::AddRef					()
{
	RefCount++;
}
void		Reference::Release					()
{
	RefCount--;

	if (RefCount == 0)
	{
		delete this;
	}
}
#endif

//************************************************************************************************

#pragma warning(default:26451)