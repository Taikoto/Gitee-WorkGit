#include "minui.h"
#include <dlfcn.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <../factorytest.h>

//https://www.freetype.org/freetype2/docs/reference/ft2-index.html

int (*Init_FreeType)( FT_Library *);
int (*New_Face)( FT_Library, const char*, FT_Long,FT_Face* );
int (*Set_Char_Size)( FT_Face, FT_F26Dot6, FT_F26Dot6, FT_UInt, FT_UInt );
int (*Set_Pixel_Sizes)( FT_Face, FT_UInt, FT_UInt );
int (*Load_Glyph)( FT_Face, FT_UInt, FT_Int32);
int (*Outline_Embolden)( FT_Outline*, FT_Pos);
int (*Get_Char_Index)( FT_Face, FT_ULong);
int (*Get_Glyph)( FT_GlyphSlot, FT_Glyph* );
void (*Done_Glyph)( FT_Glyph );
int (*Done_Face)( FT_Face );
int (*Glyph_To_Bitmap)( FT_Glyph*, FT_Render_Mode, FT_Vector*, FT_Bool );

FT_Library pFTLib = NULL;
FT_Face pFTFace = NULL;

static int Render_mode;
//static int font_size;
int utf8towc(wchar_t *, const char *, int );

void set_render_mode(int mode)
{
    Render_mode = mode;
}

void freetype_setsize(unsigned int size)
{
    LOGD("set font size = %d", size);
    if(pFTFace)
        Set_Pixel_Sizes(pFTFace, size, 0);
    else
        LOGE("do FT_Set_Char_Size error, pFTFace is NULL");
}

void *handle;

int freetype_init()
{
    FT_Error error = 1;

    handle = dlopen(FREETYPE_LIB, RTLD_NOW);
    if(handle == NULL)
    {
        LOGE("fail dlopen %s", FREETYPE_LIB);
        return 0;
    }

    Init_FreeType = dlsym(handle, "FT_Init_FreeType");
    New_Face = dlsym(handle, "FT_New_Face");
    Set_Char_Size = dlsym(handle, "FT_Set_Char_Size");
    Set_Pixel_Sizes = dlsym(handle, "FT_Set_Pixel_Sizes");
    Load_Glyph = dlsym(handle, "FT_Load_Glyph");
    Outline_Embolden = dlsym(handle, "FT_Outline_Embolden");
    Get_Char_Index = dlsym(handle, "FT_Get_Char_Index");
    Get_Glyph = dlsym(handle, "FT_Get_Glyph");
    Done_Glyph = dlsym(handle, "FT_Done_Glyph");
    Done_Face = dlsym(handle, "FT_Done_Face");
    Glyph_To_Bitmap = dlsym(handle, "FT_Glyph_To_Bitmap");

    if( Init_FreeType && New_Face && Set_Char_Size &&
    Load_Glyph && Get_Char_Index && Get_Glyph &&
    Done_Glyph && Done_Face && Glyph_To_Bitmap &&
    Set_Pixel_Sizes)
    {
        LOGD("do FT_Init_FreeType");
        error = Init_FreeType( &pFTLib);			//Init FreeType Lib to manage memory
        if (error)
        {
            pFTLib	=	0 ;
            LOGE("Init Library error");
            return	 - 1 ;
        }

        LOGD("do FT_New_Face");
        error = New_Face(pFTLib, FONTS_PATH, 0, &pFTFace);
        if ( error == FT_Err_Unknown_File_Format )
        {
            LOGE("unsupport format");
        }
        else if (error)
        {
            pFTLib	=	0 ;
            LOGE("New_Face error");
            return  -1 ;
        }
        LOGD("total face nums = %d",pFTFace->num_faces);
        return 1;
    }
    else
        return 0;
}

char* freetype(unsigned long charcode, Box *location)
{
    FT_Error error = 1;
    int i = 0, j = 0;
    static char map[100*100];       //char max size: 100*100

    FT_Glyph	glyph;

    Load_Glyph(pFTFace, Get_Char_Index(pFTFace, charcode ), FT_LOAD_DEFAULT);
    if(Render_mode & Render_BOLD)
        Outline_Embolden( &pFTFace->glyph->outline, CHAR_SIZE*4);

    error  =  Get_Glyph(pFTFace->glyph,  &glyph);
    if ( !error)
    {
        Glyph_To_Bitmap( & glyph, ft_render_mode_normal, 0, 1);
        FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
        FT_Bitmap bitmap = bitmap_glyph->bitmap;

        memset(map, 0, sizeof(map));
        memcpy(map, bitmap.buffer, bitmap.width * bitmap.rows);
        location->width = bitmap.width;
        location->height = bitmap.rows;
        location->top = bitmap_glyph->top;//pFTFace->glyph->bitmap_top;
        location->left = bitmap_glyph->left;//pFTFace->glyph->bitmap_top;

        Done_Glyph(glyph);		//free glyph
        glyph  =  NULL;
    }
    return map;
}

void ft_freeface(void)
{
	Done_Face(pFTFace);		  //free face
	pFTFace  =	NULL;
}

int get_string_room_size(const char *s)
{
    unsigned bytes;
    wchar_t unicode;
    Box char_position;
    int room_size = 0;

    while(*s)
    {
        LOGD("room_size = %d", room_size);
        if(*(unsigned char*)(s) <= 0x20)
        {
            s++;
            room_size += (CHAR_SIZE / 2);
            continue;
        }
        bytes = utf8towc(&unicode, s, strlen(s));
        if(bytes <= 0)
            break;
        s += bytes;

        freetype((unsigned long)unicode, &char_position);
        room_size += char_position.width;
    }

    return room_size;
}


