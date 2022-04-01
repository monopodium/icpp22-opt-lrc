/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
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
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
extern "C"{
#include "../Jerasure-1.2A/cauchy.h"
#include "../Jerasure-1.2A/reed_sol.h"
#include "../Jerasure-1.2A/jerasure.h"
#include "../Jerasure-1.2A/galois.h"
}
//static double *time;
static double divide_latency = 0.0;
static double encoding_latency = 0.0;
static double transmission_latency = 0.0;
enum memcached_storage_action_t {
  SET_OP,
  REPLACE_OP,
  ADD_OP,
  PREPEND_OP,
  APPEND_OP,
  CAS_OP
};

//optimal lrc add
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

//optimal lrc add
static void fill_data_ptrs(char **data_ptrs, uint32_t size, size_t block_size, const char *value, const size_t value_length) {
  size_t bytes_remained = value_length;
  for(uint32_t i = 0; i < size; ++i) {
    if(block_size <= bytes_remained) {
      memcpy(data_ptrs[i], value + i*block_size, block_size);
      bytes_remained -= block_size;
    } else {
      memcpy(data_ptrs[i], value + i*block_size, bytes_remained);
      bytes_remained -= bytes_remained;
    }
  }
}
//optimal lrc add
static void calculate_parity_ptrs_rs(char **data_ptrs, uint32_t k, char **coding_ptrs, uint32_t m, size_t block_size) {
  int *matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, block_size);
  free(matrix);
}
//optimal lrc add
static void generate_new_keys_rs(char **key_ptrs, uint32_t size, const char *key, const size_t key_length) {
  for(uint32_t i = 0; i < size; ++i) {
    memcpy(key_ptrs[i], key, key_length);
    int ten = i / 10;
    int one = i - ten * 10;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    key_ptrs[i][key_length] = ten_bit_char;
    key_ptrs[i][key_length+1] = one_bit_char;
    key_ptrs[i][key_length+2] = '\0';
  }
}
//optimal lrc add
static void add_rack(memcached_st *ptr){
  ptr->rack_group = new ip2server_key_st[20];
  uint32_t rack_number = ptr->number_of_rack;
  uint32_t count = memcached_server_count(ptr);
  if((count-1)!=ptr->number_of_n){
    printf("(count-1)!=ptr->number_of_n:count=%d,n=%d",count,ptr->number_of_n);
  }
  uint32_t server_key = 0;
  printf("count: %d\n",count);
  char network_core_ip[20];
  int network_core_host;
  char data[20];
  int host;

  FILE *fp=fopen("/home/ms/rack.txt","r");
  if(!fp){
    printf("can't open file rack.txt\n");
  }
  feof(fp);
  fscanf(fp,"%s",network_core_ip);
  
  feof(fp);
  fscanf(fp,"%d",&network_core_host);
  
  for(uint32_t ii = 0;ii < count;ii++){
    memcached_instance_st *instance = memcached_instance_fetch(ptr,ii);
    if(strcmp(instance->hostname(),network_core_ip)==0){
      if(network_core_host==instance->port()){
        ptr->network_core_server_key = ii;
        //printf("!!!!!!!!!!!!!!!!:%d\n",ii);
      }
    }
  }

  printf("network_core_ip:%s\n ",network_core_ip);
  printf("network_core_host:%d \n",network_core_host);
  printf("network_core_server_key:%d \n",ptr->network_core_server_key);
  printf("\n");

  int i = -1;
  ip2server_key_st p = NULL;
  uint32_t node_count = 0;
  uint32_t node_id = 0;
  while(!feof(fp))
  {   
    fscanf(fp,"%s",data);
    if(data[0]=='@'){
      for(int ll = 0;ll<ptr->number_of_n;ll++){
        feof(fp);
        fscanf(fp,"%d",&node_id);
        ptr->node2rack[ll] = node_id;
      }
    }
    if(data[0]=='#'){
      feof(fp);
      fscanf(fp,"%d",&node_count);
      printf("rack:%d\n",i);
      i++;
      ptr->rack_group[i] = new ip2server_key;
      
      p = ptr->rack_group[i];
    }
    else{
        feof(fp);
        fscanf(fp,"%d ",&host);
        for(uint32_t ii = 0;ii < count;ii++){
          memcached_instance_st *instance = memcached_instance_fetch(ptr,ii);

          if(strcmp(instance->hostname(),data)==0){
            if(host==instance->port()){
              printf("ii:%d\n",ii);
              printf("instance->port():%d\n",instance->port());
              p->ip_server_key = ii;
              p->hostname = new char[50];
              strcpy(p->hostname,instance->hostname());
              p->port = instance->port();
              p->use = false;
              node_count--;
              if(node_count==0){
                p->next = NULL;
              }
              else{
                p->next = new ip2server_key;
              }
              p = p->next;
              break;
            }
          }

        }
        }
        //fscanf(fp,"%s ",&data);
        
  }
    printf("\n");
    fclose(fp);
  

  if(count!=ptr->number_of_n+1){
    printf("error!");
  }
  /*
  for(uint32_t i = 0;i < ptr->number_of_rack;i++){
    printf("rack_gen_number %d\n",i);
    if(server_key == count){
      break;
    }
    

    for(uint32_t k = 0;k < ptr->number_of_b;k++){
      printf("rack_index:%d server_key:%d\n",k,server_key);
      if(server_key == count){
        break;
      }
      
      
      memcached_instance_st *instance = memcached_instance_fetch(ptr,server_key);
      
      server_key++;
      if (k == ptr->number_of_b-1 or server_key == count)
      {
        p->next = NULL;
        break;
      }
      else
      {
        p->next = new ip2server_key;
      }
      p = p->next;
      
      
    
    
    }
  // = head;

  }
*/
  
  ptr->rack_is_add = true;
}




static void cross_networkcore_libmemcached(Memcached *ptr,const char *key, size_t key_length,
                                 const char *value, size_t value_length){
  size_t * bs = NULL;
  uint32_t *fl = NULL;
  memcached_return_t rc;
  memcached_return_t *error1 = NULL;
  ptr->is_core = true;
  //memcached_instance_st *network_core_instance = memcached_instance_fetch(ptr, ptr->network_core_server_key);
  //printf("cross_networkcore:nouse:\nlibmemcached/get.cc===%d\n",int(ptr->network_core_server_key));

  memcached_set(ptr, key, key_length,value, value_length,86400,0);
  if(rc != MEMCACHED_SUCCESS) {
    printf("Add servers failure!");
  } 
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
	addr.sin_family = AF_INET; 
  addr.sin_port = htons(8899); 
  printf("start_listen:%s\n",instance->hostname());
  addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  
	inet_aton(instance->hostname(), &(addr.sin_addr));
	int addrlen = sizeof(addr);
	int listen_socket =  connect(client_socket,  (struct sockaddr *)&addr, addrlen);  
	if(listen_socket == -1)
	{
    printf("error_start_listen:%s\n",instance->hostname());
		perror("connect");
		return -1;
	}
	return listen_socket;

}
/*instance->port()*/
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

/* Inline this */
static inline const char *storage_op_string(memcached_storage_action_t verb)
{
  switch (verb)
  {
  case REPLACE_OP:
    return "replace ";

  case ADD_OP:
    return "add ";

  case PREPEND_OP:
    return "prepend ";

  case APPEND_OP:
    return "append ";

  case CAS_OP:
    return "cas ";

  case SET_OP:
    break;
  }

  return "set ";
}

static inline uint8_t can_by_encrypted(const memcached_storage_action_t verb)
{
  switch (verb)
  {
  case SET_OP:
  case ADD_OP:
  case CAS_OP:
  case REPLACE_OP:
    return true;
    
  case APPEND_OP:
  case PREPEND_OP:
    break;
  }

  return false;
}

static inline uint8_t get_com_code(const memcached_storage_action_t verb, const bool reply)
{
  if (reply == false)
  {
    switch (verb)
    {
    case SET_OP:
      return PROTOCOL_BINARY_CMD_SETQ;

    case ADD_OP:
      return PROTOCOL_BINARY_CMD_ADDQ;

    case CAS_OP: /* FALLTHROUGH */
    case REPLACE_OP:
      return PROTOCOL_BINARY_CMD_REPLACEQ;

    case APPEND_OP:
      return PROTOCOL_BINARY_CMD_APPENDQ;

    case PREPEND_OP:
      return PROTOCOL_BINARY_CMD_PREPENDQ;
    }
  }

  switch (verb)
  {
  case SET_OP:
    break;

  case ADD_OP:
    return PROTOCOL_BINARY_CMD_ADD;

  case CAS_OP: /* FALLTHROUGH */
  case REPLACE_OP:
    return PROTOCOL_BINARY_CMD_REPLACE;

  case APPEND_OP:
    return PROTOCOL_BINARY_CMD_APPEND;

  case PREPEND_OP:
    return PROTOCOL_BINARY_CMD_PREPEND;
  }

  return PROTOCOL_BINARY_CMD_SET;
}



static memcached_return_t memcached_send_binary(Memcached *ptr,
                                                memcached_instance_st* server,
                                                uint32_t server_key,
                                                const char *key,
                                                const size_t key_length,
                                                const char *value,
                                                const size_t value_length,
                                                const time_t expiration,
                                                const uint32_t flags,
                                                const uint64_t cas,
                                                const bool flush,
                                                const bool reply,
                                                memcached_storage_action_t verb)
{
  
  protocol_binary_request_set request= {};
  size_t send_length= sizeof(request.bytes);

  initialize_binary_request(server, request.message.header);

  request.message.header.request.opcode= get_com_code(verb, reply);
  request.message.header.request.keylen= htons((uint16_t)(key_length + memcached_array_size(ptr->_namespace)));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  if (verb == APPEND_OP or verb == PREPEND_OP)
  {
    send_length -= 8; /* append & prepend does not contain extras! */
  }
  else
  {
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);
  }

  request.message.header.request.bodylen= htonl((uint32_t) (key_length + memcached_array_size(ptr->_namespace) + value_length +
                                                            request.message.header.request.extlen));

  if (cas)
  {
    request.message.header.request.cas= memcached_htonll(cas);
  }

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { request.bytes, send_length },
    { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
    { key, key_length },
    { value, value_length }
  };

  /* write the header */
  memcached_return_t rc;
  if ((rc= memcached_vdo(server, vector, 5, flush)) != MEMCACHED_SUCCESS)
  {
    memcached_io_reset(server);

#if 0
    if (memcached_has_error(ptr))
    {
      memcached_set_error(*server, rc, MEMCACHED_AT);
    }
#endif

    assert(memcached_last_error(server->root) != MEMCACHED_SUCCESS);
    return memcached_last_error(server->root);
  }

  if (verb == SET_OP and ptr->number_of_replicas > 0)
  {
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SETQ;
    WATCHPOINT_STRING("replicating");

    for (uint32_t x= 0; x < ptr->number_of_replicas; x++)
    {
      ++server_key;
      if (server_key == memcached_server_count(ptr))
      {
        server_key= 0;
      }

      memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

      if (memcached_vdo(instance, vector, 5, false) != MEMCACHED_SUCCESS)
      {
        memcached_io_reset(instance);
      }
      else
      {
        memcached_server_response_decrement(instance);
      }
    }
  }

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  return memcached_response(server, NULL, 0, NULL);
}

static memcached_return_t memcached_send_ascii(Memcached *ptr,
                                               memcached_instance_st* instance,
                                               const char *key,
                                               const size_t key_length,
                                               const char *value,
                                               const size_t value_length,
                                               const time_t expiration,
                                               const uint32_t flags,
                                               const uint64_t cas,
                                               const bool flush,
                                               const bool reply,
                                               const memcached_storage_action_t verb)
{
  char flags_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int flags_buffer_length= snprintf(flags_buffer, sizeof(flags_buffer), " %u", flags);
  if (size_t(flags_buffer_length) >= sizeof(flags_buffer) or flags_buffer_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
  }

  char expiration_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int expiration_buffer_length= snprintf(expiration_buffer, sizeof(expiration_buffer), " %llu", (unsigned long long)expiration);
  if (size_t(expiration_buffer_length) >= sizeof(expiration_buffer) or expiration_buffer_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
  }

  char value_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int value_buffer_length= snprintf(value_buffer, sizeof(value_buffer), " %llu", (unsigned long long)value_length);
  if (size_t(value_buffer_length) >= sizeof(value_buffer) or value_buffer_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
  }

  char cas_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int cas_buffer_length= 0;
  if (cas)
  {
    cas_buffer_length= snprintf(cas_buffer, sizeof(cas_buffer), " %llu", (unsigned long long)cas);
    if (size_t(cas_buffer_length) >= sizeof(cas_buffer) or cas_buffer_length < 0)
    {
      return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                                 memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
    }
  }

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { storage_op_string(verb), strlen(storage_op_string(verb))},
    { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
    { key, key_length },
    { flags_buffer, size_t(flags_buffer_length) },
    { expiration_buffer, size_t(expiration_buffer_length) },
    { value_buffer, size_t(value_buffer_length) },
    { cas_buffer, size_t(cas_buffer_length) },
    { " noreply", reply ? 0 : memcached_literal_param_size(" noreply") },
    { memcached_literal_param("\r\n") },
    { value, value_length },
    { memcached_literal_param("\r\n") }
  };

  /* Send command header */
  memcached_return_t rc=  memcached_vdo(instance, vector, 12, flush);

  // If we should not reply, return with MEMCACHED_SUCCESS, unless error
  if (reply == false)
  {
    return memcached_success(rc) ? MEMCACHED_SUCCESS : rc; 
  }

  if (flush == false)
  {
    return memcached_success(rc) ? MEMCACHED_BUFFERED : rc; 
  }

  if (rc == MEMCACHED_SUCCESS)
  {
    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
    rc= memcached_response(instance, buffer, sizeof(buffer), NULL);

    if (rc == MEMCACHED_STORED)
    {
      return MEMCACHED_SUCCESS;
    }
  }

  if (rc == MEMCACHED_WRITE_FAILURE)
  {
    memcached_io_reset(instance);
  }

  assert(memcached_failed(rc));
#if 0
  if (memcached_has_error(ptr) == false)
  {
    return memcached_set_error(*ptr, rc, MEMCACHED_AT);
  }
#endif

  return rc;
}
//optimal lrc add
static memcached_return_t memcached_lrc_send_binary(Memcached *ptr,
                                                   memcached_instance_st* server,
                                                   uint32_t server_key,
                                                   const char *key,
                                                   const size_t key_length,
                                                   const char *value,
                                                   const size_t value_length,
                                                   const time_t expiration,
                                                   const uint32_t flags,
                                                   const uint64_t cas,
                                                   const bool flush,
                                                   const bool reply,
                                                   memcached_storage_action_t verb
                                                   )
{
  /* Apply optimal lrc on this object.
   * Unlike replication which uses one request, optimal lrc uses n requests, each of which corresponds
   * to a data (parity) block.
   */
  // determining block size
  //time = new double[3];
  uint32_t n = ptr->number_of_n;
  uint32_t k = ptr->number_of_k;
  uint32_t r = ptr->number_of_r;
  //uint32_t b = ptr->number_of_b;
  uint32_t g = n - (uint32_t)((n+r)/(r+1)) - k;
  uint32_t local_group_number = (uint32_t)((n+r)/(r+1));
  uint32_t rack_number = ptr->number_of_rack;

  struct timeval start_time, end_time;
  

  gettimeofday(&start_time, NULL);

  size_t block_size = determine_block_size_rs(value_length, k);
  char **key_ptrs = new char*[n];
  for(uint32_t i = 0; i < n; ++i) {
    key_ptrs[i] = new char[key_length + 3];
    memset(key_ptrs[i], 0, key_length+3);
  }
  generate_new_keys_rs(key_ptrs, n, key, key_length);

  // fill data blocks
  char **data_ptrs = new char*[k];
  for(uint32_t i = 0; i < k; ++i) {
    data_ptrs[i] = new char [block_size];
    memset(data_ptrs[i], 0, block_size);		
  }
  fill_data_ptrs(data_ptrs, k, block_size, value, value_length);

/*-----------------------------debug------------------------------*/
/*
  FILE *fptr;
    fptr = fopen("/home/ms/host111.txt","a");
    if(fptr == NULL)
    {
      printf("Error!");
      exit(1);
    }
    fprintf(fptr,"---------------------");
    fprintf(fptr,"optimal lrc, n: %d, k: %d, r: %d, b: %d, key: %s, value_length:%zu, block_size: %zu\n", n , k, r ,b ,key, value_length, block_size);
    printf("optimal lrc, n: %d, k: %d, r: %d, b: %d, key: %s, value_length:%zu, block_size: %zu\n", n , k, r ,b ,key, value_length, block_size);
    fclose(fptr);
  */
  //FILE *fp = fopen("./wusi", "a");
  //fprintf(fp, "rs, k: %d, m: %d, key: %s, value_length:%d, block_size: %d\n", k, m, key, value_length, block_size);
/*-----------------------------debug------------------------------*/

  // calculate global parity blocks and g
  

  char **global_coding_ptrs = new char*[g];
  for(uint32_t i = 0; i < g; ++i) {
    global_coding_ptrs[i] = new char[block_size];
    memset(global_coding_ptrs[i], 0, block_size);
  }
  char **all_blocks = new char*[n];
  for(uint32_t i = 0; i < n; ++i) {
    all_blocks[i] = new char[block_size];
    memset(all_blocks[i], 0, block_size);
  }

  gettimeofday(&end_time, NULL);
  divide_latency = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  //time[0] = divide_latency;
  //fprintf(stdin, "%s divide_latency: %.6lf s\n",key, divide_latency);

  gettimeofday(&start_time, NULL);
  calculate_parity_ptrs_rs(data_ptrs, k, global_coding_ptrs, g, block_size);
/*
  char **local_coding_ptrs = new char*[local_group_number];
  for(uint32_t i = 0; i < local_group_number; ++i) {
    local_coding_ptrs[i] = new char[block_size];
    memset(local_coding_ptrs[i], 0, block_size);
    
  }
*/
  
  uint32_t remain = k + g;
  uint32_t index = 0;
  for(uint32_t i = 0; i < local_group_number; ++i) {
    char **local_coding_ptrs = new char*[1];
    local_coding_ptrs[0] = new char[block_size];
    memset(local_coding_ptrs[0], 0, block_size);

    if(remain>=r){
      char **local_group = new char*[r];
      for(uint32_t ii = 0; ii < r; ++ii) {
        local_group[ii] = new char[block_size];
        memset(local_group[ii], 0, block_size);
        index = i*r+ii;
        if(index < k){
          local_group[ii] = data_ptrs[index];
          all_blocks[i*(r+1)+ii] = data_ptrs[index];
        }
        else{
          local_group[ii] = global_coding_ptrs[index-k];
          all_blocks[i*(r+1)+ii] = global_coding_ptrs[index-k];
        }
        }
      calculate_parity_ptrs_rs(local_group, r, local_coding_ptrs, 1, block_size);
      all_blocks[i*(r+1)+r] = local_coding_ptrs[0];
      remain = remain - r;
    }
    else{
      char **local_group = new char*[remain];
      for(uint32_t ii = 0; ii < remain; ++ii) {
        local_group[ii] = new char[block_size];
        memset(local_group[ii], 0, block_size);
        index = i*r+ii;
        if(index < k){
          local_group[ii] = data_ptrs[index];
          all_blocks[i*(r+1)+ii] = data_ptrs[index];
        }
        else{
          local_group[ii] = global_coding_ptrs[index-k];
          all_blocks[i*(r+1)+ii] = global_coding_ptrs[index-k];
        }
        }
      calculate_parity_ptrs_rs(local_group, remain, local_coding_ptrs, 1, block_size);
      all_blocks[n-1] = local_coding_ptrs[0];


    }

  }
  gettimeofday(&end_time, NULL);
  encoding_latency = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  //time[1] = encoding_latency;
  //fprintf(stdin, "%s encoding_latency: %.6lf s\n",key, encoding_latency);
 /*-------------rack grop-----------*/
  //uint32_t server_key_list[n+1];
  ip2server_key_st p;
  uint32_t server_key_temp;
  
  if(!ptr->rack_is_add){
    add_rack(ptr);
    for(uint32_t i = 0;i < n; i++)
    {
      p = ptr->rack_group[ptr->node2rack[i]];
      ptr->node_rack[i] = ptr->node2rack[i];
      while(p!= NULL)
      {
        if(!p->use){
          ptr->node2rack[i] = p->ip_server_key;
          p->use = true;
          break;
        }
        p = p->next;
      }
      
    }
    for (uint32_t i = 0; i < n; i++){
    printf("ptr->node2rack[%d]:%d\n",i,ptr->node2rack[i]);
    }
  }
  ptr->rack_is_add = true;
  /*
  ip2server_key_st p;
  for (uint32_t i = 0; i < rack_number; i++)
  {
    uint32_t index_in_rack = 0;
    p = ptr->rack_group[i];
    while (p!= NULL)
    {
      server_key_list[i*(ptr->number_of_r+1)+index_in_rack] = p->ip_server_key;
      printf("p->port:%d\n,p->ip_server_key:%d\n", p->port, p->ip_server_key);
      p = p->next;
      index_in_rack++;
    }
    printf("rack\n");
  }
  */
  
  
  // generate new keys for each block
  // Note, we cannot use the same key for each block, since multiple blocks of 
  // a same item would fall into one physical machine.


  

  // generate k+m requests for k+m data and parity blocks
  uint32_t last_data_index = k+ uint32_t((k-1)/r)-1;
  uint32_t storage_key = server_key;
  uint32_t flag;
  size_t last_data_request_size = value_length - block_size * (k - 1);
  char *last_data_request = new char[last_data_request_size];
  memcpy(last_data_request, data_ptrs[k-1], last_data_request_size);
  
  memcached_instance_st *network_core_instance = memcached_instance_fetch(ptr, ptr->network_core_server_key);
  uint32_t num_cross_rack = 0;
  size_t * bs = NULL;
  uint32_t *fl = NULL;
  memcached_return_t *error1 = NULL;
  /*
  ptr->is_core = true;
  char *result = memcached_get_by_key(ptr, NULL, 0, key_ptrs[0], key_length+2, bs,
                              fl, error1);
  ptr->is_core =false;  
                           
  printf("cross core  key%s \n value%s\n",key,result);
  */ 
  uint32_t cross_rack = 0;
  uint32_t first_rack = ptr->node_rack[0];

//  int client_socket = create_socket();
//  int listen_socket = start_listen(ptr,client_socket);

  gettimeofday(&start_time, NULL);
  if(ptr->encoding_scheme == 1){
    for(uint32_t i = 0; i < n; ++i){
    if(i%(r+1)==0){
      first_rack = ptr->node_rack[i];
    }
    if(first_rack != ptr->node_rack[i]){
        cross_rack = 1;
    }
    if((i+1)%(r+1)!=0 && (i+1)!=n){ 
      //printf("key_ptrs[i]:%s\n",key_ptrs[i]);  
      cross_networkcore_libmemcached(ptr,key_ptrs[i],key_length+2,
                                 all_blocks[i], block_size);
      //printf("encoding local parity\n");
    }else {
      //printf("key_ptrs[i]:%s\n",key_ptrs[i]);
      if(cross_rack==1){
      cross_networkcore_libmemcached(ptr,key_ptrs[i],key_length+2,
                                 all_blocks[i], block_size);
      //printf("encoding\n");
        }
      }
    }
  }else{
    for(uint32_t i = 0; i < n; ++i){
      cross_networkcore_libmemcached(ptr,key_ptrs[i],key_length+2,
                                 all_blocks[i], block_size);
    }

  }
  

//  close_connect(client_socket,listen_socket);
  for(uint32_t i = 0; i < n; ++i) {
    storage_key = ptr->node2rack[i];
    /*
    printf("storage_key:%d\n",storage_key);
    printf("all_blocks------%s\n",all_blocks[i]);
  */
    memcached_instance_st *instance = memcached_instance_fetch(ptr, storage_key);
    
    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 2 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    if(i != last_data_index) {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 2 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));
    } else {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 2 + memcached_array_size(ptr->_namespace) + last_data_request_size +
                                                            request.message.header.request.extlen));
    }

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    if(i != last_data_index) {
      libmemcached_io_vector_st vector[] = 
	  {
        { NULL, 0 },
        { request.bytes, send_length },
        { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
        { key_ptrs[i], key_length+2 },
        { all_blocks[i], block_size }
      };
	  // write the block
      memcached_return_t rc;
      if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
      {
        memcached_io_reset(instance);
        #if 0
          if (memcached_has_error(ptr))
          {
            memcached_set_error(*instance, rc, MEMCACHED_AT);
          }
        #endif

        assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
        return memcached_last_error(instance->root);
      }
    } else {
      libmemcached_io_vector_st vector[] = 
	  {
        { NULL, 0 },
        { request.bytes, send_length },
        { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
        { key_ptrs[i], key_length+2 },
        { last_data_request, last_data_request_size }
      };
      // write the block
      memcached_return_t rc;
      if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
      {
        memcached_io_reset(instance);
        #if 0
          if (memcached_has_error(ptr))
          {
            memcached_set_error(*instance, rc, MEMCACHED_AT);
          }
        #endif

        assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
        return memcached_last_error(instance->root);
      }
    }
  } 
  
  gettimeofday(&end_time, NULL);
  transmission_latency = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  //time[2] = transmission_latency;
  //memcached_return_t *error1 = NULL;
  //cross_networkcore_libmemcached(ptr,key_ptrs[0],key_length+2,all_blocks[0], block_size);
  /*
  ptr->is_core = true;
  *error1 = memcached_send_binary(ptr, network_core_instance, ptr->network_core_server_key,
                              key_ptrs[k-1], key_length + 2,
                              all_blocks[k-1], block_size, expiration,
                              flags, cas, flush, reply, verb);
  if(*error1 != MEMCACHED_SUCCESS) {
    printf("cross_core failure!");
  } 
  char *cross_core;
  cross_core = new char[block_size];                       
  cross_core = memcached_get_by_key(ptr, NULL, 0, key_ptrs[k-1], key_length+2, bs,
                              fl, error1);
  ptr->is_core = false;
  printf("cross_core_rrrrrr:\n",cross_core);
  delete cross_core;
  */
  
  
  for(uint32_t i = 0;i < k;i++){
    delete data_ptrs[i];
  }
  delete data_ptrs;
  for(uint32_t i = 0;i < g;i++){
    delete global_coding_ptrs[i];
  }
  delete global_coding_ptrs;

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }
   /*
  struct timeval start_time, end_time;
 
  double write_time_wwwwwwww = 0.0;
  gettimeofday(&start_time, NULL);
  memcached_return_t wwwwww = memcached_response(server, NULL, 0, NULL);
  gettimeofday(&end_time, NULL);
  write_time_wwwwwwww = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  fprintf(stdin, "~~~~~~ lrc flat write time: %.6lf s\n", write_time_wwwwwwww);
  */
  return memcached_response(server, NULL, 0, NULL);
}
void *get_encoding_time(double *tree_time){
  tree_time[0] = divide_latency;
  tree_time[1] = encoding_latency;
  tree_time[2] = transmission_latency;
}
static inline memcached_return_t memcached_send(memcached_st *shell,
                                                const char *group_key, size_t group_key_length,
                                                const char *key, size_t key_length,
                                                const char *value, size_t value_length,
                                                const time_t expiration,
                                                const uint32_t flags,
                                                const uint64_t cas,
                                                memcached_storage_action_t verb)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }

  if (memcached_failed(memcached_key_test(*ptr, (const char **)&key, &key_length, 1)))
  {
    return memcached_last_error(ptr);
  }

  //uint32_t ori_server_key= memcached_generate_hash_with_redistribution(ptr, group_key, group_key_length);
  //printf("storage.cc:911=====ori_server_key=%d\n", ori_server_key);//FIXME:
  uint32_t server_key= ptr->node2rack[ptr->fail_node_index];//(ptr, group_key, group_key_length);
  //FIXME:
  memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

  WATCHPOINT_SET(instance->io_wait_count.read= 0);
  WATCHPOINT_SET(instance->io_wait_count.write= 0);

  bool flush= true;
  if (memcached_is_buffering(instance->root) and verb == SET_OP)
  {
    flush= false;
  }

  bool reply= memcached_is_replying(ptr);

  hashkit_string_st* destination= NULL;

  if (memcached_is_encrypted(ptr))
  {
    if (can_by_encrypted(verb) == false)
    {
      return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT, 
                                 memcached_literal_param("Operation not allowed while encyrption is enabled"));
    }

    if ((destination= hashkit_encrypt(&ptr->hashkit, value, value_length)) == NULL)
    {
      return rc;
    }
    value= hashkit_string_c_str(destination);
    value_length= hashkit_string_length(destination);
  }

  if (memcached_is_binary(ptr))
  {
    if(verb == SET_OP and ptr->number_of_n > 0 and ptr->number_of_k >0 and ptr->number_of_r >0 and ptr->number_of_b > 0 and !ptr->is_core)
    {
      rc = memcached_lrc_send_binary(ptr, instance, server_key,
                                    key, key_length,
                                    value, value_length, expiration,
                                    flags, cas, flush, reply, verb);
      }
      else if(ptr->is_core){
        instance = memcached_instance_fetch(ptr, ptr->network_core_server_key);
        server_key = ptr->network_core_server_key;
          //printf("storage.cc:997  is_core \n new_server_key=%d\n", server_key);
        rc = memcached_send_binary(ptr, instance, server_key,
                                    key, key_length,
                                    value, value_length, expiration,
                                    flags, cas, flush, reply, verb);
      }
      else{
      //printf("storage.cc:1004  by_key \n new_server_key=%d\n", server_key);
      rc= memcached_send_binary(ptr, instance, server_key,
                                key, key_length,
                                value, value_length, expiration,
                                flags, cas, flush, reply, verb);
    }
  }
  else
  {
    rc= memcached_send_ascii(ptr, instance,
                             key, key_length,
                             value, value_length, expiration,
                             flags, cas, flush, reply, verb);
  }

  hashkit_string_free(destination);

  return rc;
}


memcached_return_t memcached_set(memcached_st *ptr, const char *key, size_t key_length,
                                 const char *value, size_t value_length,
                                 time_t expiration,
                                 uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_SET_START();
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, SET_OP);
  LIBMEMCACHED_MEMCACHED_SET_END();
  return rc;
}

memcached_return_t memcached_add(memcached_st *ptr,
                                 const char *key, size_t key_length,
                                 const char *value, size_t value_length,
                                 time_t expiration,
                                 uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_ADD_START();
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, ADD_OP);

  LIBMEMCACHED_MEMCACHED_ADD_END();
  return rc;
}

memcached_return_t memcached_replace(memcached_st *ptr,
                                     const char *key, size_t key_length,
                                     const char *value, size_t value_length,
                                     time_t expiration,
                                     uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_REPLACE_START();

  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, REPLACE_OP);
  LIBMEMCACHED_MEMCACHED_REPLACE_END();
  return rc;
}

memcached_return_t memcached_prepend(memcached_st *ptr,
                                     const char *key, size_t key_length,
                                     const char *value, size_t value_length,
                                     time_t expiration,
                                     uint32_t flags)
{
  memcached_return_t rc;
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, PREPEND_OP);
  return rc;
}

memcached_return_t memcached_append(memcached_st *ptr,
                                    const char *key, size_t key_length,
                                    const char *value, size_t value_length,
                                    time_t expiration,
                                    uint32_t flags)
{
  memcached_return_t rc;
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, APPEND_OP);
  return rc;
}

memcached_return_t memcached_cas(memcached_st *ptr,
                                 const char *key, size_t key_length,
                                 const char *value, size_t value_length,
                                 time_t expiration,
                                 uint32_t flags,
                                 uint64_t cas)
{
  memcached_return_t rc;

  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, cas, CAS_OP);
  return rc;
}

memcached_return_t memcached_set_by_key(memcached_st *ptr,
                                        const char *group_key,
                                        size_t group_key_length,
                                        const char *key, size_t key_length,
                                        const char *value, size_t value_length,
                                        time_t expiration,
                                        uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_SET_START();

  rc= memcached_send(ptr, group_key, group_key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, SET_OP);
  LIBMEMCACHED_MEMCACHED_SET_END();
  return rc;
}

memcached_return_t memcached_add_by_key(memcached_st *ptr,
                                        const char *group_key, size_t group_key_length,
                                        const char *key, size_t key_length,
                                        const char *value, size_t value_length,
                                        time_t expiration,
                                        uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_ADD_START();

  rc= memcached_send(ptr, group_key, group_key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, ADD_OP);
  LIBMEMCACHED_MEMCACHED_ADD_END();
  return rc;
}

memcached_return_t memcached_replace_by_key(memcached_st *ptr,
                                            const char *group_key, size_t group_key_length,
                                            const char *key, size_t key_length,
                                            const char *value, size_t value_length,
                                            time_t expiration,
                                            uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_REPLACE_START();

  rc= memcached_send(ptr, group_key, group_key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, REPLACE_OP);
  LIBMEMCACHED_MEMCACHED_REPLACE_END();
  return rc;
}

memcached_return_t memcached_prepend_by_key(memcached_st *ptr,
                                            const char *group_key, size_t group_key_length,
                                            const char *key, size_t key_length,
                                            const char *value, size_t value_length,
                                            time_t expiration,
                                            uint32_t flags)
{

  return memcached_send(ptr, group_key, group_key_length,
                        key, key_length, value, value_length,
                        expiration, flags, 0, PREPEND_OP);
}

memcached_return_t memcached_append_by_key(memcached_st *ptr,
                                           const char *group_key, size_t group_key_length,
                                           const char *key, size_t key_length,
                                           const char *value, size_t value_length,
                                           time_t expiration,
                                           uint32_t flags)
{

  return memcached_send(ptr, group_key, group_key_length,
                        key, key_length, value, value_length,
                        expiration, flags, 0, APPEND_OP);
}

memcached_return_t memcached_cas_by_key(memcached_st *ptr,
                                        const char *group_key, size_t group_key_length,
                                        const char *key, size_t key_length,
                                        const char *value, size_t value_length,
                                        time_t expiration,
                                        uint32_t flags,
                                        uint64_t cas)
{
  return  memcached_send(ptr, group_key, group_key_length,
                         key, key_length, value, value_length,
                         expiration, flags, cas, CAS_OP);
}

