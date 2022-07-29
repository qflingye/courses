import re
#常量初始化，8个哈希初值，64个哈希常量
H0 = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]

HashConstant = [0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
                0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
                0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
                0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
                0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
                0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
                0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
                0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
                0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2]

#填充，填充到模512为448
def ad(m):
    # 附加长度
    str_l = chr(len(m)<<3)
    m += '\x80'
    m += '\x00' * ((55 - len(m)) % 64)
    str_l = '\x00' * (9 - len(str_l)) + str_l
    m += str_l
    return m
#循环右移x
def shiftRight(a, x):
    return ((a >> x)|(a << (32 - x))) & (2 ** 32 - 1)

    
def SHA256(messag,H0):
    H = H0.copy() # 使用列表的复制
    message=ad(messag)
    for i in range(0, len(message), 64):
        msg = message[i:i+64]
        W = []
        for j in range(0,64,4):
            cut = msg[j:j+4]
            W.append(ord(cut[0])*(256**3)+ord(cut[1])*(256**2)+ord(cut[2])*256+ord(cut[3]))
        W = W + ([0]*48)
        for j in range(16,64):
            s0 = shiftRight(W[j-15],7) ^ shiftRight(W[j-15],18) ^ (W[j-15]>>3)
            s1 = shiftRight(W[j-2],17) ^ shiftRight(W[j-2],19) ^ (W[j-2]>>10)
            W[j] = (W[j-16] + s0 + W[j-7] + s1) &(2**32-1)
        a,b,c,d,e,f,g,h = H
        for i in range(64):
            s0 = (shiftRight(a, 2) ^ shiftRight(a, 13) ^ shiftRight(a, 22)) & ((2**32)-1)
            maj = ((a & b) ^ (a & c) ^ (b & c)) & ((2**32)-1)
            t2 = (s0 + maj) &  ((2**32)-1)
            s1 = (shiftRight(e,6) ^ shiftRight(e,11) ^ shiftRight(e,25)) & ((2**32)-1)
            ch = ((e & f) ^ (~e & g)) & ((2**32)-1)
            t1 = (h + s1 + ch + HashConstant[i] + W[i]) & ((2**32)-1)
            a=(t1+t2) & ((2**32)-1)
            b=a
            c=b
            d=c
            e=(2**32)-1
            f=e
            g=f
            h =g
        temp = [a,b,c,d,e,f,g,h]
        for i in range(8):
            H[i] += temp[i]
            H[i] = H[i] & ((2**32)-1)
    return H

#int转换为十六进制字符串
def int_str(H):
    en = [str(hex(it))[2:] for it in H]
    for i in range(8):
        it = en[i]
        if len(it)<8:
            en[i] = '0'*(8-len(it))+it

    return ''.join(en)
  
#长度扩展攻击，已知H(it)，求H(it+th)
def attack(mm):
    th='abc'
    mee=SHA256(th,mm)
    return int_str(mee)

if __name__ == '__main__':
    it = 'abc'
    mm=SHA256(it,H0)
    print('杂凑值:', int_str(mm))
    print('攻击后:', attack(mm))
    
