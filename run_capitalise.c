#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MESSAGE "Hello, World!"
#define TEST_CHARS  1000000000
#define SHOW_CHARS  100

/**
 * Capitalise a null terminated string using branch free operations.
 * implemented in capitalise.asm
 * Should be approximately the same as the following:
 *  
 * void capitalise(char *s) {
 *     for(int i=0; s[i] != 0; ++i) {
 *         s[i] -= ('A' - 'a') * (s[i] >= 'a' && s[i] <= 'z');
 *     }
 * }
 *
 * The compiler can actually optimise this slightly better than I managed by 
 * hand (shocked pikachu), but it fails to find a good optimisation if an `if`
 * clause was used int the above. 
 */
void capitalise(char *s);


/**
 * Demonstrate the use of the capitalise(char *) function then test its 
 * performacne.
 */
int main(int argc, char **argv) {
    char *hello = malloc(strlen(MESSAGE));
    strncpy(hello, MESSAGE, strlen(MESSAGE));

    printf("Before:\n%s\n", hello);

    capitalise(hello);

    printf("After:\n%s\n", hello);

    free(hello);

    hello = malloc(TEST_CHARS);

    printf("Doing performance test.\n");
    printf("Filling array of %d chars\n", TEST_CHARS);
    clock_t start = clock();
    for(int i=0; i<TEST_CHARS-1; ++i){
        hello[i] = ' ' + rand() % ('~' - ' ');
    }
    hello[TEST_CHARS-1]=0;
    clock_t end = clock();
    printf("Took %lf seconds\n", (double) (end - start)/CLOCKS_PER_SEC);

    printf("Running capitalise\n");
    start = clock();
    capitalise(hello);
    end = clock();
    printf("Took %lf seconds\n", (double) (end - start)/CLOCKS_PER_SEC);

    char short_str[SHOW_CHARS + 1];
    strncpy(short_str, hello, SHOW_CHARS);
    printf("First %d chars are:\n%s\n", SHOW_CHARS, short_str); 
    free(hello);
}
