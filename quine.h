#ifndef _QUINE_M_H_
#define _QUINE_M_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef struct 
{
    int group;
    int *minterms;
    char *bit_representation;
    char check;
    int minterms_size;//size of minterms
}mintable_t;

typedef struct 
{
    mintable_t *mintable;
    int var_num;
    int table_size;//size of mintable
}qmtable_t;

qmtable_t* read_minterms(FILE *in, int variables_num, int minterms_num);
//this function only prints the final formula, which means that it cannot be represented in a string variable
//this can be modified, but the whole purpose is to compute one final formula without further modification of it
void write_formula(qmtable_t *input);

void delete_qmtable(qmtable_t *input);
void debug_table(qmtable_t *input);

#endif