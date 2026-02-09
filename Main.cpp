// developed by dl6822
# include <array>
# include <cmath>
# include <memory>
# include <string>
# include <vector>
# include <cstddef>
# include <utility>
# include <algorithm>
# include <functional>
# include <unordered_map>
# include <unordered_set>
# define NOMINMAX
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <windowsx.h>
# pragma comment(lib, "opengl32.lib")
# pragma comment(lib, "glu32.lib")
# include <gl/GL.h>
# include <gl/GLU.h>
struct button {
	HWND handle = nullptr;
	UINT shortcut = 0;
	std::function<void()> function = {};
	bool drawable = false;
};
struct drawable {
	int state = 0;
	int identifier = 0;
	inline static int next_identifier = 0;
	drawable() : identifier(drawable::next_identifier += 1) {}
	virtual void left_mouse_button_down(float, float) {}
	virtual void left_mouse_button_up(float, float) {}
	virtual void left_clicked(float, float) {}
	virtual void right_clicked(float, float) {}
	virtual void draw(float, float, bool) const = 0;
	virtual ~drawable() = default;
};
struct frame;
struct eraser_stroke;
static frame &current_frame();
static bool brush_point_erased(float point_x, float point_y, int brush_identifier);
struct brush_stroke : drawable {
	std::vector<float> x;
	std::vector<float> y;
	float size = 1.0f;
	COLORREF color = RGB(0, 0, 0);
	void left_mouse_button_down(float point_x, float point_y) override {
		state = 1;
		if (!x.empty() && !y.empty()) {
			float last_x = x.back();
			float last_y = y.back();
			float dx = point_x - last_x;
			float dy = point_y - last_y;
			float distance = std::sqrt(dx * dx + dy * dy);
			float step = size * 0.5f;
			if (step < 1.0f) {
				step = 1.0f;
			}
			if (distance > step) {
				int count = static_cast<int>(distance / step);
				for (int index = 1; index < count; index += 1) {
					float t = static_cast<float>(index) / static_cast<float>(count);
					x.push_back(last_x + dx * t);
					y.push_back(last_y + dy * t);
				}
			}
		}
		x.push_back(point_x);
		y.push_back(point_y);
	}
	void left_mouse_button_up(float, float) override {
		state = 0;
	}
	void draw(float, float, bool) const override {
		if (x.empty() || y.empty()) {
			return;
		}
		glPointSize(size);
		glColor3ub(
			static_cast<GLubyte>(GetRValue(color)),
			static_cast<GLubyte>(GetGValue(color)),
			static_cast<GLubyte>(GetBValue(color))
		);
		glBegin(GL_POINTS);
		for (std::size_t index = 0;
			index < x.size() && index < y.size(); index += 1) {
			if (!brush_point_erased(x[index], y[index], identifier)) {
				glVertex2f(x[index], y[index]);
			}
		}
		glEnd();
	}
};
struct line_stroke : drawable {
	float x1 = 0.0f;
	float y1 = 0.0f;
	float x2 = 0.0f;
	float y2 = 0.0f;
	float width = 1.0f;
	COLORREF color = RGB(0, 0, 0);
	void left_mouse_button_down(float point_x, float point_y) override {
		if (state == 0) {
			x1 = point_x;
			y1 = point_y;
			state = 1;
		}
	}
	void left_mouse_button_up(float point_x, float point_y) override {
		if (state == 1) {
			x2 = point_x;
			y2 = point_y;
			state = 0;
		}
	}
	void draw(float cursor_x, float cursor_y, bool cursor_valid) const override {
		if (state == 0) {
			if (x1 == x2 && y1 == y2) {
				return;
			}
		} else if (!cursor_valid) {
			return;
		}
		float end_x = (state == 0) ? x2 : cursor_x;
		float end_y = (state == 0) ? y2 : cursor_y;
		glLineWidth(width);
		glColor3ub(
			static_cast<GLubyte>(GetRValue(color)),
			static_cast<GLubyte>(GetGValue(color)),
			static_cast<GLubyte>(GetBValue(color))
		);
		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(end_x, end_y);
		glEnd();
	}
};
struct quad_stroke : drawable {
	float x1 = 0.0f;
	float y1 = 0.0f;
	float x2 = 0.0f;
	float y2 = 0.0f;
	float width = 1.0f;
	COLORREF color = RGB(0, 0, 0);
	void left_mouse_button_down(float point_x, float point_y) override {
		if (state == 0) {
			x1 = point_x;
			y1 = point_y;
			state = 1;
		}
	}
	void left_mouse_button_up(float point_x, float point_y) override {
		if (state == 1) {
			x2 = point_x;
			y2 = point_y;
			state = 0;
		}
	}
	void draw(float cursor_x, float cursor_y, bool cursor_valid) const override {
		float end_x = x2;
		float end_y = y2;
		if (state != 0) {
			if (!cursor_valid) {
				return;
			}
			end_x = cursor_x;
			end_y = cursor_y;
		}
		glColor3ub(
			static_cast<GLubyte>(GetRValue(color)),
			static_cast<GLubyte>(GetGValue(color)),
			static_cast<GLubyte>(GetBValue(color))
		);
		if (state == 0) {
			if (x1 == x2 && y1 == y2) {
				return;
			}
			glBegin(GL_QUADS);
		} else {
			glBegin(GL_LINE_LOOP);
		}
		glVertex2f(x1, y1);
		glVertex2f(end_x, y1);
		glVertex2f(end_x, end_y);
		glVertex2f(x1, end_y);
		glEnd();
	}
};
struct triangle_stroke : drawable {
	float x1 = 0.0f;
	float y1 = 0.0f;
	float x2 = 0.0f;
	float y2 = 0.0f;
	float x3 = 0.0f;
	float y3 = 0.0f;
	float width = 1.0f;
	COLORREF color = RGB(0, 0, 0);
	void left_clicked(float point_x, float point_y) override {
		if (state == 0) {
			x1 = point_x;
			y1 = point_y;
			state = 1;
			return;
		}
		if (state == 1) {
			x2 = point_x;
			y2 = point_y;
			state = 2;
			return;
		}
		x3 = point_x;
		y3 = point_y;
		state = 0;
	}
	void draw(float cursor_x, float cursor_y, bool cursor_valid) const override {
		if (state == 0) {
			if (x1 == x2 && y1 == y2) {
				return;
			}
			glColor3ub(
				static_cast<GLubyte>(GetRValue(color)),
				static_cast<GLubyte>(GetGValue(color)),
				static_cast<GLubyte>(GetBValue(color))
			);
			glBegin(GL_TRIANGLES);
			glVertex2f(x1, y1);
			glVertex2f(x2, y2);
			glVertex2f(x3, y3);
			glEnd();
			return;
		}
		if (!cursor_valid) {
			return;
		}
		glLineWidth(width);
		glColor3ub(
			static_cast<GLubyte>(GetRValue(color)),
			static_cast<GLubyte>(GetGValue(color)),
			static_cast<GLubyte>(GetBValue(color))
		);
		glBegin(GL_LINE_LOOP);
		glVertex2f(x1, y1);
		if (state == 1) {
			glVertex2f(cursor_x, cursor_y);
		} else {
			glVertex2f(x2, y2);
			glVertex2f(cursor_x, cursor_y);
		}
		glEnd();
	}
};
struct polygon_stroke : drawable {
	std::vector<float> x;
	std::vector<float> y;
	float width = 1.0f;
	COLORREF color = RGB(0, 0, 0);
	struct point {
		float x = 0.0f;
		float y = 0.0f;
		point(float point_x, float point_y) : x(point_x), y(point_y) {}
	};
	static float cross(const point &a, const point &b, const point &c) {
		float abx = b.x - a.x;
		float aby = b.y - a.y;
		float acx = c.x - a.x;
		float acy = c.y - a.y;
		return abx * acy - aby * acx;
	}
	static std::vector<point> build_hull(const std::vector<point> &points) {
		if (points.size() <= 2U) {
			return points;
		}
		std::vector<point> sorted = points;
		std::sort(
			sorted.begin(), sorted.end(),
			[](const point &left, const point &right) {
				if (left.x == right.x) {
					return left.y < right.y;
				}
				return left.x < right.x;
			}
		);
		std::vector<point> hull;
		for (const point &p : sorted) {
			while (hull.size() >= 2U) {
				std::size_t last = hull.size() - 1U;
				if (cross(hull[last - 1U], hull[last], p) > 0.0f) {
					break;
				}
				hull.pop_back();
			}
			hull.push_back(p);
		}
		std::size_t lower_size = hull.size();
		for (std::size_t index = sorted.size(); index-- > 0U;) {
			const point &p = sorted[index];
			while (hull.size() > lower_size) {
				std::size_t last = hull.size() - 1U;
				if (cross(hull[last - 1U], hull[last], p) > 0.0f) {
					break;
				}
				hull.pop_back();
			}
			hull.push_back(p);
		}
		if (!hull.empty()) {
			hull.pop_back();
		}
		return hull;
	}
	void rebuild_hull() {
		std::vector<point> points;
		std::size_t count = x.size() < y.size() ? x.size() : y.size();
		points.reserve(count);
		for (std::size_t index = 0; index < count; index += 1) {
			points.push_back(point(x[index], y[index]));
		}
		std::vector<point> hull = build_hull(points);
		x.clear();
		y.clear();
		for (const point &p : hull) {
			x.push_back(p.x);
			y.push_back(p.y);
		}
	}
	void left_clicked(float point_x, float point_y) override {
		if (state == 0) {
			state = 1;
		}
		x.push_back(point_x);
		y.push_back(point_y);
		rebuild_hull();
	}
	void right_clicked(float point_x, float point_y) override {
		if (state == 0) {
			return;
		}
		x.push_back(point_x);
		y.push_back(point_y);
		rebuild_hull();
		if (x.size() >= 3 && y.size() >= 3) {
			state = 0;
		}
	}
	void draw(float cursor_x, float cursor_y, bool cursor_valid) const override {
		std::size_t count = x.size() < y.size() ? x.size() : y.size();
		if (count == 0) {
			return;
		}
		if (state == 0) {
			cursor_valid = false;
		}
		std::vector<point> points;
		points.reserve(count + (cursor_valid ? 1U : 0U));
		for (std::size_t index = 0; index < count; index += 1) {
			points.push_back(point(x[index], y[index]));
		}
		if (cursor_valid) {
			points.push_back(point(cursor_x, cursor_y));
		}
		std::vector<point> hull = build_hull(points);
		std::size_t hull_count = hull.size();
		if (hull_count == 0U) {
			return;
		}
		glColor3ub(
			static_cast<GLubyte>(GetRValue(color)),
			static_cast<GLubyte>(GetGValue(color)),
			static_cast<GLubyte>(GetBValue(color))
		);
		if (state == 0 && hull_count >= 3U) {
			glBegin(GL_POLYGON);
			for (const point &p : hull) {
				glVertex2f(p.x, p.y);
			}
			glEnd();
			return;
		}
		glLineWidth(width);
		glBegin(GL_LINE_LOOP);
		for (const point &p : hull) {
			glVertex2f(p.x, p.y);
		}
		glEnd();
	}
};
struct eraser_stroke : drawable {
	std::vector<float> x;
	std::vector<float> y;
	float size = 1.0f;
	std::unordered_set<int> erased = {};
	std::unordered_set<int> affected_brushes = {};
	void left_mouse_button_down(float point_x, float point_y) override {
		state = 1;
		if (!x.empty() && !y.empty()) {
			float last_x = x.back();
			float last_y = y.back();
			float dx = point_x - last_x;
			float dy = point_y - last_y;
			float distance = std::sqrt(dx * dx + dy * dy);
			float step = size * 0.5f;
			if (step < 1.0f) {
				step = 1.0f;
			}
			if (distance > step) {
				int count = static_cast<int>(distance / step);
				for (int index = 1; index < count; index += 1) {
					float t = static_cast<float>(index) / static_cast<float>(count);
					x.push_back(last_x + dx * t);
					y.push_back(last_y + dy * t);
				}
			}
		}
		x.push_back(point_x);
		y.push_back(point_y);
	}
	void left_mouse_button_up(float, float) override {
		state = 0;
	}
	void draw(float, float, bool) const override {}
};
struct frame {
	std::vector<std::unique_ptr<drawable>> drawables = {};
	int current_step = -1;
};
static void apply_eraser_at_point(eraser_stroke &eraser, float point_x, float point_y);
static const wchar_t *window_class_name = L"Flash Window Class";
static const wchar_t *window_title = L"Flash 2026";
static std::unordered_map<int, button> buttons = {};
static std::vector<frame> frames(1);
static const std::array<const wchar_t *, 6> tool_names = {
	L"Brush", L"Line", L"Quad", L"Triangle", L"Polygon", L"Eraser"
};
static const std::array<const wchar_t *, 6> tool_hints = {
	L"Brush Tool: click and drag to paint.",
	L"Line Tool: press and release.",
	L"Quad Tool: press and release.",
	L"Triangle Tool: click three points.",
	L"Convex Polygon Tool: click points, right-click to finish.",
	L"Eraser Tool: click and drag to clear."
};
static const std::array<int, 6> tool_button_identifiers = {
	1001, 1002, 1003, 1004, 1005, 1006
};
static int current_tool = 0;
static const std::array<const wchar_t *, 3> brush_size_names = {
	L"Small", L"Medium", L"Large"
};
static const std::array<int, 3> brush_size_button_identifiers = {
	2001, 2002, 2003
};
static const std::array<const wchar_t *, 3> line_width_names = {
	L"Thin", L"Medium", L"Thick"
};
static const std::array<int, 3> line_width_button_identifiers = {
	2101, 2102, 2103
};
static const std::array<COLORREF, 24> color_values = {
	RGB(0, 0, 0), RGB(255, 255, 255), RGB(255, 0, 0), RGB(0, 200, 0),
	RGB(0, 0, 255), RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255),
	RGB(255, 128, 0), RGB(153, 102, 51), RGB(128, 0, 128), RGB(255, 105, 180),
	RGB(64, 64, 64), RGB(128, 128, 128), RGB(192, 192, 192), RGB(128, 0, 0),
	RGB(0, 128, 0), RGB(0, 0, 128), RGB(128, 128, 0), RGB(0, 128, 128),
	RGB(0, 64, 128), RGB(255, 215, 0), RGB(128, 255, 0), RGB(135, 206, 235)
};
static const std::array<bool, 24> color_is_light = {
	false, true, false, false,
	false, true, false, true,
	true, false, false, true,
	false, true, true, false,
	false, false, true, false,
	false, true, true, true
};
static const std::array<int, 24> color_button_identifiers = {
	3001, 3002, 3003, 3004, 3005, 3006,
	3007, 3008, 3009, 3010, 3011, 3012,
	3013, 3014, 3015, 3016, 3017, 3018,
	3019, 3020, 3021, 3022, 3023, 3024
};
static const int undo_button_identifier = 4001;
static const int redo_button_identifier = 4002;
static const int clear_button_identifier = 4003;
static const int hint_button_identifier = 4004;
static int current_brush_size = 0;
static int current_line_width = 0;
static int current_color = 0;
static bool left_mouse_down = false;
static float stage_left = 0.0f;
static float stage_right = 0.0f;
static float stage_bottom = 0.0f;
static float stage_top = 0.0f;
static float cursor_x = 0.0f;
static float cursor_y = 0.0f;
static bool cursor_valid = false;
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
	} else if (message == WM_DRAWITEM) {
		const DRAWITEMSTRUCT *draw_item = reinterpret_cast<const DRAWITEMSTRUCT *>(
			long_parameter
		);
		if (draw_item && draw_item->CtlType == ODT_BUTTON) {
			int identifier = static_cast<int>(draw_item->CtlID);
			std::unordered_map<int, button>::iterator iterator = (
				buttons.find(identifier)
			);
			if (iterator != buttons.end() && iterator->second.drawable) {
				bool is_color = false;
				bool selected_button = false;
				int color_index = -1;
				COLORREF fill_color = GetSysColor(COLOR_BTNFACE);
				bool disabled = (draw_item->itemState & ODS_DISABLED) != 0;
				for (std::size_t index = 0;
					index < tool_button_identifiers.size(); index += 1) {
					if (tool_button_identifiers[index] == identifier) {
						selected_button = static_cast<int>(index) == current_tool;
						break;
					}
				}
				for (std::size_t index = 0;
					index < brush_size_button_identifiers.size(); index += 1) {
					if (brush_size_button_identifiers[index] == identifier) {
						selected_button = static_cast<int>(index) == current_brush_size;
						break;
					}
				}
				for (std::size_t index = 0;
					index < line_width_button_identifiers.size(); index += 1) {
					if (line_width_button_identifiers[index] == identifier) {
						selected_button = static_cast<int>(index) == current_line_width;
						break;
					}
				}
				for (std::size_t index = 0;
					index < color_button_identifiers.size(); index += 1) {
					if (color_button_identifiers[index] == identifier) {
						selected_button = static_cast<int>(index) == current_color;
						is_color = true;
						color_index = static_cast<int>(index);
						fill_color = color_values[index];
						break;
					}
				}
				if (is_color) {
					HBRUSH brush = CreateSolidBrush(fill_color);
					FillRect(draw_item->hDC, &draw_item->rcItem, brush);
					DeleteObject(brush);
					SetBkMode(draw_item->hDC, TRANSPARENT);
					if (color_index >= 0) {
						SetTextColor(
							draw_item->hDC,
							color_is_light[color_index] ?
							RGB(0, 0, 0) : RGB(255, 255, 255)
						);
					}
					HGDIOBJ old_font = nullptr;
					if (font_handle) {
						old_font = SelectObject(draw_item->hDC, font_handle);
					}
					wchar_t text_buffer[8] = {};
					GetWindowTextW(
						draw_item->hwndItem, text_buffer,
						static_cast<int>(sizeof(text_buffer) / sizeof(text_buffer[0]))
					);
					RECT text_rect = draw_item->rcItem;
					DrawTextW(
						draw_item->hDC, text_buffer, -1, &text_rect,
						DT_CENTER | DT_VCENTER | DT_SINGLELINE
					);
					if (old_font) {
						SelectObject(draw_item->hDC, old_font);
					}
				} else {
					COLORREF background = selected_button && !disabled ?
						RGB(235, 90, 90) : GetSysColor(COLOR_BTNFACE);
					COLORREF text_color = selected_button && !disabled ?
						RGB(255, 255, 255) : GetSysColor(COLOR_BTNTEXT);
					if (disabled) {
						text_color = GetSysColor(COLOR_GRAYTEXT);
					}
					HBRUSH brush = CreateSolidBrush(background);
					FillRect(draw_item->hDC, &draw_item->rcItem, brush);
					DeleteObject(brush);
					SetBkMode(draw_item->hDC, TRANSPARENT);
					SetTextColor(draw_item->hDC, text_color);
					HGDIOBJ old_font = nullptr;
					if (font_handle) {
						old_font = SelectObject(draw_item->hDC, font_handle);
					}
					wchar_t text_buffer[128] = {};
					GetWindowTextW(
						draw_item->hwndItem, text_buffer,
						static_cast<int>(sizeof(text_buffer) / sizeof(text_buffer[0]))
					);
					RECT text_rect = draw_item->rcItem;
					DrawTextW(
						draw_item->hDC, text_buffer, -1, &text_rect,
						DT_CENTER | DT_VCENTER | DT_SINGLELINE
					);
					if (draw_item->itemState & ODS_FOCUS) {
						DrawFocusRect(draw_item->hDC, &draw_item->rcItem);
					}
					if (old_font) {
						SelectObject(draw_item->hDC, old_font);
					}
				}
				return TRUE;
			}
		}
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
				SetFocus(window_handle);
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
	int x, int y, int width, int height, std::function<void()> function,
	bool drawable = false) {
	button new_button = button();
	DWORD style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
	if (drawable) {
		style |= BS_OWNERDRAW;
	}
	new_button.handle = CreateWindowW(
		L"BUTTON", text.c_str(), style,
		x, y, width, height, window_handle,
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(identifier)),
		instance_handle, nullptr
	);
	new_button.shortcut = shortcut;
	new_button.function = std::move(function);
	new_button.drawable = drawable;
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
		InvalidateRect(iterator->second.handle, nullptr, TRUE);
	}
}
static void set_button_enabled(int identifier, bool enabled) {
	std::unordered_map<int, button>::iterator iterator = (
		buttons.find(identifier)
	);
	if (iterator != buttons.end()) {
		EnableWindow(iterator->second.handle, enabled ? TRUE : FALSE);
		InvalidateRect(iterator->second.handle, nullptr, TRUE);
		if (!enabled && GetFocus() == iterator->second.handle) {
			SetFocus(window_handle);
		}
	}
}
static frame &current_frame() {
	return frames[0];
}
static float distance_point_to_segment(
	float point_x, float point_y, float start_x, float start_y, float end_x, float end_y) {
	float abx = end_x - start_x;
	float aby = end_y - start_y;
	float apx = point_x - start_x;
	float apy = point_y - start_y;
	float ab_len_sq = abx * abx + aby * aby;
	float t = 0.0f;
	if (ab_len_sq > 0.0f) {
		t = (apx * abx + apy * aby) / ab_len_sq;
		if (t < 0.0f) {
			t = 0.0f;
		} else if (t > 1.0f) {
			t = 1.0f;
		}
	}
	float cx = start_x + abx * t;
	float cy = start_y + aby * t;
	float dx = point_x - cx;
	float dy = point_y - cy;
	return std::sqrt(dx * dx + dy * dy);
}
static bool brush_point_erased(float point_x, float point_y, int brush_identifier) {
	frame &active_frame = current_frame();
	for (int index = 0; index <= active_frame.current_step; index += 1) {
		if (index >= static_cast<int>(active_frame.drawables.size())) {
			break;
		}
		drawable *item = active_frame.drawables[static_cast<std::size_t>(index)].get();
		eraser_stroke *eraser = dynamic_cast<eraser_stroke *>(item);
		if (!eraser) {
			continue;
		}
		if (eraser->affected_brushes.find(brush_identifier)
			== eraser->affected_brushes.end()) {
			continue;
		}
		std::size_t count = eraser->x.size() < eraser->y.size()
			? eraser->x.size() : eraser->y.size();
		if (count == 0U) {
			continue;
		}
		for (std::size_t i = 0; i + 1U < count; i += 1) {
			float distance = distance_point_to_segment(
				point_x, point_y,
				eraser->x[i], eraser->y[i],
				eraser->x[i + 1U], eraser->y[i + 1U]
			);
			if (distance <= eraser->size * 2.0f) {
				return true;
			}
		}
		float dx = point_x - eraser->x[count - 1U];
		float dy = point_y - eraser->y[count - 1U];
		float radius = eraser->size * 2.0f;
		if (dx * dx + dy * dy <= radius * radius) {
			return true;
		}
	}
	return false;
}
static void update_undo_redo_buttons() {
	frame &active_frame = current_frame();
	set_button_enabled(undo_button_identifier, active_frame.current_step >= 0);
	set_button_enabled(
		redo_button_identifier,
		active_frame.current_step + 1 < static_cast<int>(active_frame.drawables.size())
	);
	set_button_enabled(clear_button_identifier, !active_frame.drawables.empty());
}
static float brush_size_value() {
	static const float values[] = {4.0f, 8.0f, 14.0f};
	return values[current_brush_size];
}
static float line_width_value() {
	return brush_size_value();
}
static std::unique_ptr<drawable> create_drawable_for_tool(int tool_index) {
	if (tool_index == 0) {
		std::unique_ptr<brush_stroke> stroke = std::make_unique<brush_stroke>();
		stroke->size = brush_size_value();
		stroke->color = color_values[static_cast<std::size_t>(current_color)];
		return stroke;
	}
	if (tool_index == 1) {
		std::unique_ptr<line_stroke> stroke = std::make_unique<line_stroke>();
		stroke->width = line_width_value();
		stroke->color = color_values[static_cast<std::size_t>(current_color)];
		return stroke;
	}
	if (tool_index == 2) {
		std::unique_ptr<quad_stroke> stroke = std::make_unique<quad_stroke>();
		stroke->width = line_width_value();
		stroke->color = color_values[static_cast<std::size_t>(current_color)];
		return stroke;
	}
	if (tool_index == 3) {
		std::unique_ptr<triangle_stroke> stroke = std::make_unique<triangle_stroke>();
		stroke->width = line_width_value();
		stroke->color = color_values[static_cast<std::size_t>(current_color)];
		return stroke;
	}
	if (tool_index == 4) {
		std::unique_ptr<polygon_stroke> stroke = std::make_unique<polygon_stroke>();
		stroke->width = line_width_value();
		stroke->color = color_values[static_cast<std::size_t>(current_color)];
		return stroke;
	}
	if (tool_index == 5) {
		std::unique_ptr<eraser_stroke> stroke = std::make_unique<eraser_stroke>();
		stroke->size = brush_size_value();
		return stroke;
	}
	return {};
}
static drawable *current_drawable() {
	frame &active_frame = current_frame();
	if (active_frame.current_step < 0) {
		return nullptr;
	}
	if (active_frame.current_step >= static_cast<int>(active_frame.drawables.size())) {
		return nullptr;
	}
	return active_frame.drawables[static_cast<std::size_t>(active_frame.current_step)].get();
}
static void add_drawable(std::unique_ptr<drawable> item) {
	if (!item) {
		return;
	}
	frame &active_frame = current_frame();
	int keep_count = active_frame.current_step + 1;
	if (keep_count < static_cast<int>(active_frame.drawables.size())) {
		active_frame.drawables.erase(
			active_frame.drawables.begin() + keep_count,
			active_frame.drawables.end()
		);
	}
	active_frame.drawables.push_back(std::move(item));
	active_frame.current_step = static_cast<int>(active_frame.drawables.size()) - 1;
	update_undo_redo_buttons();
}
static void undo_action() {
	frame &active_frame = current_frame();
	if (active_frame.current_step >= 0) {
		active_frame.current_step -= 1;
		update_undo_redo_buttons();
	}
}
static void redo_action() {
	frame &active_frame = current_frame();
	if (active_frame.current_step + 1 < static_cast<int>(active_frame.drawables.size())) {
		active_frame.current_step += 1;
		update_undo_redo_buttons();
	}
}
static void clear_action() {
	frame &active_frame = current_frame();
	active_frame.drawables.clear();
	active_frame.current_step = -1;
	update_undo_redo_buttons();
}
static void handle_left_mouse_down(float x, float y) {
	if (current_tool == 3 || current_tool == 4) {
		return;
	}
	drawable *active = current_drawable();
	if (!active || active->state == 0) {
		if (x < stage_left || x > stage_right || y < stage_bottom || y > stage_top) {
			return;
		}
		std::unique_ptr<drawable> created = create_drawable_for_tool(current_tool);
		if (created) {
			created->left_mouse_button_down(x, y);
			add_drawable(std::move(created));
			active = current_drawable();
			if (current_tool == 5) {
				eraser_stroke *eraser = dynamic_cast<eraser_stroke *>(active);
				if (eraser) {
					apply_eraser_at_point(*eraser, x, y);
				}
			}
		}
		return;
	}
	active->left_mouse_button_down(x, y);
	if (current_tool == 5) {
		eraser_stroke *eraser = dynamic_cast<eraser_stroke *>(active);
		if (eraser) {
		apply_eraser_at_point(*eraser, x, y);
	}
}
}
static void handle_left_mouse_up(float x, float y) {
	drawable *active = current_drawable();
	if (active) {
		active->left_mouse_button_up(x, y);
	}
}
static void handle_left_clicked(float x, float y) {
	if (current_tool != 3 && current_tool != 4) {
		return;
	}
	drawable *active = current_drawable();
	if (!active || active->state == 0) {
		if (x < stage_left || x > stage_right || y < stage_bottom || y > stage_top) {
			return;
		}
		std::unique_ptr<drawable> created = create_drawable_for_tool(current_tool);
		if (created) {
			created->left_clicked(x, y);
			add_drawable(std::move(created));
		}
		return;
	}
	active->left_clicked(x, y);
}
static void handle_right_clicked(float x, float y) {
	drawable *active = current_drawable();
	if (active) {
		active->right_clicked(x, y);
	}
}
static float cross_product(
	float start_x, float start_y, float end_x, float end_y, float point_x, float point_y) {
	float abx = end_x - start_x;
	float aby = end_y - start_y;
	float acx = point_x - start_x;
	float acy = point_y - start_y;
	return abx * acy - aby * acx;
}
static bool on_segment(
	float start_x, float start_y, float end_x, float end_y, float point_x, float point_y) {
	float min_x = start_x < end_x ? start_x : end_x;
	float max_x = start_x > end_x ? start_x : end_x;
	float min_y = start_y < end_y ? start_y : end_y;
	float max_y = start_y > end_y ? start_y : end_y;
	return point_x >= min_x && point_x <= max_x
		&& point_y >= min_y && point_y <= max_y;
}
static bool segments_intersect(
	float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy) {
	float d1 = cross_product(ax, ay, bx, by, cx, cy);
	float d2 = cross_product(ax, ay, bx, by, dx, dy);
	float d3 = cross_product(cx, cy, dx, dy, ax, ay);
	float d4 = cross_product(cx, cy, dx, dy, bx, by);
	if (((d1 > 0.0f && d2 < 0.0f) || (d1 < 0.0f && d2 > 0.0f)) &&
		((d3 > 0.0f && d4 < 0.0f) || (d3 < 0.0f && d4 > 0.0f))) {
		return true;
	}
	if (d1 == 0.0f && on_segment(ax, ay, bx, by, cx, cy)) {
		return true;
	}
	if (d2 == 0.0f && on_segment(ax, ay, bx, by, dx, dy)) {
		return true;
	}
	if (d3 == 0.0f && on_segment(cx, cy, dx, dy, ax, ay)) {
		return true;
	}
	if (d4 == 0.0f && on_segment(cx, cy, dx, dy, bx, by)) {
		return true;
	}
	return false;
}
static bool nearly_equal(float a, float b) {
	float diff = a - b;
	if (diff < 0.0f) {
		diff = -diff;
	}
	return diff <= 0.001f;
}
static bool point_in_polygon(
	float point_x, float point_y, const std::vector<float> &vx, const std::vector<float> &vy) {
	std::size_t count = vx.size() < vy.size() ? vx.size() : vy.size();
	if (count < 3U) {
		return false;
	}
	bool inside = false;
	for (std::size_t i = 0, j = count - 1; i < count; j = i, i += 1) {
		float xi = vx[i];
		float yi = vy[i];
		float xj = vx[j];
		float yj = vy[j];
		bool intersect = ((yi > point_y) != (yj > point_y)) &&
			(point_x < (xj - xi) * (point_y - yi) / (yj - yi + 0.0f) + xi);
		if (intersect) {
			inside = !inside;
		}
	}
	return inside;
}
static bool point_in_triangle(
	float point_x, float point_y,
	float ax, float ay,
	float bx, float by,
	float cx, float cy) {
	float ab = (bx - ax) * (point_y - ay) - (by - ay) * (point_x - ax);
	float bc = (cx - bx) * (point_y - by) - (cy - by) * (point_x - bx);
	float ca = (ax - cx) * (point_y - cy) - (ay - cy) * (point_x - cx);
	bool has_neg = (ab < 0.0f) || (bc < 0.0f) || (ca < 0.0f);
	bool has_pos = (ab > 0.0f) || (bc > 0.0f) || (ca > 0.0f);
	return !(has_neg && has_pos);
}
static bool is_brush_or_eraser(drawable *item) {
	return dynamic_cast<brush_stroke *>(item) != nullptr ||
		dynamic_cast<eraser_stroke *>(item) != nullptr;
}
static bool is_drawable_complete(drawable *item) {
	return item && item->state == 0;
}
static bool is_erased(drawable *item) {
	if (!item) {
		return false;
	}
	frame &active_frame = current_frame();
	for (int index = 0; index <= active_frame.current_step; index += 1) {
		if (index >= static_cast<int>(active_frame.drawables.size())) {
			break;
		}
		drawable *candidate = active_frame.drawables[static_cast<std::size_t>(index)].get();
		eraser_stroke *eraser = dynamic_cast<eraser_stroke *>(candidate);
		if (!eraser) {
			continue;
		}
		if (eraser->erased.find(item->identifier) != eraser->erased.end()) {
			return true;
		}
	}
	return false;
}
static void apply_eraser_at_point(eraser_stroke &eraser, float point_x, float point_y) {
	bool has_prev = eraser.x.size() >= 2U && eraser.y.size() >= 2U;
	float prev_x = has_prev ? eraser.x[eraser.x.size() - 2U] : point_x;
	float prev_y = has_prev ? eraser.y[eraser.y.size() - 2U] : point_y;
	frame &active_frame = current_frame();
	for (int index = 0; index <= active_frame.current_step; index += 1) {
		if (index >= static_cast<int>(active_frame.drawables.size())) {
			break;
		}
		drawable *item = active_frame.drawables[static_cast<std::size_t>(index)].get();
		if (!item || item == &eraser) {
			continue;
		}
		if (!is_drawable_complete(item)) {
			continue;
		}
		if (dynamic_cast<brush_stroke *>(item)) {
			brush_stroke *brush = static_cast<brush_stroke *>(item);
			std::size_t count = brush->x.size() < brush->y.size()
				? brush->x.size() : brush->y.size();
			float radius = eraser.size * 2.0f;
			float radius_sq = radius * radius;
			for (std::size_t i = 0; i < count; i += 1) {
				float dx = brush->x[i] - point_x;
				float dy = brush->y[i] - point_y;
				if (dx * dx + dy * dy <= radius_sq) {
					eraser.affected_brushes.insert(brush->identifier);
					break;
				}
			}
			continue;
		}
		if (dynamic_cast<eraser_stroke *>(item)) {
			continue;
		}
		if (dynamic_cast<line_stroke *>(item)) {
			line_stroke *line = static_cast<line_stroke *>(item);
			if (segments_intersect(
				prev_x, prev_y, point_x, point_y, line->x1, line->y1, line->x2, line->y2
			)) {
				eraser.erased.insert(item->identifier);
			}
			continue;
		}
		if (dynamic_cast<quad_stroke *>(item)) {
			quad_stroke *quad = static_cast<quad_stroke *>(item);
			float left = quad->x1 < quad->x2 ? quad->x1 : quad->x2;
			float right = quad->x1 < quad->x2 ? quad->x2 : quad->x1;
			float bottom = quad->y1 < quad->y2 ? quad->y1 : quad->y2;
			float top = quad->y1 < quad->y2 ? quad->y2 : quad->y1;
			if (point_x >= left && point_x <= right && point_y >= bottom && point_y <= top) {
				eraser.erased.insert(item->identifier);
			}
			continue;
		}
		if (dynamic_cast<triangle_stroke *>(item)) {
			triangle_stroke *tri = static_cast<triangle_stroke *>(item);
			if (point_in_triangle(
				point_x, point_y,
				tri->x1, tri->y1,
				tri->x2, tri->y2,
				tri->x3, tri->y3
			)) {
				eraser.erased.insert(item->identifier);
			}
			continue;
		}
		if (dynamic_cast<polygon_stroke *>(item)) {
			polygon_stroke *poly = static_cast<polygon_stroke *>(item);
			std::vector<polygon_stroke::point> points;
			std::size_t count = poly->x.size() < poly->y.size() ?
				poly->x.size() : poly->y.size();
			points.reserve(count + 1U);
			for (std::size_t i = 0; i < count; i += 1) {
				points.push_back(polygon_stroke::point(poly->x[i], poly->y[i]));
			}
			points.push_back(polygon_stroke::point(point_x, point_y));
			std::vector<polygon_stroke::point> hull = polygon_stroke::build_hull(points);
			bool found = false;
			for (const polygon_stroke::point &p : hull) {
				if (nearly_equal(p.x, point_x) && nearly_equal(p.y, point_y)) {
					found = true;
					break;
				}
			}
			if (!found) {
				eraser.erased.insert(item->identifier);
			}
			continue;
		}
	}
}
static bool is_opengl_message(const MSG &message) {
	return message.hwnd == opengl_window_handle;
}
static void to_opengl_coordinates(float &x, float &y) {
	RECT client = RECT();
	GetClientRect(opengl_window_handle, &client);
	y = static_cast<float>(client.bottom - client.top) - y;
}
static bool handle_key_shortcuts(UINT key) {
	if (key == 'Z') {
		undo_action();
		return true;
	}
	if (key == 'X') {
		redo_action();
		return true;
	}
	return false;
}
static std::wstring format_tool_button_text(int index) {
	std::wstring text = tool_names[static_cast<std::size_t>(index)];
	text += L" [";
	text += static_cast<wchar_t>(L'1' + index);
	text += L"]";
	return text;
}
static void update_tool_buttons() {
	for (std::size_t index = 0;
		index < tool_button_identifiers.size(); index += 1) {
		update_button_text(
			tool_button_identifiers[index],
			format_tool_button_text(
				static_cast<int>(index)
			)
		);
	}
}
static void update_brush_size_buttons() {
	for (std::size_t index = 0;
		index < brush_size_button_identifiers.size(); index += 1) {
		update_button_text(
			brush_size_button_identifiers[index],
			brush_size_names[index]
		);
	}
}
static void update_line_width_buttons() {
	for (std::size_t index = 0;
		index < line_width_button_identifiers.size(); index += 1) {
		update_button_text(
			line_width_button_identifiers[index],
			line_width_names[index]
		);
	}
}
static void update_color_buttons() {
	for (std::size_t index = 0;
		index < color_button_identifiers.size(); index += 1) {
		update_button_text(
			color_button_identifiers[index],
			static_cast<int>(index) == current_color ? L"X" : L""
		);
	}
}
static void update_tool_hint() {
	update_button_text(
		hint_button_identifier,
		tool_hints[static_cast<std::size_t>(current_tool)]
	);
}
static void set_current_tool(int index) {
	if (index < 0 || index >= static_cast<int>(tool_button_identifiers.size())) {
		return;
	}
	if (current_tool == index) {
		return;
	}
	current_tool = index;
	update_tool_buttons();
	update_tool_hint();
}
static void set_current_brush_size(int index) {
	if (index < 0 || index >= static_cast<int>(brush_size_button_identifiers.size())) {
		return;
	}
	if (current_brush_size == index) {
		return;
	}
	current_brush_size = index;
	update_brush_size_buttons();
}
static void set_current_line_width(int index) {
	if (index < 0 || index >= static_cast<int>(line_width_button_identifiers.size())) {
		return;
	}
	if (current_line_width == index) {
		return;
	}
	current_line_width = index;
	update_line_width_buttons();
}
static void set_current_color(int index) {
	if (index < 0 || index >= static_cast<int>(color_button_identifiers.size())) {
		return;
	}
	if (current_color == index) {
		return;
	}
	current_color = index;
	update_color_buttons();
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	frame &active_frame = current_frame();
	for (int index = 0; index <= active_frame.current_step; index += 1) {
		if (index >= static_cast<int>(active_frame.drawables.size())) {
			break;
		}
		drawable *item = active_frame.drawables[static_cast<std::size_t>(index)].get();
		if (!item) {
			continue;
		}
		if (!is_brush_or_eraser(item) && is_erased(item)) {
			continue;
		}
		item->draw(cursor_x, cursor_y, cursor_valid);
	}
	if (cursor_valid && (current_tool == 0 || current_tool == 5)) {
		float size = brush_size_value();
		if (current_tool == 5) {
			float outer_size = size * 3.2f;
			float inner_size = size * 2.4f;
			glPointSize(outer_size);
			glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			glBegin(GL_POINTS);
			glVertex2f(cursor_x, cursor_y);
			glEnd();
			glPointSize(inner_size);
			glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
			glBegin(GL_POINTS);
			glVertex2f(cursor_x, cursor_y);
			glEnd();
		} else {
			COLORREF color = color_values[static_cast<std::size_t>(current_color)];
			glPointSize(size);
			glColor4f(
				static_cast<GLfloat>(GetRValue(color)) / 255.0f,
				static_cast<GLfloat>(GetGValue(color)) / 255.0f,
				static_cast<GLfloat>(GetBValue(color)) / 255.0f,
				0.5f
			);
			glBegin(GL_POINTS);
			glVertex2f(cursor_x, cursor_y);
			glEnd();
		}
	}
	glDisable(GL_BLEND);
	float stage_width = static_cast<float>(client_width) * 0.6f;
	float stage_height = static_cast<float>(client_height) * 0.6f;
	stage_left = (static_cast<float>(client_width) - stage_width) * 0.5f;
	stage_bottom = (static_cast<float>(client_height) - stage_height) * 0.5f;
	stage_right = stage_left + stage_width;
	stage_top = stage_bottom + stage_height;
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
	int button_gap = button_height / 4;
	int group_gap = button_height;
	if (!create_main_window(window_width, window_height)) {
		destroy_window_class();
		return 2;
	}
	if (!create_ui_font(button_height)) {
		destroy_main_window();
		destroy_window_class();
		return 3;
	}
	int tools_total_height = (
		static_cast<int>(tool_button_identifiers.size()) * button_height
		+ static_cast<int>(tool_button_identifiers.size() - 1) * button_gap
	);
	int tools_start_y = (window_height - tools_total_height) / 2;
	tools_start_y -= window_height / 10;
	if (tools_start_y < 5) {
		tools_start_y = 5;
	}
	int top_margin = 5;
	int top_gap = 5;
	int undo_x = top_margin;
	int redo_x = undo_x + button_width + top_gap;
	int clear_x = redo_x + button_width + top_gap;
	int about_x = window_width - button_width - top_margin;
	int hint_x = clear_x + button_width + top_gap;
	int hint_width = about_x - top_gap - hint_x;
	if (hint_width < button_width) {
		hint_width = button_width;
	}
	create_button(
		undo_button_identifier, L"Undo [Z]", 'Z',
		undo_x, top_margin,
		button_width, button_height,
		[]() {
			undo_action();
		},
		true
	);
	create_button(
		redo_button_identifier, L"Redo [X]", 'X',
		redo_x, top_margin,
		button_width, button_height,
		[]() {
			redo_action();
		},
		true
	);
	create_button(
		clear_button_identifier, L"Clear [C]", 'C',
		clear_x, top_margin,
		button_width, button_height,
		[]() {
			clear_action();
		},
		true
	);
	create_button(
		hint_button_identifier,
		tool_hints[static_cast<std::size_t>(current_tool)],
		0,
		hint_x, top_margin,
		hint_width, button_height,
		[]() {}
	);
	set_button_enabled(undo_button_identifier, false);
	set_button_enabled(redo_button_identifier, false);
	set_button_enabled(clear_button_identifier, true);
	set_button_enabled(hint_button_identifier, false);
	update_undo_redo_buttons();
	int tools_x = 10;
	for (std::size_t index = 0;
		index < tool_button_identifiers.size(); index += 1) {
		int y = tools_start_y + static_cast<int>(index) * (button_height + button_gap);
		int tool_index = static_cast<int>(index);
		create_button(
			tool_button_identifiers[index],
			format_tool_button_text(tool_index),
			static_cast<UINT>(L'1' + index),
			tools_x, y,
			button_width, button_height,
			[tool_index]() {
				set_current_tool(tool_index);
			},
			true
		);
	}
	int sizes_height = (
		static_cast<int>(brush_size_button_identifiers.size()) * button_height
		+ static_cast<int>(brush_size_button_identifiers.size() - 1) * button_gap
	);
	int lines_height = (
		static_cast<int>(line_width_button_identifiers.size()) * button_height
		+ static_cast<int>(line_width_button_identifiers.size() - 1) * button_gap
	);
	int color_gap = button_height / 6;
	int color_columns = 4;
	int color_cell = (button_width - (color_columns - 1) * color_gap) / color_columns;
	if (color_cell < 6) {
		color_cell = 6;
	}
	int color_rows = static_cast<int>(color_button_identifiers.size()) / color_columns;
	int colors_height = color_rows * color_cell + (color_rows - 1) * color_gap;
	int right_margin = 10;
	int sizes_x = window_width - button_width - right_margin;
	int line_x = sizes_x;
	int colors_width = color_columns * color_cell + (color_columns - 1) * color_gap;
	int colors_x = window_width - colors_width - right_margin;
	int right_total_height = sizes_height + group_gap + lines_height + group_gap + colors_height;
	int right_start_y = (window_height - right_total_height) / 2;
	if (right_start_y < 5) {
		right_start_y = 5;
	}
	int sizes_start_y = right_start_y;
	int lines_start_y = sizes_start_y + sizes_height + group_gap;
	int colors_start_y = lines_start_y + lines_height + group_gap;
	for (std::size_t index = 0;
		index < brush_size_button_identifiers.size(); index += 1) {
		int y = sizes_start_y + static_cast<int>(index) * (button_height + button_gap);
		int size_index = static_cast<int>(index);
		create_button(
			brush_size_button_identifiers[index],
			brush_size_names[index],
			0,
			sizes_x, y,
			button_width, button_height,
			[size_index]() {
				set_current_brush_size(size_index);
			},
			true
		);
	}
	for (std::size_t index = 0;
		index < line_width_button_identifiers.size(); index += 1) {
		int y = lines_start_y + static_cast<int>(index) * (button_height + button_gap);
		int line_index = static_cast<int>(index);
		create_button(
			line_width_button_identifiers[index],
			line_width_names[index],
			0,
			line_x, y,
			button_width, button_height,
			[line_index]() {
				set_current_line_width(line_index);
			},
			true
		);
	}
	for (std::size_t index = 0;
		index < color_button_identifiers.size(); index += 1) {
		int row = static_cast<int>(index) / color_columns;
		int column = static_cast<int>(index) % color_columns;
		int x = colors_x + column * (color_cell + color_gap);
		int y = colors_start_y + row * (color_cell + color_gap);
		int color_index = static_cast<int>(index);
		create_button(
			color_button_identifiers[index],
			color_index == current_color ? L"X" : L"",
			0, x, y, color_cell, color_cell,
			[color_index]() {
				set_current_color(color_index);
			},
			true
		);
	}
	create_button(
		9999, L"About [A]", 'A',
		about_x, top_margin,
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
			}
			if (message.message == WM_KEYDOWN) {
				UINT key = static_cast<UINT>(message.wParam);
				if (handle_key_shortcuts(key)) {
					SetFocus(window_handle);
					continue;
				}
				if (process_button_shortcut(key)) {
					SetFocus(window_handle);
					continue;
				}
			}
			if (message.message == WM_LBUTTONDOWN) {
				if (is_opengl_message(message)) {
					SetCapture(opengl_window_handle);
					left_mouse_down = true;
					float x = static_cast<float>(GET_X_LPARAM(message.lParam));
					float y = static_cast<float>(GET_Y_LPARAM(message.lParam));
					to_opengl_coordinates(x, y);
					cursor_x = x;
					cursor_y = y;
					cursor_valid = true;
					handle_left_mouse_down(x, y);
				}
			}
			if (message.message == WM_LBUTTONUP) {
				if (left_mouse_down) {
					ReleaseCapture();
					left_mouse_down = false;
				}
				if (is_opengl_message(message)) {
					float x = static_cast<float>(GET_X_LPARAM(message.lParam));
					float y = static_cast<float>(GET_Y_LPARAM(message.lParam));
					to_opengl_coordinates(x, y);
					cursor_x = x;
					cursor_y = y;
					cursor_valid = true;
					handle_left_mouse_up(x, y);
					handle_left_clicked(x, y);
				}
			}
			if (message.message == WM_MOUSEMOVE && left_mouse_down) {
				if (is_opengl_message(message)) {
					float x = static_cast<float>(GET_X_LPARAM(message.lParam));
					float y = static_cast<float>(GET_Y_LPARAM(message.lParam));
					to_opengl_coordinates(x, y);
					cursor_x = x;
					cursor_y = y;
					cursor_valid = true;
					handle_left_mouse_down(x, y);
				}
			}
			if (message.message == WM_MOUSEMOVE && !left_mouse_down) {
				if (is_opengl_message(message)) {
					float x = static_cast<float>(GET_X_LPARAM(message.lParam));
					float y = static_cast<float>(GET_Y_LPARAM(message.lParam));
					to_opengl_coordinates(x, y);
					cursor_x = x;
					cursor_y = y;
					cursor_valid = true;
				}
			}
			if (message.message == WM_RBUTTONDOWN) {
				if (is_opengl_message(message)) {
					float x = static_cast<float>(GET_X_LPARAM(message.lParam));
					float y = static_cast<float>(GET_Y_LPARAM(message.lParam));
					to_opengl_coordinates(x, y);
					cursor_x = x;
					cursor_y = y;
					cursor_valid = true;
					handle_right_clicked(x, y);
				}
			}
			TranslateMessage(&message);
			DispatchMessageW(&message);
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
