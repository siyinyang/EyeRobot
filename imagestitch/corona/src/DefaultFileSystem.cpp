#include <stdio.h>
#include "string.h"
#include "DefaultFileSystem.h"
#include "Utility.h"


namespace corona {

  class CFile : public DLLImplementation<File> {
  public:
    CFile(FILE* file) {
      m_file = file;
    }

    ~CFile() {
      fclose(m_file);
    }

    int COR_CALL read(void* buffer, int size) {
      return fread(buffer, 1, size, m_file);
    }

    int COR_CALL write(const void* buffer, int size) {
      return fwrite(buffer, 1, size, m_file);
    }

    bool COR_CALL seek(int position, SeekMode mode) {
      int m;
      switch (mode) {
        case BEGIN:   m = SEEK_SET; break;
        case CURRENT: m = SEEK_CUR; break;
        case END:     m = SEEK_END; break;
        default: return false;
      }
      return fseek(m_file, position, m) == 0;
    }

    int COR_CALL tell() {
      return ftell(m_file);
    }

  private:
    FILE* m_file;
  };


  File* OpenDefaultFile(const char* filename, bool writeable) {
    char mode_str[3] = " b";
    mode_str[0] = (writeable ? 'w' : 'r');

    FILE* file = fopen(filename, mode_str);
    return (file ? new CFile(file) : 0);
  }

}
