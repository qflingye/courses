#SM3生日攻击
import re
import threading
import random
#几个常量定值
iv='7380166f4914b2b9172442d7da8a0600a96f30bc163138aae38dee4db0fb0e4e'
sstr=['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z']

T = [0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,
     0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,0x79CC4519,
     0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,
     0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,
     0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,
     0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,
     0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,
     0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A,0x7A879D8A]

#布尔函数 FF、GG
def FF(X,Y,Z,j):
    if j < 16:
        return X^Y^Z
    else:
        return (X&Y)|(X&Z)|(Y&Z)

    
def GG(X,Y,Z,j):
    if j < 16:
        return X^Y^Z
    else:
        return (X&Y)|(~X&Z)


#置换函数P0、P1
def P0(X):
    return X ^ (zy(X, 9)) ^ (zy(X, 17))

def P1(X):
    return X ^ (zy(X, 15)) ^ (zy(X, 23))

#a左移x位
def zy(n,k):        
    k=k%32
    b=str(bin(n))
    b=b.split('0b')[1]
    b=(32-len(b))*'0'+b
    return int(b[k:]+b[:k],2)

#切片
def fl(text,ll):  
    M=[]
    for i in range(0, len(text), ll):
        M.append(text[i:i+ll])
    return M

#填充
def ad(s):               
    r = ""
    x = ""
    for i in s:
        l = 8 - len((x + bin(ord(i))).split('0b')[1]) % 8
        r = r + l * '0' + (x + bin(ord(i))).split('0b')[1]
    k=512-(64+(len(r)+1))%512
    out=r+'1'+k*'0'
    length=bin(len(r)).split('0b')[1]
    t=64-len(length)
    out=out+t*'0'+length
    out=fl(out,512)
    return out

#SM3
def cf(v,b):
    w68 = fl(b, 32)
    w64 = []
    for j in range(16):
        w68[j]=int(w68[j],2)
    for j in range(16, 68):
        x = P1(w68[j - 16] ^ w68[j - 9] ^ zy(w68[j - 3] ,15)) ^ zy(w68[j - 13] ,7) ^ w68[j - 6]
        w68.append(x)
    for j in range(64):
        x = w68[j] ^ w68[j + 4]
        w64.append(x)
    A=fl(v,8)
    for i in range(8):
        A[i]=int(A[i],16)
    for j in range(64):
        ss1=zy((zy(A[0],12)+A[4]+zy(T[j],j))%(2**32),7)%(2**32)
        ss2=(ss1^zy(A[0],12))%(2**32)
        tt1=(FF(A[0],A[1],A[2],j)+A[3]+ss2+w64[j])%(2**32)
        tt2=(GG(A[4],A[5],A[6],j)+A[7]+ss1+w68[j])%(2**32)
        A[3]=A[2]
        A[2]=zy(A[1],9)
        A[1]=A[0]
        A[0]=tt1
        A[7]=A[6]
        A[6]=zy(A[5],19)
        A[5]=A[4]
        A[4]=P0(tt2)
    a=''
    for i in range(8):
        A[i]=str(hex(A[i])).split('0x')[1]
        k=8-len(A[i])
        a=a+k*'0'+A[i]
    v1=int(a,16)^int(v,16)
    v1=hex(v1).split('0x')[1]
    if len(v1)<64:
        v1="0"*(64-len(v1))+str(v1)
    return v1


#生成n个字母的小写字符串
def crea(n):
    rs=""
    for i in range(n):
        rs+=sstr[random.randint(0,25)]  
    return rs

#整合
def res1():
    me=crea(4)
    for b in ad(me):
        if b!='':
            v1=cf(iv,b)
        return me,v1[:8]
def tt():
    str1,v1=res1()
    str2,v2=res1()
    while(v1!=v2 or str1==str2):
        str1,v1=res1()
        str2,v2=res1()
    print("生日攻击成功！")

if __name__ ==  '__main__':
    print("32比特")
    t1=threading.Thread(target=tt)
    t2=threading.Thread(target=tt)
    t3=threading.Thread(target=tt)
    t4=threading.Thread(target=tt)
    t1.start()
    t1.join()
    t2.start()
    t2.join()
    t3.start()
    t3.join()
    t4.start()
    t4.join()
