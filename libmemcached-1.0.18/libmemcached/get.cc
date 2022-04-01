/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011-2013 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <libmemcached/common.h>
extern "C"{
#include "../Jerasure-1.2A/cauchy.h"
#include "../Jerasure-1.2A/reed_sol.h"
#include "../Jerasure-1.2A/jerasure.h"
#include "../Jerasure-1.2A/galois.h"
}
#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

/* get function by the RS code */
/* optimal lrc*/
static void decoding_parity_ptrs_lrc(char **data_ptrs, uint32_t k, char **coding_ptrs, uint32_t m, size_t block_size,int *erasures) {
  int *matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  //jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, block_size);
  int i = 0;
  i = jerasure_matrix_decode(k, m, 8, matrix, 1, erasures, data_ptrs, coding_ptrs, block_size);
  if (i == -1) {
			printf("Unsuccessful!\n");
		}

  //free(matrix);
}
static size_t determine_block_size_rs(const size_t value_length, uint32_t k) {
  size_t block_size = 0;
  size_t temp_value_length = 0;
  int size_of_long = sizeof(long);
  int mod = value_length % (k*size_of_long);
  if(mod == 0) {
    block_size = value_length / k;
  } else {
    temp_value_length = value_length + k*size_of_long - mod;
    block_size = temp_value_length / k;
  }
  return block_size;
}

static void cross_networkcore_libmemcached(Memcached *ptr,const char *key, size_t key_length,
                                 const char *value, size_t value_length){
  size_t * bs = NULL;
  uint32_t *fl = NULL;
  memcached_return_t *error1 = NULL;
  ptr->is_core = true;

  //memcached_instance_st *network_core_instance = memcached_instance_fetch(ptr, ptr->network_core_server_key);
  //printf("cross_networkcore:nouse:libmemcached/get.cc===%d",int(ptr->network_core_server_key));
  memcached_set(ptr, key, key_length,value, value_length,86400,0);
                      
  char *result = memcached_get_by_key(ptr, NULL, 0, key, key_length, bs,
                              fl, error1);
  //printf("cross core  key%s \n value%s\n",key,result);
  ptr->is_core =false;
}

static int create_socket()
{
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket == -1)
	{
		perror("socket");
		return -1;
	}
	return client_socket;
}
static int start_listen(Memcached *ptr,int client_socket)
{
	memcached_instance_st* instance= memcached_instance_fetch(ptr, ptr->network_core_server_key);
  
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;  /* Internet地址族 */
  addr.sin_port = htons(8899/*instance->port()*/);  /* 端口号 */
  printf("start_listen:%s\n",instance->hostname());
  addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* IP地址 */
  
	inet_aton(instance->hostname(), &(addr.sin_addr));
	int addrlen = sizeof(addr);
	int listen_socket =  connect(client_socket,  (struct sockaddr *)&addr, addrlen);  //连接服务器
	if(listen_socket == -1)
	{
    printf("error_start_listen:%s\n",instance->hostname());
		perror("connect");
		return -1;
	}
	return listen_socket;

}
static void cross_networkcore(Memcached *ptr,const char *key, size_t key_length,
                                 const char *value, size_t value_length,int client_socket)
{
	char buf_key[key_length] = {0};	
  char buf_value[value_length] = {0};	
	write(client_socket, key, key_length);
		
	int ret = read(client_socket, buf_key, key_length);

  write(client_socket, value, value_length);
  ret = read(client_socket, buf_value, value_length);
		
	//printf("buf_key = %s\n", buf_key);
  //printf("value_length = %s\n", buf_value);
	//printf("\n");

}
static void close_connect(int client_socket,int listen_socket)
{
  /*
	char buf_end[4] = {0};
  buf_end[0] = 'e';
  buf_end[1] = 'n';
  buf_end[2] = 'd';
  buf_end[4] = '\0';
	write(client_socket, buf_end, 3);	
	int ret = read(client_socket, buf_end, 3);
  */
	close(listen_socket);
  /*
	printf("buf_end = %s\n", buf_end);
	printf("\n");
  */

}
void helpmemplease(){
  printf("help me!");
}
static char *memcached_get_lrc(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t *value_length,
                    uint32_t *flags,
                    memcached_return_t *error)
{
  uint32_t n = ptr->number_of_n;
  uint32_t k = ptr->number_of_k;
  //uint32_t b = ptr->number_of_b;
  uint32_t r = ptr->number_of_r;
  size_t temp_value_len = 0;
  char **sub_values = new char*[k];
  char **coding_ptrs = new char*[1];
  size_t block_size = determine_block_size_rs(*value_length,k);
  size_t last_data_request_size = *value_length - block_size * (k - 1);
  int *erasures;
  //int numerased = 0;
  erasures = (int *)malloc(sizeof(int)*(r+1));
  uint32_t fail_index = n;
  size_t node_fail[k] = {0};
  

  char **key_ptrs = new char*[k];

  size_t *key_len_ptrs = new size_t[k];
  int index_of_rack = 0;
  // generate new keys for each data block
  for(uint32_t i = 0; i < k; ++i) {
      index_of_rack = int(i/r);
      key_ptrs[i] = new char[key_length + 3];
      memset(key_ptrs[i], 0, key_length+3);
      memcpy(key_ptrs[i], key, key_length);
      int ten = (i + index_of_rack)/ 10;
      int one = (i + index_of_rack) - ten * 10;
      char ten_bit_char = '0' + ten;
      char one_bit_char = '0' + one;
      key_ptrs[i][key_length] = ten_bit_char;
      key_ptrs[i][key_length+1] = one_bit_char;
      key_ptrs[i][key_length+2] = '\0';
      key_len_ptrs[i] = key_length+2;
      //printf("index_key:%d key %s\n",i,key_ptrs[i]);
  }

  // multiple get the data blocks
  memcached_return_t unused;
  if(error == NULL) {
      error = &unused;
  }


  *error = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, k);
  //helpmemplease();
  if(*error == MEMCACHED_SUCCESS) {
      //fprintf(fp, "rs, multiple get success !\n");
      memcached_result_st *result_buffer = &ptr->result;
      char *temp_key = new char[key_length + 3];
      //numerased = 0;
      for(uint32_t count_data = 0;count_data<k;count_data++){
        if((result_buffer = memcached_fetch_result(ptr, result_buffer, error)) != NULL){
          
          strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
          temp_key[key_length + 2] = '\0';
          //printf("get.cc:line:153:\n lrc_key: %s\n", temp_key);
          uint32_t i;
          for(i = 0; i < k; ++i) {
            if(strcmp(temp_key, key_ptrs[i]) == 0) break;
          }
          //printf("index: %d\n", i);
          node_fail[i] = 1;
          if(i == 0) {
            block_size = memcached_result_length(result_buffer);
          }
          if(i == k-1) {
            last_data_request_size = memcached_result_length(result_buffer);
          }
          temp_value_len += memcached_result_length(result_buffer);

          sub_values[i] = new char[block_size];
          memset(sub_values[i],0,block_size);
          memcpy(sub_values[i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
          if(flags) {
            *flags= result_buffer->item_flags;
        }
          
        }
        
      }
      for(uint32_t count_data = 0;count_data<k;count_data++){
        if(node_fail[count_data] == 0){
          fail_index = count_data;
          erasures[0] = fail_index-r*int(fail_index/r);
          //numerased++;
        }
      }
      //int client_socket = create_socket();
      //int listen_socket = start_listen(ptr,client_socket);
      for(uint32_t count_data = 0;count_data<k;count_data++){
        if(count_data!=fail_index){
          cross_networkcore_libmemcached(ptr,key_ptrs[count_data],key_len_ptrs[count_data],
                                 sub_values[count_data], block_size);
          //printf("get cross networkcore\n");
        }
        
      }
      
      if(fail_index!=n){
        ptr->fail_node_index = fail_index + int(fail_index/r);
        uint32_t group_id = int(fail_index/r);
        uint32_t repair_number = r + 1;
        uint32_t last_block_index = k+uint32_t((k-1)/r)-1;
   /*     memcached_repair_by_index(ptr,key,
                    key_length,*value_length,flags,
                    error,fail_index,0);
   */ 
        if((group_id+1)*(r+1)>n){
          repair_number = n - group_id*(r+1);
        }
        
        sub_values[fail_index] = new char[block_size];//(char *)malloc(sizeof(char)*block_size);
        memset(sub_values[fail_index],0,block_size);
        char **data_ptrs = new char*[repair_number];
        uint32_t cross_rack_num = 0; 
        
        uint32_t first_rack = ptr->node_rack[group_id*(r+1)];
        for(uint32_t bi = group_id*(r+1);bi<group_id*(r+1)+repair_number-1;bi++){
          if(first_rack!=ptr->node_rack[bi]){
            cross_rack_num++;
            first_rack=ptr->node_rack[bi];
          }
          if(bi<=last_block_index){
            //memset(sub_values[bi],0,block_size);
            //memcpy(data_ptrs[i], value + i*block_size, block_size);
            data_ptrs[bi-group_id*(r+1)] = new char[block_size];
            memset(data_ptrs[bi-group_id*(r+1)], 0, block_size);
            data_ptrs[bi-group_id*(r+1)] = sub_values[bi-group_id*(r+1)+group_id*r];
          }else{
            printf("_____________To_fetch________\n");
            break;
          }
           
        }
        if(first_rack!=ptr->node_rack[group_id*(r+1)+repair_number-1]){
            cross_rack_num++;
          }
        uint32_t To_fetch = 1;
        if(k%r!=0 && uint32_t(fail_index/r)==uint32_t((k-1)/r)){
          To_fetch = repair_number-(k-group_id*r);
        }
        char **local_parity_key_ptrs = new char*[To_fetch]; 
        size_t *local_key_len_ptrs = new size_t[To_fetch];
        for(uint32_t bi = 0;bi<To_fetch;bi++){
           local_parity_key_ptrs[bi] = new char[key_length + 3];
            memset(local_parity_key_ptrs[bi], 0, key_length+3);
            memcpy(local_parity_key_ptrs[bi], key, key_length);
            int ten = (int(fail_index/r)*(r+1)+r+bi)/ 10;
            int one = (int(fail_index/r)*(r+1)+r+bi) - ten * 10;
            char ten_bit_char = '0' + ten;
            char one_bit_char = '0' + one;
            local_parity_key_ptrs[bi][key_length] = ten_bit_char;
            local_parity_key_ptrs[bi][key_length+1] = one_bit_char;
            local_parity_key_ptrs[bi][key_length+2] = '\0';
            local_key_len_ptrs[bi] = key_length+2;
            //printf("coding_index_key:%d key %s\n",int(fail_index/r)*(r+1)+r+bi,local_parity_key_ptrs[bi]);

        }
        *error = memcached_mget(ptr, (const char * const *)local_parity_key_ptrs, local_key_len_ptrs, To_fetch);
        //helpmemplease();
        //printf("To_fetch: %d\n", To_fetch);
        if(*error != MEMCACHED_SUCCESS){
          printf("fail!!!!!!!\n");
        }
        else{
          char *temp_key = new char[key_length + 3];
          for(uint32_t bi = 0;bi<To_fetch;bi++){
          if((result_buffer = memcached_fetch_result(ptr, result_buffer, error)) != NULL){  
                    strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
                    temp_key[key_length + 2] = '\0';
                    //printf("To_fetch_key: %s\n", temp_key);
                    //printf("To_fetch: %d\n", To_fetch);
                    uint32_t i;
                    for(i = 0; i < To_fetch; ++i) {
                      if(strcmp(temp_key, local_parity_key_ptrs[i]) == 0) break;
                      
                    }
                    
                    if(i == To_fetch-1){
                        //printf("coding_index: %d\n", i);
                        coding_ptrs[0] = new char[memcached_result_length(result_buffer)];
                        memset(coding_ptrs[0], 0, block_size);
                        memcpy(coding_ptrs[0], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
                    }else{
                        //printf("data_index: %d\n", i);
                        data_ptrs[repair_number-To_fetch+i] = new char[block_size];
                        memset(data_ptrs[repair_number-To_fetch+i], 0, block_size);
                        memcpy(data_ptrs[repair_number-To_fetch+i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
                    }
                    
                    if(flags) {
                      *flags= result_buffer->item_flags;
                      }
          }
          }
        }
        if(cross_rack_num!=0){
          for(uint32_t bi = 0;bi<cross_rack_num;bi++){
              cross_networkcore_libmemcached(ptr,local_parity_key_ptrs[0],local_key_len_ptrs[0],
                                 coding_ptrs[0], block_size);
              //printf("get repair cross networkcore\n");
          }
          
        }      
        

        //printf("erasures[0]:%d\n",erasures[0]);
        erasures[1] = -1;
        data_ptrs[erasures[0]] = new char[block_size];
        memset(data_ptrs[erasures[0]], 0, block_size);

        decoding_parity_ptrs_lrc(data_ptrs, repair_number-1 , coding_ptrs,1, block_size,erasures);
        sub_values[fail_index] = data_ptrs[erasures[0]];
    
        //ptr->is_core = true;
        ptr->number_of_b = 0;
        if(fail_index == k-1){
          memcached_set(ptr, key_ptrs[fail_index], key_len_ptrs[fail_index],
                                 sub_values[fail_index], last_data_request_size,86400,0);

        }
        else{
          memcached_set(ptr, key_ptrs[fail_index], key_len_ptrs[fail_index],
                                 sub_values[fail_index], block_size,86400,0);
        }
        ptr->number_of_b = 1;
        //ptr->is_core = false;
        //////////////////
        if(local_parity_key_ptrs!=NULL){
        for(uint32_t i = 0;i<To_fetch;i++){
          if(local_parity_key_ptrs[i]!=NULL){
              delete local_parity_key_ptrs[i];
          }
        }
          delete local_parity_key_ptrs;
        }
      }
      




      *error = MEMCACHED_SUCCESS;
      //value_length = &temp_value_len;
      //printf("value size: %zu\n", temp_value_len);
      //printf("\n");
      char *temp_value = new char[*value_length];
      for(uint32_t i = 0; i < k; ++i) {
        if(i < k-1) {
          memcpy(temp_value + i*block_size, sub_values[i], block_size);
        } else {
          memcpy(temp_value + i*block_size, sub_values[i], last_data_request_size);
        }
      }
      //close_connect(client_socket,listen_socket);
      if(sub_values != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(sub_values[i] != NULL) {
            delete sub_values[i];
          }
        }
        delete sub_values;
      }
      delete temp_key;
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      if(coding_ptrs != NULL) {
        if(coding_ptrs[0]!=NULL){
          delete coding_ptrs[0];
        }
        delete coding_ptrs;
      }
      
      
      
      return temp_value;
  } else {
      printf("rs, multiple get error !\n");
      printf("\n");
      if(sub_values != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(sub_values[i] != NULL) {
            delete sub_values[i];
          }
        }
        delete sub_values;
      }
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      if(coding_ptrs != NULL) {
        if(coding_ptrs[0]!=NULL){
          delete coding_ptrs[0];
        }
        delete coding_ptrs;
      }
      
      //free(erasures);
      return NULL;
  }
}

/*
  What happens if no servers exist?S
*/

char *memcached_get(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t *value_length,
                    uint32_t *flags,
                    memcached_return_t *error)
{
  if(ptr->number_of_n > 0 and ptr->number_of_k >0 and ptr->number_of_r >0 and ptr->number_of_b > 0 and !(ptr->is_core)){
    return memcached_get_lrc(ptr, key,
                    key_length,
                    value_length,
                    flags,
                    error);

  }
  return memcached_get_by_key(ptr, NULL, 0, key, key_length, value_length,
                              flags, error);
}

static memcached_return_t __mget_by_key_real(memcached_st *ptr,
                                             const char *group_key,
                                             size_t group_key_length,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             size_t number_of_keys,
                                             const bool mget_mode);
char *memcached_get_by_key(memcached_st *shell,
                           const char *group_key,
                           size_t group_key_length,
                           const char *key, size_t key_length,
                           size_t *value_length,
                           uint32_t *flags,
                           memcached_return_t *error)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t unused;
  if (error == NULL)
  {
    error= &unused;
  }

  uint64_t query_id= 0;
  if (ptr)
  {
    query_id= ptr->query_id;
  }

  /* Request the key */
  *error= __mget_by_key_real(ptr, group_key, group_key_length,
                             (const char * const *)&key, &key_length, 
                             1, false);
  if (ptr)
  {
    assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");
  }

  if (memcached_failed(*error))
  {
    if (ptr)
    {
      if (memcached_has_current_error(*ptr)) // Find the most accurate error
      {
        *error= memcached_last_error(ptr);
      }
    }

    if (value_length) 
    {
      *value_length= 0;
    }

    return NULL;
  }

  char *value= memcached_fetch(ptr, NULL, NULL,
                               value_length, flags, error);
  assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");

  /* This is for historical reasons */
  if (*error == MEMCACHED_END)
  {
    *error= MEMCACHED_NOTFOUND;
  }
  if (value == NULL)
  {
    if (ptr->get_key_failure and *error == MEMCACHED_NOTFOUND)
    {
      memcached_result_st key_failure_result;
      memcached_result_st* result_ptr= memcached_result_create(ptr, &key_failure_result);
      memcached_return_t rc= ptr->get_key_failure(ptr, key, key_length, result_ptr);

      /* On all failure drop to returning NULL */
      if (rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED)
      {
        if (rc == MEMCACHED_BUFFERED)
        {
          uint64_t latch; /* We use latch to track the state of the original socket */
          latch= memcached_behavior_get(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS);
          if (latch == 0)
          {
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 1);
          }

          rc= memcached_set(ptr, key, key_length,
                            (memcached_result_value(result_ptr)),
                            (memcached_result_length(result_ptr)),
                            0,
                            (memcached_result_flags(result_ptr)));

          if (rc == MEMCACHED_BUFFERED and latch == 0)
          {
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 0);
          }
        }
        else
        {
          rc= memcached_set(ptr, key, key_length,
                            (memcached_result_value(result_ptr)),
                            (memcached_result_length(result_ptr)),
                            0,
                            (memcached_result_flags(result_ptr)));
        }

        if (rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED)
        {
          *error= rc;
          *value_length= memcached_result_length(result_ptr);
          *flags= memcached_result_flags(result_ptr);
          char *result_value=  memcached_string_take_value(&result_ptr->value);
          memcached_result_free(result_ptr);

          return result_value;
        }
      }

      memcached_result_free(result_ptr);
    }
    assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");

    return NULL;
  }

  return value;
}

memcached_return_t memcached_mget(memcached_st *ptr,
                                  const char * const *keys,
                                  const size_t *key_length,
                                  size_t number_of_keys)
{
  
  return memcached_mget_by_key(ptr, NULL, 0, keys, key_length, number_of_keys);
}

static memcached_return_t binary_mget_by_key(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             const bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys,
                                             const bool mget_mode);

static memcached_return_t __mget_by_key_real(memcached_st *ptr,
                                             const char *group_key,
                                             const size_t group_key_length,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             size_t number_of_keys,
                                             const bool mget_mode)
{
  bool failures_occured_in_sending= false;
  const char *get_command= "get";
  uint8_t get_command_length= 3;
  unsigned int master_server_key= (unsigned int)-1; /* 0 is a valid server id! */

  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }

  if (memcached_is_udp(ptr))
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  LIBMEMCACHED_MEMCACHED_MGET_START();

  if (number_of_keys == 0)
  {
    return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("Numbers of keys provided was zero"));
  }

  if (memcached_failed((rc= memcached_key_test(*ptr, keys, key_length, number_of_keys))))
  {
    assert(memcached_last_error(ptr) == rc);

    return rc;
  }

  bool is_group_key_set= false;
  if (group_key and group_key_length)
  {
    master_server_key= memcached_generate_hash_with_redistribution(ptr, group_key, group_key_length);
    is_group_key_set= true;
  }

  /*
    Here is where we pay for the non-block API. We need to remove any data sitting
    in the queue before we start our get.

    It might be optimum to bounce the connection if count > some number.
  */
  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);
    //SSchar *S
    //char *host11;
    //host11 = instance->hostname();
    /*
    FILE *fptr;
    fptr = fopen("/home/ms/host111.txt","w");
    if(fptr == NULL)
    {
      printf("Error!");
      exit(1);
    }
    fprintf(fptr,"%s:%d", instance->hostname(),instance->port());
    fclose(fptr);
    */
    if (instance->response_count())
    {
      char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

      if (ptr->flags.no_block)
      {
        memcached_io_write(instance);
      }

      while(instance->response_count())
      {
        (void)memcached_response(instance, buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, &ptr->result);
      }
    }
  }

  if (memcached_is_binary(ptr))
  {
    return binary_mget_by_key(ptr, master_server_key, is_group_key_set, keys,
                              key_length, number_of_keys, mget_mode);
  }

  if (ptr->flags.support_cas)
  {
    get_command= "gets";
    get_command_length= 4;
  }

  /*
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  WATCHPOINT_ASSERT(rc == MEMCACHED_SUCCESS);
  size_t hosts_connected= 0;
  for (uint32_t x= 0; x < number_of_keys; x++)
  {
    uint32_t server_key;

    if (is_group_key_set)
    {
      server_key= master_server_key;
    }
    else
    {
      server_key= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
    }

    memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

    libmemcached_io_vector_st vector[]=
    {
      { get_command, get_command_length },
      { memcached_literal_param(" ") },
      { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
      { keys[x], key_length[x] }
    };


    if (instance->response_count() == 0)
    {
      rc= memcached_connect(instance);

      if (memcached_failed(rc))
      {
        memcached_set_error(*instance, rc, MEMCACHED_AT);
        continue;
      }
      hosts_connected++;

      if ((memcached_io_writev(instance, vector, 1, false)) == false)
      {
        failures_occured_in_sending= true;
        continue;
      }
      WATCHPOINT_ASSERT(instance->cursor_active_ == 0);
      memcached_instance_response_increment(instance);
      WATCHPOINT_ASSERT(instance->cursor_active_ == 1);
    }

    {
      if ((memcached_io_writev(instance, (vector + 1), 3, false)) == false)
      {
        memcached_instance_response_reset(instance);
        failures_occured_in_sending= true;
        continue;
      }
    }
  }

  if (hosts_connected == 0)
  {
    LIBMEMCACHED_MEMCACHED_MGET_END();

    if (memcached_failed(rc))
    {
      return rc;
    }

    return memcached_set_error(*ptr, MEMCACHED_NO_SERVERS, MEMCACHED_AT);
  }


  /*
    Should we muddle on if some servers are dead?
  */
  bool success_happened= false;
  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

    if (instance->response_count())
    {
      /* We need to do something about non-connnected hosts in the future */
      if ((memcached_io_write(instance, "\r\n", 2, true)) == -1)
      {
        failures_occured_in_sending= true;
      }
      else
      {
        success_happened= true;
      }
    }
  }

  LIBMEMCACHED_MEMCACHED_MGET_END();

  if (failures_occured_in_sending and success_happened)
  {
    return MEMCACHED_SOME_ERRORS;
  }

  if (success_happened)
  {
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_FAILURE; // Complete failure occurred
}

memcached_return_t memcached_mget_by_key(memcached_st *shell,
                                         const char *group_key,
                                         size_t group_key_length,
                                         const char * const *keys,
                                         const size_t *key_length,
                                         size_t number_of_keys)
{
  Memcached* ptr= memcached2Memcached(shell);
  return __mget_by_key_real(ptr, group_key, group_key_length, keys, key_length, number_of_keys, true);
}

memcached_return_t memcached_mget_execute(memcached_st *ptr,
                                          const char * const *keys,
                                          const size_t *key_length,
                                          size_t number_of_keys,
                                          memcached_execute_fn *callback,
                                          void *context,
                                          unsigned int number_of_callbacks)
{
  return memcached_mget_execute_by_key(ptr, NULL, 0, keys, key_length,
                                       number_of_keys, callback,
                                       context, number_of_callbacks);
}

memcached_return_t memcached_mget_execute_by_key(memcached_st *shell,
                                                 const char *group_key,
                                                 size_t group_key_length,
                                                 const char * const *keys,
                                                 const size_t *key_length,
                                                 size_t number_of_keys,
                                                 memcached_execute_fn *callback,
                                                 void *context,
                                                 unsigned int number_of_callbacks)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, false)))
  {
    return rc;
  }

  if (memcached_is_udp(ptr))
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  if (memcached_is_binary(ptr) == false)
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT,
                               memcached_literal_param("ASCII protocol is not supported for memcached_mget_execute_by_key()"));
  }

  memcached_callback_st *original_callbacks= ptr->callbacks;
  memcached_callback_st cb= {
    callback,
    context,
    number_of_callbacks
  };

  ptr->callbacks= &cb;
  rc= memcached_mget_by_key(ptr, group_key, group_key_length, keys,
                            key_length, number_of_keys);
  ptr->callbacks= original_callbacks;

  return rc;
}



static memcached_return_t simple_binary_mget(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys, const bool mget_mode)
{
  memcached_return_t rc= MEMCACHED_NOTFOUND;

  bool flush= (number_of_keys == 1);

  if (memcached_failed(rc= memcached_key_test(*ptr, keys, key_length, number_of_keys)))
  {
    return rc;
  }

  /*
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  //optimal lrc add
  //uint32_t n = ptr->number_of_n;
  uint32_t k = ptr->number_of_k;
  uint32_t r = ptr->number_of_r;
  //uint32_t b = ptr->number_of_b;
  //uint32_t g = n - (uint32_t)((n+r)/(r+1)) - k;
  //uint32_t local_group_number = (uint32_t)((n+r)/(r+1));

  //uint32_t local_of_data = uint32_t((k+r-1)/r);
  
  uint32_t *server_key_list = new uint32_t[k];

  //printf("local_of_data:%d\n",local_of_data);

  ip2server_key_st p;
  /*
  for (uint32_t i = 0; i < local_of_data; i++)
  {
    uint32_t index_in_rack = 0;
    p = ptr->rack_group[i];
    while (p->next!= NULL)
    {
      server_key_list[i*ptr->number_of_r+index_in_rack] = p->ip_server_key;
      printf("p->port:%d\n,p->ip_server_key:%d\n", p->port, p->ip_server_key);
      p = p->next;
      if((i*ptr->number_of_r + index_in_rack) == (k-1)){
        break;
      }
      index_in_rack++;
    }

    printf("rack\n");
  }
  for (uint32_t i = 0; i < k; i++){
    printf("server_key_list[%d]:%d\n",i,server_key_list[i]);
  }
  */
  for (uint32_t x= 0; x < number_of_keys; ++x)
  {
    uint32_t server_key;

    //optimal lrc add
    if(number_of_keys > 1 && ptr->number_of_n > 0 && ptr->number_of_k > 0 && ptr->number_of_r > 0 && ptr->number_of_b > 0 &&(!ptr->is_core)) {
        /* number_of_keys > 1 is to distinguish get key or get sub-keys, 19-10-14 */
        int one = int(keys[x][key_length[x]-1])-48;
        int ten = int(keys[x][key_length[x]-2])-48;
        uint32_t index = ten*10 + one;
        server_key = ptr->node2rack[index];
        
        //printf("get_index:%d server_key:%d\n",index,server_key);

    } 
    else if(ptr->is_core){
        server_key = ptr->network_core_server_key;
        memcached_instance_st* instance1= memcached_instance_fetch(ptr, server_key);
        //printf("server_key:%d\n",server_key);
        //printf("name:%s host: %d\n",instance1->hostname(),instance1->port());

    }
    else {

      if (is_group_key_set)
      {

        //server_key= master_server_key;
        int one = int(keys[x][key_length[x]-1])-48;
        int ten = int(keys[x][key_length[x]-2])-48;
        uint32_t index = ten*10 + one;
        server_key = ptr->node2rack[index];
        //printf("get_index:%d server_key:%d\n",index,server_key);
      }
      else
      {
        //server_key= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
        int one = int(keys[x][key_length[x]-1])-48;
        int ten = int(keys[x][key_length[x]-2])-48;
        uint32_t index = ten*10 + one;
        server_key = ptr->node2rack[index];
        //printf("get_index:%d server_key:%d\n",index,server_key);
      }
    }
    

    memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);
    

    if (instance->response_count() == 0)
    {
      rc= memcached_connect(instance);
      if (memcached_failed(rc))
      {
        continue;
      }
    }

    protocol_binary_request_getk request= { }; //= {.bytes= {0}};
    initialize_binary_request(instance, request.message.header);
    if (mget_mode)
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETKQ;
    }
    else
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETK;
    }

#if 0
    {
      memcached_return_t vk= memcached_validate_key_length(key_length[x], ptr->flags.binary_protocol);
      if (memcached_failed(rc= memcached_key_test(*memc, (const char **)&key, &key_length, 1)))
      {
        memcached_set_error(ptr, vk, MEMCACHED_AT, memcached_literal_param("Key was too long."));

        if (x > 0)
        {
          memcached_io_reset(instance);
        }

        return vk;
      }
    }
#endif

    request.message.header.request.keylen= htons((uint16_t)(key_length[x] + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.bodylen= htonl((uint32_t)( key_length[x] + memcached_array_size(ptr->_namespace)));

    libmemcached_io_vector_st vector[]=
    {
      { request.bytes, sizeof(request.bytes) },
      { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
      { keys[x], key_length[x] }
    };

    if (memcached_io_writev(instance, vector, 3, flush) == false)
    {
      memcached_server_response_reset(instance);
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }

    /* We just want one pending response per server */
    memcached_server_response_reset(instance);
    memcached_server_response_increment(instance);
    if ((x > 0 and x == ptr->io_key_prefetch) and memcached_flush_buffers(ptr) != MEMCACHED_SUCCESS)
    {
      rc= MEMCACHED_SOME_ERRORS;
    }
  }

  if (mget_mode)
  {
    /*
      Send a noop command to flush the buffers
    */
    protocol_binary_request_noop request= {}; //= {.bytes= {0}};
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_NOOP;
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;

    for (uint32_t x= 0; x < memcached_server_count(ptr); ++x)
    {
      memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

      if (instance->response_count())
      {
        initialize_binary_request(instance, request.message.header);
        if ((memcached_io_write(instance) == false) or
            (memcached_io_write(instance, request.bytes, sizeof(request.bytes), true) == -1))
        {
          memcached_instance_response_reset(instance);
          memcached_io_reset(instance);
          rc= MEMCACHED_SOME_ERRORS;
        }
      }
    }
  }

  return rc;
}




static char *memcached_repair_by_index(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t value_length,
                    uint32_t *flags,
                    memcached_return_t *error,uint32_t fail_index,uint32_t inner)
{
  //ptr->node_rack[fail_index];
  //printf("******************\n");
  uint32_t k = ptr->number_of_k;
  uint32_t r = ptr->number_of_r;
  uint32_t n = ptr->number_of_n;
  uint32_t group_number = 1; 
  uint32_t group_id = uint32_t(fail_index/(r+1));
  if((r+1)*(group_id+1)>n){
      group_number = n - group_id*(r+1);
  }
  else{
      group_number = r + 1;
  }
  
  
  char **key_ptrs = new char*[group_number];
  size_t *key_len_ptrs = new size_t[group_number];

  // generate new keys for each block
  for(uint32_t i = 0; i < group_number; ++i) {
      key_ptrs[i] = new char[key_length + 3];
      memset(key_ptrs[i], 0, key_length+3);
      memcpy(key_ptrs[i], key, key_length);
      int ten = (i + group_id*(r+1))/ 10;
      int one = (i + group_id*(r+1)) - ten * 10;
      char ten_bit_char = '0' + ten;
      char one_bit_char = '0' + one;
      key_ptrs[i][key_length] = ten_bit_char;
      key_ptrs[i][key_length+1] = one_bit_char;
      key_ptrs[i][key_length+2] = '\0';
      key_len_ptrs[i] = key_length+2;
      //printf("repair_index_key:%d repair_key %s\n",i,key_ptrs[i]);
  }
  // multiple get the data blocks
  memcached_return_t unused;
  if(error == NULL) {
      error = &unused;
  }
  char **sub_values = new char*[group_number-1];
  char **coding_ptrs = new char*[1];

  size_t block_size = determine_block_size_rs(value_length, k);//size_t block_size = 1000;
  size_t last_data_request_size = value_length - block_size * (k - 1);
  int *erasures;
  uint32_t cross_rack_num = 0;
  uint32_t first_rack = ptr->node_rack[group_id*(r+1)];
  erasures = new int[r+1];//(int *)malloc(sizeof(int)*(r+1));
  erasures[0] = fail_index-group_id*(r+1);
  erasures[1] = -1;
  uint32_t last_block = k+uint32_t((k-1)/r)-1;

  *error = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, group_number);
  if(*error == MEMCACHED_SUCCESS) {
    memcached_result_st *result_buffer = &ptr->result;
    char *temp_key = new char[key_length + 3];
    for(uint32_t count_data = 0;count_data<group_number;count_data++){
      if(first_rack!=ptr->node_rack[count_data+group_id*(r+1)]){
            cross_rack_num++;
            first_rack=ptr->node_rack[count_data+group_id*(r+1)];
          }
      if((result_buffer = memcached_fetch_result(ptr, result_buffer, error)) != NULL){
          strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
          temp_key[key_length + 2] = '\0';
          //printf("key: %s\n", temp_key);
          uint32_t i;
          for(i = 0; i < group_number; ++i) {
            if(strcmp(temp_key, key_ptrs[i]) == 0) break;
          }
          //printf("index: %d\n", i);
          

          if(i+group_id*(r+1) == last_block) { 
            last_data_request_size = memcached_result_length(result_buffer);
            //printf("last_data_request_size:%d\n",last_data_request_size);
          }else{
            block_size = memcached_result_length(result_buffer);
            //printf("block_size:%d\n",block_size);
          }
          ///
          if(i == group_number-1){
            coding_ptrs[0] = new char[block_size];
            memset(coding_ptrs[0], 0, block_size);
            memcpy(coding_ptrs[0], memcached_result_value(result_buffer), memcached_result_length(result_buffer));

          }else{
            sub_values[i] = new char[block_size];
            memset(sub_values[i], 0, block_size);
            memcpy(sub_values[i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
          }
                
          if(flags) {
              *flags= result_buffer->item_flags;
            }
        }   
    }
    //coding
    /*
    if((result_buffer = memcached_fetch_result(ptr, result_buffer, error)) != NULL){
      block_size = memcached_result_length(result_buffer);
      coding_ptrs[0] = new char[memcached_result_length(result_buffer)];
      memset(coding_ptrs[0], 0, block_size);
      memcpy(coding_ptrs[0], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
      if(flags) {
              *flags= result_buffer->item_flags;
              }
    }
    else{
      printf("fail_here!!!!!!\n");
      printf("coding_ptr: %s\n",key_ptrs[group_number-1]);

    }
    */
    if(erasures[0] == int(group_number)-1){
      
      coding_ptrs[0] = new char[block_size];//(char *)malloc(sizeof(char)*block_size);
      memset(coding_ptrs[0], 0, block_size);
    }
    else{
      sub_values[erasures[0]] = new char[block_size];//(char *)malloc(sizeof(char)*block_size);//(char *)malloc(sizeof(char)*block_size);
      memset(sub_values[erasures[0]], 0, block_size);
    }
    //int client_socket = create_socket();
    //int listen_socket = start_listen(ptr,client_socket);
    struct timeval start_time, end_time;
    if(cross_rack_num!=0 && inner){
          for(uint32_t bi = 0;bi<cross_rack_num;bi++){
              //printf("cross_rack_num%d\n",cross_rack_num);
              //printf("key_ptrs[group_number-1]:%s,fail_index%d\n",key_ptrs[group_number-1],fail_index);
              //gettimeofday(&start_time, NULL);
              cross_networkcore_libmemcached(ptr,key_ptrs[group_number-1],key_len_ptrs[group_number-1],
                                 coding_ptrs[0], block_size);
              //gettimeofday(&end_time, NULL);
              double repair_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
              //printf("--------------------------------\n");
              //fprintf(stderr, "core_time: %.6lf s\n", repair_time);
              //printf("repair cross networkcore\n");
          }
      }  
    //close_connect(client_socket,listen_socket);
    //helpmemplease();
    //printf("come here0!");
    /*
    int *matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  //jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, block_size);
  int i = 0;
  i = jerasure_matrix_decode(k, m, 8, matrix, 1, erasures,sub_values, coding_ptrs, block_size);
  if (i == -1) {
			printf("Unsuccessful!\n");
		}*/
    decoding_parity_ptrs_lrc(sub_values, group_number-1 , coding_ptrs,1, block_size,erasures);
    //printf("come here1!");
    //return NULL;
  
    ptr->fail_node_index = fail_index;//+ int(fail_index/r);
 
    ptr->number_of_b = 0;
    //gettimeofday(&start_time, NULL);
    if(fail_index == last_block){
      if(erasures[0]==group_number-1){
        memcached_set(ptr, key_ptrs[erasures[0]], key_len_ptrs[erasures[0]],
                               coding_ptrs[0], block_size,86400,0);
                              
      }else{
        memcached_set(ptr, key_ptrs[erasures[0]], key_len_ptrs[erasures[0]],
                               sub_values[erasures[0]], last_data_request_size,86400,0);
                              }
  
      }
      else{
        if(erasures[0]==group_number-1){
          memcached_set(ptr, key_ptrs[erasures[0]], key_len_ptrs[erasures[0]],
                               coding_ptrs[0], block_size,86400,0);
                              
          }
        else{
          memcached_set(ptr, key_ptrs[erasures[0]], key_len_ptrs[erasures[0]],
                                 sub_values[erasures[0]], block_size,86400,0);
        }
      
      }
    //gettimeofday(&end_time, NULL);
              //double repair_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
              //printf("--------------------------------\n");
              //fprintf(stderr, "set_time: %.6lf s\n", repair_time);
    ptr->number_of_b = 1;
    //printf("come here2!");
    *error = MEMCACHED_SUCCESS;


    if(sub_values != NULL) {
      for(uint32_t i = 0; i < group_number-1; ++i) {
        if(sub_values[i] != NULL) {
          delete sub_values[i];
        }
      }
        delete sub_values;
      }
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < group_number; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      if(coding_ptrs != NULL){
        if(coding_ptrs[0]!=NULL){
          delete coding_ptrs[0];
        }
        
        delete coding_ptrs;
      }
      if(erasures!=NULL){
        delete erasures;
      }
      
      //free(erasures);



    return NULL;
  }else{
    printf("rs, multiple get error !\n");
    printf("\n");
    if(sub_values != NULL) {
      for(uint32_t i = 0; i < group_number; ++i) {
        if(sub_values[i] != NULL) {
          delete sub_values[i];
        }
      }
        delete sub_values;
      }
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < group_number; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      if(coding_ptrs != NULL){
        if(coding_ptrs[0]!=NULL){
          delete coding_ptrs[0];
        }
        delete coding_ptrs;
      }
      if(erasures!=NULL){
        delete erasures;
      }
      //free(erasures);
    return NULL;
  }
}



/* optimal lrc add*/
char *memcached_repair(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t value_length,
                    uint32_t *flags,
                    memcached_return_t *error,uint32_t fail_index)
{
  //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  return memcached_repair_by_index(ptr,key,
                    key_length,value_length,flags,
                    error,fail_index,1);
}



static memcached_return_t replication_binary_mget(memcached_st *ptr,
                                                  uint32_t* hash,
                                                  bool* dead_servers,
                                                  const char *const *keys,
                                                  const size_t *key_length,
                                                  const size_t number_of_keys)
{
  memcached_return_t rc= MEMCACHED_NOTFOUND;
  uint32_t start= 0;
  uint64_t randomize_read= memcached_behavior_get(ptr, MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ);

  if (randomize_read)
  {
    start= (uint32_t)random() % (uint32_t)(ptr->number_of_replicas + 1);
  }

  /* Loop for each replica */
  for (uint32_t replica= 0; replica <= ptr->number_of_replicas; ++replica)
  {
    bool success= true;

    for (uint32_t x= 0; x < number_of_keys; ++x)
    {
      if (hash[x] == memcached_server_count(ptr))
      {
        continue; /* Already successfully sent */
      }

      uint32_t server= hash[x] +replica;

      /* In case of randomized reads */
      if (randomize_read and ((server + start) <= (hash[x] + ptr->number_of_replicas)))
      {
        server+= start;
      }

      while (server >= memcached_server_count(ptr))
      {
        server -= memcached_server_count(ptr);
      }

      if (dead_servers[server])
      {
        continue;
      }

      memcached_instance_st* instance= memcached_instance_fetch(ptr, server);

      if (instance->response_count() == 0)
      {
        rc= memcached_connect(instance);

        if (memcached_failed(rc))
        {
          memcached_io_reset(instance);
          dead_servers[server]= true;
          success= false;
          continue;
        }
      }

      protocol_binary_request_getk request= {};
      initialize_binary_request(instance, request.message.header);
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETK;
      request.message.header.request.keylen= htons((uint16_t)(key_length[x] + memcached_array_size(ptr->_namespace)));
      request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
      request.message.header.request.bodylen= htonl((uint32_t)(key_length[x] + memcached_array_size(ptr->_namespace)));

      /*
       * We need to disable buffering to actually know that the request was
       * successfully sent to the server (so that we should expect a result
       * back). It would be nice to do this in buffered mode, but then it
       * would be complex to handle all error situations if we got to send
       * some of the messages, and then we failed on writing out some others
       * and we used the callback interface from memcached_mget_execute so
       * that we might have processed some of the responses etc. For now,
       * just make sure we work _correctly_
     */
      libmemcached_io_vector_st vector[]=
      {
        { request.bytes, sizeof(request.bytes) },
        { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
        { keys[x], key_length[x] }
      };

      if (memcached_io_writev(instance, vector, 3, true) == false)
      {
        memcached_io_reset(instance);
        dead_servers[server]= true;
        success= false;
        continue;
      }

      memcached_server_response_increment(instance);
      hash[x]= memcached_server_count(ptr);
    }

    if (success)
    {
      break;
    }
  }

  return rc;
}

static memcached_return_t binary_mget_by_key(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys,
                                             const bool mget_mode)
{
  if (ptr->number_of_replicas == 0)
  {
    return simple_binary_mget(ptr, master_server_key, is_group_key_set,
                              keys, key_length, number_of_keys, mget_mode);
  }

  uint32_t* hash= libmemcached_xvalloc(ptr, number_of_keys, uint32_t);
  bool* dead_servers= libmemcached_xcalloc(ptr, memcached_server_count(ptr), bool);

  if (hash == NULL or dead_servers == NULL)
  {
    libmemcached_free(ptr, hash);
    libmemcached_free(ptr, dead_servers);
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  if (is_group_key_set)
  {
    for (size_t x= 0; x < number_of_keys; x++)
    {
      hash[x]= master_server_key;
    }
  }
  else
  {
    for (size_t x= 0; x < number_of_keys; x++)
    {
      hash[x]= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
    }
  }

  memcached_return_t rc= replication_binary_mget(ptr, hash, dead_servers, keys,
                                                 key_length, number_of_keys);

  WATCHPOINT_IFERROR(rc);
  libmemcached_free(ptr, hash);
  libmemcached_free(ptr, dead_servers);

  return MEMCACHED_SUCCESS;
}
