#include "cutils.h"
#include <stdarg.h>

#ifdef NDEBUG
#define DEBUGLOG(...)
#else
#define DEBUGLOG(...) fprintf(stderr, __VA_ARGS__)
#endif

int sb_appendf(StringBuilder *sb, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    da_reserve(sb, sb->count + n + 1);
    char *dst = sb->items + sb->count;
    va_start(args, fmt);
    vsnprintf(dst, n+1, fmt, args);
    va_end(args);
    sb->count += n;
    return n;
}

bool read_entire_file(const char *filepath, StringBuilder *sb)
{
    FILE *f = fopen(filepath, "rb");
    if(!f) {
        DEBUGLOG("error: Could not open a file '%s'\n", filepath);
        return false;
    }
    if (fseek(f, 0, SEEK_END) < 0) {
        DEBUGLOG("error: Could not seek into file\n");
        fclose(f);
        return false;
    }
    int64_t fsz = ftell(f);
    if (fsz <= 0) {
        DEBUGLOG("error: Could not get the file size\n");
        fclose(f);
        return false;
    }
    if (fseek(f, 0, SEEK_SET) < 0) {
        DEBUGLOG("error: Could not seek into file\n");
        fclose(f);
        return false;
    }

    da_reserve(sb, sb->count + fsz + 1);
    fread(sb->items + sb->count, fsz, 1, f);
    sb->items[sb->count + fsz] = 0;
    if (ferror(f)) {
        DEBUGLOG("error: Could not read into file\n");
        fclose(f);
        return false;
    }
    sb->count += fsz;
    fclose(f);
    return true;
}

bool write_entire_file(const char *filepath, const void *data, size_t size)
{
    FILE *f = fopen(filepath, "wb");
    if(!f) {
        DEBUGLOG("error: Could not open a file '%s'\n", filepath);
        return false;
    }

    const char *buf = data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, f);
        if (ferror(f)) {
            DEBUGLOG("error: Could not write into file '%s': %s\n", filepath, strerror(errno));
            fclose(f);
            return false;
        }
        size -= n;
        buf  += n;
    }

    fclose(f);
    return true;
}


#define HEAP_PAGE_SIZE 4096
void *arena_alloc(Arena *a, size_t bytesize)
{
    size_t size = (bytesize + sizeof(uintptr_t) - 1)/sizeof(uintptr_t);

    if (a->end == NULL) {
        assert(a->begin == NULL);
        size_t capacity = HEAP_PAGE_SIZE;
        if (capacity < size) capacity = size;
        size_t allocated_bytes = sizeof(ArenaRegion) + sizeof(uintptr_t) * capacity;

        ArenaRegion *r = (ArenaRegion*)malloc(allocated_bytes);
        assert(r != NULL);
        r->next = NULL;
        r->count = 0;
        r->capacity = capacity;
        a->end = r;
        a->begin = r;
    }

    while (a->end->count + size > a->end->capacity && a->end->next != NULL) {
        a->end = a->end->next;
    }

    if (a->end->count + size > a->end->capacity) {
        assert(a->end->next == NULL);
        size_t capacity = HEAP_PAGE_SIZE;
        if (capacity < size) capacity = size;

        size_t allocated_bytes = sizeof(ArenaRegion) + sizeof(uintptr_t) * capacity;
        ArenaRegion *r = (ArenaRegion*)malloc(allocated_bytes);
        assert(r != NULL);
        r->next = NULL;
        r->count = 0;
        r->capacity = capacity;
        a->end->next = r;
        a->end = a->end->next;
    }

    void *result = &a->end->data[a->end->count];
    a->end->count += size;
    return result;
}

void arena_reset(Arena *a)
{
    for (ArenaRegion *r = a->begin; r != NULL; r = r->next) {
        r->count = 0;
    }
    a->end = a->begin;
}

void arena_free(Arena *a)
{
    ArenaRegion *r = a->begin;
    while (r) {
        ArenaRegion *r0 = r;
        r = r->next;
        free(r0);
    }
    a->begin = NULL;
    a->end = NULL;
}

char *arena_strndup(Arena *a, const char *cstr, size_t cstrlen)
{
    char *result = arena_alloc(a, cstrlen + 1);
    result[cstrlen] = 0;
    memcpy(result, cstr, cstrlen);
    return result;
}

char *arena_strdup(Arena *a, const char *cstr)
{
    return arena_strndup(a, cstr, strlen(cstr));
}

char *arena_sprintf(Arena *a, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char *dst = arena_alloc(a, n + 1);
    dst[n] = 0;
    va_start(args, fmt);
    vsnprintf(dst, n+1, fmt, args);
    va_end(args);
    return dst;
}

size_t arena_get_usage(Arena *a)
{
    size_t result = 0;
    for (ArenaRegion *r = a->begin; r != NULL; r = r->next) {
        result += r->count;
    }
    return result;
}
