#include <iostream>
#include <string>
#include <cmath>
#include <windows.h>
#include<stdio.h>
#include <immintrin.h>
using namespace std;
#define u32 unsigned int
#define u256 __m256i
#define loadu256(A, B)  A = _mm256_loadu_si256((u256 *)(B))
#define storeu256(A, B)  _mm256_storeu_si256((u256 *)(A), B)
#define pxor256(C, A, B)    C = _mm256_xor_si256(A, B)
#define set256_32(a, e7,e6,e5,e4,e3,e2,e1,e0) a = _mm256_set_epi32 (e7,e6,e5,e4,e3,e2,e1,e0)
#define U32From256(A, I) (_mm256_extract_epi32((A), (I)))

static const u32 IV[8] =
{
	0x7380166F,	0x4914B2B9,
	0x172442D7,	0xDA8A0600,
	0xA96F30BC,	0x163138AA,
	0xE38DEE4D,	0xB0FB0E4E,
};

//布尔函数FF、GG
static u32 FF(u32 X, u32 Y, u32 Z, u32 j) {
	u32 a, b, c;
	if (j >= 0 && j < 16)
		return (X ^ Y ^ Z);
	else if (j >= 16 && j < 64)
		a = X & Y;  b = X & Z; c = Y & Z;
	return (a | b | c);
}
static u32 GG(u32 X, u32 Y, u32 Z, u32 j)
{
	u32 a, b;
	if (j >= 0 && j < 16)
		return (X ^ Y ^ Z);
	else if (j >= 16 && j < 64)
		a = X & Y; b = (~X) & Z;
	return (a | b);
}
//左移
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
	u32 a, b;
	a = left_move(X, 9); b = left_move(X, 17);
	return (X ^ a ^ b);
}
static u32 P1(u32 X)
{
	u32 a, b;
	a = left_move(X, 15); b = left_move(X, 23);
	return (X ^ a ^ b);
}

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
	int l = m.size()<<2;
	int n = s<<7;//填充后长度
	res += m;//原有字符
	res += "8";//1
	string res_len = DecToHex(l);
	int mm = 128 - res.size() % 128 - res_len.size();
	string rr(mm, '0');
	res = res + rr + res_len;
	int i = 0;
	for (int j = 0; j < n ; j += 8) {
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
	u32 W68[68];
	u32 W64[64];
	
	W68[0] = M[0]; W68[1] = M[1]; W68[2] = M[2]; W68[3] = M[3]; W68[4] = M[4]; W68[5] = M[5]; W68[6] = M[6]; W68[7] = M[7];
	W68[8] = M[8]; W68[9] = M[9]; W68[10] = M[10]; W68[11] = M[11]; W68[12] = M[12]; W68[13] = M[13]; W68[14] = M[14]; W68[15] = M[15];
	
	for (int j = 16; j < 68; j += 4)
	{
		u32 a; u32 b; u32 c; u32 d; u32 e; u32 f; u32 a1; u32 b1; u32 c1; u32 d1; u32 e1; u32 f1;
		a = W68[j - 16] ^ W68[j - 9]; b = left_move(W68[j - 3], 15); c = left_move(W68[j - 13], 7);
		d = W68[j - 15] ^ W68[j - 8]; e = left_move(W68[j - 2], 15); f = left_move(W68[j - 12], 7);
		W68[j] = P1(a ^ b) ^ c ^ W68[j - 6];
		W68[j + 1] = P1(d ^ e) ^ f ^ W68[j - 5];
		a1 = W68[j - 14] ^ W68[j - 7]; b1 = left_move(W68[j - 1], 15); c1 = left_move(W68[j - 11], 7);
		d1 = W68[j - 13] ^ W68[j - 6]; e1 = left_move(W68[j], 15); f1 = left_move(W68[j - 10], 7);
		W68[j + 2] = P1(a1 ^ b1) ^ c1 ^ W68[j - 4];
		W68[j + 3] = P1(d1 ^ e1) ^ f1 ^ W68[j - 3];
	}
	
	
	for (int j = 0; j < 64; j += 8)
	{
		u256 cc1; u256 aa1; u256 bb1;
		set256_32(aa1, W68[j + 7], W68[j + 6], W68[j + 5], W68[j + 4], W68[j + 3], W68[j + 2], W68[j + 1], W68[j]);
		set256_32(bb1, W68[j + 11], W68[j + 10], W68[j + 9], W68[j + 8], W68[j + 7], W68[j + 6], W68[j + 5], W68[j + 4]);
		set256_32(cc1, W64[j + 7], W64[j + 6], W64[j + 5], W64[j + 4], W64[j + 3], W64[j + 2], W64[j + 1], W64[j]);
		pxor256(cc1, aa1, bb1);
		W64[j] = U32From256(cc1, 0); W64[j + 1] = U32From256(cc1, 1); W64[j + 2] = U32From256(cc1, 2); W64[j + 3] = U32From256(cc1, 3);
		W64[j + 4] = U32From256(cc1, 4); W64[j + 5] = U32From256(cc1, 5); W64[j + 6] = U32From256(cc1, 6); W64[j + 7] = U32From256(cc1, 7);
	}
	
	u32 A_G[8] = { V[0],V[1],V[2],V[3],V[4],V[5],V[6],V[7] };

	//压缩函数
	u32 SS1 = 0, SS2 = 0, TT1 = 0, TT2 = 0;
	int T[64];
	T[0] = 0x79CC4519; T[1] = 0x79CC4519; T[2] = 0x79CC4519; T[3] = 0x79CC4519; T[4] = 0x79CC4519; T[5] = 0x79CC4519; T[6] = 0x79CC4519; T[7] = 0x79CC4519;
	T[8] = 0x79CC4519; T[9] = 0x79CC4519; T[10] = 0x79CC4519; T[11] = 0x79CC4519; T[12] = 0x79CC4519; T[13] = 0x79CC4519; T[14] = 0x79CC4519; T[15] = 0x79CC4519;
	for (int i = 16; i < 64; i += 16) {
		T[i + 1] = 0x7A879D8A; T[i + 2] = 0x7A879D8A; T[i + 3] = 0x7A879D8A; T[i + 4] = 0x7A879D8A; T[i + 5] = 0x7A879D8A; T[i + 6] = 0x7A879D8A; T[i + 7] = 0x7A879D8A;
		T[i] = 0x7A879D8A; T[i + 8] = 0x7A879D8A; T[i + 9] = 0x7A879D8A; T[i + 10] = 0x7A879D8A; T[i + 11] = 0x7A879D8A; T[i + 12] = 0x7A879D8A; T[i + 13] = 0x7A879D8A; T[i + 14] = 0x7A879D8A; T[i + 15] = 0x7A879D8A;
	}

    for (int j = 0; j < 64; j++)
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
	u256 a1; u256 b1; u256 v;
	loadu256(a1,A_G);
	loadu256(b1,V);
	pxor256(v, a1, b1);
	storeu256(V, v);
}
int main() {
	string str = "616263";//输入消息，16进制
	cout << "消息：" << str << endl;
	double run_time, tt;
	_LARGE_INTEGER time_start, t1;  //开始时间
	_LARGE_INTEGER time_over, t2;   //结束时间
	double dqFreq;      //计时器频率
	LARGE_INTEGER f;    //计时器频率
	QueryPerformanceFrequency(&f);
	dqFreq = (double)f.QuadPart;

	QueryPerformanceCounter(&time_start);   //计时开始

	int length = str.size();//消息长度，字节数
	u32 M[100];//填充后的消息
	int le = (length + 73) / 64;//分组数
	fill(str, M, le);//消息，填充后消息，分组数
	u32 v[8];
	v[0] = IV[0]; v[1] = IV[1]; v[2] = IV[2]; v[3] = IV[3];
	v[4] = IV[4]; v[5] = IV[5]; v[6] = IV[6]; v[7] = IV[7];
	for (int i = 0; i < le; i++) {
		CF(M, v);
	}

	QueryPerformanceCounter(&time_over);    //计时结束
	run_time = 1000000 * (time_over.QuadPart - time_start.QuadPart) / dqFreq;//精度为1000 000/（cpu主频）微秒
	printf("\n优化后：%fus\n", run_time);

	printf("杂凑值：");
	printf("%08x %08x %08x %08x %08x %08x %08x %08x\n", v[0], v[1], v[2], v[3],v[4], v[5], v[6], v[7]);

	return 0;
}
