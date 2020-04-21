#ifndef UNICODE
#define UNICODE
#endif 

#define POINT_SIZE 10
#define CHARACTER_LOAD 94
#define UNICODE_GLYPH_LOAD_START_INDEX 33
#define UNICODE_GLYPH_LOAD_END_INDEX 127
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

static void print_fontinfo(FT_Face face)
{
	wchar_t output_buffer[256];

	OutputDebugString(L"\n");
	if ((face->face_flags & FT_FACE_FLAG_SCALABLE) == FT_FACE_FLAG_SCALABLE)
		OutputDebugString(L"This font contains outline glyphs. Note that a face can contain bitmap strikes also.\n");
	if ((face->face_flags & FT_FACE_FLAG_FIXED_SIZES) == FT_FACE_FLAG_FIXED_SIZES)
		OutputDebugString(L"This font contains bitmap strikes.\n");
	if ((face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) == FT_FACE_FLAG_FIXED_WIDTH)
		OutputDebugString(L"This font contains fixed width characters.\n");
	if ((face->face_flags & FT_FACE_FLAG_SFNT) == FT_FACE_FLAG_SFNT)
		OutputDebugString(L"This font uses the SFNT storage scheme.\n");
	if ((face->face_flags & FT_FACE_FLAG_HORIZONTAL) == FT_FACE_FLAG_HORIZONTAL)
		OutputDebugString(L"This font contains horizontal glyph metrics.\n");
	if ((face->face_flags & FT_FACE_FLAG_VERTICAL) == FT_FACE_FLAG_VERTICAL)
		OutputDebugString(L"This font contains vertical glyph metrics.\n");
	if ((face->face_flags & FT_FACE_FLAG_KERNING) == FT_FACE_FLAG_KERNING)
		OutputDebugString(L"This font contains kerning information. The kerning distance can be retrieved using the function Get_FT_Kerning()\n");

	swprintf(output_buffer, sizeof(output_buffer), L"The face contains %d  glyphs.\n", face->num_glyphs);
	OutputDebugString(output_buffer);
	swprintf(output_buffer, sizeof(output_buffer), L"The face uses %d font units per EM.\n", face->units_per_EM);
	OutputDebugString(output_buffer);
	OutputDebugString(L"\n");
}

static void resize(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	uint32_t dpi = GetDpiForWindow(hwnd);

	rt_properties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	rt_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
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
	bmp_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
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
	static uint32_t call_counter = 0;
	call_counter++;
	wchar_t output_buffer[256];
	swprintf(output_buffer, sizeof(output_buffer), L"the text is about to be rendered %d times.\n", call_counter);
	OutputDebugString(output_buffer);

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

	swprintf(output_buffer, sizeof(output_buffer), L"the text was rendered %d times.\n", call_counter);
	OutputDebugString(output_buffer);
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
		renderLine("my texteditor is working! :)", 10, 40, 1.0f);

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
	uint8_t pixels[10000];
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
	memory = (uint8_t*)malloc(1024 * 1024 * 1024);
	assert(memory);

	FT_Init_FreeType(&library);
	// set 3rd param to -1 to retrieve number of faces in the font by accessing face->num_faces.
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
		// load glyph's bitmap
		uint32_t glyph_index = FT_Get_Char_Index(face, i);
		assert(glyph_index);
		FT_Load_Glyph(face, glyph_index, FT_LOAD_COLOR);
		assert(face->glyph->format != FT_GLYPH_FORMAT_BITMAP);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		assert(face->glyph->bitmap.buffer);

		// place bitmap in heap memory
		memcpy(memory + (1024 * 1024 * (i - UNICODE_GLYPH_LOAD_START_INDEX)),
			face->glyph->bitmap.buffer,
			face->glyph->bitmap.pitch * face->glyph->bitmap.rows);

		stretchAlphaDataToBGRAData(memory + (1024 * 1024 * (i - UNICODE_GLYPH_LOAD_START_INDEX)),
			face->glyph->bitmap.pitch* face->glyph->bitmap.rows);


		// save relevant info and pointer to bitmap
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

	for (int i = 0; i < 96; i++)
	{
		SafeRelease(&(all_chars[i].d2bitmap));
	}

	SafeRelease(&hwnd_render_target);
	SafeRelease(&factory);

	return 0;
}