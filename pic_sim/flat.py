import math
def flat(code):
#对于flat来说,每个Node修复都需要r个块(除了最后一组)
#n%(r+1)计算出最后一组的数量
    #print("======================Flat=======================")
    #print(code)
    n = code[0]
    k = code[1]
    r = code[2]
    last = n%(r+1)
    integer_local = n - last
    #print(code)
    #print(integer_local)
    #如果有纯正的全局校验块儿组成的组,那么n/(r+1)上取整会大于k/r上取整
    #总的组数>数据块儿出现的组数
    if(math.ceil(n/(r+1)) > math.ceil(k/r)):
        DRC = r
    else:
        if(n%(r+1)==0):
            DRC = r
        else:
            DRC = ((k-k%r)*r+(k%r)*(n%(r+1)-1))/k
    #print('DRC = ',DRC)
    NRC = 0.0+(integer_local*r+last*(last-1))/k
    #print('NRC = ',NRC) 
    return DRC,NRC