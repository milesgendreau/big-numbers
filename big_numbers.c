/*
 * File: big_numbers.c
 * Author: Miles Gendreau
 * 
 * A library for doing arithmetic with arbitrarily large integers (positive integers
 * only).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "big_numbers.h"

#define BLOCK_SIZE 32
#define BLOCK_MASK 4294967295 // 2^32 - 1

void add_block(Bnum*, uint32_t);


/* ---------- Library Functions ---------- */

/*
 * Create a Bnum, with `num` as its value. The allocated memory must be freed
 * by the caller, using `Bnum_destroy()`.
 *
 * Parameters:  num     The value to initialize Bnum with.
 *
 * Returns: A pointer to the newly created Bnum.
 */
Bnum* Bnum_create(uint64_t num) {
    Bnum* big_num = malloc(sizeof(Bnum));
    big_num->num_blocks = 0;
    big_num->least_significant = NULL;
    big_num->most_significant = NULL;

    if (num == 0) { return big_num; }

    for (; num > 0; num >>= BLOCK_SIZE) {
        add_block(big_num, (uint32_t) num & BLOCK_MASK);
    }

    return big_num;
}

/*
 * Destroy a Bnum and free all of its associated memory.
 *
 * Parameters:  big_num     The Bnum to destroy.
 */
void Bnum_destroy(Bnum* big_num) {
    Block* cur = big_num->least_significant;
    Block* prev;
    while (cur) {
        prev = cur;
        cur = cur->next;
        free(prev);
    }
    free(big_num);
}

/*
 * Print the value of a Bnum to stdout (binary format).
 * 
 * Parameters:  big_num     The Bnum to print.
 */
void Bnum_print(Bnum* big_num) {
    Block* cur = big_num->most_significant;
    while (cur) {
        printf("%032b", cur->val);
        cur = cur->prev;
    }
    printf(" (blocks: %d)", big_num->num_blocks);
    printf("\n");
}

/*
 * Determine if `a` and `b` are equal. Analogous to `a == b`.
 *
 * Parameters:  a   Left hand side of the comparison.
 *              b   Right hand side of the comparison.
 *
 * Returns: 1 if the values of `a` and `b` are equal,
 *          0 otherwise.
 */
int Bnum_eq(Bnum* a, Bnum* b) {
    Block* cur_block_a = a->least_significant;
    Block* cur_block_b = b->least_significant;

    while (cur_block_a && cur_block_b) {
        if (cur_block_a->val != cur_block_b->val) { return 0; }
        cur_block_a = cur_block_a->next;
        cur_block_b = cur_block_b->next;
    }

    if (cur_block_a || cur_block_b) { return 0; }

    return 1;
}

/*
 * Determine if `a` and `b` are not equal. Analogous to `a != b`.
 *
 * Parameters:  a   Left hand side of the comparison.
 *              b   Right hand side of the comparison.
 *
 * Returns: 1 if the values of `a` and `b` are not equal,
 *          0 otherwise.
 */
int Bnum_ne(Bnum* a, Bnum* b) {
    return !Bnum_eq(a, b);
}

/*
 * Determine if `a` is less than or equal to `b`. Analogous to `a <= b`.
 *
 * Parameters:  a   Left hand side of the comparison.
 *              b   Right hand side of the comparison.
 *
 * Returns: 1 if the value of `a` is less than or equal to that of `b`,
 *          0 otherwise.
 */
int Bnum_le(Bnum* a, Bnum* b) {
    if (a->num_blocks != b->num_blocks) { return a->num_blocks < b->num_blocks; }

    Block* cur_block_a = a->most_significant;
    Block* cur_block_b = b->most_significant;
    
    while (cur_block_a && cur_block_b) {
        if (cur_block_a->val != cur_block_b->val) {
            return cur_block_a->val < cur_block_b->val;
        }
        cur_block_a = cur_block_a->prev;
        cur_block_b = cur_block_b->prev;
    }

    return 1;
}

/*
 * Determine if `a` is less than `b`. Analogous to `a < b`.
 *
 * Parameters:  a   Left hand side of the comparison.
 *              b   Right hand side of the comparison.
 *
 * Returns: 1 if the value of `a` is less than that of `b`,
 *          0 otherwise.
 */
int Bnum_lt(Bnum* a, Bnum* b) {
    return !Bnum_ge(a, b);
}

/*
 * Determine if `a` is greater than or equal to `b`. Analogous to `a >= b`.
 *
 * Parameters:  a   Left hand side of the comparison.
 *              b   Right hand side of the comparison.
 *
 * Returns: 1 if the value of `a` is greater than or equal to that of `b`,
 *          0 otherwise.
 */
int Bnum_ge(Bnum* a, Bnum* b) {
    if (a->num_blocks != b->num_blocks) { return a->num_blocks > b->num_blocks; }

    Block* cur_block_a = a->most_significant;
    Block* cur_block_b = b->most_significant;
    
    while (cur_block_a && cur_block_b) {
        if (cur_block_a->val != cur_block_b->val) {
            return cur_block_a->val > cur_block_b->val;
        }
        cur_block_a = cur_block_a->prev;
        cur_block_b = cur_block_b->prev;
    }

    return 1;
}

/*
 * Determine if `a` is greater than `b`. Analogous to `a > b`.
 *
 * Parameters:  a   Left hand side of the comparison.
 *              b   Right hand side of the comparison.
 *
 * Returns: 1 if the value of `a` is greater than that of `b`,
 *          0 otherwise.
 */
int Bnum_gt(Bnum* a, Bnum* b) {
    return !Bnum_le(a, b);
}

/*
 * Compute the sum of `a` and `b` and return it inside of a new Bnum. This Bnum
 * should be destroyed by the caller.
 *
 * Parameters:  a   Left hand side of the expression.
 *              b   Right hand side of the expression.
 *
 * Returns: A pointer to a new Bnum with value equal to the sum of `a` and `b`.
 */
Bnum* Bnum_sum(Bnum* a, Bnum* b) {
    Bnum* sum = Bnum_create(0);

    Block* cur_block_a = a->least_significant;
    Block* cur_block_b = b->least_significant;
    uint64_t block_sum;
    uint64_t carry = 0;

    while (cur_block_a && cur_block_b) {
        block_sum = ((uint64_t) cur_block_a->val) +
            ((uint64_t) cur_block_b->val) + carry;
        carry = block_sum >> BLOCK_SIZE;
        add_block(sum, (uint32_t) block_sum & BLOCK_MASK);

        cur_block_a = cur_block_a->next;
        cur_block_b = cur_block_b->next;
    }

    Block* cur_block = cur_block_a ? cur_block_a : cur_block_b;
    while (cur_block) {
        block_sum = ((uint64_t) cur_block->val) + carry;
        carry = block_sum >> BLOCK_SIZE;
        add_block(sum, (uint32_t) block_sum & BLOCK_MASK);

        cur_block = cur_block->next;
    }

    return sum;
}

/*
 * Compute the product of `a` and `b` and return it inside of a new Bnum. This Bnum
 * should be destroyed by the caller.
 *
 * Parameters:  a   Left hand side of the expression.
 *              b   Right hand side of the expression.
 *
 * Returns: A pointer to a new Bnum with value equal to the product of `a` and `b`.
 */
Bnum* Bnum_mult(Bnum* a, Bnum* b) {
    Bnum* product = Bnum_create(0);
    Bnum* cur_product;

    Block* cur_block_a = a->least_significant;
    Block* cur_block_b;
    uint64_t block_product;
    uint64_t carry = 0;
    int shift = 0;

    while (cur_block_a) {
        cur_product = Bnum_create(0);
        for (int i = 0; i < shift; i++) { add_block(cur_product, (uint32_t) 0); }

        // multiply by each block of [b]
        cur_block_b = b->least_significant;
        carry = 0;
        while (cur_block_b) {
            block_product = ((uint64_t) cur_block_a->val) *
                ((uint64_t) cur_block_b->val) + carry;
            carry = block_product >> BLOCK_SIZE;
            add_block(cur_product, (uint32_t) block_product & BLOCK_MASK);

            cur_block_b = cur_block_b->next;
        }
        if (carry) { add_block(cur_product, (uint32_t) carry); }

        // add result to [product]
        Bnum* temp = product;
        product = Bnum_sum(temp, cur_product);
        Bnum_destroy(temp);

        Bnum_destroy(cur_product);
        cur_block_a = cur_block_a->next;
        shift++;
    }

    return product;
}

/*
 * Compute the value of `a` to the power of `n` and return it inside of a new Bnum.
 * Caller should use `Bnum_destroy()` to free this Bnum.
 *
 * Parameters:  a   Left hand side of the expression.
 *              b   Right hand side of the expression.
 *
 * Returns: A pointer to a new Bnum with value equal to `a` to the power of `n`.
 */
Bnum* Bnum_pow(Bnum* a, int n) {
    Bnum* result = Bnum_create(1);
    
    for (int i = 0; i < n; i++) {
        Bnum* temp = result;
        result = Bnum_mult(temp, a);
        Bnum_destroy(temp);
    }

    return result;
}


/* ---------- Helper Functions ---------- */

/*
 * Add a new Block to a Bnum such that the newly added block is the most numerically
 * significant.
 *
 * Parameters:  big_num     The Bnum to add to.
 *              val         Value to be stored in the new Block.
 */
void add_block(Bnum* big_num, uint32_t val) {
    // create block
    Block* cur_block = malloc(sizeof(Block));
    cur_block->val = val;
    cur_block->next = NULL;
    cur_block->prev = NULL;

    // add block to list
    if (big_num->least_significant == NULL) {
        big_num->least_significant = cur_block;
        big_num->most_significant = cur_block;
    }
    else {
        big_num->most_significant->next = cur_block;
        cur_block->prev = big_num->most_significant;
        big_num->most_significant = cur_block;
    }

    big_num->num_blocks++;
}
