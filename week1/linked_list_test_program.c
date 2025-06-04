#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linked_list.h"

#define TEST(x)                       \
     printf("Running test " #x "\n"); \
     fflush(stdout);
#define SUBTEST(x)                             \
     printf("    Executing subtest " #x "\n"); \
     fflush(stdout);                           \
     alarm(1);
#define FAIL(cond, msg)         \
     if (cond)                  \
     {                          \
          printf("    FAIL! "); \
          printf(#msg "\n");    \
          exit(-1);             \
     }
#define PASS(x)         \
     printf("PASS!\n"); \
     alarm(0);

bool instrumented_malloc_fail_next = false;
bool instrumented_malloc_last_alloc_successful = false;

void gracefully_exit_on_suspected_infinite_loop(int signal_number)
{
     // Use write() to tell the tester that they're probably stuck
     // in an infinite loop.
     //
     // Why not printf()/fprintf()? It goes against POSIX rules for
     // signal handlers to call a non-reentrant function, of which both
     // of those are. I had no about that constraint prior to writing
     // this function. Cool!
     //
     const char *err_msg = "        Likely stuck in infinite loop! Exiting.\n";
     ssize_t retval = write(STDOUT_FILENO, err_msg, strlen(err_msg));
     fflush(stdout);

     // We really don't care about whether write() succeeded or failed
     // or whether a partial write occurred. Further, we only install
     // this function to one signal handler, so we can ignore that as well.
     //
     (void)retval;
     (void)signal_number;

     // Exit.
     //
     exit(1);
}

void *instrumented_malloc(size_t size)
{
     if (instrumented_malloc_fail_next)
     {
          instrumented_malloc_fail_next = false;
          instrumented_malloc_last_alloc_successful = false;
          return NULL;
     }

     void *ptr = malloc(size);
     instrumented_malloc_last_alloc_successful = (ptr != NULL);

     return ptr;
}

void check_empty_list_properties(void)
{
     TEST(check_empty_list_properties)
     SUBTEST(linked_list_create)
     struct linked_list *ll = linked_list_create();

     // Sanity check that linked_list_create() works on memory allocation
     // success.
     //
     FAIL((instrumented_malloc_last_alloc_successful && (ll == NULL)),
          "linked_list_create() failed when malloc() returned a valid pointer")

     // Check invariant that head is null when empty.
     //
     FAIL((ll->head != NULL),
          "ll->head is non-null in empty linked_list");

     linked_list_delete(ll);

     // Force the memory allocator fail, ensure that NULL is returned.
     //
     SUBTEST(linked_list_memory_alloc_fail)
     instrumented_malloc_fail_next = true;
     ll = linked_list_create();
     FAIL(ll != NULL,
          "linked_list_create() returns non-null pointer on allocation failure")

     // Attempt to create an iterator for index 0.
     //
     SUBTEST(empty_linked_list_iterator)
     ll = linked_list_create();
     struct iterator *iter = linked_list_create_iterator(ll, 0);
     FAIL(iter != NULL,
          "linked_list_create_iterator returned an iterator for an empty linked_list")

     // Cleanup.
     //
     linked_list_delete_iterator(iter);
     linked_list_delete(ll);
     PASS(check_empty_list_properties)
}

void check_insertion_functionality(void)
{
     TEST(check_insertion_functionality)
     SUBTEST(insert_end)
     // Check insertion at end with an iterator.
     // Inserts 1, 2, 3, 4 into the list, verifies
     // data.
     //
     struct linked_list *ll = linked_list_create();
     size_t ll_size = SIZE_MAX;
     FAIL(ll == NULL,
          "Failed to create new linked_list (#1)")
     for (size_t i = 1; i <= 4; i++)
     {
          bool status = linked_list_insert_end(ll, i);
          FAIL(status == false,
               "Failed to insert node into linked_list #1")
     }

     struct iterator *iter = linked_list_create_iterator(ll, 0);
     FAIL(iter == NULL,
          "Failed to create new iterator for linked_list (#1)")

     SUBTEST(iterate_over_linked_list_1)
     for (size_t i = 1; i <= 4; i++)
     {
          FAIL(iter->data != i,
               "Iterator does not contain correct data for linked_list (#1)")
          FAIL(iter->current_index != (i - 1),
               "Iterator does not contain correct index for linked_list (#1)")

          // Next element.
          //
          linked_list_iterate(iter);
     }
     linked_list_delete(ll);
     linked_list_delete_iterator(iter);

     // Check insertion at front with an iterator.
     // Inserts 4, 3, 2, 1 into the list, verifies data.
     //
     SUBTEST(insert_front)
     ll = linked_list_create();
     ll_size = linked_list_size(ll);
     FAIL(ll_size != 0,
          "linked_list (#2) size is non-zero when created")
     FAIL(ll == NULL,
          "Failed to create new linked_list (#2)")
     for (size_t i = 4; i != 0; i--)
     {
          bool status = linked_list_insert_front(ll, i);
          FAIL(status == false,
               "Failed to insert node into linked_list #2")
     }
     ll_size = linked_list_size(ll);
     FAIL(ll_size != 4,
          "linked_list (#2) size was not equal to 4")

     SUBTEST(iterate_over_linked_list_2)
     iter = linked_list_create_iterator(ll, 0);
     for (size_t i = 1; i <= 4; i++)
     {
          FAIL(iter->data != i,
               "Iterator does not contain correct data for linked_list (#2)")
          FAIL(iter->current_index != (i - 1),
               "Iterator does not contain correct index for linked_list (#2)")

          // Next element.
          //
          linked_list_iterate(iter);
     }
     ll_size = linked_list_size(ll);
     FAIL(ll_size != 4,
          "linked_list (#2) size was not equal to 4")
     linked_list_delete(ll);
     linked_list_delete_iterator(iter);

     PASS(check_insertion_functionality)
}

int main(void)
{
     // Set up signal handler for catching infinite loops.
     //
     signal(SIGALRM, gracefully_exit_on_suspected_infinite_loop);

     // Setup instrumented memory allocation/deallocation.
     //
     linked_list_register_malloc(&instrumented_malloc);
     linked_list_register_free(&free);

     check_empty_list_properties();
     check_insertion_functionality();

     return 0;
}
