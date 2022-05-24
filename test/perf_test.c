#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "capitalise.h"


#define TEST_CHARS  1000000000


void time_it(void (fn(char *, size_t)), char *data, size_t data_size) {
    char *data_copy = malloc(data_size);
    memcpy(data_copy, data, data_size + 1);
    clock_t start = clock();
    fn(data_copy, data_size);
    clock_t end = clock();
    printf("Took %lf seconds\n", (double) (end - start)/CLOCKS_PER_SEC);
    free(data_copy);
}

/**
 * Demonstrate the use of the capitalise(char *) function then test its 
 * performacne.
 */
int main(int argc, char **argv) {
    char *source = malloc(TEST_CHARS);

    printf("Doing performance test.\n");
    printf("Filling array of %d chars\n", TEST_CHARS);
    clock_t start = clock();
    for(int i=0; i<TEST_CHARS-1; ++i){
        source[i] = ' ' + rand() % ('~' - ' ');
    }
    source[TEST_CHARS-1] = 0;
    clock_t end = clock();
    printf("Took %lf seconds\n", (double) (end - start)/CLOCKS_PER_SEC);

    printf("\nRunning branching version.\n");
    time_it(&capitalise, source, TEST_CHARS);    
    
    printf("\nRunning non-branching version.\n");
    time_it(&branchless_capitalise, source, TEST_CHARS);    

    printf("\nRunning SIMD version.\n");
    time_it(&simd_capitalise, source, TEST_CHARS);    

    printf("\n");
    free(source);
}
