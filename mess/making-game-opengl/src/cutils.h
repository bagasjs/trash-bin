#ifndef CUTILS_H_
#define CUTILS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(xs) (sizeof(xs)/sizeof((xs)[0]))
#endif

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif

#define shiftargs(argc, argv) (assert(argc > 0), (argc)--, *(argv)++)

#if !defined(CUT_MALLOC) && !defined(CUT_FREE)
#include <stdlib.h>
#define CUT_MALLOC malloc
#define CUT_FREE   free
#endif
#if !defined(CUT_MALLOC) || !defined(CUT_FREE)
#error "Please define both CUT_MALLOC and CUT_FREE macros"
#endif
#ifndef CUT_ASSERT
#include <assert.h>
#define CUT_ASSERT assert
#endif

#define UNUSED(value) (void)(value)
#define TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s", __FILE__, __LINE__, (message)); abort(); } while(0)

#define da_free(da) free((da)->items)

#define da_reserve(da, required_cap)                                \
    do {                                                            \
        size_t item_size = sizeof(*(da)->items);                    \
        if(required_cap > (da)->capacity) {                         \
            if((da)->capacity == 0) (da)->capacity = DA_INIT_CAP;   \
            while((da)->capacity < required_cap)                    \
                (da)->capacity *= 2;                                \
            void *items = CUT_MALLOC((da)->capacity * item_size);   \
            CUT_ASSERT(items != NULL && "Buy More RAM LOL!");       \
            if((da)->items) {                                       \
                memcpy(items, (da)->items, (da)->count*item_size);  \
                CUT_FREE((da)->items);                              \
            }                                                       \
            (da)->items = items;                                    \
        }                                                           \
    } while(0)

#define da_append(da, item)                                         \
    do {                                                            \
        da_reserve((da), (da)->count + 1);                          \
        (da)->items[(da)->count++] = (item);                        \
    } while(0)

#define da_append_many(da, new_items, new_items_count)              \
    do {                                                            \
        da_reserve((da), (da)->count + new_items_count);            \
        memcpy((da)->items + (da)->count, (new_items),              \
            (new_items_count)*item_size);                           \
        (da)->count += (new_items_count);                           \
    } while(0)


typedef struct StringBuilder {
    char *items;
    size_t count;
    size_t capacity;
} StringBuilder;

int sb_appendf(StringBuilder *sb, const char *fmt, ...);

bool read_entire_file(const char *filepath, StringBuilder *sb);
bool write_entire_file(const char *filepath, const void *data, size_t datasize);

typedef struct ArenaRegion ArenaRegion;
struct ArenaRegion {
    ArenaRegion *next;
    size_t count;
    size_t capacity;
    uintptr_t data[];
};
typedef struct Arena {
    ArenaRegion *begin;
    ArenaRegion *end;
} Arena;

void *arena_alloc(Arena *a, size_t bytesize);
void  arena_reset(Arena *a);
void  arena_free(Arena *a);
char *arena_strndup(Arena *a, const char *cstr, size_t cstrlen);
char *arena_strdup(Arena *a, const char *cstr);
char *arena_sprintf(Arena *a, const char *fmt, ...);
size_t arena_get_usage(Arena *a);

#include <stdio.h>
#define arena_da_reserve(arena, da, required_cap)                               \
    do {                                                                        \
        size_t item_size = sizeof(*(da)->items);                                \
        if(required_cap > (da)->capacity) {                                     \
            if((da)->capacity == 0) (da)->capacity = DA_INIT_CAP;               \
            while((da)->capacity < required_cap)                                \
                (da)->capacity *= 2;                                            \
            void *items = arena_alloc((arena), (da)->capacity * item_size);     \
            CUT_ASSERT(items != NULL && "Buy More RAM LOL!");                   \
            if((da)->items) memcpy(items, (da)->items, (da)->count*item_size);  \
            (da)->items = items;                                                \
        }                                                                       \
    } while(0)

#define arena_da_append(arena, da, item)                            \
    do {                                                            \
        arena_da_reserve((arena), (da), (da)->count + 1);           \
        (da)->items[(da)->count++] = (item);                        \
    } while(0)

#endif // CUTILS_H_
