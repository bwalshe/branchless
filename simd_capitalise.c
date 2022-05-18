#include <string.h>

#define CHUNK_SIZE 16 
typedef char chunk_t __attribute__ ((vector_size (CHUNK_SIZE)));

void simd_capitalise(char *source, char *target, size_t len) {
    chunk_t chunk, A, a, z, dif;
    memset(&A, 'A', CHUNK_SIZE);
    memset(&a, 'a', CHUNK_SIZE);
    memset(&z, 'z', CHUNK_SIZE);
    dif = A - a;
    long num_chunks = len / CHUNK_SIZE;
    for(long i=0; i < num_chunks; ++i) {
        size_t offset = i * CHUNK_SIZE;
        memcpy(&chunk, source + offset, CHUNK_SIZE);

        chunk += dif * ((chunk >= a) * (chunk <= z));
        memcpy(target + offset, &chunk, CHUNK_SIZE);
    }
    for(size_t i = num_chunks * CHUNK_SIZE; i < len; ++i) {
        target[i] = source[i] 
            + ('A' - 'a') * (source[i] >= 'a' && source[i] <= 'z');
    }
}
