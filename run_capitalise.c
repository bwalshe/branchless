#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MESSAGE "Hello, World!"
#define TEST_CHARS  1000000015
#define SUMMARY_FMT "%s 100 chars are:\n%.100s\n"


/**
 * Capitalise a null terminated string using branch free operations.
 * implemented in capitalise.asm
 * Should be approximately the same as the following:
 *  
 * void capitalise(char *s, size_t l) {
 *     for(int i=0; i < l; ++i) {
 *         s[i] -= ('A' - 'a') * (s[i] >= 'a' && s[i] <= 'z');
 *     }
 * }
 *
 * The compiler can actually optimise this slightly better than I managed by 
 * hand (shocked pikachu), but it fails to find a good optimisation if an `if`
 * clause was used int the above. 
 */
void capitalise(char *s, char *t, size_t l);

void simd_capitalise(char *s, char *t, size_t l);

void naive_capitalise(char *s, char *t, size_t l);

void summarise(char *data, size_t length) {
    printf(SUMMARY_FMT, "First", data);
    printf(SUMMARY_FMT, "Last", data + length-100);
}

void time_it(void (fn(char *, char *, size_t)), char *data, size_t data_size) {
    char *target = calloc(data_size, sizeof(char*));
    clock_t start = clock();
    fn(data, target, data_size);
    clock_t end = clock();
    printf("Took %lf seconds\n", (double) (end - start)/CLOCKS_PER_SEC);
    summarise(target, data_size);
    free(target);
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
    summarise(source, TEST_CHARS);

    printf("\nTesting naive version.\n");
    time_it(&naive_capitalise, source, TEST_CHARS);    
    
    printf("\nTesting branchless version.\n");
    time_it(&capitalise, source, TEST_CHARS);    

    printf("\nTesting SIMD version.\n");
    time_it(&simd_capitalise, source, TEST_CHARS);    
    printf("\n");

    free(source);
}
