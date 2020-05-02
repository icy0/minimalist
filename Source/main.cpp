#ifndef UNICODE
#define UNICODE
#endif 

#define HEAP_MEMORY_ALLOC_SIZE static_cast<long long>(1024 * 1024 * 1024) 
#define POINT_SIZE 15
#define UNICODE_GLYPH_LOAD_START_INDEX 33
#define UNICODE_GLYPH_LOAD_END_INDEX 127
#define CHARACTER_LOAD (UNICODE_GLYPH_LOAD_END_INDEX - UNICODE_GLYPH_LOAD_START_INDEX)
#define CHARACTER_LOAD_DIFFERENCE (UNICODE_GLYPH_LOAD_END_INDEX - UNICODE_GLYPH_LOAD_START_INDEX)

#include <iostream>
#include <assert.h>
#include <cwchar>
#include <d2d1.h>
#include <ft2build.h>
#include <windows.h>

#include FT_FREETYPE_H

struct uivec2
{
	uint32_t x;
	uint32_t y;
};

struct ivec2
{
	int32_t x;
	int32_t y;
};

struct Character
{
	wchar_t character;
	uivec2 size;
	ivec2 bearing;
	uint32_t advance;
	uint32_t pitch;
	ID2D1Bitmap* d2bitmap;
};

static D2D1_RENDER_TARGET_PROPERTIES rt_properties = {};
static D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_rt_properties = {};
static ID2D1HwndRenderTarget* hwnd_render_target;
static ID2D1Factory* factory;
static uint32_t* data;
static FT_Library library;
static FT_Face face;
static uint8_t* memory;
static Character all_chars[CHARACTER_LOAD];
static int32_t global_space_advance;

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

static void resize(HWND hwnd)
{
	assert(hwnd);
	RECT rc;
	GetClientRect(hwnd, &rc);

	uint32_t dpi = GetDpiForWindow(hwnd);
	assert(dpi);

	rt_properties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	rt_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	rt_properties.dpiX = dpi;
	rt_properties.dpiY = dpi;
	rt_properties.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	rt_properties.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

	hwnd_rt_properties.hwnd = hwnd;
	hwnd_rt_properties.pixelSize = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	hwnd_rt_properties.presentOptions = D2D1_PRESENT_OPTIONS_NONE; // Vsync on

	SafeRelease(&hwnd_render_target);
	factory->CreateHwndRenderTarget(rt_properties, hwnd_rt_properties, &hwnd_render_target);
	assert(hwnd_render_target);

	D2D1_BITMAP_PROPERTIES bmp_properties = {};
	bmp_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	bmp_properties.dpiX = dpi;
	bmp_properties.dpiY = dpi;

	for (int i = 0; i < CHARACTER_LOAD_DIFFERENCE; i++)
	{
		Character c = all_chars[i];

		SafeRelease(&(c.d2bitmap));
		HRESULT error = hwnd_render_target->CreateBitmap(D2D1::SizeU(c.size.x, c.size.y), // size in pixels
														memory + (static_cast<size_t>(1024) * 1024 * i),
														c.pitch * 4,
														&bmp_properties,
														&(c.d2bitmap));

		assert(error == S_OK);
		assert(c.d2bitmap);

		all_chars[i] = c;
	}
}

void renderLine(std::string text, int32_t x, int32_t y, float scale)
{
	ivec2 pen_position = { x, y };

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == ' ')
		{
			pen_position = { pen_position.x + global_space_advance, pen_position.y };
		}
		else
		{
			Character& ch = all_chars[*c - UNICODE_GLYPH_LOAD_START_INDEX];
			int32_t top = pen_position.x + ch.bearing.x;
			int32_t left = pen_position.y - ch.bearing.y;

			hwnd_render_target->DrawBitmap(ch.d2bitmap, D2D1::RectF(top, left, top + ch.size.x, left + ch.size.y),
											1.0f,
											D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
											D2D1::RectF(0.0f, 0.0f, ch.size.x, ch.size.y));

			pen_position = { pen_position.x + static_cast<int32_t>(ch.advance), pen_position.y };
		}
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
		D2D1_COLOR_F clear_color = D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f);
		hwnd_render_target->BeginDraw();
		hwnd_render_target->Clear(clear_color);

		renderLine("all ascii-signs: !\"#$ % &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 10, 22, 1.0f);
		renderLine("my texteditor is working! :)", 10, 45, 1.0f);

		hwnd_render_target->EndDraw();

		EndPaint(hwnd, &ps);
	}

	case WM_SIZE:
		resize(hwnd);

	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void stretchAlphaDataToBGRAData(uint8_t* data, size_t size)
{
	assert(data);
	assert(size);
	assert(size < 100000);
	uint8_t pixels[100000];
	memcpy(pixels, data, size);
	for (int i = 0; i < size; i++)
	{
		uint8_t value = pixels[i];
		if (value != 0)
		{
			data[(i * 4)] = value;
			data[(i * 4) + 1] = value;
			data[(i * 4) + 2] = value;
		}
		else
		{
			data[(i * 4)] = 0xFF / 10;
			data[(i * 4) + 1] = 0xFF / 10;
			data[(i * 4) + 2] = 0xFF / 10;
		}
		data[(i * 4) + 3] = 0xFF;
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"Window Class";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	D2D1_FACTORY_OPTIONS factory_options = {};
	factory_options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &factory_options, (void**)(&factory));
	assert(factory);

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Minimalist", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								NULL, NULL, hInstance, NULL);
	assert(hwnd);

	uint32_t dpi = GetDpiForWindow(hwnd);
	assert(dpi);
	memory = (uint8_t*) VirtualAlloc(NULL, HEAP_MEMORY_ALLOC_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	assert(memory);

	FT_Init_FreeType(&library);
	assert(library);
	FT_New_Face(library, "Resources/LiberationMono-Regular.ttf", 0, &face);
	assert(face);
	FT_Set_Char_Size(face, 0, POINT_SIZE * 64, dpi, dpi);

	uint32_t glyph_index = FT_Get_Char_Index(face, 32);
	assert(glyph_index);
	FT_Load_Glyph(face, glyph_index, FT_LOAD_COLOR);
	assert(face->glyph->format != FT_GLYPH_FORMAT_BITMAP);
	global_space_advance = face->glyph->advance.x / 64;

	for (int i = UNICODE_GLYPH_LOAD_START_INDEX; i < UNICODE_GLYPH_LOAD_END_INDEX; i++)
	{
		uint32_t glyph_index = FT_Get_Char_Index(face, i);
		assert(glyph_index);
		FT_Load_Glyph(face, glyph_index, FT_LOAD_COLOR);
		assert(face->glyph->format != FT_GLYPH_FORMAT_BITMAP);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		assert(face->glyph->bitmap.buffer);

		memcpy(memory + (1024 * 1024 * (i - UNICODE_GLYPH_LOAD_START_INDEX)),
				face->glyph->bitmap.buffer,
				face->glyph->bitmap.pitch * face->glyph->bitmap.rows);

		stretchAlphaDataToBGRAData(memory + (1024 * 1024 * (i - UNICODE_GLYPH_LOAD_START_INDEX)),
									face->glyph->bitmap.pitch* face->glyph->bitmap.rows);


		Character c = {};
		c.character = i;
		c.size = { face->glyph->bitmap.width, face->glyph->bitmap.rows };
		c.bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
		c.advance = face->glyph->advance.x / 64;
		c.pitch = face->glyph->bitmap.pitch;

		all_chars[i - UNICODE_GLYPH_LOAD_START_INDEX] = c;
	}

	ShowWindow(hwnd, nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	for (int i = 0; i < CHARACTER_LOAD; i++)
	{
		SafeRelease(&(all_chars[i].d2bitmap));
	}

	SafeRelease(&hwnd_render_target);
	SafeRelease(&factory);

	return 0;
}