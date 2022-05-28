#include "module.h"
#include "options.h"

FT_Library         freetypeHandle     = { 0 };
FT_Face            freetypeFaceNormal = { 0 };
cairo_font_face_t *fontFaceNormal     = { 0 };

double glyphHeight   = 0;
double lineHeight    = 0;
double glyphWidth    = 0;
double capitalHeight = 0;

/* Interface_loadFonts
 * Initializes FreeType, loads all fonts, and gathers information about font
 * dimensions.
 */
Error Interface_loadFonts (void) {
        int err = FT_Init_FreeType(&freetypeHandle);
        if (err) { return Error_cantInitFreetype; }
        err = FT_New_Face (
                freetypeHandle,
                Options_fontName,
                0, &freetypeFaceNormal);
        if (err) { return Error_cantLoadFont; }
        fontFaceNormal = cairo_ft_font_face_create_for_ft_face (
                freetypeFaceNormal, 0);

        Interface_fontNormal();

        cairo_text_extents_t textExtents;
        cairo_text_extents(Window_context, "M", &textExtents);
        capitalHeight = textExtents.height;
        
        cairo_font_extents_t fontExtents;
        cairo_font_extents(Window_context, &fontExtents);
        lineHeight  = fontExtents.height;
        glyphHeight = fontExtents.ascent;
        glyphWidth  = fontExtents.max_x_advance;

        return Error_none;
}

/* Interface_fontNormal
 * Sets the font to the standard normal font.
 */
void Interface_fontNormal (void) {
        cairo_set_font_size(Window_context, Options_fontSize);
        cairo_set_font_face(Window_context, fontFaceNormal);
}

// void Interface_fontBold (void) {
        // cairo_set_font_size(Window_context, fontSize);
        // cairo_set_font_face(Window_context, fontFaceBold);
// }

// void Interface_fontItalic (void) {
        // cairo_set_font_size(Window_context, fontSize);
        // cairo_set_font_face(Window_context, fontFaceItalic);
// }

// void Interface_fontBoldItalic (void) {
        // cairo_set_font_size(Window_context, fontSize);
        // cairo_set_font_face(Window_context, fontFaceBoldItalic);
// }
