#include "greatest/greatest.h"
#include "circular_buffer.h"

CIRCULAR_BUFFER_INIT(test_buffer, int)


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

    int after_left_pop[] = {2, 3, 4, 5, 6, 7, 8, 9};

    int j = 0;
    circular_buffer_foreach(buf, elem, {
        ASSERT_EQ(elem, after_left_pop[j++]);
    })

    i = DEFAULT_BUFFER_SIZE;

    ASSERT(test_buffer_push(buf, i++));
    ASSERT_EQ(buf->tail, 1);

    ASSERT(test_buffer_push(buf, i++));
    ASSERT_EQ(buf->n, buf->m);
    ASSERT_EQ(buf->tail, 2);

    int logical_after_wraparound[] = {2, 3, 4, 5, 6, 7, 8, 9};
    j = 0;
    circular_buffer_foreach(buf, elem, {
        ASSERT_EQ(elem, logical_after_wraparound[j++]);
    })

    int physical_after_wraparound[] = {8, 9, 2, 3, 4, 5, 6, 7};

    for (j = 0; j < buf->m; j++) {
        ASSERT_EQ(buf->a[j], physical_after_wraparound[j]);
    }

    ASSERT_EQ(buf->head, 2);
    ASSERT_EQ(buf->tail, 2);

    size_t prev_size = buf->m;

    ASSERT(test_buffer_push(buf, i++));
    ASSERT_NEQ(buf->n, buf->m);
    ASSERT_EQ(buf->head, 0);
    ASSERT_EQ(buf->tail, prev_size + 1);

    int physical_after_resize[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (j = 0; j < buf->n; j++) {
        ASSERT_EQ(buf->a[j], physical_after_resize[j]);
    }

    for (int j = 0; j < 4; j++) {
        ASSERT(test_buffer_push(buf, i++));
    }

    int physical_after_no_wrap_resize[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    for (j = 0; j < buf->n; j++) {
        ASSERT_EQ(buf->a[j], physical_after_no_wrap_resize[j]);
    }

    PASS();
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_TEST(test_buffer_wraparound_and_resize);

    GREATEST_MAIN_END();        /* display results */
}
