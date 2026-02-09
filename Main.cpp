// developed by dl6822
# include <string>
# include <utility>
# include <functional>
# include <unordered_map>
# define NOMINMAX
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# pragma comment(lib, "opengl32.lib")
# pragma comment(lib, "glu32.lib")
# include <gl/GL.h>
# include <gl/GLU.h>
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
static HWND opengl_window_handle = nullptr;
static HDC device_context_handle = nullptr;
static HGLRC rendering_context_handle = nullptr;
static LRESULT CALLBACK procedure(
	HWND procedure_window_handle, UINT message,
	WPARAM word_parameter, LPARAM long_parameter) {
	if (message == WM_DESTROY) {
		PostQuitMessage(0);
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
static bool create_window_class() {
	if (!SetProcessDpiAwarenessContext(
		DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
		SetProcessDPIAware();
	}
	instance_handle = GetModuleHandleW(nullptr);
	WNDCLASSW window_class = WNDCLASSW();
	window_class.style = CS_OWNDC;
	window_class.lpfnWndProc = procedure;
	window_class.hInstance = instance_handle;
	window_class.hCursor = LoadCursorW(nullptr, IDC_CROSS);
	window_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	window_class.lpszClassName = window_class_name;
	return RegisterClassW(&window_class) != 0;
}
static void destroy_window_class() {
	UnregisterClassW(
		window_class_name,
		instance_handle
	);
	instance_handle = nullptr;
}
static bool create_main_window(int width, int height) {
	DWORD style = (
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS
	);
	RECT rectangle = RECT();
	rectangle.left = 0;
	rectangle.top = 0;
	rectangle.right = width;
	rectangle.bottom = height;
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
		return false;
	}
	ShowWindow(window_handle, SW_SHOW);
	UpdateWindow(window_handle);
	return true;
}
static void destroy_main_window() {
	if (window_handle) {
		for (const std::pair<const int, button> &button : buttons) {
			if (button.second.handle) {
				DestroyWindow(button.second.handle);
			}
		}
		DestroyWindow(window_handle);
	}
	buttons.clear();
	window_handle = nullptr;
}
static bool create_ui_font(int button_height) {
	font_handle = CreateFontW(
		-button_height / 2, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
	);
	return font_handle != nullptr;
}
static void destroy_ui_font() {
	if (font_handle) {
		DeleteObject(font_handle);
	}
	font_handle = nullptr;
}
static void create_button(
	int identifier, const std::wstring &text, UINT shortcut,
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
	int identifier, const std::wstring &text) {
	std::unordered_map<int, button>::iterator iterator = (
		buttons.find(identifier)
	);
	if (iterator != buttons.end()) {
		SetWindowTextW(iterator->second.handle, text.c_str());
	}
}
static void show_message_box(
	const std::wstring &title,
	const std::wstring &content) {
	MessageBoxW(
		window_handle,
		content.c_str(),
		title.c_str(),
		MB_OK
	);
}
static bool initialize_opengl() {
	opengl_window_handle = window_handle;
	device_context_handle = GetDC(opengl_window_handle);
	if (!device_context_handle) {
		opengl_window_handle = nullptr;
		return false;
	}
	PIXELFORMATDESCRIPTOR pixel_format = PIXELFORMATDESCRIPTOR();
	pixel_format.nSize = sizeof(pixel_format);
	pixel_format.nVersion = 1;
	pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixel_format.iPixelType = PFD_TYPE_RGBA;
	pixel_format.cColorBits = 32;
	pixel_format.cDepthBits = 24;
	pixel_format.cStencilBits = 8;
	pixel_format.iLayerType = PFD_MAIN_PLANE;
	int format_index = ChoosePixelFormat(device_context_handle, &pixel_format);
	if (format_index == 0) {
		ReleaseDC(opengl_window_handle, device_context_handle);
		device_context_handle = nullptr;
		opengl_window_handle = nullptr;
		return false;
	}
	if (!SetPixelFormat(device_context_handle, format_index, &pixel_format)) {
		ReleaseDC(opengl_window_handle, device_context_handle);
		device_context_handle = nullptr;
		opengl_window_handle = nullptr;
		return false;
	}
	rendering_context_handle = wglCreateContext(device_context_handle);
	if (!rendering_context_handle) {
		ReleaseDC(opengl_window_handle, device_context_handle);
		device_context_handle = nullptr;
		opengl_window_handle = nullptr;
		return false;
	}
	if (!wglMakeCurrent(device_context_handle, rendering_context_handle)) {
		wglDeleteContext(rendering_context_handle);
		rendering_context_handle = nullptr;
		ReleaseDC(opengl_window_handle, device_context_handle);
		device_context_handle = nullptr;
		opengl_window_handle = nullptr;
		return false;
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	return true;
}
static void shutdown_opengl() {
	if (rendering_context_handle) {
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(rendering_context_handle);
	}
	if (device_context_handle && opengl_window_handle) {
		ReleaseDC(opengl_window_handle, device_context_handle);
	}
	rendering_context_handle = nullptr;
	device_context_handle = nullptr;
	opengl_window_handle = nullptr;
}
static bool process_button_shortcut(UINT key) {
	for (const std::pair<const int, button> &button : buttons) {
		if (button.second.handle && button.second.shortcut == key) {
			SetFocus(button.second.handle);
			SendMessageW(button.second.handle, BM_CLICK, 0, 0);
			return true;
		}
	}
	return false;
}
static void render_frame() {
	RECT client = RECT();
	GetClientRect(opengl_window_handle, &client);
	int client_width = client.right - client.left;
	int client_height = client.bottom - client.top;
	if (client_width <= 0 || client_height <= 0) {
		return;
	}
	glViewport(0, 0, client_width, client_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(
		0.0, static_cast<GLdouble>(client_width),
		0.0, static_cast<GLdouble>(client_height)
	);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	float stage_width = static_cast<float>(client_width) * 0.6f;
	float stage_height = static_cast<float>(client_height) * 0.6f;
	float stage_left = (static_cast<float>(client_width) - stage_width) * 0.5f;
	float stage_bottom = (static_cast<float>(client_height) - stage_height) * 0.5f;
	float stage_right = stage_left + stage_width;
	float stage_top = stage_bottom + stage_height;
	glColor3f(0.1f, 0.1f, 0.1f);
	glBegin(GL_QUADS);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(static_cast<float>(client_width), 0.0f);
	glVertex2f(static_cast<float>(client_width), stage_bottom);
	glVertex2f(0.0f, stage_bottom);
	glVertex2f(0.0f, stage_top);
	glVertex2f(static_cast<float>(client_width), stage_top);
	glVertex2f(static_cast<float>(client_width), static_cast<float>(client_height));
	glVertex2f(0.0f, static_cast<float>(client_height));
	glVertex2f(0.0f, stage_bottom);
	glVertex2f(stage_left, stage_bottom);
	glVertex2f(stage_left, stage_top);
	glVertex2f(0.0f, stage_top);
	glVertex2f(stage_right, stage_bottom);
	glVertex2f(static_cast<float>(client_width), stage_bottom);
	glVertex2f(static_cast<float>(client_width), stage_top);
	glVertex2f(stage_right, stage_top);
	glEnd();
	SwapBuffers(device_context_handle);
}
int main() {
	if (!create_window_class()) {
		return 1;
	}
	int window_width = GetSystemMetrics(SM_CXSCREEN) * 2 / 3;
	int window_height = GetSystemMetrics(SM_CYSCREEN) * 2 / 3;
	int button_height = window_height / 20;
	int button_width = button_height * 4;
	if (!create_main_window(window_width, window_height)) {
		destroy_window_class();
		return 2;
	}
	if (!create_ui_font(button_height)) {
		destroy_main_window();
		destroy_window_class();
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
	if (!initialize_opengl()) {
		destroy_ui_font();
		destroy_main_window();
		destroy_window_class();
		return 4;
	}
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
		if (opengl_window_handle) {
			render_frame();
		}
	}
	shutdown_opengl();
	destroy_ui_font();
	destroy_main_window();
	destroy_window_class();
	return 0;
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	return main();
}
