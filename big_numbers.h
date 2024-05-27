#ifndef __BIG_NUMBERS_H__
#define __BIG_NUMBERS_H__

// Linked list of integers, representing "blocks" in positional notation.
typedef struct Block {
    uint32_t val;
    struct Block* next;
    struct Block* prev;
} Block;

// Bnum - "Big number". Data structure used to store large numbers.
typedef struct Bnum {
    int num_blocks;
    Block* least_significant;
    Block* most_significant;
} Bnum;


/* ---------- Library Functions ---------- */

// basic utilities
Bnum* Bnum_create(uint64_t);
void Bnum_destroy(Bnum*);
void Bnum_print(Bnum*);

// comparison operations
int Bnum_eq(Bnum*, Bnum*);
int Bnum_ne(Bnum*, Bnum*);
int Bnum_le(Bnum*, Bnum*);
int Bnum_lt(Bnum*, Bnum*);
int Bnum_ge(Bnum*, Bnum*);
int Bnum_gt(Bnum*, Bnum*);

// arith operations
Bnum* Bnum_sum(Bnum*, Bnum*);
Bnum* Bnum_mult(Bnum*, Bnum*);
Bnum* Bnum_pow(Bnum*, int);

#endif // __BIG_NUMBERS_H__
