/**
  *  \file t_stand.c
  *  \brief Standalone Tests (No Game Universe Required)
  */

#include <assert.h>
#include <string.h>             /* memcpy, memcmp */
#include "phostpdk.h"
#include "doscvt.h"

/** Test behaviour of types. */
static void
TestTypes()
{
    size_t enumSize;

    /* Fixed-size types */
    assert(sizeof(Int32) == 4);
    assert(sizeof(Uns32) == 4);
    assert(sizeof(Int16) == 2);
    assert(sizeof(Uns16) == 2);
    assert((Uns16)-1 > 0);
    assert((Uns32)-1 > 0);
    assert((Int16)-1 < 0);
    assert((Int32)-1 < 0);

    /* Enums. doscvt assumes that enums are two or four bytes, and all
       enums are the same size. This is not guaranteed by the C
       standard, but in practice, all enums have the same size as int. */
    enumSize = sizeof(DOSCVT_Def);
    assert(enumSize == 2 || enumSize == 4);
    assert(enumSize == sizeof(Boolean));
    assert(enumSize == sizeof(IO_Def));
    assert(enumSize == sizeof(CargoType_Def));
    assert(enumSize == sizeof(RaceType_Def));
    assert(enumSize == sizeof(NativeType_Def));
    assert(enumSize == sizeof(NativeGovm_Def));
    assert(enumSize == sizeof(ShipMission_Def));
    assert(enumSize == sizeof(BaseMission_Def));
    assert(enumSize == sizeof(BaseFixMission_Def));
    assert(enumSize == sizeof(BaseTech_Def));
    assert(enumSize == sizeof(Language_Def));
    assert(enumSize == sizeof(BuildQueue_Def));
    assert(enumSize == sizeof(Tristate));
}

/** Test constants. */
static void
TestConstants()
{
    assert(RACE_NR == 11);
    assert(SHIP_NR == 500 || SHIP_NR == 999);
    assert(PLANET_NR == 500);
    assert(BASE_NR == 500);
    assert(MINE_NR == 10000);
    assert(HULL_NR == 105);
    assert(ENGINE_NR == 9);
    assert(BEAM_NR == 10);
    assert(TORP_NR == 10);
    assert(STORM_NR == 50);
    assert(LONGNAME_SIZE == 30);
    assert(SHORTNAME_SIZE == 20);
    assert(ADJECTIVE_SIZE == 12);
    assert(RACENAME_RACE_SIZE == 62);
    assert(RACENAME_SIZE == 682);
    assert(PLANETNAME_SIZE == 20);
    assert(RACEHULLS == 20);
}

/** Test allocator. */
static void
TestAlloc()
{
    void *p, *q;
    int i;

    /* Test "allocate 0 bytes" case. Must return two null pointers, or
       different pointers. */
    p = MemAlloc(0);
    q = MemAlloc(0);
    assert((p == 0 && q == 0) || (p != q));
    MemFree(p);
    MemFree(q);

    /* Test "free null pointer" case. Must not crash. */
    MemFree(0);

    /* Test "allocate 0 bytes" with MemRealloc. */
    MemFree(MemRealloc(0, 0));

    /* Test basic allocation */
    p = MemCalloc(10, 1);
    assert(p);
    for (i = 0; i < 10; ++i)
        assert(((char*)p)[i] == 0);
    memcpy(p, "ThisIsTest", 10);
    assert(memcmp(p, "ThisIsTest", 10) == 0);

    p = MemRealloc(p, 20);
    assert(p);
    assert(memcmp(p, "ThisIsTest", 10) == 0);

    p = MemRealloc(p, 6);
    assert(p);
    assert(memcmp(p, "ThisIs", 6) == 0);

    MemFree(p);

    /* Test alignment */
    p = MemAlloc(sizeof(double));
    *(double*)p = 3.14;

    q = MemAlloc(sizeof(Uns32));
    *(Uns32*)q = 314;

    MemFree(p);
    MemFree(q);
}

/** Test endian conversion. */
static void
TestEndian()
{
    typedef union Union16 { unsigned char bytes[2]; Uns16 u16; Int16 i16; } Union16;
    typedef union Union32 { unsigned char bytes[4]; Uns32 u32; Int32 i32; } Union32;

    assert(sizeof(Union16) == 2);
    assert(sizeof(Union32) == 4);

    /* Word swap 16 bit */
    {
        enum { N = 6 };
        Union16 u[N] = {
            {{ 0, 0 }},           /* 0 */
            {{ 1, 0 }},           /* 1 */
            {{ 0, 1 }},           /* 256 */
            {{ 0x80, 0 }},        /* 128 */
            {{ 0, 0x80 }},        /* +/-32768 */
            {{ 0xFF, 0xFF }},     /* 0xFFFF/-1 */
        };
        char c[2*N];

        assert(ReadDOSUns16(u[0].bytes) == 0);
        assert(ReadDOSUns16(u[1].bytes) == 1);
        assert(ReadDOSUns16(u[2].bytes) == 256);
        assert(ReadDOSUns16(u[3].bytes) == 128);
        assert(ReadDOSUns16(u[4].bytes) == 32768U);
        assert(ReadDOSUns16(u[5].bytes) == 65535U);

        WriteDOSUns16(c+0, 0);
        WriteDOSUns16(c+2, 1);
        WriteDOSUns16(c+4, 256);
        WriteDOSUns16(c+6, 128);
        WriteDOSUns16(c+8, 32768U);
        WriteDOSUns16(c+10, 65535U);
        assert(sizeof(c) == sizeof(u));
        assert(memcmp(u, c, sizeof(c)) == 0);

        WordSwapShort(u, N);
        assert(u[0].u16 == 0);
        assert(u[1].u16 == 1);
        assert(u[2].u16 == 256);
        assert(u[3].u16 == 128);
        assert(u[4].u16 == 32768U);
        assert(u[5].u16 == 65535U);

        assert(u[0].i16 == 0);
        assert(u[1].i16 == 1);
        assert(u[2].i16 == 256);
        assert(u[3].i16 == 128);
        assert(u[4].i16 == -32768L);
        assert(u[5].i16 == -1);
    }

    /* Word swap 32 bit */
    {
        enum { N = 11 };
        Union32 u[N] = {
            {{ 0, 0, 0, 0 }},             /* 0 */
            {{ 1, 0, 0, 0 }},             /* 1 */
            {{ 0, 1, 0, 0 }},             /* 256 */
            {{ 0, 0, 1, 0 }},             /* 65536 */
            {{ 0, 0, 0, 1 }},             /* 16777216 */
            {{ 0x80, 0, 0, 0 }},          /* 128 */
            {{ 0, 0x80, 0, 0 }},          /* 32768 */
            {{ 0xFF, 0xFF, 0, 0 }},       /* 0xFFFF */
            {{ 0xFF, 0xFF, 0xFF, 0xFF }}, /* -1/0xFFFFFFFF */
            {{ 0, 0, 0x80, 0 }},          /* 8388608 */
            {{ 0, 0, 0, 0x80 }},          /* 0x80000000 */
        };
        char c[4*N];

        assert(ReadDOSUns32(u[0].bytes) == 0);
        assert(ReadDOSUns32(u[1].bytes) == 1);
        assert(ReadDOSUns32(u[2].bytes) == 256);
        assert(ReadDOSUns32(u[3].bytes) == 65536UL);
        assert(ReadDOSUns32(u[4].bytes) == 16777216UL);
        assert(ReadDOSUns32(u[5].bytes) == 128);
        assert(ReadDOSUns32(u[6].bytes) == 32768UL);
        assert(ReadDOSUns32(u[7].bytes) == 65535UL);
        assert(ReadDOSUns32(u[8].bytes) == 0xFFFFFFFFUL);
        assert(ReadDOSUns32(u[9].bytes) == 8388608);
        assert(ReadDOSUns32(u[10].bytes) == 0x80000000UL);

        WriteDOSUns32(c+0, 0);
        WriteDOSUns32(c+4, 1);
        WriteDOSUns32(c+8, 256);
        WriteDOSUns32(c+12, 65536UL);
        WriteDOSUns32(c+16, 16777216UL);
        WriteDOSUns32(c+20, 128);
        WriteDOSUns32(c+24, 32768UL);
        WriteDOSUns32(c+28, 65535UL);
        WriteDOSUns32(c+32, 0xFFFFFFFFUL);
        WriteDOSUns32(c+36, 8388608);
        WriteDOSUns32(c+40, 0x80000000UL);
        assert(sizeof(c) == sizeof(u));
        assert(memcmp(u, c, sizeof(c)) == 0);

        WordSwapLong(u, N);

        assert(u[0].u32 == 0);
        assert(u[1].u32 == 1);
        assert(u[2].u32 == 256);
        assert(u[3].u32 == 65536UL);
        assert(u[4].u32 == 16777216UL);
        assert(u[5].u32 == 128);
        assert(u[6].u32 == 32768UL);
        assert(u[7].u32 == 65535UL);
        assert(u[8].u32 == 0xFFFFFFFFUL);
        assert(u[9].u32 == 8388608);
        assert(u[10].u32 == 0x80000000UL);

        assert(u[0].i32 == 0);
        assert(u[1].i32 == 1);
        assert(u[2].i32 == 256);
        assert(u[3].i32 == 65536UL);
        assert(u[4].i32 == 16777216UL);
        assert(u[5].i32 == 128);
        assert(u[6].i32 == 32768UL);
        assert(u[7].i32 == 65535UL);
        assert(u[8].i32 == -1);
        assert(u[9].i32 == 8388608);
        assert(u[10].i32+1 == -2147483647L); /* 2147483648 would be unsigned, giving a nasty warning */
    }
}

/** Test Random Number Generator. */
static void
TestRandom()
{
    enum { N = 1000 };
    int data[N];
    int count[2][2];
    int prev;
    int i;

    /* Test range */
    SetRandomSeed(12345);
    for (i = 0; i < N; ++i) {
        data[i] = RandomRange(100);
        assert(data[i] >= 0);
        assert(data[i] < 100);
    }

    /* Test reproducibility */
    SetRandomSeed(12345);
    for (i = 0; i < N; ++i) {
        int j = RandomRange(100);
        assert(data[i] == j);
    }

    /* Test range for doubles */
    for (i = 0; i < 10000; ++i) {
        double d = RandomReal();
        assert(d >= 0.0);
        assert(d < 1.0);
    }

    /* Test quality. A bad PRNG toggles between 0 and 1 in RandomRange(2).
       This one produces all pairs with similar probability. */
    count[0][0] = count[0][1] = count[1][0] = count[1][1] = 0;
    prev = RandomRange(2);
    for (i = 0; i < 10000; ++i) {
        int j = RandomRange(2);
        assert(j >= 0);
        assert(j < 2);
        ++count[prev][j];
        prev = j;
    }
    assert(count[0][0] > 2000);
    assert(count[0][1] > 2000);
    assert(count[1][0] > 2000);
    assert(count[1][1] > 2000);
}

int main()
{
    TestTypes();
    TestConstants();
    TestAlloc();
    TestEndian();
    TestRandom();
    return 0;
}
