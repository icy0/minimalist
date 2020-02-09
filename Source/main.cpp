#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H


int main()
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

	if((face->face_flags & FT_FACE_FLAG_SCALABLE) == FT_FACE_FLAG_SCALABLE)
		std::cout << "This font contains outline glyphs. Note that a face can contain bitmap strikes also.\n";
	if((face->face_flags & FT_FACE_FLAG_FIXED_SIZES) == FT_FACE_FLAG_FIXED_SIZES)
		std::cout << "This font contains bitmap strikes.\n";
	if((face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) == FT_FACE_FLAG_FIXED_WIDTH)
		std::cout << "This font contains fixed width characters.\n";
	if((face->face_flags & FT_FACE_FLAG_SFNT) == FT_FACE_FLAG_SFNT)
		std::cout << "This font uses the SFNT storage scheme.\n";
	if((face->face_flags & FT_FACE_FLAG_HORIZONTAL) == FT_FACE_FLAG_HORIZONTAL)
		std::cout << "This font contains horizontal glyph metrics.\n";
	if ((face->face_flags & FT_FACE_FLAG_VERTICAL) == FT_FACE_FLAG_VERTICAL)
		std::cout << "This font contains vertical glyph metrics.\n";
	if ((face->face_flags & FT_FACE_FLAG_KERNING) == FT_FACE_FLAG_KERNING)
		std::cout << "This font contains kerning information. The kerning distance can be retrieved using the function Get_FT_Kerning()\n";

	std::cout << '\n' << "The face contains " << face->num_glyphs << " glyphs.\n";
	std::cout << '\n' << "The face uses " << face->units_per_EM << " font units per EM.\n";


}