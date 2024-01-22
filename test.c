#include "greatest/greatest.h"

#define DEFAULT_BUFFER_SIZE 8

#define BUFFER_NAME test_buffer
#define BUFFER_TYPE int
#include "circular_buffer.h"
#undef BUFFER_NAME
#undef BUFFER_TYPE

#define BUFFER_NAME test_fixed_buffer
#define BUFFER_TYPE int
#define BUFFER_FIXED
#include "circular_buffer.h"
#undef BUFFER_NAME
#undef BUFFER_TYPE
#undef BUFFER_FIXED

TEST test_buffer_wraparound_and_resize(void) {
    test_buffer *buf = test_buffer_new();
    ASSERT_EQ(buf->m, DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buf->head, 0);
    ASSERT_EQ(buf->tail, 0);

    int i;

    for (i = 0; i < DEFAULT_BUFFER_SIZE; i++) {
        test_buffer_push(buf, i);
    }
    ASSERT_EQ(buf->m, DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buf->head, 0);
    ASSERT_EQ(buf->tail, 0);
    ASSERT_EQ(buf->n, buf->m);

    int elem;
    int elem2;
    ASSERT(test_buffer_pop_left(buf, &elem));
    ASSERT(test_buffer_pop_left(buf, &elem2));
    ASSERT_EQ(elem, 0);
    ASSERT_EQ(elem2, 1);

    int after_left_pop[] = {2, 3, 4, 5, 6, 7};

    int p = 0;
    int j = 0;
    circular_buffer_foreach(buf, j, elem, p, {
        ASSERT_EQ(elem, after_left_pop[j]);
    })

    i = DEFAULT_BUFFER_SIZE;

    ASSERT(test_buffer_push(buf, i++));
    ASSERT_EQ(buf->tail, 1);

    ASSERT(test_buffer_push(buf, i++));
    ASSERT_EQ(buf->n, buf->m);
    ASSERT_EQ(buf->tail, 2);

    int logical_after_wraparound[] = {2, 3, 4, 5, 6, 7, 8, 9};
    j = 0;
    circular_buffer_foreach(buf, j, elem, p, {
        ASSERT_EQ(elem, logical_after_wraparound[j]);
    })

    int physical_after_wraparound[] = {8, 9, 2, 3, 4, 5, 6, 7};

    for (p = 0; p < buf->m; p++) {
        ASSERT_EQ(buf->a[p], physical_after_wraparound[p]);
    }

    ASSERT_EQ(buf->head, 2);
    ASSERT_EQ(buf->tail, 2);

    size_t prev_size = buf->m;

    ASSERT(test_buffer_push(buf, i++));
    ASSERT_NEQ(buf->n, buf->m);
    ASSERT_EQ(buf->head, 0);
    ASSERT_EQ(buf->tail, prev_size + 1);

    int physical_after_resize[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (p = 0; p < buf->n; p++) {
        ASSERT_EQ(buf->a[p], physical_after_resize[p]);
    }

    for (int j = 0; j < 4; j++) {
        ASSERT(test_buffer_push(buf, i++));
    }

    int physical_after_no_wrap_resize[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    for (p = 0; p < buf->n; p++) {
        ASSERT_EQ(buf->a[p], physical_after_no_wrap_resize[p]);
    }

    PASS();
}


TEST test_fixed_buffer_wraparound_and_overwrite(void) {
    test_fixed_buffer *buf = test_fixed_buffer_new();
    ASSERT_EQ(buf->m, DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buf->head, 0);
    ASSERT_EQ(buf->tail, 0);

    int i;

    for (i = 0; i < DEFAULT_BUFFER_SIZE; i++) {
        test_fixed_buffer_push(buf, i);
    }
    ASSERT_EQ(buf->m, DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buf->head, 0);
    ASSERT_EQ(buf->tail, 0);
    ASSERT_EQ(buf->n, buf->m);

    int elem;
    int elem2;
    ASSERT(test_fixed_buffer_pop_left(buf, &elem));
    ASSERT(test_fixed_buffer_pop_left(buf, &elem2));
    ASSERT_EQ(elem, 0);
    ASSERT_EQ(elem2, 1);

    ASSERT_EQ(buf->m, DEFAULT_BUFFER_SIZE);
    ASSERT_EQ(buf->head, 2);
    ASSERT_EQ(buf->tail, 0);

    int after_left_pop[] = {2, 3, 4, 5, 6, 7};

    int p = 0;
    int j = 0;
    circular_buffer_foreach(buf, j, elem, p, {
        ASSERT_EQ(elem, after_left_pop[j]);
    })

    i = DEFAULT_BUFFER_SIZE;

    ASSERT(test_fixed_buffer_push(buf, i++));
    ASSERT_EQ(buf->tail, 1);

    ASSERT(test_fixed_buffer_push(buf, i++));
    ASSERT_EQ(buf->n, buf->m);
    ASSERT_EQ(buf->tail, 2);

    int logical_after_wraparound[] = {2, 3, 4, 5, 6, 7, 8, 9};
    j = 0;
    circular_buffer_foreach(buf, j, elem, p, {
        ASSERT_EQ(elem, logical_after_wraparound[j]);
    })

    int physical_after_wraparound[] = {8, 9, 2, 3, 4, 5, 6, 7};

    for (j = 0; j < buf->m; j++) {
        ASSERT_EQ(buf->a[j], physical_after_wraparound[j]);
    }

    ASSERT_EQ(buf->head, 2);
    ASSERT_EQ(buf->tail, 2);

    size_t prev_size = buf->m;

    ASSERT(test_fixed_buffer_push(buf, i++));
    ASSERT_EQ(buf->n, buf->m);
    ASSERT_EQ(buf->head, 3);
    ASSERT_EQ(buf->tail, 3);

    int logical_after_overwrite[] = {3, 4, 5, 6, 7, 8, 9, 10};
    j = 0;
    circular_buffer_foreach(buf, j, elem, p, {
        ASSERT_EQ(elem, logical_after_overwrite[j]);
    })

    int physical_after_overwrite[] = {8, 9, 10, 3, 4, 5, 6, 7};

    for (j = 0; j < buf->m; j++) {
        ASSERT_EQ(buf->a[j], physical_after_overwrite[j]);
    }


    PASS();
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_TEST(test_buffer_wraparound_and_resize);
    RUN_TEST(test_fixed_buffer_wraparound_and_overwrite);

    GREATEST_MAIN_END();        /* display results */
}
