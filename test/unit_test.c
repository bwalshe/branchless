#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include  "capitalise.h"


#define ALL_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!"
#define ALL_LOWER "abcdefghijklmnopqrstuvwxyz0123456789!"


void test_empty(char * name, void test_fn(char *, size_t)) { 
    printf("Testing %s with an empty string.\n", name);
    char s[1];
    s[0] = 0;
    test_fn(s, 0);
}

void test_single_char(char *name, void test_fn(char *, size_t)) {
    printf("Testing %s with a string of length 1.\n", name);

    char s[2];
    s[1] = 0;

    s[0] = 'a';
    test_fn(s, 1);
    assert(strcmp(s, "A") ==  0);

    s[0] = 'A';
    test_fn(s, 1);
    assert(strcmp(s, "A") ==  0);

    s[0] = 'z';
    test_fn(s, 1);
    assert(strcmp(s, "Z") ==  0);

    s[0] = '@';
    test_fn(s, 1);
    assert(strcmp(s, "@") ==  0);

    s[0] = '{';
    test_fn(s, 1);
    assert(strcmp(s, "{") ==  0);


    s[0] = 'Z';
    test_fn(s, 1);
    assert(strcmp(s, "Z") ==  0);

}


void test_multi_char(char *name, void test_fn(char *, size_t)) {
    printf("Testing %s with an multi char string.\n", name);
    
    char * s =  malloc(strlen(ALL_UPPER) +  1);
    memcpy(s, ALL_LOWER, strlen(ALL_LOWER));
    test_fn(s, strlen(ALL_LOWER));
    assert(strcmp(s,  ALL_UPPER) == 0);
}


void test_capitalise(char *name, void test_fn(char *, size_t)) {
    test_empty(name, test_fn);
    test_single_char(name, test_fn);
    test_multi_char(name, test_fn);
}


void test_chunked_capitalise(char *name, 
        void test_fn(char *, size_t),
        int chunks,  int chunk_size, int extra) {
    int total_size = chunks * chunk_size  + extra;
    char  s[total_size+1], expected[total_size+1];
    memset(s, 'a', total_size);
    memset(expected, 'A', total_size);
    s[total_size] = expected[total_size] = '\0';
    printf("Testing %s with %d chunks of %d bytes + %d extra.\n", 
            name, chunks, chunk_size, extra);
    test_fn(s, total_size);
    assert(strcmp(s, expected) ==  0);
}

int main(int argc, char **argv) {
    test_capitalise("capitalise", capitalise);
    test_capitalise("branchless_capitalise", branchless_capitalise);
    test_capitalise("simd_capitalise",  simd_capitalise);
    test_chunked_capitalise("simd_capitalise", simd_capitalise, 3, 32, 5);
    test_chunked_capitalise("simd_capitalise", simd_capitalise, 3, 32, 0);
    test_chunked_capitalise("simd_capitalise", simd_capitalise, 0, 32, 5);
    printf("All tests passed\n");
    return 0;
}
