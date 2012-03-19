#include <string.h>
#include <memory>
#include "corona.h"
#include "DefaultFileSystem.h"
#include "MemoryFile.h"
#include "Open.h"
#include "Save.h"
#include "SimpleImage.h"
#include "Utility.h"


namespace corona {
  namespace hidden {

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(const char*, CorGetVersion)() {
      return "1.0.0";
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(const char*, CorGetSupportedReadFormats)() {
      return
#ifndef NO_PNG
	"PNG Files:png"  ";"
#endif
#ifndef NO_JPEG
	"JPEG Files:jpeg,jpg"  ";"
#endif
	"PCX Files:pcx"  ";"
	"BMP Files:bmp"  ";"
	"TGA Files:tga"  ";"
	"GIF Files:gif"  ;
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(const char*, CorGetSupportedWriteFormats)() {
      return ""
#ifndef NO_PNG
	"PNG Files:png"
#endif
        ;
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(Image*, CorCreateImage)(
      int width,
      int height,
      PixelFormat format)
    {
      // this function only supports creation of non-palettized images
      if (!IsDirect(format)) {
        return 0;
      }

      int size = width * height * GetPixelSize(format);
      byte* pixels = new byte[size];
      memset(pixels, 0, size);
      return new SimpleImage(width, height, format, pixels);
    }
 
    COR_EXPORT(Image*, CorCreateImageWithPixels)(
      int width,
      int height,
      PixelFormat format,
      void* pixels)
    { 	
	if(!IsDirect(format)){
		return 0;
	}
	
	return new SimpleImage(width, height, format, (byte*)pixels);
    }
    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(Image*, CorCreatePalettizedImage)(
      int width,
      int height,
      PixelFormat format,
      int palette_size,
      PixelFormat palette_format)
    {
      // only support creation of palettized images
      if (!IsPalettized(format) || !IsDirect(palette_format)) {
        return 0;
      }

      // make sure the palette is the right size
      if (palette_size != GetPaletteSize(format)) {
        return 0;
      }

      int size = width * height * GetPixelSize(format);
      byte* pixels = new byte[size];
      memset(pixels, 0, size);

      int palette_bytes = palette_size * GetPixelSize(palette_format);
      byte* palette = new byte[palette_bytes];
      memset(palette, 0, palette_bytes);

      return new SimpleImage(width, height, format, pixels,
                             palette, palette_size, palette_format);      
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(Image*, CorCloneImage)(
      Image* source,
      PixelFormat format)
    {
      if (!source) {
        // we need an image to clone :)
        return 0;
      }

      const int width = source->getWidth();
      const int height = source->getHeight();
      const PixelFormat source_format = source->getFormat();

      const int source_pixel_size = GetPixelSize(source_format);
      if (source_pixel_size == 0) {
        // unknown pixel size?
        return 0;
      }

      // duplicate the image
      int image_size = width * height * source_pixel_size;
      byte* pixels = new byte[image_size];
      memcpy(pixels, source->getPixels(), image_size);
      
      if (IsPalettized(source_format)) {
        // clone palette
        int palette_size = source->getPaletteSize();
        PixelFormat palette_format = source->getPaletteFormat();
        int palette_bytes = palette_size * GetPixelSize(palette_format);
        byte* palette = new byte[palette_bytes];
        memcpy(palette, source->getPalette(), palette_bytes);
        Image* image = new SimpleImage(width, height, source_format, pixels,
                                       palette, palette_size, palette_format);
        return ConvertImage(image, format);
      }

      Image* image = new SimpleImage(width, height, source_format, pixels);
      return ConvertImage(image, format);
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(Image*, CorOpenImage)(
      const char* filename,
      FileFormat file_format)
    {
      if (!filename) {
        return 0;
      }

      std::auto_ptr<File> file(OpenDefaultFile(filename, false));
      return CorOpenImageFromFile(file.get(), file_format);
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(Image*, CorOpenImageFromFile)(
      File* file,
      FileFormat file_format)
    {
      if (!file) {
        return 0;
      }

#define TRY_TYPE(type)                                 \
  {                                                    \
    Image* image = CorOpenImageFromFile(file, (type)); \
    if (image) { return image; }                       \
  }

      file->seek(0, File::BEGIN);
      switch (file_format) {
        case FF_AUTODETECT: {
#ifndef NO_PNG
          TRY_TYPE(FF_PNG);
#endif
#ifndef NO_JPEG
          TRY_TYPE(FF_JPEG);
#endif
          TRY_TYPE(FF_PCX);
          TRY_TYPE(FF_BMP);
          TRY_TYPE(FF_TGA);
          TRY_TYPE(FF_GIF);
          return 0;
        }
        
#ifndef NO_PNG
        case FF_PNG:  return OpenPNG(file);
#endif
#ifndef NO_JPEG
        case FF_JPEG: return OpenJPEG(file);
#endif
        case FF_PCX:  return OpenPCX(file);
        case FF_BMP:  return OpenBMP(file);
        case FF_TGA:  return OpenTGA(file);
        case FF_GIF:  return OpenGIF(file);
        default:      return 0;
      }
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(bool, CorSaveImage)(
      const char* filename,
      FileFormat file_format,
      Image* image)
    {
      if (!filename) {
        return false;
      }

      std::auto_ptr<File> file(OpenDefaultFile(filename, true));
      return CorSaveImageToFile(file.get(), file_format, image);
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(bool, CorSaveImageToFile)(
      File* file,
      FileFormat file_format,
      Image* image)
    {
      if (!file || !image) {
        return false;
      }

      switch (file_format) {
#ifndef NO_PNG
        case FF_PNG:  return SavePNG(file, image);
#endif
        case FF_JPEG: return false;
        case FF_PCX:  return false;
        case FF_BMP:  return false;
        case FF_TGA:  return false;
        case FF_GIF:  return false;
        default:      return false;
      }
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(File*, CorCreateMemoryFile)(
      const void* buffer,
      int size)
    {
      if (size && !buffer) {
        return 0;
      }
      if (size < 0) {
        return 0;
      }

      return new MemoryFile(buffer, size);
    }

    ///////////////////////////////////////////////////////////////////////////

    COR_EXPORT(int, CorGetPixelSize)(PixelFormat format) {
      switch (format) {
        case PF_R8G8B8A8: return 4;
        case PF_R8G8B8:   return 3;
        case PF_B8G8R8A8: return 4;
        case PF_B8G8R8:   return 3;
        case PF_I8:       return 1;
        default:          return 0;
      }
    }

    ///////////////////////////////////////////////////////////////////////////

  }
}
