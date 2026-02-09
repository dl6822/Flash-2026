// developed by dl6822
# define NOMINMAX
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
static const wchar_t *window_class_name = L"Flash Window Class";
static const wchar_t *window_title = L"Flash 2026";
static LRESULT CALLBACK procedure(
	HWND window_handle, UINT message,
	WPARAM word_parameter, LPARAM long_parameter) {
	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	} else {
		return DefWindowProcW(
			window_handle, message,
			word_parameter, long_parameter
		);
	}
}
int main() {
	HINSTANCE instance_handle = GetModuleHandleW(nullptr);
	WNDCLASSW window_class = WNDCLASSW();
	window_class.style = CS_OWNDC;
	window_class.lpfnWndProc = procedure;
	window_class.hInstance = instance_handle;
	window_class.hCursor = LoadCursorW(nullptr, IDC_CROSS);
	window_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	window_class.lpszClassName = window_class_name;
	if (!RegisterClassW(&window_class)) {
		return 1;
	}
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	RECT rectangle = RECT();
	rectangle.left = 0;
	rectangle.top = 0;
	rectangle.right = 800;
	rectangle.bottom = 600;
	AdjustWindowRectEx(
		&rectangle, style, FALSE, WS_EX_TOOLWINDOW
	);
	HWND window_handle = CreateWindowExW(
		WS_EX_TOOLWINDOW, window_class_name, window_title, style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rectangle.right - rectangle.left,
		rectangle.bottom - rectangle.top,
		nullptr, nullptr, instance_handle, nullptr
	);
	if (!window_handle) {
		return 2;
	}
	ShowWindow(window_handle, SW_SHOW);
	UpdateWindow(window_handle);
	bool running = true;
	while (running) {
		MSG message = MSG();
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) {
				running = false;
				break;
			} else {
				TranslateMessage(&message);
				DispatchMessageW(&message);
			}
		}
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	return main();
}
