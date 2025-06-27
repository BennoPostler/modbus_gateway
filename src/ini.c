/* ini.c -- simple .INI file parser

This code is released under the MIT license.

See: https://github.com/benhoyt/inih
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ini.h"

#define MAX_LINE 200

static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

static char* find_char_or_comment(const char* s, char c)
{
    int was_whitespace = 0;
    while (*s && *s != c && !(was_whitespace && *s == ';')) {
        was_whitespace = isspace((unsigned char)(*s));
        s++;
    }
    return (char*)s;
}

static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

int ini_parse_file(FILE* file, int (*handler)(void*, const char*, const char*, const char*), void* user)
{
    char line[MAX_LINE];
    char section[MAX_LINE] = "";
    char prev_name[MAX_LINE] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

    while (fgets(line, MAX_LINE, file) != NULL) {
        lineno++;
        start = lskip(rstrip(line));

        if (*start == ';' || *start == '#') {
            // skip comment
        } else if (*start == '[') {
            // [section]
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            } else if (!error) {
                error = lineno;
            }
        } else if (*start && *start != ';') {
            // name=value
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                if (!error) error = lineno;
            } else {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                rstrip(value);

                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            }
        }
    }

    return error;
}

int ini_parse(const char* filename,
              int (*handler)(void*, const char*, const char*, const char*),
              void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file) return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}
