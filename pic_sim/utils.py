import math

def cal(code):
    #print("===================")
    #print(code)
    
    n = code[0]
    k = code[1]
    r = code[2]
    if(n%(r+1)==1):
        print("not optimal!")
    g = n - k -math.ceil(n/(r+1))
    gl = math.ceil(n/(r+1))-math.ceil(k/r)
    d = 0
    if(math.ceil((g+1)/r)>gl):
        d = g + gl + 2
    else:
        d = g + gl + 1
    #print(d)
    return d
