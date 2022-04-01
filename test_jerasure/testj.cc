#include <bits/stdint-uintn.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

extern "C"
{
#include "./Jerasure-1.2A/cauchy.h"
#include "./Jerasure-1.2A/reed_sol.h"
#include "./Jerasure-1.2A/jerasure.h"
#include "./Jerasure-1.2A/galois.h"
}

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

//ERSadd
static uint32_t lcd(uint32_t a, uint32_t b)
{
    uint32_t largerOne = a >= b ? a : b;
    uint32_t smallerOne = a >= b ? b : a;
    while (smallerOne != 0)
    {
        uint32_t remain = largerOne % smallerOne;
        largerOne = smallerOne;
        smallerOne = remain;
    }
    return largerOne;
}

//ERSadd
static uint32_t lcm(uint32_t a, uint32_t b)
{
    return a * b / lcd(a, b);
}

static size_t determine_block_size_ers(const size_t value_length, uint32_t k, uint32_t s)
{
    uint32_t l = lcm(k, s);
    size_t block_size = 0;
    size_t temp_value_length = 0;
    int size_of_long = sizeof(long);
    int mod = value_length % (l * size_of_long);
    if (mod == 0)
    {
        block_size = value_length / l;
    }
    else
    {
        temp_value_length = value_length + l * size_of_long - mod;
        block_size = temp_value_length / l;
    }
    return block_size;
}

//ERSadd
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

//ERSadd
static void calculate_parity_ptrs_rs(char **data_ptrs, uint32_t k, char **coding_ptrs, uint32_t m, size_t block_size)
{
    int *matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
    jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, block_size);
    free(matrix);
}

int main()
{
}