#include <Windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND g_hwnd = NULL;
bool g_quit = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	const char CLASS_NAME[] = "D3D12HelloTriangle";

	WNDCLASS wc = { };
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.lpfnWndProc = WndProc;

	RegisterClass(&wc);

	g_hwnd = CreateWindowEx(
		NULL,
		CLASS_NAME,
		"DirectX 12 Hello triangle",
		WS_OVERLAPPEDWINDOW,

		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,

		NULL, NULL, hInstance, NULL
	);

	if (!g_hwnd) {
		MessageBox(NULL, "An error occurred at CreateWindowEx.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(g_hwnd, nShowCmd);

	MSG msg = { };
	while (!g_quit) {
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		g_quit = true;
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}