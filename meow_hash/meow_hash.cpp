#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include<string.h>
#include<string>
#include<iostream>
#include <intrin.h>
using namespace std;

#define meow_u8 char unsigned
#define meow_u64 long long unsigned
#define meow_u128 __m128i
#define meow_umm int unsigned
#define INSTRUCTION_REORDER_BARRIER _ReadWriteBarrier()
#define MeowU32From(A, I) (_mm_extract_epi32((A), (I)))

#if !defined INSTRUCTION_REORDER_BARRIER
#define INSTRUCTION_REORDER_BARRIER
#endif

#if !defined MEOW_PAGESIZE
#define MEOW_PAGESIZE 4096
#endif

#if !defined MEOW_PREFETCH
#define MEOW_PREFETCH 4096
#endif

#if !defined MEOW_PREFETCH_LIMIT
#define MEOW_PREFETCH_LIMIT 0x3ff
#endif

#define prefetcht0(A) _mm_prefetch((char *)(A), _MM_HINT_T0)
#define movdqu(A, B)  A = _mm_loadu_si128((__m128i *)(B))
#define movdqu_mem(A, B)  _mm_storeu_si128((__m128i *)(A), B)
#define movq(A, B) A = _mm_set_epi64x(0, B);
#define aesdec(A, B)  A = _mm_aesdec_si128(A, B)
#define pshufb(A, B)  A = _mm_shuffle_epi8(A, B)
#define pxor(A, B)    A = _mm_xor_si128(A, B)
#define paddq(A, B) A = _mm_add_epi64(A, B)
#define pand(A, B)    A = _mm_and_si128(A, B)
#define palignr(A, B, i) A = _mm_alignr_epi8(A, B, i)
#define pxor_clear(A, B)    A = _mm_setzero_si128(); 
#define MEOW_DUMP_STATE(...)
#define psubq(A, B) A = _mm_sub_epi64(A, B)
#define aesenc(A, B)  A = _mm_aesenc_si128(A, B)

#define MEOW_MIX_REG_inv(r1, r2, r3, r4, r5,  i1, i2, i3, i4) \
pxor(r4, i4);\
paddq(r5, i3); \
aesenc(r2, r4); \
INSTRUCTION_REORDER_BARRIER; \
pxor(r2, i2); \
paddq(r3, i1); \
aesenc(r1, r2); \
INSTRUCTION_REORDER_BARRIER 

#define MEOW_MIX_inv(r1, r2, r3, r4, r5,  ptr) \
MEOW_MIX_REG_inv(r1, r2, r3, r4, r5, _mm_loadu_si128( (__m128i *) ((ptr) + 15) ), _mm_loadu_si128( (__m128i *) ((ptr) + 0)  ), _mm_loadu_si128( (__m128i *) ((ptr) + 1)  ), _mm_loadu_si128( (__m128i *) ((ptr) + 16) ))

#define MEOW_SHUFFLE_inv(r1, r2, r3, r4, r5, r6) \
pxor(r2, r3);\
psubq(r5, r6); \
aesenc(r4, r2); \
pxor(r4, r6); \
psubq(r2, r5); \
aesenc(r1, r4)

static meow_u8 MeowShiftAdjust[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
static meow_u8 MeowMaskLen[32] = { 255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };

//打印
static void PrintHash(meow_u128 Hash)
{
    printf("%08x%08x%08x%08x\n",
        MeowU32From(Hash, 3),
        MeowU32From(Hash, 2),
        MeowU32From(Hash, 1),
        MeowU32From(Hash, 0));
}

void MeowHash_inv(void* Hash_value, meow_umm Len, void* Message)//hash_value， size， message
{
    meow_u128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7; // NOTE(casey): xmm0-xmm7 are the hash accumulation lanes 
    meow_u128 xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15; // NOTE(casey): xmm8-xmm15 hold values to be appended (residual, length) 

    meow_u8* rax = (meow_u8*)Message;
    meow_u8* rcx = (meow_u8*)Hash_value;

    //将hash_value赋给xmm0，xmm1-xmm7自行取值
    movdqu(xmm0, rcx);
    movdqu(xmm1, rcx + 0x10);
    movdqu(xmm2, rcx + 0x20);
    movdqu(xmm3, rcx + 0x30);
    movdqu(xmm4, rcx + 0x40);
    movdqu(xmm5, rcx + 0x50);
    movdqu(xmm6, rcx + 0x60);
    movdqu(xmm7, rcx + 0x70);
    MEOW_DUMP_STATE("Seed_inv", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);

    
    //将一个 128 位哈希分为八个通道
    psubq(xmm4, xmm0);
    pxor(xmm1, xmm0);
    pxor(xmm5, xmm4);
    psubq(xmm2, xmm0);
    psubq(xmm3, xmm1);
    psubq(xmm6, xmm4);
    psubq(xmm7, xmm5);
    MEOW_DUMP_STATE("PostFold_inv", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);



    MEOW_SHUFFLE_inv(xmm0, xmm1, xmm2, xmm4, xmm5, xmm6);
    MEOW_SHUFFLE_inv(xmm1, xmm2, xmm3, xmm5, xmm6, xmm7);
    MEOW_SHUFFLE_inv(xmm2, xmm3, xmm4, xmm6, xmm7, xmm0);
    MEOW_SHUFFLE_inv(xmm3, xmm4, xmm5, xmm7, xmm0, xmm1);
    MEOW_SHUFFLE_inv(xmm4, xmm5, xmm6, xmm0, xmm1, xmm2);
    MEOW_SHUFFLE_inv(xmm5, xmm6, xmm7, xmm1, xmm2, xmm3);
    MEOW_SHUFFLE_inv(xmm6, xmm7, xmm0, xmm2, xmm3, xmm4);
    MEOW_SHUFFLE_inv(xmm7, xmm0, xmm1, xmm3, xmm4, xmm5);
    MEOW_SHUFFLE_inv(xmm0, xmm1, xmm2, xmm4, xmm5, xmm6);
    MEOW_SHUFFLE_inv(xmm1, xmm2, xmm3, xmm5, xmm6, xmm7);
    MEOW_SHUFFLE_inv(xmm2, xmm3, xmm4, xmm6, xmm7, xmm0);
    MEOW_SHUFFLE_inv(xmm3, xmm4, xmm5, xmm7, xmm0, xmm1);
    MEOW_DUMP_STATE("PostMix_inv", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);


    pxor_clear(xmm9, xmm9);
    pxor_clear(xmm11, xmm11);
    meow_u8* Last = (meow_u8*)Message + (Len & ~0xf);
    int unsigned Len8 = (Len & 0xf);
    if (Len8)
    {
        movdqu(xmm8, &MeowMaskLen[0x10 - Len8]);
        meow_u8* LastOk = (meow_u8*)((((meow_umm)(((meow_u8*)Message) + Len - 1)) | (MEOW_PAGESIZE - 1)) - 16);
        int Align = (Last > LastOk) ? ((int)(meow_umm)Last) & 0xf : 0;
        movdqu(xmm10, &MeowShiftAdjust[Align]);
        movdqu(xmm9, Last - Align);
        pshufb(xmm9, xmm10);
        pand(xmm9, xmm8);
    }
    if (Len & 0x10)
    {
        xmm11 = xmm9;
        movdqu(xmm9, Last - 0x10);
    }
    xmm8 = xmm9;
    xmm10 = xmm9;
    palignr(xmm8, xmm11, 15);
    palignr(xmm10, xmm11, 1);
    pxor_clear(xmm12, xmm12);
    pxor_clear(xmm13, xmm13);
    pxor_clear(xmm14, xmm14);
    movq(xmm15, Len);
    palignr(xmm12, xmm15, 15);
    palignr(xmm14, xmm15, 1);
    MEOW_DUMP_STATE("Residuals_inv", xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15, 0);


    int unsigned LaneCount = (Len >> 5) & 0x7;
    if (LaneCount == 0) goto MixDown; MEOW_MIX_inv(xmm2, xmm6, xmm0, xmm3, xmm4, rax + 0x00); --LaneCount;
    if (LaneCount == 0) goto MixDown; MEOW_MIX_inv(xmm3, xmm7, xmm1, xmm4, xmm5, rax + 0x20); --LaneCount;
    if (LaneCount == 0) goto MixDown; MEOW_MIX_inv(xmm4, xmm0, xmm2, xmm5, xmm6, rax + 0x40); --LaneCount;
    if (LaneCount == 0) goto MixDown; MEOW_MIX_inv(xmm5, xmm1, xmm3, xmm6, xmm7, rax + 0x60); --LaneCount;
    if (LaneCount == 0) goto MixDown; MEOW_MIX_inv(xmm6, xmm2, xmm4, xmm7, xmm0, rax + 0x80); --LaneCount;
    if (LaneCount == 0) goto MixDown; MEOW_MIX_inv(xmm7, xmm3, xmm5, xmm0, xmm1, rax + 0xa0); --LaneCount;
    if (LaneCount == 0) goto MixDown; MEOW_MIX_inv(xmm0, xmm4, xmm6, xmm1, xmm2, rax + 0xc0); --LaneCount;

    MixDown:
    MEOW_DUMP_STATE("PostLanes", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);


    MEOW_MIX_REG_inv(xmm1, xmm5, xmm7, xmm2, xmm3, xmm12, xmm13, xmm14, xmm15);
    MEOW_MIX_REG_inv(xmm0, xmm4, xmm6, xmm1, xmm2, xmm8, xmm9, xmm10, xmm11);
    MEOW_DUMP_STATE("PostAppend_inv", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);


    meow_umm BlockCount = (Len >> 8);
    if(BlockCount > MEOW_PREFETCH_LIMIT)
    {
        while(BlockCount--)
        {
            prefetcht0(rax + MEOW_PREFETCH + 0x00);
            prefetcht0(rax + MEOW_PREFETCH + 0x40);
            prefetcht0(rax + MEOW_PREFETCH + 0x80);
            prefetcht0(rax + MEOW_PREFETCH + 0xc0);
            
            MEOW_MIX_inv(xmm0,xmm4,xmm6,xmm1,xmm2, rax + 0x00);
            MEOW_MIX_inv(xmm1,xmm5,xmm7,xmm2,xmm3, rax + 0x20);
            MEOW_MIX_inv(xmm2,xmm6,xmm0,xmm3,xmm4, rax + 0x40);
            MEOW_MIX_inv(xmm3,xmm7,xmm1,xmm4,xmm5, rax + 0x60);
            MEOW_MIX_inv(xmm4,xmm0,xmm2,xmm5,xmm6, rax + 0x80);
            MEOW_MIX_inv(xmm5,xmm1,xmm3,xmm6,xmm7, rax + 0xa0);
            MEOW_MIX_inv(xmm6,xmm2,xmm4,xmm7,xmm0, rax + 0xc0);
            MEOW_MIX_inv(xmm7,xmm3,xmm5,xmm0,xmm1, rax + 0xe0);
            
            rax += 0x100;
        }
    }
    else
    {
        while(BlockCount--)
        {
            MEOW_MIX_inv(xmm0,xmm4,xmm6,xmm1,xmm2, rax + 0x00);
            MEOW_MIX_inv(xmm1,xmm5,xmm7,xmm2,xmm3, rax + 0x20);
            MEOW_MIX_inv(xmm2,xmm6,xmm0,xmm3,xmm4, rax + 0x40);
            MEOW_MIX_inv(xmm3,xmm7,xmm1,xmm4,xmm5, rax + 0x60);
            MEOW_MIX_inv(xmm4,xmm0,xmm2,xmm5,xmm6, rax + 0x80);
            MEOW_MIX_inv(xmm5,xmm1,xmm3,xmm6,xmm7, rax + 0xa0);
            MEOW_MIX_inv(xmm6,xmm2,xmm4,xmm7,xmm0, rax + 0xc0);
            MEOW_MIX_inv(xmm7,xmm3,xmm5,xmm0,xmm1, rax + 0xe0);
            
            rax += 0x100;
        }
    }
    
    MEOW_DUMP_STATE("PostBlocks_inv", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);
    
    PrintHash(xmm0); PrintHash(xmm1); PrintHash(xmm2); PrintHash(xmm3);
    PrintHash(xmm4); PrintHash(xmm5); PrintHash(xmm6); PrintHash(xmm7);
}


void str_to_hex(char a[],char c[]) {
    //转换成十六进制ASCII码
    for (int i = 0; i < 16; i++) {
        c[i] = (int)a[i];
    }
}

static void find_key(void)
{
    char message[] = "JingJing202000460073";//消息
    cout << "message:\n" << message << endl;
    int Size = strlen(message);

    char Hash_value[] = "sdu_cst_20220610";//哈希值
    cout << "\nHash_value:\n" << Hash_value << endl;

    char ff[50] = "";
    cout << "\nHash_value转换成ASCII码：" << endl;
    str_to_hex(Hash_value, ff);
    for (int i = 0; i < 16; i++) {
        printf("%02x", ff[i]);
    }
    printf("\n\nkeys:\n");//密钥
    MeowHash_inv(ff, Size, message);

}

int main()
{
    find_key();
    return 0;
}
