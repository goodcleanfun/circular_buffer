#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFAULT_BUFFER_SIZE
#define NO_DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 8
#endif

#ifndef BUFFER_NAME
#error "BUFFER_NAME must be defined"
#endif

#ifndef BUFFER_TYPE
#error "BUFFER_TYPE must be defined"
#endif

#define circular_buffer_foreach(buffer, logical, var, physical, code) { \
    physical = buffer->head;                                            \
    for (logical = 0; logical < buffer->n; logical++) {                 \
        var = buffer->a[physical];                                      \
        code;                                                           \
        physical = (physical + 1) % buffer->m;                          \
    }                                                                   \
}

#endif // CIRCULAR_BUFFER_H


#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a, b)
#define BUFFER_FUNC(name) CONCAT(BUFFER_NAME, _##name)


static void BUFFER_FUNC(reverse_slice)(BUFFER_TYPE *a, size_t start, size_t end) {
    size_t i = start;
    size_t j = end;
    while (i < j) {
        BUFFER_TYPE tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
        i++;
        j--;
    }
}

typedef struct {
    bool full;
    size_t head;
    size_t tail;
    size_t n;
    size_t m;
    BUFFER_TYPE *a;
} BUFFER_NAME;

static void BUFFER_FUNC(rotate)(BUFFER_NAME *buf, size_t head, size_t tail, size_t size) {
    if (tail > 0 && size > 0) {
        BUFFER_FUNC(reverse_slice)(buf->a, head, size - 1);
        BUFFER_FUNC(reverse_slice)(buf->a, 0, tail - 1);
        BUFFER_FUNC(reverse_slice)(buf->a, 0, size - 1);
    }
}

static inline BUFFER_NAME *BUFFER_FUNC(new_size)(size_t size) {
    BUFFER_NAME *buffer = malloc(sizeof(BUFFER_NAME));
    if (buffer == NULL) return NULL;
    buffer->a = malloc((size > 0 ? size : 1) * sizeof(BUFFER_TYPE));
    if (buffer->a == NULL) return NULL;
    buffer->n = 0;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->m = size;
    return buffer;
}

static inline BUFFER_NAME *BUFFER_FUNC(new)(void) {
    return BUFFER_FUNC(new_size)(DEFAULT_BUFFER_SIZE);
}

static inline void BUFFER_FUNC(clear)(BUFFER_NAME *array) {
    array->m = 0;
    array->n = 0;
    array->head = 0;
    array->tail = 0;
}

static inline void BUFFER_FUNC(destroy)(BUFFER_NAME *buffer) {
    if (buffer == NULL) return;
    if (buffer->a != NULL) {
        #ifdef BUFFER_FREE_DATA
        for (size_t i = 0; i < buffer->m; i++) {
            BUFFER_FREE_DATA(buffer->a[i]);
        }
        #endif  
        free(buffer->a);
    }
    free(buffer);
}

#ifndef BUFFER_FIXED
static inline bool BUFFER_FUNC(resize)(BUFFER_NAME *buffer, size_t new_size) {
    size_t prev_size = buffer->m;
    if (new_size <= prev_size) return true;
    BUFFER_TYPE *ptr = realloc(buffer->a, sizeof(BUFFER_TYPE) * new_size);
    if (ptr == NULL) return false;
    buffer->a = ptr;
    buffer->m = new_size;
    if (buffer->tail <= buffer->head) {
        BUFFER_FUNC(rotate)(buffer, buffer->head, buffer->tail, prev_size);
        buffer->head = 0;
        buffer->tail = prev_size;
    }
    return true;
}
#endif

static inline bool BUFFER_FUNC(copy)(BUFFER_NAME *dst, BUFFER_NAME *src, size_t n) {
    bool ret = true;
    #ifndef BUFFER_FIXED
    if (dst->m < n) ret = BUFFER_FUNC(resize)(dst, n);
    #else
    if (dst->m < n) return false;
    #endif
    if (!ret) return false;
    memcpy(dst->a, src->a, n * sizeof(BUFFER_TYPE));
    dst->m = n;
    dst->n = n;
    return ret;
}

static inline BUFFER_NAME *BUFFER_FUNC(new_copy)(BUFFER_NAME *buffer, size_t n) {
    BUFFER_NAME *cpy = BUFFER_FUNC(new_size)(n);
    if (!BUFFER_FUNC(copy)(cpy, buffer, n)) return NULL;
    return cpy;
}

#ifndef BUFFER_FIXED
static inline bool BUFFER_FUNC(resize_if_full)(BUFFER_NAME *buffer) {
    if (buffer->n < buffer->m) return true;
    size_t current_size = buffer->m;
    size_t new_size = current_size ? current_size * 3 / 2 : DEFAULT_BUFFER_SIZE;
    return BUFFER_FUNC(resize)(buffer, new_size);
}
#endif

static inline bool BUFFER_FUNC(push)(BUFFER_NAME *buffer, BUFFER_TYPE value) {
#ifdef BUFFER_FIXED
    // fixed size
    size_t tail = buffer->tail;
    buffer->a[tail] = value;
    buffer->tail = (tail + 1) % buffer->m;
    if (buffer->n < buffer->m) {
        buffer->n++;
    } else {
        buffer->head = buffer->tail;
    }
#else
    // dynamic resizing
    if (!BUFFER_FUNC(resize_if_full)(buffer)) return false;
    size_t tail = buffer->tail;
    buffer->a[tail] = value;
    buffer->n++;
    buffer->tail = (tail + 1) % buffer->m;
#endif
    return true;
}

static inline bool BUFFER_FUNC(push_left)(BUFFER_NAME *buffer, BUFFER_TYPE value) {
#ifdef BUFFER_FIXED
    // fixed size
    if (buffer->head == 0) {
        buffer->head = buffer->m - 1;
    } else {
        buffer->head--;
    }
    buffer->a[buffer->head] = value;
    if (buffer->n < buffer->m) {
        buffer->n++;
    } else {
        buffer->tail = buffer->head;
    }
#else
    // dynamic resizing
    if (!BUFFER_FUNC(resize_if_full)(buffer)) return false;
    if (buffer->head == 0) {
        buffer->head = buffer->m - 1;
    } else {
        buffer->head--;
    }
    buffer->a[buffer->head] = value;
    buffer->n++;
#endif
    return true;
}

static inline bool BUFFER_FUNC(pop)(BUFFER_NAME *buffer, BUFFER_TYPE *result) {
    if (!buffer) return false;
    if (buffer->n == 0) return false;
    *result = buffer->a[buffer->tail];
    if (buffer->tail == 0) {
        buffer->tail = buffer->m - 1;
    } else {
        buffer->tail--;
    }
    buffer->n--;
    return true;
}

static inline bool BUFFER_FUNC(pop_left)(BUFFER_NAME *buffer, BUFFER_TYPE *result) {
    if (!buffer) return false;
    if (buffer->n == 0) return false;
    *result = buffer->a[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->m;
    buffer->n--;
    return true;
}

#ifndef BUFFER_FIXED
static inline bool BUFFER_FUNC(extend)(BUFFER_NAME *array, BUFFER_TYPE *other, size_t size) {
    bool ret = false;
    size_t new_size = array->m + size;
    if (new_size > array->m) ret = BUFFER_FUNC(resize)(array, new_size);
    if (!ret) return false;
    for (size_t i = 0; i < size; i++) {
        BUFFER_FUNC(push)(array, other[i]);
    }
    return true;
}

static inline bool BUFFER_FUNC(extend_left)(BUFFER_NAME *array, BUFFER_TYPE *other, size_t size) {
    bool ret = false;
    size_t new_size = array->m + size;
    if (new_size > array->m) ret = BUFFER_FUNC(resize)(array, new_size);
    if (!ret) return false;
    for (size_t i = 0; i < size; i++) {
        BUFFER_FUNC(push_left)(array, other[i]);
    }
    return true;
}
#endif
