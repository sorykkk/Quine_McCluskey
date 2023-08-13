#include "quine.h"
//====================================UTILITY STRUCTURES====================================//
typedef struct 
{
    int freq;
    int i;//register row where freq first occured
}map_t;

//====================================UTILITY FUNCTIONS====================================//
//counting bits of an int
int count_set_bits(int x)
{
    int cnt = 0;
    for(int i = 0; i < sizeof(int)*8; i++)
        if(x & (1 << i))
            cnt++;

    return cnt;
}

//displaying bits for initialization
char *show_bits(int x, int n)
{
    char *s = NULL;

    if((s = (char*)calloc((n+1), sizeof(char))) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for bits string ###ALOC01###\n");
        exit(EXIT_FAILURE);
    }

    for(int i = n-1; i >=0 ; i--)
    {
        if((1 << i) & x)
            s[n-i-1] = '1';
        else s[n-i-1] = '0';
    }
    s[n] = 0;
    return s;
}

//check if two numbers are adiacent
int is_adiacent(char *x, char *y, int n_var)//returns position of adiacent bit
{
    int p = -1, flag = 0;
    if(x != NULL && y != NULL)
    {
        for(int i = 0; i < n_var; i++)
        {
            if(x[i]  != y[i])
            {
                if(!flag)
                {
                    flag = 1;
                    p = i;
                }
                else return -1;
            }
        }
    }
    return p;
}

//deep copy of the info from src to dst (dst should be NULL)
//it needs deep copy because of all pointers that should be alloc'ed
void deep_copy(qmtable_t **dst, qmtable_t *src)
{
    if(((*dst) = (qmtable_t*)calloc(1, sizeof(qmtable_t))) == NULL)
    {
        fprintf(stderr, "\n### MEMORY ERROR ALOC02 ###\n");
        exit(EXIT_FAILURE);
    }

    (*dst)->table_size = src->table_size;
    (*dst)->var_num = src->var_num;

    if(((*dst)->mintable = (mintable_t*)calloc(src->table_size, sizeof(mintable_t))) == NULL)
    {
        fprintf(stderr, "### MEMORY ERROR ALOC03 ###");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < src->table_size; i++)
    {
        if(((*dst)->mintable[i].minterms = (int*)calloc(src->mintable[i].minterms_size, sizeof(int))) == NULL)
        {
            fprintf(stderr, "### MEMORY ERROR ALOC04 ###");
            exit(EXIT_FAILURE);
        }
        if(((*dst)->mintable[i].bit_representation = (char*)calloc(src->var_num+1, sizeof(char))) == NULL)
        {
            fprintf(stderr, "### MEMORY ERROR ALOC05 ###");
            exit(EXIT_FAILURE);
        }

        memcpy((*dst)->mintable[i].minterms, src->mintable[i].minterms, src->mintable[i].minterms_size*sizeof(int));
        strcpy((*dst)->mintable[i].bit_representation, src->mintable[i].bit_representation);

        (*dst)->mintable[i].check = src->mintable[i].check;
        (*dst)->mintable[i].group = src->mintable[i].group;
        (*dst)->mintable[i].minterms_size = src->mintable[i].minterms_size;
    }
}

//comparator function first compare groups then it's members
int cmp_group(const void *a, const void *b)
{
    mintable_t *x = (mintable_t*)a, *y = (mintable_t*)b;

    if(x->group != y->group)
        return x->group - y->group;

    else 
        return x->minterms[0] - y->minterms[0];
}

//comparator function for elements of mintable's minterms
int cmp_minterms(const void *a, const void *b)
{
    return abs(*(int*)a) - abs(*(int*)b);
}

//====================================RECURSIVE ELIMINATION FUNCTIONS====================================//
//function that pushes to the table
void add_to_table(qmtable_t *table, int *minterms, int minterms_size, int group, char *bit_rep, int var_num)
{
    table->var_num = var_num;

    if((table->mintable = (mintable_t*)realloc(table->mintable, (table->table_size+1)*sizeof(mintable_t))) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for interation of mintable ###ALOC06###\n");
        exit(EXIT_FAILURE);
    }

    if((table->mintable[table->table_size].minterms = (int*)malloc(sizeof(int)*minterms_size)) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for interation of minterms ###ALOC07###\n");
        exit(EXIT_FAILURE);
    }

    if((table->mintable[table->table_size].bit_representation = (char*)malloc(sizeof(char)*(table->var_num+1))) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for interation of minterms bit rep ###ALOC08###\n");
        exit(EXIT_FAILURE);
    }


    table->mintable[table->table_size].check = 0;
    strcpy(table->mintable[table->table_size].bit_representation, bit_rep);
    table->mintable[table->table_size].group = group;
    table->mintable[table->table_size].minterms_size = minterms_size;

    //searching for duplicates
    //if yes then we decrease size to be reallocated next time with the same size
    //by gnu libc if the realloc for memory is the same size as memory then it returns the same address
    for(int i = 0; i < table->table_size; i++)
    {
        if(strcmp(table->mintable[i].bit_representation, table->mintable[table->table_size].bit_representation) == 0)
        {
            free(table->mintable[table->table_size].minterms);
            free(table->mintable[table->table_size].bit_representation);
            return;
        }
    }

    for(int i = 0; i < minterms_size; i++)
        table->mintable[table->table_size].minterms[i] = minterms[i];
    
    table->table_size++;
}

//instantiation and allocation routine for general table
void qmtable_init(qmtable_t *input_table, int *input_minterms, int n_minterms, int n_var)
{
    input_table->table_size = 0;
    char *var_bits = NULL;

    for(int i = 0; i < n_minterms; i++)
    {
        var_bits = show_bits(abs(input_minterms[i]), n_var);
        add_to_table(input_table, input_minterms, 1, count_set_bits(abs(input_minterms[i])), var_bits, n_var);
        input_table->mintable[i].minterms[0] = input_minterms[i];
        free(var_bits);
        var_bits = NULL;
    }
}

//function that validate if there is a match that can be added to table
int validate_add(qmtable_t *current_table, qmtable_t *next_table, int *copy_minterms, char *copy_bits)
{
    int is_changed = 0;
    for(int i = 0; i < current_table->table_size-1; i++)
    {
        for(int j = i+1; j < current_table->table_size; j++)
        {
            if((current_table->mintable[i].group != current_table->mintable[j].group))
            {
                int adj_pos = is_adiacent(current_table->mintable[i].bit_representation, current_table->mintable[j].bit_representation, current_table->var_num);
                if(adj_pos != -1 )
                {
                    is_changed = 1;
                    current_table->mintable[i].check = current_table->mintable[j].check = 1;
                    
                    int aux_counter = 0;
                    for(int k = 0; k < current_table->mintable[i].minterms_size; k++)
                    {
                        copy_minterms[aux_counter++] = current_table->mintable[i].minterms[k];
                        copy_minterms[aux_counter++] = current_table->mintable[j].minterms[k];
                    }

                    strcpy(copy_bits, current_table->mintable[i].bit_representation);
                    copy_bits[adj_pos] = '-';
                    add_to_table(next_table, copy_minterms, current_table->mintable[i].minterms_size*2, current_table->mintable[i].group, copy_bits, current_table->var_num);  
                }
            }
        }
    }

    return is_changed;
}

void check_leftovers(qmtable_t*final_table, qmtable_t *current_table,  int *copy_minterms, char *copy_bits)
{
    for(int i = 0; i < current_table->table_size; i++)
    {
        if(current_table->mintable[i].check == 0)
        {
            for(int j = 0; j < current_table->mintable[i].minterms_size; j++)
                copy_minterms[j] = current_table->mintable[i].minterms[j];

            strcpy(copy_bits, current_table->mintable[i].bit_representation);
            add_to_table(final_table, copy_minterms, current_table->mintable[i].minterms_size, current_table->mintable[i].group, copy_bits, current_table->var_num);
        }
    }
}

///TODO:  change without aux minterms array
qmtable_t *form_final_table(qmtable_t *input_table)
{
    qmtable_t *final_table = NULL, *next_table = NULL, *current_table = NULL;
    int *copy_minterms = NULL, copy_size = 1;
    char *copy_bits = NULL;
    char is_changed = 0;

    deep_copy(&current_table, input_table);
    if((final_table = (qmtable_t*)calloc(1, sizeof(qmtable_t))) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for final qm table ###ALOC09###\n");
        exit(EXIT_FAILURE);
    }
    if((copy_bits = (char*)calloc(input_table->var_num+1, sizeof(char))) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for auxiliar bit rep ###ALOC10###\n");
        exit(EXIT_FAILURE);
    }

    do
    {
        if((copy_minterms = (int*)malloc(2*copy_size*sizeof(int))) == NULL)
        {
            fprintf(stderr, "\nFail in memory allocation for final qm table ###ALOC11###\n");
            exit(EXIT_FAILURE);
        }
        if((next_table = (qmtable_t*)calloc(1, sizeof(qmtable_t))) == NULL)
        {
            fprintf(stderr, "\nFail in memory allocation for final qm table ###ALOC12###\n");
            exit(EXIT_FAILURE);
        }

        //check if there were added new elements to table
        is_changed = validate_add(current_table, next_table, copy_minterms, copy_bits);

        //treating leftovers
        check_leftovers(final_table, current_table, copy_minterms, copy_bits);

        copy_size*=2;

        #ifdef DEBUG
            debug_table(current_table);
        #endif

        free(copy_minterms);
        copy_minterms = NULL;
        delete_qmtable(current_table);
        current_table = NULL;
        
        deep_copy(&current_table, next_table);

        delete_qmtable(next_table);
        next_table = NULL;
    } while(is_changed);

    free(copy_bits);
    delete_qmtable(next_table);
    delete_qmtable(current_table);

    //sort minterms for a beter searching of certain elements
    for(int i = 0; i < final_table->table_size; i++)
        qsort(final_table->mintable[i].minterms, final_table->mintable[i].minterms_size, sizeof(int), cmp_minterms);

    #ifdef DEBUG
        printf("\nFINAL TABLE:");
        debug_table(final_table);
    #endif

    return final_table;
}

//read all variables from file
qmtable_t* read_minterms(FILE *in, int variables_num, int minterms_num)
{
    qmtable_t *input_table = NULL;
    if((input_table = (qmtable_t*)calloc(1, sizeof(qmtable_t))) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for qm table ###ALOC13###\n");
        exit(EXIT_FAILURE);
    }

    int *input_minterms = NULL;    //aux array to store all input minterms
    if((input_minterms = (int*)malloc(sizeof(int)*minterms_num)) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for minterm aux table ###ALOC14###\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < minterms_num; i++)
        fscanf(in, "%d", &input_minterms[i]);

    qmtable_init(input_table, input_minterms, minterms_num, variables_num);
    
    free(input_minterms);

    qsort(input_table->mintable, minterms_num, sizeof(mintable_t), cmp_group);

    return input_table;
}

//====================================FORMULA FORMATING FUNCTIONS====================================//
//creates a variable label pool, where we can take variable labels
char *generate_variable_labels(qmtable_t *input)
{
    char *minterm_labels = NULL;
    if((minterm_labels = (char*)malloc(sizeof(char)*input->var_num)) == NULL)
    {
        fprintf(stderr, "\nFail in allocation of bit_representation table ###ALOC15###\n");
        exit(EXIT_FAILURE);
    }
    //assignment of variables
    for(int i = 0; i < input->var_num; i++)
        minterm_labels[i] = (i <= 25)? 'a'+i: 'A'+i-26;

    return minterm_labels;
}

//matches the bit representation of minterm with labels
void print_literal(char *var_labels, const char *bit_representation, int var_num)
{
    //variables matching
    for(int i = 0; i < var_num; i++)
    {
        switch(bit_representation[i])
        {
            case '1': printf("%c",var_labels[i]);
                break;
            case '0': printf("%c\'", var_labels[i]);
                break;
        }
    }
}

void print_minterm(qmtable_t *input, int row, char *var_labels, int *first_printed_minterm)
{
    if(*first_printed_minterm)
        printf(" + ");
    else *first_printed_minterm = 1;
    
    print_literal(var_labels, input->mintable[row].bit_representation, input->var_num);
}

//====================================FORMULA EXTRACTION FUNCTIONS====================================//
//makes an frequency array that grows exponentially 2^(number of variables)
//register it's frequency and in which row it first occured
//can be modified to store only minterms that are involved in calculations
//but this approach is easier to index each minterms, but it has a huge space disadvantage
///TODO: make minterms_freq_arr to store only non-zero values, not all generated minterms
map_t *make_freq_array(qmtable_t *input)
{
    map_t *minterms_freq_arr = NULL;
    if((minterms_freq_arr = calloc(1 << input->var_num, sizeof(map_t))) == NULL)
    {
        fprintf(stderr, "\nFail in memory allocation for minterms frequency array ###ALOC16###\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < input->table_size; i++)
    {
        for(int j = 0; j < input->mintable[i].minterms_size; j++)
        {
            int minterm = input->mintable[i].minterms[j];

            if(minterm >=0 )
            {
                minterms_freq_arr[minterm].freq++;

                //writes first occurance of the minterm
                if(minterms_freq_arr[minterm].i == 0)
                    minterms_freq_arr[minterm].i = i;
            }
        }
    }

    return minterms_freq_arr;
}

int print_essentials(qmtable_t *final_table, map_t *minterms_freq_arr, int freq_arr_size, char *var_labels, int *first_printed_minterm)
{
    int is_essential = 0;
    for(int i = 0; i < freq_arr_size; i++)
    {
        if(minterms_freq_arr[i].freq == 1)
        {
            is_essential = 1;
            print_minterm(final_table, minterms_freq_arr[i].i, var_labels, first_printed_minterm);
            int fi = minterms_freq_arr[i].i;
            
            for(int j = 0; j < final_table->mintable[fi].minterms_size; j++)
                minterms_freq_arr[abs(final_table->mintable[fi].minterms[j])] = (map_t){0, 0};
        }
    }
    return is_essential;
}

//select position of certain minterm
int select_minterm(qmtable_t *input, int i, int minterm)
{
    for(int j = 0; j < input->mintable[i].minterms_size; j++)
        if(minterm == abs(input->mintable[i].minterms[j]))
            return j;

    return -1;
}

//prints leftover minterms to have the biggest cover
int print_leftovers(qmtable_t *final_table, map_t *minterms_freq_arr, int freq_arr_size, char *var_labels, int *first_printed_minterm)
{
    int max_row = 0, count = 0, max = 0, is_leftover = 0;
    
    for(int i = 0; i < freq_arr_size; i++)
    {
        if(minterms_freq_arr[i].freq != 0 && minterms_freq_arr[i].freq != 1)
        {
            max_row = 0;//stores the row that has maximum eliminations of minterms
            count = 0;
            max = 0;
            is_leftover = 1;
            //count row that has maximum minterms elimination
            for(int j = minterms_freq_arr[i].i; j < final_table->table_size; j++)
            {
                count = 0;
                for(int k = select_minterm(final_table, j, i); k != -1 && k < final_table->mintable[j].minterms_size; k++)
                    count += minterms_freq_arr[abs(final_table->mintable[j].minterms[k])].freq;

                if(max < count)
                {
                    max = count;
                    max_row = j;
                }
            }

            print_minterm(final_table, max_row, var_labels, first_printed_minterm);

            for(int j = 0; j < final_table->mintable[max_row].minterms_size; j++)
                minterms_freq_arr[abs(final_table->mintable[max_row].minterms[j])] = (map_t){0, 0};
        }
    }
    return is_leftover;
}

//writes final formula
void write_formula(qmtable_t *input)
{
    qmtable_t *final_table = form_final_table(input);
    map_t *minterms_freq_arr = make_freq_array(final_table);
    char *var_labels = generate_variable_labels(input);

    int freq_arr_size = 0, first_printed_minterm = 0;
    freq_arr_size = (1 << final_table->var_num);

    //first print essential minterms
    //second print leftovers
    //if there is no essential and leftovers minterms to print then the output formula is zero
    if(!(print_essentials(final_table, minterms_freq_arr, freq_arr_size, var_labels, &first_printed_minterm) || 
        print_leftovers(final_table, minterms_freq_arr, freq_arr_size, var_labels, &first_printed_minterm)))
            printf("0");

    //check if all minterms were eliminated
    for(int i = 0; i < freq_arr_size; i++)
    {
        if(minterms_freq_arr[i].freq != 0)
        {
            fprintf(stderr, "\nERROR in writing formula, there are minterms left ### LOG01 ###\n");
            break;
        }
    }

    free(minterms_freq_arr);
    free(var_labels);
    delete_qmtable(final_table);
}

//====================================DELETE FUNCTIONS====================================//
//delete minterms array
void delete_mintable(mintable_t mintable)
{
    free(mintable.bit_representation);
    free(mintable.minterms);
}

//delete the entire table
void delete_qmtable(qmtable_t *input)
{
    if(input == NULL)
        return;

    for(int i = 0; i < input->table_size; i++)
        delete_mintable(input->mintable[i]);
    
    if(input->mintable != NULL)
        free(input->mintable);
    free(input);
}

//====================================DEBUG FUNCTIONS====================================//
//function to print the table
void debug_table(qmtable_t *input)
{
    printf("\nDebugging minterms:\n");
    for(int i = 0; i < input->table_size; i++)
    {
        printf("m = ");   
        for(int j = 0; j < input->mintable[i].minterms_size; j++)
            printf("%d ", input->mintable[i].minterms[j]);
        printf("g = %d b = %s c= %d\n",input->mintable[i].group, input->mintable[i].bit_representation, input->mintable[i].check);
    }
    printf("\n");
}
