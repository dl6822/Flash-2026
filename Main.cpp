// developed by dl6822
# include <string>
# include <utility>
# include <functional>
# include <unordered_map>
# define NOMINMAX
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
struct button {
	HWND handle = nullptr;
	UINT shortcut = 0;
	std::function<void()> function = {};
};
static const wchar_t *window_class_name = L"Flash Window Class";
static const wchar_t *window_title = L"Flash 2026";
static std::unordered_map<int, button> buttons = {};
static HINSTANCE instance_handle = nullptr;
static HWND window_handle = nullptr;
static HFONT font_handle = nullptr;
static void create_button(
	int identifier, const std::wstring& text, UINT shortcut,
	int x, int y, int width, int height, std::function<void()> function) {
	button new_button = button();
	new_button.handle = CreateWindowW(
		L"BUTTON", text.c_str(),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, width, height, window_handle,
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(identifier)),
		instance_handle, nullptr
	);
	new_button.shortcut = shortcut;
	new_button.function = std::move(function);
	SendMessageW(
		new_button.handle, WM_SETFONT,
		reinterpret_cast<WPARAM>(font_handle), TRUE
	);
	buttons[identifier] = std::move(new_button);
}
static void update_button_text(
	int identifier, const std::wstring& text) {
	std::unordered_map<int, button>::iterator iterator = (
		buttons.find(identifier)
	);
	if (iterator != buttons.end()) {
		SetWindowTextW(iterator->second.handle, text.c_str());
	}
}
static bool process_button_shortcut(UINT key) {
	for (const std::pair<const int, button>& button : buttons) {
		if (button.second.handle && button.second.shortcut == key) {
			SetFocus(button.second.handle);
			SendMessageW(button.second.handle, BM_CLICK, 0, 0);
			return true;
		}
	}
	return false;
}
static void show_message_box(
	const std::wstring& title,
	const std::wstring& content) {
	MessageBoxW(
		window_handle,
		content.c_str(),
		title.c_str(),
		MB_OK
	);
}
static LRESULT CALLBACK procedure(
	HWND procedure_window_handle, UINT message,
	WPARAM word_parameter, LPARAM long_parameter) {
	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		window_handle = nullptr;
		return 0;
	} else if (message == WM_COMMAND) {
		int code = HIWORD(word_parameter);
		if (code == BN_CLICKED) {
			int identifier = static_cast<int>(
				LOWORD(word_parameter)
			);
			std::unordered_map<int, button>::iterator iterator = (
				buttons.find(identifier)
			);
			if (iterator != buttons.end()) {
				if (iterator->second.function) {
					iterator->second.function();
				}
				return 0;
			}
		}
	}
	return DefWindowProcW(
		procedure_window_handle, message,
		word_parameter, long_parameter
	);
}
int main() {
	if (!SetProcessDpiAwarenessContext(
		DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
		SetProcessDPIAware();
	}
	int window_width = GetSystemMetrics(SM_CXSCREEN) * 2 / 3;
	int window_height = GetSystemMetrics(SM_CYSCREEN) * 2 / 3;
	int button_height = window_height / 20;
	int button_width = button_height * 4;
	instance_handle = GetModuleHandleW(nullptr);
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
	rectangle.right = window_width;
	rectangle.bottom = window_height;
	AdjustWindowRectEx(
		&rectangle, style, FALSE, WS_EX_TOOLWINDOW
	);
	window_handle = CreateWindowExW(
		WS_EX_TOOLWINDOW, window_class_name, window_title, style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rectangle.right - rectangle.left,
		rectangle.bottom - rectangle.top,
		nullptr, nullptr, instance_handle, nullptr
	);
	if (!window_handle) {
		UnregisterClassW(
			window_class_name,
			instance_handle
		);
		return 2;
	}
	font_handle = CreateFontW(
		-button_height / 2, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
	);
	if (!font_handle) {
		DestroyWindow(window_handle);
		UnregisterClassW(
			window_class_name,
			instance_handle
		);
		return 3;
	}
	create_button(
		9999, L"About [A]", 'A',
		window_width - button_width - 5, 5,
		button_width, button_height,
		[]() {
			show_message_box(
				L"Flash 2026 - About",
				L"Developed by dl6822.\n\nHRRaaa!!!!"
			);
		}
	);
	ShowWindow(window_handle, SW_SHOW);
	UpdateWindow(window_handle);
	bool running = true;
	while (running) {
		MSG message = MSG();
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) {
				running = false;
				break;
			} else if (message.message == WM_KEYDOWN) {
				UINT key = static_cast<UINT>(message.wParam);
				if (process_button_shortcut(key)) {
					continue;
				}
			} else {
				TranslateMessage(&message);
				DispatchMessageW(&message);
			}
		}
	}
	if (window_handle) {
		for (const std::pair<const int, button>& button : buttons) {
			if (button.second.handle) {
				DestroyWindow(button.second.handle);
			}
		}
		DestroyWindow(window_handle);
	}
	DeleteObject(font_handle);
	UnregisterClassW(
		window_class_name,
		instance_handle
	);
	return 0;
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	return main();
}
