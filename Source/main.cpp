#include <iostream>
#include <ft2build.h>
#include <windows.h>
#include FT_FREETYPE_H

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
	FT_Library library;
	FT_Face face;

	FT_Error error = FT_Init_FreeType(&library);

	if (error)
	{
		std::cout << "couldn't init FreeType.\n";
	}

	error = FT_New_Face(library, "Resources/LiberationMono-Regular.ttf", 0, &face); 
	// set 3rd param to -1 to retrieve number of faces in the font by accessing face->num_faces.

	if (error == FT_Err_Unknown_File_Format)
	{
		std::cout << "this font face's file format is not supported.\n";
	}
	if (error)
	{
		std::cout << "this font face's file couldn't be read.\n";
	}

	if ((face->face_flags & FT_FACE_FLAG_SCALABLE) == FT_FACE_FLAG_SCALABLE)
		OutputDebugString("This font contains outline glyphs. Note that a face can contain bitmap strikes also.\n");
	if((face->face_flags & FT_FACE_FLAG_FIXED_SIZES) == FT_FACE_FLAG_FIXED_SIZES)
		OutputDebugString("This font contains bitmap strikes.\n");
	if ((face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) == FT_FACE_FLAG_FIXED_WIDTH)
		OutputDebugString("This font contains fixed width characters.\n");
	if((face->face_flags & FT_FACE_FLAG_SFNT) == FT_FACE_FLAG_SFNT)
		OutputDebugString("This font uses the SFNT storage scheme.\n");
	if((face->face_flags & FT_FACE_FLAG_HORIZONTAL) == FT_FACE_FLAG_HORIZONTAL)
		OutputDebugString("This font contains horizontal glyph metrics.\n");
	if ((face->face_flags & FT_FACE_FLAG_VERTICAL) == FT_FACE_FLAG_VERTICAL)
		OutputDebugString("This font contains vertical glyph metrics.\n");
	if ((face->face_flags & FT_FACE_FLAG_KERNING) == FT_FACE_FLAG_KERNING)
		OutputDebugString("This font contains kerning information. The kerning distance can be retrieved using the function Get_FT_Kerning()\n");

	char output_buffer[256];
	sprintf_s(output_buffer, "\nThe face contains %d  glyphs.\n", face->num_glyphs);
	OutputDebugString(output_buffer);
	sprintf_s(output_buffer, "\nThe face uses %d font units per EM.\n", face->units_per_EM);
	OutputDebugString(output_buffer);

	return 0;
}