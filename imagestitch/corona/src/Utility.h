#ifndef CORONA_UTILITY_H
#define CORONA_UTILITY_H


#include "Types.h"


#define COR_EXPORT(ret, name)  ret COR_CALL name


namespace corona {

  template<typename T>
  class auto_array {
  public:
    explicit auto_array(T* initial = 0) {
      array = initial;
    }
    
    ~auto_array() {
      delete[] array;
    }

    operator T*() const {
      return array;
    }

    T* get() const {
      return array;
    }

    T* release() {
      T* old = array;
      array = 0;
      return old;
    }

    auto_array<T>& operator=(T* a) {
      if (array != a) {
	delete array;
	array = a;
      }
      return *this;
    }

  private:
    T* array;
  };


  // replace these with std::{min,max} when VC7 is out  :(
  template<typename T>
  T Min(T a, T b) {
    return a < b ? a : b;
  }
  template<typename T>
  T Max(T a, T b) {
    return a > b ? a : b;
  }


  // does this format not use a palette?
  inline bool IsDirect(PixelFormat format) {
    return (format == PF_R8G8B8A8 || format == PF_R8G8B8 ||
            format == PF_B8G8R8A8 || format == PF_B8G8R8);
  }

  // does this format require a palette?
  inline bool IsPalettized(PixelFormat format) {
    return format == PF_I8;
  }

  inline int GetPaletteSize(PixelFormat format) {
    return (format == PF_I8 ? 256 : 0);
  }


  inline u16 read16_le(const byte* b) {
    return b[0] + (b[1] << 8);
  }

  inline u16 read16_be(const byte* b) {
    return (b[0] << 8) + b[1];
  }

  inline u32 read32_le(const byte* b) {
    return read16_le(b) + (read16_le(b + 2) << 16);
  }

  inline u32 read32_be(const byte* b) {
    return (read16_be(b) << 16) + read16_be(b + 2);
  }


  struct RGB {
    byte red;
    byte green;
    byte blue;
  };

  struct RGBA {
    byte red;
    byte green;
    byte blue;
    byte alpha;
  };

  struct BGR {
    byte blue;
    byte green;
    byte red;
  };

  struct BGRA {
    byte blue;
    byte green;
    byte red;
    byte alpha;
  };

}


#endif
