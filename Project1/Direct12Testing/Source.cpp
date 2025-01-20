#include "GraphicsDirect3D12.h"
#include "Resource.h"
#pragma comment(lib, "Graphics.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	IGD12* pGraphics { NULL };

	HWND Handle = CreateDefaultWindow(hInstance, nCmdShow, WindowProc);
	if (!Handle)
		return 1;

	HRESULT hr = CreateIGD12(Handle, &pGraphics);
	if FAILED(hr)
		return 1;

	GD12_SHAPE_RECT rect{ 500, 100, 100, 100, 1, 0, .4, 1 };
	GD12_SHAPE_RECT rect2{ 500, 0, 50, 50, 1, 1, 0.5, 1};
	GD12_SHAPE_TEXTURE tex{ 0.f, 200.f, 0.4f };
	GD12_SHAPE_TEXTURE tex2{ 200.f, 400.f, 0.5f};

	pGraphics->CreateTexture(L"../Resource/Sprite/Twitter.jpg");

	//		LOOP

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			rect.x += -0.1f;
			tex.x += 0.1f;


			for (UINT i = 0; i < 15; i++)
			{
				GD12_SHAPE_RECT rect{ 30 * i, 0, 10, 20, 0.3, 0.3, 0.3, 1 };

				pGraphics->DrawRect(&rect);
			}
			
			pGraphics->DrawTexture(&tex, 0);
			pGraphics->DrawTexture(&tex2, 0);

			HRESULT hr = pGraphics->Render();
		}
	}

	SafeRelease(&pGraphics);

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static DWORD clickTimer{ 0 };

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hwnd, &ps);

		return 0;
	}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}