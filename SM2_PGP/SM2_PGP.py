from gmssl import sm2,func
from gmssl.sm4 import CryptSM4, SM4_ENCRYPT, SM4_DECRYPT

#会话秘钥
key = b'zjj0073jing20200'

#SM2公钥和私钥
sk = '00B9AB0B828FF68872F21A837FC303668428DEA11DCD1B24429D0C99E24EED83D5'
pk = 'B9C9A6E04E9C91F7BA880429273747D7EF5DDEB0BB2FF6317EB00BEF331A83081A6994B8993F3F5D6EADDDB81872266C87C018FB4162F5AF347B483E24620207'

#消息
m = b"jingjing202000460073"

print("初始会话秘钥：",key)
print("初始消息：",m)
#初始化
iv = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
sm4_ = CryptSM4()
sm2_ = sm2.CryptSM2(public_key=pk, private_key=sk)


print("-------------------------PGP加密-----------------------")
#先用sm4加密(cbc模式)  
sm4_.set_key(key, SM4_ENCRYPT)
k1 = sm4_.crypt_cbc(iv , m)
print("用会话秘钥加密消息：",k1)
#再用sm2加密  
k2 = sm2_.encrypt(key)
print("用公钥加密会话密钥：",k2)
#sm4加密(cbc模式） 用k2做密钥加密k1
sm4_.set_key(k2, SM4_ENCRYPT)
k3 = sm4_.crypt_cbc(iv , k1)
print("拼合：",k3)

print("-------------------------PGP解密-----------------------")
#sm4解密
sm4_.set_key(k2, SM4_DECRYPT)
s0 = sm4_.crypt_cbc(iv , k3) 
print("分解：",s0)
#先用sm2解密
s1 =sm2_.decrypt(k2)
print("恢复会话秘钥",s1)
#再用sm4解密
sm4_.set_key(s1, SM4_DECRYPT)
s2 = sm4_.crypt_cbc(iv , s0) 
print("用会话秘钥解密消息：",s2)
