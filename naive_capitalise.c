#include <stddef.h>


void naive_capitalise(char *s, char *t, size_t l) {
    for(size_t i=0; i < l; ++i) {
        if(s[i] >= 'a' && s[i] < 'z') {
            t[i] = s[i] + ('A' - 'a');
        } else {
            t[i] = s[i];
        }
    }
}
