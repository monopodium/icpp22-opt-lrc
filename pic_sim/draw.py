from random_gen import random_gen
from parameters import XORBAS_NRC_LIST,XORBAS_DRC_LIST,PARAMETERS_DIFF_PLACEMENT,\
    STR_PARAMETERS_DIFF_PLACEMENT,PARAMETERS_DIFF_CODES,STR_PARAMETERS_DIFF_CODES
from flat import flat
from our_placement import our_placement
from Azure_LRC import azure_LRC
from Azure_LRC_1 import azure_LRC_1
from draw_bar import create_multi_bars,create_multi_bars1


def gen_data_diff_placement(random_count = 10):  
    flat_NRC_list = []
    random_NRC_list = []
    our_NRC_list = []
    flat_DRC_list = []
    random_DRC_list = []
    our_DRC_list = []
    for code in PARAMETERS_DIFF_PLACEMENT:
        print("===========================================")
        print(code)
        flat_DRC,flat_NRC = flat(code) 
        print("flat_DRC",flat_DRC)
        print("flat_NRC",flat_NRC)
        print("")
        sum_DRC = 0
        sum_NRC = 0
        for i in range(random_count):
            DRC,NRC = random_gen(code)
            sum_DRC = sum_DRC + DRC
            sum_NRC = sum_NRC + NRC
        print("random_DRC",sum_DRC/random_count)
        print("random_NRC",sum_NRC/random_count)
        print("")
        our_DRC,our_NRC = our_placement(code)
        print("our_DRC",our_DRC)
        print("our_NRC",our_NRC)
        flat_NRC_list.append(flat_NRC)
        random_NRC_list.append(sum_NRC/random_count)
        our_NRC_list.append(our_NRC)
        flat_DRC_list.append(flat_DRC)
        random_DRC_list.append(sum_DRC/random_count)
        our_DRC_list.append(our_DRC)
    data_NRC = [flat_NRC_list,random_NRC_list,our_NRC_list] 
    data_DRC = [flat_DRC_list,random_DRC_list,our_DRC_list]
    return data_NRC,data_DRC
code_temp = [[16,11,4]]
def compare_diff_code():
    azure_DRC_list = []
    azure_NRC_list = []
    optimal_DRC_list = []
    optimal_NRC_list = []
    azure_1_DRC_list = []
    azure_1_NRC_list = []
    i = 0
    for code in PARAMETERS_DIFF_CODES:
        print("-------------------------------")
        print("code",code)
        azure_DRC,azure_NRC = azure_LRC(code,0)
        azure_DRC_list.append(azure_DRC)
        azure_NRC_list.append(azure_NRC)
        optimal_DRC,optimal_NRC = our_placement(code)
        optimal_DRC_list.append(optimal_DRC)
        optimal_NRC_list.append(optimal_NRC)
        azure_1_DRC,azure_1_NRC = azure_LRC_1(code,0)
        azure_1_DRC_list.append(azure_1_DRC)
        azure_1_NRC_list.append(azure_1_NRC)
        #print("azure_DRC ",azure_DRC)
        print("azure_DRC",azure_DRC)
        print("azure_NRC",azure_NRC)
        print("optimal_DRC",optimal_DRC)
        print("optimal_NRC",optimal_NRC)
        print("azure_1_DRC",azure_1_DRC)
        print("azure_1_NRC",azure_1_NRC)
        if XORBAS_DRC_LIST[i]!=None:
            print("XORBAS_DRC",XORBAS_DRC_LIST[i])
            print("XORBAS_NRC",XORBAS_NRC_LIST[i])
        i = i + 1
    data_DRC = [optimal_DRC_list,azure_DRC_list,azure_1_DRC_list,XORBAS_DRC_LIST] 
    data_NRC = [optimal_NRC_list,azure_NRC_list,azure_1_NRC_list,XORBAS_NRC_LIST]
    return data_NRC,data_DRC

r_list = []
for item in PARAMETERS_DIFF_PLACEMENT:
    r_list.append(item[0]/item[1])
    print(item,item[0]/item[1])
print("len(PARAMETERS_DIFF_PLACEMENT)",len(PARAMETERS_DIFF_PLACEMENT))
print("min:",min(r_list))
print("max:",max(r_list))
r_list = []
for item in PARAMETERS_DIFF_CODES:
    r_list.append(item[0]/item[1])
print("len(PARAMETERS_DIFF_CODES)",len(PARAMETERS_DIFF_CODES))
print("min:",min(r_list))
print("max:",max(r_list))

# my_label = ["Flat NRC","Random NRC","R-Opt NRC","DRC"] #Our Placement   
# data_NRC,data_DRC = gen_data_diff_placement(random_count = 10)
# yticks = [2,4,6,8,10,12]
# create_multi_bars1(yticks,my_label,STR_PARAMETERS_DIFF_PLACEMENT,data_NRC,data_DRC, tick_step=10,group_gap=2,bar_gap=0,nu_list_str = STR_PARAMETERS_DIFF_PLACEMENT)

# my_label = ["Opt-LRC NRC","Azure NRC","Azure+1 NRC","Xorbas NRC","DRC"] #Our Placement
# data_NRC,data_DRC = compare_diff_code()
# yticks = [0,0.5,1,1.5,2,2.5,3,3.5,4]
# create_multi_bars(yticks,my_label,STR_PARAMETERS_DIFF_CODES, data_NRC,data_DRC, tick_step=10,group_gap=2,bar_gap=0,nu_list_str = STR_PARAMETERS_DIFF_CODES)