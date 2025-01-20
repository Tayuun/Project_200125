#include "UIElements.h"

#pragma warning(disable:28159)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Render();


UIPanel* pPanel;


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	HRESULT hr;

	SetCurrentDirectoryW(L"../");
	hr = GRAPHICS::InitializeGraphics(hInstance, nCmdShow, WindowProc);

	if FAILED(hr)
	{
		DebugLog(L"\nFailed to initialize Direct");

		return 1;
	}

	pPanel = new UIPanel;

	MSG msg = {};
	while(msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			static float total { 1 };
			static float times { 1 };

			if(times == 10000)
			{
				total = 1;
				times = 1;
			}

			float FPS = 1000.f / (total / times);

			DWORD time = GetTickCount();

			std::wstring str;

			str = std::to_wstring(FPS);

			
			GRAPHICS::DrawString(str.c_str(), 0, 0);
			Render();
			GRAPHICS::Render();


			total += GetTickCount() - time;
			times++;


			G_MOUSE_SHIFT_X = 0;
			G_MOUSE_SHIFT_Y = 0;
			G_MOUSE_LCLICK  = false;
		}
	}

	SafeRelease(&pPanel);
	GRAPHICS::UninitializeGraphics();

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static DWORD clickTimer{ 0 };

	switch(uMsg)
	{
	case WM_MOUSEMOVE:
	{
		G_MOUSE_X = (float)GET_X_LPARAM(lParam);
		G_MOUSE_Y = (float)GET_Y_LPARAM(lParam);


		G_MOUSE_SHIFT_X = (G_MOUSE_X - G_MOUSE_X_PREV) * (wParam && MK_LBUTTON);
		G_MOUSE_SHIFT_Y = (G_MOUSE_Y - G_MOUSE_Y_PREV) * (wParam && MK_LBUTTON);


		G_MOUSE_X_PREV = G_MOUSE_X;
		G_MOUSE_Y_PREV = G_MOUSE_Y;

		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		float scale = (float)GET_WHEEL_DELTA_WPARAM(wParam) / (500.f / G_WHEEL);
		

		RECT wndRect;

		GetWindowRect(hwnd, &wndRect);
		
		if (G_WHEEL + scale < 0.01f)
		{
			return 0;
		}

		G_WHEEL += scale;

		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		clickTimer = GetTickCount();
		return 0;
	}
	case WM_LBUTTONUP:
	{
		G_MOUSE_LCLICK = ( (GetTickCount() - clickTimer) < 200 );
		return 0;
	}
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

void Render()
{
	batch.Action();

	if(G_MOUSE_LCLICK)
	{
		if ( CollisionPointRect({G_MOUSE_X, G_MOUSE_Y}, pPanel->GetCollisionBox()) )
			pPanel->OnClick();
	}

	batch.Draw();
	pPanel->Draw();
}

#pragma warning(default:28159)