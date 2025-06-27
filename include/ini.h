#ifndef __INI_H__
#define __INI_H__

#ifdef __cplusplus
extern "C" {
#endif

int ini_parse(const char* filename,
              int (*handler)(void*, const char*, const char*, const char*),
              void* user);

int ini_parse_file(FILE* file,
                   int (*handler)(void*, const char*, const char*, const char*),
                   void* user);

#ifdef __cplusplus
}
#endif

#endif /* __INI_H__ */
