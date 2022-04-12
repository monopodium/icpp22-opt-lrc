import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import copy
def create_multi_bars1(yticks, my_label,labels, datas,data2, tick_step=2, group_gap=0.4, bar_gap=0,nu_list_str = []):
    '''
    labels : x轴坐标标签序列
    datas ：数据集,二维列表,要求列表每个元素的长度必须与labels的长度一致
    tick_step ：默认x轴刻度步长为1,通过tick_step可调整x轴刻度步长。
    group_gap : 柱子组与组之间的间隙,最好为正值,否则组与组之间重叠
    bar_gap ：每组柱子之间的空隙,默认为0,每组柱子紧挨,正值每组柱子之间有间隙,负值每组柱子之间重叠
    '''
    # ticks为x轴刻度
    
    ticks = np.arange(len(labels)) * tick_step
    # group_num为数据的组数,即每组柱子的柱子个数
    group_num = len(datas)
    # group_width为每组柱子的总宽度,group_gap 为柱子组与组之间的间隙。
    group_width = tick_step - group_gap
    # bar_span为每组柱子之间在x轴上的距离,即柱子宽度和间隙的总和
    bar_span = group_width / group_num
    # bar_width为每个柱子的实际宽度
    bar_width = bar_span - bar_gap
    # baseline_x为每组柱子第一个柱子的基准x轴位置,随后的柱子依次递增bar_span即可
    baseline_x = ticks #- bar_span#(group_width - bar_span) / 2
    #print(baseline_x )
    
    co = ["#2878B5","#9AC9DB","#C82423","#F8AC8C"]#["#8A7C6B","#2F4F4F","#CD5B44"]
    i = 0 
    plt_list = []
    for index, y in enumerate(datas):
        x_index = baseline_x + (index-1)*bar_span
        print(x_index)
        print(len(x_index),len(y))
        y1 = copy.deepcopy(y)
        for ii in range(len(y1)):
            if not y1[ii]:
                y1[ii] = 0
        print(y1)
        tmp_bar = plt.bar(x_index, y1, bar_width,color = co[i],edgecolor = "black",linewidth = 1,zorder=0)

        plt_list.append(tmp_bar)
        i = i+1
        for a,b in zip(x_index,y):
            if(b!=None):
                plt.text(a+0.3,b+0.05,np.round(b,decimals=1),ha = 'center', va = 'bottom',fontsize=10)
    
    x_index = baseline_x - bar_span
    tmp_scatter = plt.scatter(x_index, data2[0],s = 80, marker='_',c='k',zorder=1)#,bar_width
    plt_list.append(tmp_scatter)
    x_index = baseline_x
    plt.scatter(x_index, data2[1],s = 80, marker='_',c='k',zorder=1)#,bar_width
    x_index = baseline_x + bar_span
    plt.scatter(x_index, data2[2],s = 80, marker='_',c='k',zorder=1)#,bar_width

    plt.ylabel('Repair Cost (#Blocks)',fontsize = 18)
    plt.xlabel('Coding Parameter',fontsize = 18)
    #plt.title('multi datasets',fontsize = 14)
    ax = plt.gca()

    plt.xlim([-8,248]) # added by aoxuyang.ms@bytedance.com
    ax.spines['left'].set_position(('data',-8))
    ax.spines['right'].set_position(('data',248))
    # x轴刻度标签位置与x轴刻度一致
    ax.set_yticks(yticks)
    plt.xticks(ticks, labels,fontsize = 14)
    plt.xticks(rotation=45,fontsize = 14)
    plt.yticks(fontsize = 14)
    plt.legend(handles=plt_list,labels=my_label,loc='best',fontsize = 14,frameon=False,)
    plt.show()
    plt.savefig("pic1.pdf")

def create_multi_bars(yticks,my_label,labels, datas,data2, tick_step=2, group_gap=0.1, bar_gap=0,nu_list_str = []):
    '''
    labels : x轴坐标标签序列
    datas ：数据集,二维列表,要求列表每个元素的长度必须与labels的长度一致
    tick_step ：默认x轴刻度步长为1,通过tick_step可调整x轴刻度步长。
    group_gap : 柱子组与组之间的间隙,最好为正值,否则组与组之间重叠
    bar_gap ：每组柱子之间的空隙,默认为0,每组柱子紧挨,正值每组柱子之间有间隙,负值每组柱子之间重叠
    '''
    # ticks为x轴刻度
    
    ticks = np.arange(len(labels)) * tick_step
    # group_num为数据的组数,即每组柱子的柱子个数
    group_num = len(datas)
    # group_width为每组柱子的总宽度,group_gap 为柱子组与组之间的间隙。
    group_width = tick_step - group_gap
    # bar_span为每组柱子之间在x轴上的距离,即柱子宽度和间隙的总和
    bar_span = group_width / group_num
    # bar_width为每个柱子的实际宽度
    bar_width = bar_span - bar_gap
    # baseline_x为每组柱子第一个柱子的基准x轴位置,随后的柱子依次递增bar_span即可
    baseline_x = ticks #- bar_span#(group_width - bar_span) / 2
    #print(baseline_x )
    
    co = ["#2878B5","#9AC9DB","#C82423","#F8AC8C"]#["#8A7C6B","#2F4F4F","#CD5B44"]
    i = 0 
    plt_list = []
    for index, y in enumerate(datas):
        x_index = baseline_x + (index-1)*bar_span
        print(x_index)
        print(len(x_index),len(y))
        y1 = copy.deepcopy(y)
        for ii in range(len(y1)):
            if not y1[ii]:
                y1[ii] = 0
        print(y1)
        tmp_bar = plt.bar(x_index, y1, bar_width,color = co[i],edgecolor = "black",linewidth = 1,zorder=0)
        tmp_scatter = plt.scatter(x_index, data2[i],s = 80, marker='_',c='k',zorder=1)#,bar_width
        plt_list.append(tmp_bar)
        i = i+1
        for a,b in zip(x_index,y):
            if(b!=None):
                plt.text(a,b+0.05,np.round(b,decimals=1),ha = 'center', va = 'bottom',fontsize=8)
    
    x_index = baseline_x - bar_span
    
    plt_list.append(tmp_scatter)
    # x_index = baseline_x
    # plt.scatter(x_index, data2[1],s = 80, marker='_',c='k',zorder=1)#,bar_width
    # x_index = baseline_x + bar_span
    # plt.scatter(x_index, data2[2],s = 80, marker='_',c='k',zorder=1)#,bar_width
    
    plt.ylabel('Repair Cost (#Blocks)',fontsize = 18)
    plt.xlabel('Coding Parameter',fontsize = 18)
    #plt.title('multi datasets',fontsize = 14)
    ax = plt.gca()

    plt.xlim([-8,248]) # added by aoxuyang.ms@bytedance.com
    ax.spines['left'].set_position(('data',-8))
    ax.spines['right'].set_position(('data',248))
    ax.set_yticks(yticks)
    # x轴刻度标签位置与x轴刻度一致
    plt.xticks(ticks, labels,fontsize = 14)
    plt.xticks(rotation=45,fontsize = 14)
    plt.yticks(fontsize = 14)
    plt.legend(handles=plt_list,labels=my_label,loc='best',fontsize = 11,frameon=False,)
    plt.show()
    plt.savefig("pic2.pdf")