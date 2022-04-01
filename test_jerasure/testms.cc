#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C"
{
#include "./Jerasure-1.2A/cauchy.h"
#include "./Jerasure-1.2A/reed_sol.h"
#include "./Jerasure-1.2A/jerasure.h"
#include "./Jerasure-1.2A/galois.h"
}

using namespace std;

//optimal lrc add
static size_t determine_block_size_rs(const size_t value_length, uint32_t k)
{
    size_t block_size = 0;
    size_t temp_value_length = 0;
    int size_of_long = sizeof(long);
    int mod = value_length % (k * size_of_long);
    if (mod == 0)
    {
        block_size = value_length / k;
    }
    else
    {
        temp_value_length = value_length + k * size_of_long - mod;
        block_size = temp_value_length / k;
    }
    return block_size;
}
//optimal lrc add
static void fill_data_ptrs(char **data_ptrs, uint32_t size, size_t block_size, const char *value, const size_t value_length)
{
    size_t bytes_remained = value_length;
    for (uint32_t i = 0; i < size; ++i)
    {
        if (block_size <= bytes_remained)
        {
            memcpy(data_ptrs[i], value + i * block_size, block_size);
            bytes_remained -= block_size;
        }
        else
        {
            memcpy(data_ptrs[i], value + i * block_size, bytes_remained);
            bytes_remained -= bytes_remained;
        }
    }
}
//optimal lrc add
static void calculate_parity_ptrs_rs(char **data_ptrs, uint32_t k, char **coding_ptrs, uint32_t m, size_t block_size)
{
    int *matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
    jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, block_size);
    free(matrix);
}

int main(int argc, char **argv)
{

    if (argc != 7)
    {
        cout << "./testms n? k r b rank__number !" << endl;
        cout << "n" << endl;
        cout << "k" << endl;
        cout << "r" << endl;
        cout << "b" << endl;
        cout << "rank_number" << endl;
        //cout<<"./test_libmem replication number_of_replicas value_size en p0 !"<<endl;
        exit(1);
    }

    uint32_t n = atoi(argv[1]);
    uint32_t k = atoi(argv[2]);
    uint32_t r = atoi(argv[3]);
    uint32_t b = atoi(argv[4]);
    uint32_t rank_number = atoi(argv[5]);
    uint32_t value_size = atoi(argv[6]) * 1024;
    uint32_t g = n - (uint32_t)((n + r) / (r + 1)) - k;
    uint32_t local_group_number = (uint32_t)((n + r) / (r + 1));
    uint32_t rack_number = 11;

    string key = "lrc_big_obj_k" + to_string(value_size);
    string input_file = "./input_item_" + to_string(value_size) + "K_SRS";
    string output_file = "./output_item_" + to_string(value_size) + "K_SRS";

    size_t value_length = value_size;

    size_t block_size = determine_block_size_rs(value_length, k);

    // fill data blocks
    char **data_ptrs = new char *[k];
    for (uint32_t i = 0; i < k; ++i)
    {
        data_ptrs[i] = new char[block_size];
        memset(data_ptrs[i], 0, block_size);
    }
    char *value = new char[value_size];
    FILE *fp = fopen(input_file.c_str(), "r");
    fread(value, 1, value_size, fp);
    fclose(fp);
    fill_data_ptrs(data_ptrs, k, block_size, value, value_length);
}