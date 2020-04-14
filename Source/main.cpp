#ifndef UNICODE
#define UNICODE
#endif 

#include <iostream>
#include <cwchar>
#include <d2d1.h>
#include <ft2build.h>
#include <windows.h>
#include FT_FREETYPE_H

static void print_fontinfo()
{
	FT_Library library;
	FT_Face face;

	FT_Error error = FT_Init_FreeType(&library);

	if (error)
	{
		OutputDebugString(L"couldn't init FreeType.\n");
	}

	error = FT_New_Face(library, "Resources/LiberationMono-Regular.ttf", 0, &face);
	// set 3rd param to -1 to retrieve number of faces in the font by accessing face->num_faces.

	if (error == FT_Err_Unknown_File_Format)
	{
		OutputDebugString(L"this font face's file format is not supported.\n");
	}
	if (error)
	{
		OutputDebugString(L"this font face's file couldn't be read.\n");
	}

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

	wchar_t output_buffer[256];
	swprintf(output_buffer, sizeof(output_buffer), L"The face contains %d  glyphs.\n", face->num_glyphs);
	OutputDebugString(output_buffer);
	swprintf(output_buffer, sizeof(output_buffer), L"The face uses %d font units per EM.\n", face->units_per_EM);
	OutputDebugString(output_buffer);
	OutputDebugString(L"\n");
}

static void resize(HWND hwnd);

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

		EndPaint(hwnd, &ps);
	}

	case WM_SIZE:
		resize(hwnd);

	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static D2D1_RENDER_TARGET_PROPERTIES rt_properties;
static D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_rt_properties;
static ID2D1Factory** factory;

static void resize(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	rt_properties = {};
	rt_properties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	rt_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R32G32B32A32_UINT, D2D1_ALPHA_MODE_STRAIGHT);
	rt_properties.dpiX = 0;
	rt_properties.dpiY = 0;
	rt_properties.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	rt_properties.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

	hwnd_rt_properties = {};
	hwnd_rt_properties.hwnd = hwnd;
	hwnd_rt_properties.pixelSize = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	hwnd_rt_properties.presentOptions = D2D1_PRESENT_OPTIONS_NONE; // Vsync on

	ID2D1HwndRenderTarget** hwnd_render_target = nullptr;

	if ((*factory)->CreateHwndRenderTarget(&rt_properties, &hwnd_rt_properties, hwnd_render_target) != S_OK)
	{
		OutputDebugString(L"Failed to create a render target from the HWND handle.\n");
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	print_fontinfo();

	D2D1_FACTORY_OPTIONS factory_options = {};
	factory_options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	if (D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &factory_options, (void**)factory) != S_OK) // 
	{
		OutputDebugString(L"Failed to create a Direct2D factory.\n");
	}

	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}