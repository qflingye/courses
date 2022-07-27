#include <iostream>
#include <string>
#include <cmath>
#include<stdio.h>
#define u32 unsigned int
using namespace std;

static u32 IV[8] =
{
	0x7380166F,	0x4914B2B9,
	0x172442D7,	0xDA8A0600,
	0xA96F30BC,	0x163138AA,
	0xE38DEE4D,	0xB0FB0E4E,
};
//布尔函数 FF、GG
static u32 FF(u32 X, u32 Y, u32 Z, u32 j) {
	if (0 <= j && j < 16)
		return (X ^ Y ^ Z);
	else if (16 <= j && j < 64)
		return ((X & Y) | (X & Z) | (Y & Z));
	return 0;
}
static u32 GG(u32 X, u32 Y, u32 Z, u32 j)
{
	if (0 <= j && j < 16)
		return (X ^ Y ^ Z);
	else if (16 <= j && j < 64)
		return ((X & Y) | ((~X) & Z));
	return 0;
}
//a左移length位
u32 left_move(u32 a, int length) {
	short i;
	for (i = 0; i < length; i++) {
		a = a * 2 + a / 0x80000000;
	}
	return a;
}
//置换函数P0、P1
static u32 P0(u32 X)
{
	return (X ^ (left_move(X, 9)) ^ (left_move(X, 17)));
}

static u32 P1(u32 X)
{
	return (X ^ (left_move(X, 15)) ^ (left_move(X, 23)));
}
//int十进制转换为string十六进制
string DecToHex(int str) {
	string hex = "";
	int temp = 0;
	while (str >= 1) {
		temp = str % 16;
		if (temp < 10 && temp >= 0) {
			hex = to_string(temp) + hex;
		}
		else {
			hex += ('a' + (temp - 10));
		}
		str = str / 16;
	}
	return hex;
}
//char转换为int
int StrToInt(char s) {
	int h = 0;
	if (s >= '0' && s <= '9') {
		h += s - '0';
	}
	else {
		int tt = s - 'a' + 10;
		h += tt;
	}
	return h;
}
//填充
void fill(string m, u32 M[], int s) {//消息，填充后消息,分组数
	string res = "";
	int l = m.size() * 4;
	int n = s * 16;//填充后长度
	res += m;//原有字符
	res += "8";//1
	while (res.size() % 128 != 112) {
		res += "0";//“0”数据填充
	}
	string res_len = DecToHex(l);
	while (res_len.size() != 16) {
		res_len = "0" + res_len;
	}
	res += res_len;
	int i = 0;
	for (int j = 0; j < n * 8; j += 8) {
		int t1 = StrToInt(res[j]);
		int t2 = StrToInt(res[j + 1]);
		int t3 = StrToInt(res[j + 2]);
		int t4 = StrToInt(res[j + 3]);
		int t5 = StrToInt(res[j + 4]);
		int t6 = StrToInt(res[j + 5]);
		int t7 = StrToInt(res[j + 6]);
		int t8 = StrToInt(res[j + 7]);
		M[i] = t1 * 0x10000000 + t2 * 0x1000000 + t3 * 0x100000 + t4 * 0x10000 + t5 * 0x1000 + t6 * 0x100 + t7 * 0x10 + t8 * 0x1;
		i++;
	}
}
//迭代压缩
void CF(u32 M[], u32 V[])
{
	//消息扩展
	u32 W68[68] = { 0 };
	u32 W64[64] = { 0 };
	int j = 0;
	for (j = 0; j < 16; j++)
	{
		W68[j] = M[j];
	}
	for (j = 16; j < 68; j++)
	{
		W68[j] = P1(W68[j - 16] ^ W68[j - 9] ^ (left_move(W68[j - 3], 15))) ^ (left_move(W68[j - 13], 7)) ^ W68[j - 6];
	}
	for (j = 0; j < 64; j++)
	{
		W64[j] = W68[j] ^ W68[j + 4];
	}

	u32 A_G[8] = { 0 };
	for (j = 0; j < 8; j++)
	{
		A_G[j] = V[j];
	}

	//压缩函数
	u32 SS1 = 0, SS2 = 0, TT1 = 0, TT2 = 0;
	int i = 0; int T[64];
	for (; i < 16; i++)
		T[i] = 0x79CC4519;
	for (; i < 64; i++)
		T[i] = 0x7A879D8A;
	for (j = 0; j < 64; j++)
	{
		SS1 = left_move((left_move(A_G[0], 12) + A_G[4] + left_move(T[j], j % 32)), 7);
		SS2 = SS1 ^ (left_move(A_G[0], 12));
		TT1 = FF(A_G[0], A_G[1], A_G[2], j) + A_G[3] + SS2 + W64[j];
		TT2 = GG(A_G[4], A_G[5], A_G[6], j) + A_G[7] + SS1 + W68[j];
		A_G[3] = A_G[2];
		A_G[2] = left_move(A_G[1], 9);
		A_G[1] = A_G[0];
		A_G[0] = TT1;
		A_G[7] = A_G[6];
		A_G[6] = left_move(A_G[5], 19);
		A_G[5] = A_G[4];
		A_G[4] = P0(TT2);
	}
	for (j = 0; j < 8; j++)
	{
		V[j] = A_G[j] ^ V[j];
	}

}
void all(string str,u32 v[],u32 IV[]) {  //SM3算法，输入str，输出v[]
	int length = str.size();//消息长度，字节数（3）
	u32 M[100];//填充后的消息
	int le = (length + 1 + 8) / 64 + 1;//分组数
	fill(str, M, le);//消息，填充后消息，分组数
	for (int i = 0; i < 8; i++) {
		v[i] = IV[i];
	}
	for (int i = 0; i < le; i++) {
		CF(M, v);
	}
}
//长度扩展攻击  已知H(salt)，求 H(salt||data)  salt:after padding
void attack(u32 H[],u32 HH[]) {//输入，输出
	string data;
	data = "80";
	all(data, HH, H);
}
int main() {
	string str = "616263";//输入消息,16进制
	u32 v[8] = { 0 }; u32 v1[8] = { 0 };
	all(str,v,IV);
	//printf("杂凑值：");
	//printf("%08x %08x %08x %08x %08x %08x %08x %08x\n", v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
	//printf("%08x\n", vv);
	attack(v,v1);
	printf("杂凑值：");
	printf("%08x %08x %08x %08x %08x %08x %08x %08x\n", v1[0], v1[1], v1[2], v1[3], v1[4], v1[5], v1[6], v1[7]);
	return 0;
}
