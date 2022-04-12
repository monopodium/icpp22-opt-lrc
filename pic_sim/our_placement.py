import math
from utils import cal
from tkinter import Label, _flatten

def our_placement(code):
    #print("======================Our_Placement=======================")
    #print(code)
    n = code[0]
    #数据块儿的数量k
    k = code[1]
    r = code[2]
    #局部校验快儿的数量 = 组数
    node = []
    for i in range(k):
        node_id = 'D'
        node_id = node_id + str(i)
        node.append(node_id)
    local_node = math.ceil(n/(r+1))
    for i in range(local_node):
        node_id = 'L'
        node_id = node_id + str(i)
        node.append(node_id)
    #全局校验块儿的数量 = 总数-局部校验快儿的数量-数据块儿
    global_node = n - local_node - k
    for i in range(global_node):
        node_id = 'G'
        node_id = node_id + str(i)
        node.append(node_id)
    #计算一个粗糙的单个cluster的最大容量
    #print(node)
    node_stable = node.copy()
    b = cal(code) - 1
    '''
    if(n%(r+1)==0):
        b = n - k - math.ceil(k/r)+1
    else:
        b = n - k - math.ceil(k/r)
    '''
    #rack的数量可能是从ceil(n/b)到n个
    count = 0
    rank = []
    group_count = math.ceil(n/(r+1))
    dict = {}
    for one_node in node_stable:
        #print(one_node[0])
        id = int(one_node[1])
        if(one_node[0]=='D'):#数据块儿所属的组别编号
            if(len(one_node)>2):
                dict[one_node] = math.floor((int(one_node[1])*10+int(one_node[2]))/r)
            else:
                dict[one_node] = math.floor(int(one_node[1])/r)   
            #dict[one_node] = math.floor(int(one_node[1])/r)
        if(one_node[0]=='L'): #本地数据块儿直接就是id
            dict[one_node]=int(one_node[1])
        if(one_node[0]=='G'):
            #print(id)
            #print((id+k+1)/r)
            dict[one_node] = math.floor((id+k)/r)
    #print('dict')
    #print(dict)
    rank_group = []
    for i in range(group_count):
        rank_group.append([])
    #print(rank_group)
    for each_node in node:
        rank_group[dict[each_node]].append(each_node)
    #print(rank_group)
    if(b>=r+1):
        each_cluster_count = math.floor((b/(r+1)))
        num_cluster = math.ceil((group_count/each_cluster_count))
        for eg in range(0,group_count,each_cluster_count):
            rank.append(list(_flatten(rank_group[eg:eg+each_cluster_count])))

    if(b<r+1):
        for each_group in rank_group:
            for eg in range(0,len(each_group),b):
                rank.append(each_group[eg:eg+b])
    #print('rank')
    #print(rank)

    #对于每个Node,在其余的rank里面寻找编号相同的,
    cost = {}
    for item in rank:
        for each_node in item:
            cost[each_node] = 0
            rank1 = rank.copy()
            rank1.remove(item)
            id = dict[each_node]
            #print('rank1')
            #print(rank1)
            for other_item in rank1:
                for other_node in other_item:
                    if(dict[other_node]==id):
                        cost[each_node] = cost[each_node]+1
                        break
    sum_DRC = 0
    for DRC_node in node_stable:
        if(DRC_node[0]=='D'):
            sum_DRC = sum_DRC + cost[DRC_node]
    DRC = sum_DRC/k

    sum_NRC = 0
    for NRC_node in node_stable:
        sum_NRC = sum_NRC + cost[NRC_node]
    NRC = sum_NRC/k
    #print('DRC',DRC)
    #print('NRC',NRC)
    #print('cost')
    #print(cost)
    return DRC,NRC 