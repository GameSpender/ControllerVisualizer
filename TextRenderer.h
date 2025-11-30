#pragma once
#include <map>
#include <string>
#include <glm/glm.hpp>

#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Transform2D.h"
#include "SpriteRenderer.h"

#include <vector>
#include <iostream>


struct Glyph {
    unsigned int texture;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class TextRenderer : public Transform2D
{
public:
    std::map<char, Glyph> glyphs;

    bool LoadFont(const char* path, int pixelSize, glm::vec3 bakeColor);
    void DrawText(SpriteRenderer& renderer, const std::string& text);
};


bool TextRenderer::LoadFont(const char* path, int pixelSize, glm::vec3 bakeColor)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        return false;

    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face))
        return false;

    FT_Set_Pixel_Sizes(face, 0, pixelSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        int w = face->glyph->bitmap.width;
        int h = face->glyph->bitmap.rows;

        // Convert 8-bit FreeType bitmap → 32-bit RGBA
        std::vector<unsigned char> rgba(w * h * 4);

        for (int i = 0; i < w * h; i++)
        {
            unsigned char alpha = face->glyph->bitmap.buffer[i];
            rgba[i * 4 + 0] = (unsigned char)(bakeColor.r * 255);
            rgba[i * 4 + 1] = (unsigned char)(bakeColor.g * 255);
            rgba[i * 4 + 2] = (unsigned char)(bakeColor.b * 255);
            rgba[i * 4 + 3] = alpha; // glyph alpha
        }

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA,
            w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            rgba.data()
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glyphs[c] = {
            tex,
            glm::ivec2(w, h),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return true;
}

void TextRenderer::DrawText(SpriteRenderer& renderer, const std::string& text)
{
    glm::vec2 cursor(0.0f);

    for (char c : text)
    {
        if (!glyphs.count(c)) continue;
        const Glyph& g = glyphs[c];

        // Compute glyph position relative to the TextRenderer
        // Y-axis correction
        float xpos = cursor.x + (g.bearing.x + g.size.x / 2.0f) * scale.x;
        float ypos = cursor.y - (g.size.y - g.bearing.y) * scale.y;

        Transform2D glyphTransform;
        glyphTransform.position = position + glm::vec2(xpos, ypos);
        glyphTransform.rotation = rotation;
        glyphTransform.scale = scale * glm::vec2(g.size);
        // Advance cursor by FreeType's horizontal advance (1/64 pixels)
        

        renderer.Draw(g.texture, glyphTransform.getWorldMatrix());
        cursor.x += (g.advance >> 6) * scale.x;

    }
}

