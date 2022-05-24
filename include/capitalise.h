/**
 * Capitalise a null terminated string.
 * implemented in capitalise.asm
 * Should be approximately the same as the following:
 *  
 * void capitalise(char *s, char *t, size_t l) {
 *     for(int i=0; i < l; ++i) {
 *         if(s[i] >= 'a' && s[i] <= 'z')
 *             s[i] += ('A' - 'a');
 *     }
 * }
 */
void capitalise(char *s, size_t l);


/**
 * This version uses branchless operations. It is equivelent to
 * void brahcnless_capitalise(char *s, size_t l) {
 *     for(int i=0; i < l; ++i) {
 *         s[i] += ('A' - 'a') * (s[i] >= 'a' && s[i] <= 'z');
 *     }
 * }
 *
 * The compiler can actually optimise this slightly better than I managed by 
 * hand (shocked pikachu), but it fails to find a good optimisation if an `if`
 * clause was used in the the sample given for 
 * `capitalise(char *, size_t)`. 
 */
void branchless_capitalise(char *s, size_t l);


void simd_capitalise(char *s, size_t l);

