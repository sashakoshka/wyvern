#include "module.h"
#include "options.h"

/* Interface_loadFonts
 * Initializes FreeType, loads all fonts, and gathers information about font
 * dimensions.
 */
Error Interface_loadFonts (void) {
        int err = FT_Init_FreeType(&interface.fonts.freetypeHandle);
        if (err) { return Error_cantInitFreetype; }
        err = FT_New_Face (
                interface.fonts.freetypeHandle,
                Options_fontName,
                0, &interface.fonts.freetypeFaceNormal);
        if (err) { return Error_cantLoadFont; }
        interface.fonts.fontFaceNormal = cairo_ft_font_face_create_for_ft_face (
                interface.fonts.freetypeFaceNormal, 0);

        Interface_fontNormal();

        cairo_text_extents_t textExtents;
        cairo_text_extents(Window_context, "M", &textExtents);
        interface.fonts.capitalHeight = textExtents.height;
        
        cairo_font_extents_t fontExtents;
        cairo_font_extents(Window_context, &fontExtents);
        interface.fonts.lineHeight  = fontExtents.height;
        interface.fonts.glyphHeight = fontExtents.ascent;
        interface.fonts.glyphWidth  = fontExtents.max_x_advance;

        return Error_none;
}

/* Interface_fontNormal
 * Sets the font to the standard normal font.
 */
void Interface_fontNormal (void) {
        cairo_set_font_size(Window_context, Options_fontSize);
        cairo_set_font_face(Window_context, interface.fonts.fontFaceNormal);
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
