import math
from utils import cal
import random
def random_gen(code):
    random.seed(10)
    #print("======================Random=======================")
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
    #rack的数量可能是从ceil(n/b)到n个
    count = 0
    rank = []
    for i in range(n):
        #rank.append([])
        rank_number = random.randint(1,b+1)
        if((count+rank_number)>n):
            rank_number = n - count
        rank_i = []
        for ek in range(rank_number):
            node_rand = random.choice(node)
            rank_i.append(node_rand)
            node.remove(node_rand)
        rank.append(rank_i)
        count = count + rank_number
        if(count == n):
            break

    #print("rank")
    #print(rank)
    #print('--------------------------------------------------------')
    #每一个组里面的每一个node,只要找自己的组所在的rank的数量就可以了
    #如何确定一个组的数据？
    dict = {}
    for one_node in node_stable:
        #print(one_node[0])
        id = int(one_node[1])
        if(one_node[0]=='D'):#数据块儿所属的组别编号   
            dict[one_node] = math.floor(int(one_node[1])/r)
        if(one_node[0]=='L'): #本地数据块儿直接就是id
            dict[one_node]=int(one_node[1])
        if(one_node[0]=='G'):
            #print(id)
            #print((id+k+1)/r)
            dict[one_node] = math.floor((id+k)/r)
    #print('dict')
    #print(dict)

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
    #print('DRC=',DRC)
    #print('NRC=',NRC)
    #print('cost')
    #print(cost)
    return DRC,NRC