#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <libmemcached/memcached.h>

using namespace std;

double find_min(double* temp,int n){
  double temp_time = temp[0];
  for(uint32_t i = 0;i<n;i++){
    if(temp_time > temp[i]){
      temp_time = temp[i];
    }
  }
  return temp_time;
}
double find_max(double* temp,int n){
  double temp_time = temp[0];
  for(uint32_t i = 0;i<n;i++){
    if(temp_time < temp[i]){
      temp_time = temp[i];
    }
  }
  return temp_time;
}
void lrc_upload_download_test(string key, int value_size, string input_file, string output_file, uint64_t n, uint64_t k, uint64_t r, uint64_t b, uint64_t rank_number) {
  memcached_st* memc;
  memcached_return_t rc;
  memcached_server_list_st servers;
  time_t expiration = 0;
  uint32_t flags = 0;
  memc = memcached_create(NULL);
  servers = memcached_server_list_append(NULL, "10.0.0.54", 12111, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.54", 12112, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.54", 12113, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.54", 12114, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.54", 12123, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.54", 12124, &rc);
  
  servers = memcached_server_list_append(servers, "10.0.0.55", 12115, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.55", 12116, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.55", 12117, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.55", 12118, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.55", 12125, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.55", 12126, &rc);
  
  servers = memcached_server_list_append(servers, "10.0.0.56", 12119, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.56", 12120, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.56", 12121, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.56", 12122, &rc);
  servers = memcached_server_list_append(servers, "10.0.0.53", 8888, &rc);
  
  rc = memcached_server_push(memc, servers);
  if(rc != MEMCACHED_SUCCESS) {
    cout<<"Add servers failure!"<<endl;
  }
  memcached_server_list_free(servers);
  // change behavior, MEMCACHED_BEHAVIOR_DISTRIBUTION
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
  // change behavior, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, (uint64_t)1);
  // set k, m, s
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_N, n);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_K, k);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_R, r);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_B, b);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_RACK, r);

  double write_time = 0.0;
  double repair_time = 0.0;
  double delete_time = 0.0;
  double max_time = 0.0;
  double min_time = 0.0;
  double temp_time = 0.0;
  struct timeval start_time, end_time;
  struct timeval start_time_min, end_time_min;
  size_t key_length = key.length();
  char* value = new char[value_size];
  FILE* fp = fopen(input_file.c_str(), "r");
  fread(value, 1, value_size, fp);
  fclose(fp);


  printf("------------------------------------\n");
  size_t value_length = value_size;
  string new_key;
  uint32_t number = 10;
  char **repair = new char*[n];
  double **tree_time_our = new double*[number];
  double **tree_time_default = new double*[number];
  double sum_divide_latency = 0.0;
  double sum_encoding_latency = 0.0;
  double sum_transmission_latency = 0.0;
  memcached_set(memc, key.c_str(), key.length(), value, value_length, expiration, flags);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ENCODING_SCHEME, 0);
  printf("--------------------------------\n");
  gettimeofday(&start_time, NULL);
  temp_time = 0.0;
  for (uint32_t i = 0;i<number;i++){
    gettimeofday(&start_time_min, NULL);
    new_key = "lrc_big_obj_kk" + to_string(value_size);
    rc = memcached_set(memc, new_key.c_str(), new_key.length(), value, value_length, expiration, flags);   
    tree_time_default[i] = new double[3];
    get_encoding_time(tree_time_default[i]);
    gettimeofday(&end_time_min, NULL);
    temp_time = end_time_min.tv_sec-start_time_min.tv_sec+(end_time_min.tv_usec-start_time_min.tv_usec)*1.0/1000000;
    if(temp_time>max_time){
      max_time = temp_time;
    }
    if(temp_time < min_time or i == 0){
      min_time = temp_time;
    }
  }
  gettimeofday(&end_time, NULL);

  write_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  fprintf(stderr, "default lrc basic write time: %.6lf s\n", write_time/number);
  fprintf(stderr, "default lrc basic write time(min): %.6lf s\n", min_time);
  fprintf(stderr, "default lrc basic write time(max): %.6lf s\n", max_time);
  

  double temp_list[number*n];
  for (uint32_t i = 0;i<number;i++){
    sum_divide_latency = sum_divide_latency + tree_time_default[i][0];
    temp_list[i] = tree_time_default[i][0];
  }
  fprintf(stderr, "default_divide_latency: %.6lf s\n", sum_divide_latency/number);
  fprintf(stderr, "default_divide_latency(min): %.6lf s\n", find_min(temp_list,number));
  fprintf(stderr, "default_divide_latency(max): %.6lf s\n", find_max(temp_list,number));

  for (uint32_t i = 0;i<number;i++){
    sum_encoding_latency = sum_encoding_latency + tree_time_default[i][1];
    temp_list[i] = tree_time_default[i][1];
  }
  fprintf(stderr, "default_encoding_latency: %.6lf s\n", sum_encoding_latency/number);
  fprintf(stderr, "default_encoding_latency(min): %.6lf s\n", find_min(temp_list,number));
  fprintf(stderr, "default_encoding_latency(max): %.6lf s\n", find_max(temp_list,number));

  for (uint32_t i = 0;i<number;i++){
    sum_transmission_latency = sum_transmission_latency + tree_time_default[i][2];
    temp_list[i] = tree_time_default[i][2];
  }
  fprintf(stderr, "default_transmission_latency: %.6lf s\n", sum_transmission_latency/number);
  fprintf(stderr, "default_transmission_latency(min): %.6lf s\n", find_min(temp_list,number));
  fprintf(stderr, "default_transmission_latency(max): %.6lf s\n", find_max(temp_list,number));


  for(uint32_t j = 0;j<number;j++){
      new_key = "lrc_big_obj_kk" + to_string(value_size);
      for(uint32_t i = 0;i<n;i++){
  	      memcached_delete_by_key_lrc(memc,new_key.c_str(), new_key.length(),new_key.c_str(), new_key.length(),0,i);
        }
  }
   
  if(rc == MEMCACHED_SUCCESS) {
    cout<<"Save data:"<<key<<endl;
  }

  
  
  sum_divide_latency = 0.0;
  sum_encoding_latency = 0.0;
  sum_transmission_latency = 0.0;
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ENCODING_SCHEME, 1);
  gettimeofday(&start_time, NULL);
  for (uint32_t i = 0;i<number;i++){
    gettimeofday(&start_time_min, NULL);
    //new_key = "lrc_big_obj_k" + to_string(value_size);
    rc = memcached_set(memc, key.c_str(), key.length(), value, value_length, expiration, flags);
    tree_time_our[i] = new double[3];
    get_encoding_time(tree_time_our[i]);
    gettimeofday(&end_time_min, NULL);
    temp_list[i] = end_time_min.tv_sec-start_time_min.tv_sec+(end_time_min.tv_usec-start_time_min.tv_usec)*1.0/1000000;
    
  }
  gettimeofday(&end_time, NULL);

  write_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  printf("--------------------------------\n");
  fprintf(stderr, "our lrc basic write time: %.6lf s\n", write_time/number);
  fprintf(stderr, "our lrc basic write time(min): %.6lf s\n", find_min(temp_list,number));
  fprintf(stderr, "our lrc basic write time(max): %.6lf s\n", find_max(temp_list,number));

  for (uint32_t i = 0;i<number;i++){
    sum_divide_latency = sum_divide_latency + tree_time_our[i][0];
    temp_list[i] = tree_time_our[i][0];
  }
  fprintf(stderr, "our_divide_latency: %.6lf s\n", sum_divide_latency/number);
  fprintf(stderr, "our_divide_latency(min): %.6lf s\n", find_min(temp_list,number));
  fprintf(stderr, "our_divide_latency(max): %.6lf s\n", find_max(temp_list,number));
  for (uint32_t i = 0;i<number;i++){
    sum_encoding_latency = sum_encoding_latency + tree_time_our[i][1];
    temp_list[i] = tree_time_our[i][1];
  }
  fprintf(stderr, "our_encoding_latency: %.6lf s\n", sum_encoding_latency/number);
  fprintf(stderr, "our_encoding_latency(min): %.6lf s\n", find_min(temp_list,number));
  fprintf(stderr, "our_encoding_latency(max): %.6lf s\n", find_max(temp_list,number));

  for (uint32_t i = 0;i<number;i++){
    sum_transmission_latency = sum_transmission_latency + tree_time_our[i][2];
    temp_list[i] = tree_time_our[i][2];
  }
  fprintf(stderr, "our_transmission_latency: %.6lf s\n", sum_transmission_latency/number);
  fprintf(stderr, "our_transmission_latency(min): %.6lf s\n", find_min(temp_list,number));
  fprintf(stderr, "our_transmission_latency(max): %.6lf s\n", find_max(temp_list,number));

  
 

  gettimeofday(&start_time, NULL);
  //DRC
  for(uint32_t j = 0;j<number;j++){
      gettimeofday(&start_time_min, NULL);
      for(uint32_t i = 0;i<n;i++){
          //new_key = "lrc_big_obj_k" + to_string(value_size);
  	      //memcached_delete_by_key_lrc(memc,key.c_str(),key.length(),key.c_str(), key.length(),0,i);
	        memcached_repair(memc, key.c_str(), key.length(), value_length, &flags, &rc,i);
  	      //repair[i] = new char[value_length];
          //repair[i] = memcached_get(memc, new_key.c_str(), new_key.length(), &value_length, &flags, &rc);
	//printf("repair:%s      i:%d    \n",repair[i],i);
        }
      gettimeofday(&end_time_min, NULL);
      temp_list[j] = end_time_min.tv_sec-start_time_min.tv_sec+(end_time_min.tv_usec-start_time_min.tv_usec)*1.0/1000000;
  }
  gettimeofday(&end_time, NULL);
  
  repair_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  printf("--------------------------------\n");
  fprintf(stderr, "lrc basic NRC time: %.6lf s\n", repair_time/(k*number));
  fprintf(stderr, "lrc basic NRC time(min): %.6lf s\n", find_min(temp_list,number)/k);
  fprintf(stderr, "lrc basic NRC time(max): %.6lf s\n", find_max(temp_list,number)/k);

  gettimeofday(&start_time, NULL);
  //DRC
  for(uint32_t j = 0;j<number;j++){
      gettimeofday(&start_time_min, NULL);
      for(uint32_t i = 0;i<k;i++){
          uint32_t index_iii = i+uint32_t(i/r); 
          //new_key = "lrc_big_obj_k" + to_string(value_size);
  	      //memcached_delete_by_key_lrc(memc,key.c_str(),key.length(),key.c_str(), key.length(),0,index_iii);
	        memcached_repair(memc, key.c_str(), key.length(), value_length, &flags, &rc,index_iii);
  	      //repair[i] = new char[value_length];
          //repair[i] = memcached_get(memc, new_key.c_str(), new_key.length(), &value_length, &flags, &rc);
	//printf("repair:%s      i:%d    \n",repair[i],i);
        }
      gettimeofday(&end_time_min, NULL);
      temp_list[j] = end_time_min.tv_sec-start_time_min.tv_sec+(end_time_min.tv_usec-start_time_min.tv_usec)*1.0/1000000;
  }
  gettimeofday(&end_time, NULL);
  
  repair_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  printf("--------------------------------\n");
  fprintf(stderr, "lrc basic DRC time: %.6lf s\n", repair_time/(k*number));
  printf("%d\n",k);
  fprintf(stderr, "lrc basic DRC time(min): %.6lf s\n", find_min(temp_list,number)/k);
  fprintf(stderr, "lrc basic DRC time(max): %.6lf s\n", find_max(temp_list,number)/k);


  gettimeofday(&start_time, NULL);
  for(uint32_t j = 0;j<number;j++){
      new_key = "lrc_big_obj_k" + to_string(value_size);
      for(uint32_t i = 0;i<n;i++){
  	      memcached_delete_by_key_lrc(memc,key.c_str(), key.length(),key.c_str(), key.length(),0,i);
        }
  }
  gettimeofday(&end_time, NULL);
  printf("--------------------------------\n");
  delete_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  fprintf(stderr, "lrc basic delete time: %.6lf s\n",(delete_time)/(k*number));

  
  


  
  /*
    uint32_t index = 0;
  char **result = new char*[k];
  for(uint32_t i = 0;i<k;i++){
  	index = i + int(i/r);
  	memcached_delete_by_key_lrc(memc,key.c_str(), key_length,key.c_str(), key_length,expiration,index);
  	result[i] = new char[value_length];
  	result[i] = memcached_get(memc, key.c_str(), key_length, &value_length, &flags, &rc);
  
  }
 */
  repair[0] = new char[value_length];
  //repair[0] = memcached_get(memc, key.c_str(), key_length, &value_length, &flags, &rc);
  if(rc == MEMCACHED_SUCCESS || MEMCACHED_END) {
    cout<<"Get value length: "<<value_length<<endl;
    FILE* fp2 = fopen(output_file.c_str(), "w");
    /*
    for(uint32_t i = 0;i<k;i++){
    fwrite(result[i], 1, value_length, fp2);
    }
    */

    fwrite(repair[0], 1, value_length, fp2);

    fclose(fp2);
  } else {
    cout<<"Get error!"<<endl;
  }
  
  
  //fprintf(stderr, "lrc basic read time: %.6lf s\n", read_time);
  
    memcached_free(memc);

  //delete value;
}

int main(int argc, char** argv) {

  if(argc != 7) {
    cout<<"./mytest n? k r b rank__number !"<<endl;
    cout<<"n"<<endl;
	cout<<"k"<<endl;
    cout<<"r"<<endl;
    cout<<"b"<<endl;
    cout<<"rank_number"<<endl;
    //cout<<"./test_libmem replication number_of_replicas value_size en p0 !"<<endl;
    exit(1);
  }

  int n;
  int k;
  int r;
  int b;
  int rank_number;
  int value_size;
  n = atoi(argv[1]);
  k = atoi(argv[2]);
  r = atoi(argv[3]);
  b = atoi(argv[4]);
  rank_number = atoi(argv[5]);
  value_size = atoi(argv[6]);
  
  string key;
  string input_file;
  string output_file;
  key = "lrc_big_obj_k" + to_string(value_size);
  input_file = "./input_item_" + to_string(value_size) + "K_LRC";
  output_file = "./output_item_" + to_string(value_size) + "K_LRC";
  cout<<"input_file:"<<input_file<<endl;
  lrc_upload_download_test(key, value_size*1024, input_file, output_file, n, k, r, b, rank_number);
  int value_size_list[20];
  value_size_list[0] = 1;
  value_size_list[1] = 4;
  value_size_list[2] = 16;
  value_size_list[3] = 256;
  value_size_list[4] = 1024;
  value_size_list[5] = 4096;
/*
  for(uint32_t i = 0;i<6;i++){
    value_size = value_size_list[i];
    printf("value_size:%d\n",value_size);
    
  }
*/
  /*
  int k, m, s;
  
  if(strcmp(argv[5], "p0") == 0) {
    k = 2; m = 1; s = 3;
  }
  else if(strcmp(argv[5], "p1") == 0) {
    k = 4; m = 1; s = 5;
  }
  else if(strcmp(argv[5], "p2") == 0) {
    k = 5; m = 1; s = 6;
  }
*/

  
 
  return 1;
}



