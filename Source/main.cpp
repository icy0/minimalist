#ifndef UNICODE
#define UNICODE
#endif 

#include <iostream>
#include <cwchar>
#include <d2d1.h>
#include <ft2build.h>
#include <windows.h>

#include FT_FREETYPE_H

static D2D1_RENDER_TARGET_PROPERTIES rt_properties = {};
static D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_rt_properties = {};
static ID2D1HwndRenderTarget* hwnd_render_target;
static ID2D1Factory* factory;
static ID2D1Bitmap *dummy_bitmap;
static uint32_t* data;
static FT_Library library;
static FT_Face face;

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
	swprintf(output_buffer, sizeof(output_buffer), 
		L"The glyphs metrics are: rows: %d, pprow: %d, pitch: %d\n", 
		face->glyph->bitmap.rows, 
		face->glyph->bitmap.width, 
		face->glyph->bitmap.pitch);
	OutputDebugString(output_buffer);

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
	rt_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	rt_properties.dpiX = dpi;
	rt_properties.dpiY = dpi;
	rt_properties.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	rt_properties.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

	hwnd_rt_properties.hwnd = hwnd;
	hwnd_rt_properties.pixelSize = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	hwnd_rt_properties.presentOptions = D2D1_PRESENT_OPTIONS_NONE; // Vsync on

	SafeRelease(&hwnd_render_target);
	factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
									D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
									&hwnd_render_target);

	D2D1_BITMAP_PROPERTIES bmp_properties = {};
	bmp_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	bmp_properties.dpiX = dpi;
	bmp_properties.dpiY = dpi;

	SafeRelease(&dummy_bitmap);
	hwnd_render_target->CreateBitmap(D2D1::SizeU(face->glyph->bitmap.width/4, face->glyph->bitmap.rows), 
									(void *) face->glyph->bitmap.buffer, 
									face->glyph->bitmap.pitch, 
									&bmp_properties, 
									&dummy_bitmap);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int size_counter = 0;
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
		hwnd_render_target->BeginDraw();
		hwnd_render_target->Clear();

		hwnd_render_target->DrawBitmap(dummy_bitmap, D2D1::RectF(0.0f, 0.0f, face->glyph->bitmap.width / 4, face->glyph->bitmap.rows),
										1.0f, 
										D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, 
										D2D1::RectF(0.0f, 0.0f, face->glyph->bitmap.width / 4, face->glyph->bitmap.rows));
		hwnd_render_target->EndDraw();

		EndPaint(hwnd, &ps);
	}

	case WM_SIZE:
		resize(hwnd);
		size_counter++;
		wchar_t output_buffer[256];
		swprintf(output_buffer, sizeof(output_buffer), L"The window was resized %d times.\n", size_counter);
		OutputDebugString(output_buffer);

	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	D2D1_FACTORY_OPTIONS factory_options = {};
	factory_options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	if (D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &factory_options, (void**)(&factory)) != S_OK)
	{
		OutputDebugString(L"Failed to create a Direct2D factory.\n");
	}

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Minimalist", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								NULL, NULL, hInstance, NULL);
	FT_Init_FreeType(&library);

	FT_New_Face(library, "Resources/LiberationMono-Regular.ttf", 0, &face);
	// set 3rd param to -1 to retrieve number of faces in the font by accessing face->num_faces.

	uint32_t dpi = GetDpiForWindow(hwnd);
	FT_Set_Char_Size(face, 0, 20 * 64, dpi, dpi);

	uint32_t glyph_index = FT_Get_Char_Index(face, 'B');
	FT_Load_Glyph(face, glyph_index, FT_LOAD_BITMAP_METRICS_ONLY);

	if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
	{
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LCD);
	}

	print_fontinfo(face);

	if (!hwnd)
	{
		OutputDebugString(L"Couldn't create the window.\n");
	}

	ShowWindow(hwnd, nCmdShow);
	
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	SafeRelease(&hwnd_render_target);
	SafeRelease(&dummy_bitmap);
	SafeRelease(&factory);

	return 0;
}