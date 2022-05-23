#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include  "capitalise.h"


#define ALL_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALL_LOWER "abcdefghijklmnopqrstuvwxyz"


void test_empty(char * name, void test_fn(char *, char *, size_t)) { 
    printf("Testing %s with an empty string.\n", name);
    char src[1], target[1];
    src[0] = 0;
    target[0] = 0;
    test_fn(src, target, 0);
}

void test_single_char(char *name, void test_fn(char *,char*, size_t)) {
    printf("Testing %s with a string of length 1.\n", name);

    char src[2], target[2];
    src[1] = 0;

    src[0] = 'a';
    test_fn(src, target, 1);
    assert(strcmp(target, "A") ==  0);

    src[0] = 'A';
    test_fn(src, target, 1);
    assert(strcmp(target, "A") ==  0);

    src[0] = 'z';
    test_fn(src, target, 1);
    assert(strcmp(target, "Z") ==  0);

    src[0] = '@';
    test_fn(src, target, 1);
    assert(strcmp(target, "@") ==  0);

    src[0] = '{';
    test_fn(src, target, 1);
    assert(strcmp(target, "{") ==  0);


    src[0] = 'Z';
    test_fn(src, target, 1);
    assert(strcmp(target, "Z") ==  0);


}

void test_multi_char(char *name, void test_fn(char *,char*, size_t)) {
    printf("Testing %s with an multi char string.\n", name);
    
    char * target =  calloc(strlen(ALL_UPPER) +  1, sizeof(char));
    test_fn(ALL_LOWER, target, strlen(ALL_LOWER) + 1);
    assert(strcmp(target,  ALL_UPPER) == 0);
}

void test_capitalise(char *name, void test_fn(char *,  char *, size_t)) {
    test_empty(name, test_fn);
    test_single_char(name, test_fn);
    test_multi_char(name, test_fn);
}

void test_chunked_capitalise(char *name, 
        void test_fn(char *,  char *, size_t),
        int chunks,  int chunk_size, int extra) {
    int total_size = chunks * chunk_size  + extra;
    char  src[total_size + 1], target[total_size + 1], expected[total_size + 1];
    memset(src, 'a', total_size);
    memset(expected, 'A', total_size);
    src[total_size] = expected[total_size] = 0;
    printf("Testing %s with %d chunks of %d bytes + %d extra.\n", 
            name, chunks, chunk_size, extra);
    test_fn(src, target, total_size);
    assert(strcmp(target, expected) ==  0);
}

int main(int argc, char **argv) {
    test_capitalise("capitalise", capitalise);
    test_capitalise("branchless_capitalise", branchless_capitalise);
    test_capitalise("simd_capitalise",  simd_capitalise);
    test_chunked_capitalise("simd_capitalise", simd_capitalise, 3, 32, 5);
    printf("All tests passed\n");
    return 0;
}
