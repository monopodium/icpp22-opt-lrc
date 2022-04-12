import math

def azure_LRC_1(code,debug):
    n = code[0]
    #数据块儿的数量k
    k = code[1]
    r = code[2]
    b = n-k-math.ceil(k/r)
    local_node = math.ceil(k/r) + 1
    node = []
    for i in range(k):
        node_id = 'D'
        node_id = node_id + str(i)
        node.append(node_id)
    for i in range(local_node):
        node_id = 'L'
        node_id = node_id + str(i)
        node.append(node_id)
    global_node = n - local_node - k
    if(global_node<1):
        return None,None
    for i in range(global_node):
        node_id = 'G'
        node_id = node_id + str(i)
        node.append(node_id)
    node_stable = node.copy()

    rank = []
    group_count = math.ceil(k/r)+1
    dict = {}
    for one_node in node_stable:
        #print(one_node[0])
        if(one_node[0]=='D'):#数据块儿所属的组别编号   
            if(len(one_node)>2):
                dict[one_node] = math.floor((int(one_node[1])*10+int(one_node[2]))/r)
            else:
                dict[one_node] = math.floor(int(one_node[1])/r)
        if(one_node[0]=='L'): #本地数据块儿直接就是id
            if(len(one_node)>2):
                dict[one_node]=int(one_node[1])*10 + int(one_node[2])
            else:
                dict[one_node]=int(one_node[1])
        if(one_node[0]=='G'):
            #print(id)
            #print((id+k+1)/r)
            dict[one_node] = math.ceil(k/r)#math.floor((id+k)/r)
    rank_group = []
    for i in range(group_count):
        rank_group.append([])
    
    for each_node in node:
        rank_group[dict[each_node]].append(each_node)
    #print(rank_group)
    rank = []
    for each_group in rank_group:
        if(b>=len(each_group)):
            if(b>=2*(r+1)):
                print("warning!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
            rank.append(each_group)
        else:
            #print(b)
            div_group = math.ceil(len(each_group)/b)
            for eg in range(0,len(each_group),b):
                #print(each_group[eg:eg+b])
                rank.append(each_group[eg:eg+b])
    #print(rank)
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
    if(debug):
        print(rank_group)
        print(rank)
        print(cost)
    return DRC,NRC