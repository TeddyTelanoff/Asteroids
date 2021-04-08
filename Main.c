#include <Windows.h>
#include <assert.h>
#include <math.h>

#define PI (3.1415926535f)
#define TWO_PI (PI * 2)

typedef struct Window Window;
typedef struct Player Player;
typedef struct Asteroid Asteroid;

struct Window
{
	PWSTR className, title;
	WNDPROC proc;

	WNDCLASSEXW class;
	HWND handle;
	MSG msg;
	HDC dc;

	PAINTSTRUCT ps;
} wnd = { L"MyClass", L"Asteroids" };

struct Player
{
	FLOAT x, y;
	FLOAT rot;

	POINT verts[4];
} player = { 512 / 2 - 25, 512 / 2 - 25, };

DWORD gameThrdId;
HANDLE gameThrd;
BOOL running = TRUE;
HBRUSH background;

VOID Update();
VOID Paint();
LRESULT CALLBACK MyWinProc(_In_ HWND handle, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

DWORD WINAPI GameLoop(_In_ PVOID _)
{
	while (running)
	{
		Update();
		Paint();
		Sleep(50);
	}

	return 0;
}

INT main()
{
	background = CreateSolidBrush(0x404040);

	wnd.class = (WNDCLASSEXW) { sizeof(WNDCLASSEXW) };
	wnd.class.lpszClassName = wnd.className;
	wnd.class.lpfnWndProc = MyWinProc;
	RegisterClassExW(&wnd.class);

	wnd.handle = CreateWindowExW(0, wnd.className, wnd.title, WS_SYSMENU | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 512, 512, NULL, NULL, NULL, NULL);
	wnd.dc = GetDC(wnd.handle);

	gameThrd = CreateThread(NULL, 0, GameLoop, NULL, 0, &gameThrdId);
	assert(gameThrd);

	AttachThreadInput(GetCurrentThreadId(), gameThrdId, TRUE);

	while (running)
	{
		if (PeekMessageW(&wnd.msg, wnd.handle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&wnd.msg);
			DispatchMessageW(&wnd.msg);
		}
	}

	WaitForSingleObjectEx(gameThrd, INFINITE, TRUE);
}

VOID Update()
{
	static BYTE keys[256];
	BOOL val = GetKeyboardState(keys);
	if (!val)
		abort();
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		player.rot += 0.05f;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		player.rot -= 0.05f;
}

VOID Paint()
{
	static RECT wndRect = { 0, 0, 512, 512 };
	FillRect(wnd.dc, &wndRect, background);

	for (SIZE_T i = 0; i < 4; i++)
		player.verts[i] = (POINT)
	{
		sinf(player.rot + TWO_PI * (float)i / 4) * 50 + player.x,
		cosf(player.rot + TWO_PI * (float)i / 4) * 50 + player.x,
	};

	SelectObject(wnd.dc, background);
	Polygon(wnd.dc, player.verts, 4);
}

LRESULT CALLBACK MyWinProc(_In_ HWND handle, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		running = FALSE;
		return 0;
	case WM_PAINT:
	{
		HDC dc = BeginPaint(wnd.handle, &wnd.ps);
		Paint(dc);
		EndPaint(wnd.handle, &wnd.ps);
		return 0;
	}

	default:
		return DefWindowProcW(handle, msg, wParam, lParam);
	}
}