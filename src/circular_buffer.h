#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_BUFFER_SIZE 8


#define REVERSE_ARRAY_SLICE(a, type, start, end) {          \
    size_t i = start;                                       \
    size_t j = end;                                         \
    while (i < j) {                                         \
        type tmp = a[i];                                    \
        a[i] = a[j];                                        \
        a[j] = tmp;                                         \
        i++;                                                \
        j--;                                                \
    }                                                       \
}

#define ROTATE_BUFFER(buf, type, head, tail, size) {            \
    if (tail > 0 && size > 0) {                                 \
        REVERSE_ARRAY_SLICE(buf->a, type, head, size - 1);      \
        REVERSE_ARRAY_SLICE(buf->a, type, 0, tail - 1);         \
        REVERSE_ARRAY_SLICE(buf->a, type, 0, size - 1);         \
    }                                                           \
}

#define circular_buffer_foreach(buffer, var, code) {            \
    size_t i = buffer->head;                                    \
    size_t n = buffer->n;                                       \
    for (size_t count = 0; count < n; count++) {                \
        var = buffer->a[i];                                     \
        code;                                                   \
        i = (i + 1) % buffer->m;                                \
    }                                                           \
}

#define __CIRCULAR_BUFFER_BASE(name, type)                                                  \
    typedef struct {                                                                        \
        bool full;                                                                          \
        size_t head;                                                                        \
        size_t tail;                                                                        \
        size_t n;                                                                   \
        size_t m;                                                                        \
        type *a;                                                                            \
    } name;                                                                                 \
                                                                                            \
    static inline name *name##_new_size(size_t size) {                                      \
        name *buffer = malloc(sizeof(name));                                                \
        if (buffer == NULL) return NULL;                                                    \
        buffer->a = malloc((size > 0 ? size : 1) * sizeof(type));                           \
        if (buffer->a == NULL) return NULL;                                                 \
        buffer->n = 0;                                                                      \
        buffer->head = 0;                                                                   \
        buffer->tail = 0;                                                                   \
        buffer->m = size;                                                                   \
        return buffer;                                                                      \
    }                                                                                       \
    static inline name *name##_new(void) {                                                  \
        return name##_new_size(DEFAULT_BUFFER_SIZE);                                        \
    }                                                                                       \
    static inline void name##_clear(name *array) {                                          \
        array->m = 0;                                                                       \
        array->n = 0;                                                                       \
        array->head = 0;                                                                    \
        array->tail = 0;                                                                    \
    }


#define __CIRCULAR_BUFFER_DESTROY(name, type)                           \
    static inline void name##_destroy(name *buffer) {                   \
        if (buffer == NULL) return;                                     \
        if (buffer->a != NULL) free(buffer->a);                         \
        free(buffer);                                                   \
    }

#define __CIRCULAR_BUFFER_DESTROY_FREE_DATA(name, type, free_func)      \
    static inline void name##_destroy(name *buffer) {                   \
        if (buffer == NULL) return;                                     \
        if (buffer->a != NULL) {                                        \
            for (size_t i = 0; i < buffer->m; i++) {                 \
                free_func(buffer->a[i]);                                \
            }                                                           \
        }                                                               \
        free(buffer->a);                                                \
        free(buffer);                                                   \
    }

#define CIRCULAR_BUFFER_INIT(name, type)                                                    \
    __CIRCULAR_BUFFER_BASE(name, type)                                                      \
    __CIRCULAR_BUFFER_DESTROY(name, type)                                                   \
    static inline bool name##_resize(name *buffer, size_t new_size) {                       \
        size_t prev_size = buffer->m;                                                    \
        if (new_size <= prev_size) return true;                                             \
        type *ptr = realloc(buffer->a, sizeof(type) * new_size);                            \
        if (ptr == NULL) return false;                                                      \
        buffer->a = ptr;                                                                    \
        buffer->m = new_size;                                                               \
        if (buffer->tail <= buffer->head) {                                                 \
            ROTATE_BUFFER(buffer, type, buffer->head, buffer->tail, prev_size);             \
            buffer->head = 0;                                                               \
            buffer->tail = prev_size;                                                       \
        }                                                                                   \
        return true;                                                                        \
    }                                                                                       \
    static inline bool name##_copy(name *dst, name *src, size_t n) {                        \
        bool ret = true;                                                                    \
        if (dst->m < n) ret = name##_resize(dst, n);                                        \
        if (!ret) return false;                                                             \
        memcpy(dst->a, src->a, n * sizeof(type));                                           \
        dst->m = n;                                                                         \
        dst->n = n;                                                                         \
        return ret;                                                                         \
    }                                                                                       \
    static inline name *name##_new_copy(name *buffer, size_t n) {                           \
        name *cpy = name##_new_size(n);                                                     \
        if (!name##_copy(cpy, buffer, n)) return NULL;                                      \
        return cpy;                                                                         \
    }                                                                                       \
    static inline bool name##_resize_if_full(name *buffer) {                                \
        if (buffer->n < buffer->m) return true;                                             \
        size_t current_size = buffer->m;                                                    \
        size_t new_size = current_size ? current_size * 3 / 2 : DEFAULT_BUFFER_SIZE;        \
        return name##_resize(buffer, new_size);                                             \
    }                                                                                       \
    static inline bool name##_push(name *buffer, type value) {                              \
        if (!name##_resize_if_full(buffer)) return false;                                   \
        size_t tail = buffer->tail;                                                         \
        buffer->a[tail] = value;                                                            \
        buffer->n++;                                                                        \
        buffer->tail = (tail + 1) % buffer->m;                                              \
        return true;                                                                        \
    }                                                                                       \
    static inline bool name##_push_left(name *buffer, type value) {                         \
        if (!name##_resize_if_full(buffer)) return false;                                   \
        if (buffer->head == 0) {                                                            \
            buffer->head = buffer->m - 1;                                                \
        } else {                                                                            \
            buffer->head--;                                                                 \
        }                                                                                   \
        buffer->a[buffer->head] = value;                                                    \
        buffer->n++;                                                                        \
        return true;                                                                        \
    }                                                                                       \
    static inline bool name##_pop(name *buffer, type *result) {                             \
        if (!buffer) return false;                                      \
        if (buffer->n == 0) return false;                               \
        *result = buffer->a[buffer->tail];                              \
        if (buffer->tail == 0) {                                        \
            buffer->tail = buffer->m - 1;                               \
        } else {                                                        \
            buffer->tail--;                                             \
        }                                                               \
        buffer->n--;                                                    \
        return true;                                                    \
    }                                                                   \
    static inline bool name##_pop_left(name *buffer, type *result) {    \
        if (!buffer) return false;                                      \
        if (buffer->n == 0) return false;                               \
        *result = buffer->a[buffer->head];                              \
        buffer->head = (buffer->head + 1) % buffer->m;                  \
        buffer->n--;                                                    \
        return true;                                                    \
    }                                                                   \
    static inline bool name##_extend(name *array, type *other, size_t size) {  \
        bool ret = false;                                               \
        size_t new_size = array->m + size;                              \
        if (new_size > array->m) ret = name##_resize(array, new_size);  \
        if (!ret) return false;                                         \
        for (size_t i = 0; i < size; i++) {                             \
            name##_push(array, other[i]);                               \
        }                                                               \
        return true;                                                    \
    }                                                                   \
    static inline bool name##_extend_left(name *array, type *other, size_t size) {  \
        bool ret = false;                                               \
        size_t new_size = array->m + size;                              \
        if (new_size > array->m) ret = name##_resize(array, new_size);  \
        if (!ret) return false;                                         \
        for (size_t i = 0; i < size; i++) {                             \
            name##_push_left(array, other[i]);                          \
        }                                                               \
        return true;                                                    \
    }

#define CIRCULAR_BUFFER_INIT_FIXED(name, type)                                              \
    __CIRCULAR_BUFFER_BASE(name, type)                                                      \
    __CIRCULAR_BUFFER_DESTROY(name, type)                                                   \
    static inline bool name##_copy(name *dst, name *src, size_t n) {                        \
        bool ret = true;                                                                    \
        if (dst->m < n) return false;                                                       \
        if (!ret) return false;                                                             \
        memcpy(dst->a, src->a, n * sizeof(type));                                           \
        dst->m = n;                                                                         \
        dst->n = n;                                                                         \
        return ret;                                                                         \
    }                                                                                       \
    static inline name *name##_new_copy(name *buffer, size_t n) {                           \
        name *cpy = name##_new_size(n);                                                     \
        if (!name##_copy(cpy, buffer, n)) return NULL;                                      \
        return cpy;                                                                         \
    }                                                                                       \
    static inline bool name##_push(name *buffer, type value) {                              \
        size_t tail = buffer->tail;                                                         \
        buffer->a[tail] = value;                                                            \
        buffer->tail = (tail + 1) % buffer->m;                                              \
        if (buffer->n < buffer->m) {                                                        \
            buffer->n++;                                                                    \
        } else {                                                                            \
            buffer->head = buffer->tail;                                                    \
        }                                                                                   \
        return true;                                                                        \
    }                                                                                       \
    static inline bool name##_push_left(name *buffer, type value) {                         \
        size_t head = buffer->head;                                                         \
        if (head == 0) {                                                                    \
            buffer->head = buffer->m - 1;                                                   \
        } else {                                                                            \
            buffer->head--;                                                                 \
        }                                                                                   \
        buffer->a[buffer->head] = value;                                                    \
        if (buffer->n < buffer->m) {                                                        \
            buffer->n++;                                                                    \
        } else {                                                                            \
            buffer->tail = buffer->head;                                                    \
        }                                                                                   \
        return true;                                                                        \
    }                                                                                       \
    static inline bool name##_pop(name *buffer, type *result) {                             \
        if (!buffer) return false;                                      \
        if (buffer->n == 0) return false;                               \
        *result = buffer->a[buffer->tail];                              \
        if (buffer->tail == 0) {                                        \
            buffer->tail = buffer->m - 1;                               \
        } else {                                                        \
            buffer->tail--;                                             \
        }                                                               \
        buffer->n--;                                                    \
        return true;                                                    \
    }                                                                   \
    static inline bool name##_pop_left(name *buffer, type *result) {    \
        if (!buffer) return false;                                      \
        if (buffer->n == 0) return false;                               \
        *result = buffer->a[buffer->head];                              \
        buffer->head = (buffer->head + 1) % buffer->m;                  \
        buffer->n--;                                                    \
        return true;                                                    \
    }


#endif
