#include <stdio.h>
#include "quine.h"

int main()
{
    int variables_num=0, minterms_num=0;
    FILE *in = NULL;
    qmtable_t *minterms_input = NULL;
    
    if((in = fopen("input.txt", "r")) == NULL)
    {
        fprintf(stderr, "\nCannot open file ###FIL01###\n");
        exit(EXIT_FAILURE);
    }
    //reading file variables
    fscanf(in, "%d", &variables_num);
    fscanf(in, "%d", &minterms_num);

    //reading and forming mintable 
    minterms_input = read_minterms(in, variables_num, minterms_num);

    //writing final formula
    printf("FORMULA FINALA = ");
    write_formula(minterms_input);
    printf("\n");

    //deleting table
    delete_qmtable(minterms_input);
    fclose(in);

    return 0;
}
