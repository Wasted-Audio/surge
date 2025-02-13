/*
** Surge Synthesizer is Free and Open Source Software
**
** Surge is made available under the Gnu General Public License, v3.0
** https://www.gnu.org/licenses/gpl-3.0.en.html
**
** Copyright 2004-2021 by various individuals as described by the Git transaction log
**
** All source at: https://github.com/surge-synthesizer/surge.git
**
** Surge was a commercial product from 2004-2018, with Copyright and ownership
** in that period held by Claes Johanson at Vember Audio. Claes made Surge
** open source in September 2018.
*/

#include "QuadFilterUnit.h"
#include "SurgeStorage.h"
#include "DebugHelpers.h"
#include <random>
#include <cmath>

__m128 CLIP(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    const __m128 x_min = _mm_set1_ps(-1.0f);
    const __m128 x_max = _mm_set1_ps(1.0f);
    return _mm_max_ps(_mm_min_ps(_mm_mul_ps(in, drive), x_max), x_min);
}

__m128 DIGI_SSE2(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    // v1.2: return (double)((int)((double)(x*p0inv*16.f+1.0)))*p0*0.0625f;
    const __m128 m16 = _mm_set1_ps(16.f);
    const __m128 m16inv = _mm_set1_ps(0.0625f);
    const __m128 mofs = _mm_set1_ps(0.5f);

    __m128 invdrive = _mm_rcp_ps(drive);
    __m128i a = _mm_cvtps_epi32(_mm_add_ps(mofs, _mm_mul_ps(invdrive, _mm_mul_ps(m16, in))));

    return _mm_mul_ps(drive, _mm_mul_ps(m16inv, _mm_sub_ps(_mm_cvtepi32_ps(a), mofs)));
}

__m128 TANH(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    // Closer to ideal than TANH0
    // y = x * ( 27 + x * x ) / ( 27 + 9 * x * x );
    // y = clip(y)
    const __m128 m9 = _mm_set1_ps(9.f);
    const __m128 m27 = _mm_set1_ps(27.f);

    __m128 x = _mm_mul_ps(in, drive);
    __m128 xx = _mm_mul_ps(x, x);
    __m128 denom = _mm_add_ps(m27, _mm_mul_ps(m9, xx));
    __m128 y = _mm_mul_ps(x, _mm_add_ps(m27, xx));
    y = _mm_mul_ps(y, _mm_rcp_ps(denom));

    const __m128 y_min = _mm_set1_ps(-1.0f);
    const __m128 y_max = _mm_set1_ps(1.0f);
    return _mm_max_ps(_mm_min_ps(y, y_max), y_min);
}

__m128 SINUS_SSE2(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    const __m128 one = _mm_set1_ps(1.f);
    const __m128 m256 = _mm_set1_ps(256.f);
    const __m128 m512 = _mm_set1_ps(512.f);

    __m128 x = _mm_mul_ps(in, drive);
    x = _mm_add_ps(_mm_mul_ps(x, m256), m512);

    __m128i e = _mm_cvtps_epi32(x);
    __m128 a = _mm_sub_ps(x, _mm_cvtepi32_ps(e));
    e = _mm_packs_epi32(e, e);
    const __m128i UB = _mm_set1_epi16(0x3fe);
    e = _mm_max_epi16(_mm_min_epi16(e, UB), _mm_setzero_si128());

#if MAC
    // this should be very fast on C2D/C1D (and there are no macs with K8's)
    // GCC seems to optimize around the XMM -> int transfers so this is needed here
    int e4 alignas(16)[4];
    e4[0] = _mm_cvtsi128_si32(e);
    e4[1] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(1, 1, 1, 1)));
    e4[2] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(2, 2, 2, 2)));
    e4[3] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(3, 3, 3, 3)));
#else
    // on PC write to memory & back as XMM -> GPR is slow on K8
    short e4 alignas(16)[8];
    _mm_store_si128((__m128i *)&e4, e);
#endif

    __m128 ws1 = _mm_load_ss(&waveshapers[wst_sine][e4[0] & 0x3ff]);
    __m128 ws2 = _mm_load_ss(&waveshapers[wst_sine][e4[1] & 0x3ff]);
    __m128 ws3 = _mm_load_ss(&waveshapers[wst_sine][e4[2] & 0x3ff]);
    __m128 ws4 = _mm_load_ss(&waveshapers[wst_sine][e4[3] & 0x3ff]);
    __m128 ws = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));
    ws1 = _mm_load_ss(&waveshapers[wst_sine][(e4[0] + 1) & 0x3ff]);
    ws2 = _mm_load_ss(&waveshapers[wst_sine][(e4[1] + 1) & 0x3ff]);
    ws3 = _mm_load_ss(&waveshapers[wst_sine][(e4[2] + 1) & 0x3ff]);
    ws4 = _mm_load_ss(&waveshapers[wst_sine][(e4[3] + 1) & 0x3ff]);
    __m128 wsn = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));

    x = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one, a), ws), _mm_mul_ps(a, wsn));

    return x;
}

__m128 ASYM_SSE2(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    const __m128 one = _mm_set1_ps(1.f);
    const __m128 m32 = _mm_set1_ps(32.f);
    const __m128 m512 = _mm_set1_ps(512.f);
    const __m128i UB = _mm_set1_epi16(0x3fe);

    __m128 x = _mm_mul_ps(in, drive);
    x = _mm_add_ps(_mm_mul_ps(x, m32), m512);

    __m128i e = _mm_cvtps_epi32(x);
    __m128 a = _mm_sub_ps(x, _mm_cvtepi32_ps(e));
    e = _mm_packs_epi32(e, e);
    e = _mm_max_epi16(_mm_min_epi16(e, UB), _mm_setzero_si128());

#if MAC
    // this should be very fast on C2D/C1D (and there are no macs with K8's)
    int e4 alignas(16)[4];
    e4[0] = _mm_cvtsi128_si32(e);
    e4[1] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(1, 1, 1, 1)));
    e4[2] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(2, 2, 2, 2)));
    e4[3] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(3, 3, 3, 3)));

#else
    // on PC write to memory & back as XMM -> GPR is slow on K8
    short e4 alignas(16)[8];
    _mm_store_si128((__m128i *)&e4, e);
#endif

    __m128 ws1 = _mm_load_ss(&waveshapers[wst_asym][e4[0] & 0x3ff]);
    __m128 ws2 = _mm_load_ss(&waveshapers[wst_asym][e4[1] & 0x3ff]);
    __m128 ws3 = _mm_load_ss(&waveshapers[wst_asym][e4[2] & 0x3ff]);
    __m128 ws4 = _mm_load_ss(&waveshapers[wst_asym][e4[3] & 0x3ff]);
    __m128 ws = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));
    ws1 = _mm_load_ss(&waveshapers[wst_asym][(e4[0] + 1) & 0x3ff]);
    ws2 = _mm_load_ss(&waveshapers[wst_asym][(e4[1] + 1) & 0x3ff]);
    ws3 = _mm_load_ss(&waveshapers[wst_asym][(e4[2] + 1) & 0x3ff]);
    ws4 = _mm_load_ss(&waveshapers[wst_asym][(e4[3] + 1) & 0x3ff]);
    __m128 wsn = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));

    x = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one, a), ws), _mm_mul_ps(a, wsn));

    return x;
}

template <int xRes, int xCenter, int size>
__m128 WS_LUT(QuadFilterWaveshaperState *__restrict s, const float *table, __m128 in, __m128 drive)
{
    const __m128 one = _mm_set1_ps(1.f);
    const __m128 m32 = _mm_set1_ps(xRes);
    const __m128 m512 = _mm_set1_ps(xCenter);
    const __m128i UB = _mm_set1_epi16(size - 1);

    __m128 x = _mm_mul_ps(in, drive);
    x = _mm_add_ps(_mm_mul_ps(x, m32), m512);

    __m128i e = _mm_cvtps_epi32(x);
    __m128 a = _mm_sub_ps(x, _mm_cvtepi32_ps(e));
    e = _mm_packs_epi32(e, e);
    e = _mm_max_epi16(_mm_min_epi16(e, UB), _mm_setzero_si128());

#if MAC
    // this should be very fast on C2D/C1D (and there are no macs with K8's)
    int e4 alignas(16)[4];
    e4[0] = _mm_cvtsi128_si32(e);
    e4[1] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(1, 1, 1, 1)));
    e4[2] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(2, 2, 2, 2)));
    e4[3] = _mm_cvtsi128_si32(_mm_shufflelo_epi16(e, _MM_SHUFFLE(3, 3, 3, 3)));

#else
    // on PC write to memory & back as XMM -> GPR is slow on K8
    short e4 alignas(16)[8];
    _mm_store_si128((__m128i *)&e4, e);
#endif

    __m128 ws1 = _mm_load_ss(&table[e4[0] & size]);
    __m128 ws2 = _mm_load_ss(&table[e4[1] & size]);
    __m128 ws3 = _mm_load_ss(&table[e4[2] & size]);
    __m128 ws4 = _mm_load_ss(&table[e4[3] & size]);
    __m128 ws = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));
    ws1 = _mm_load_ss(&table[(e4[0] + 1) & size]);
    ws2 = _mm_load_ss(&table[(e4[1] + 1) & size]);
    ws3 = _mm_load_ss(&table[(e4[2] + 1) & size]);
    ws4 = _mm_load_ss(&table[(e4[3] + 1) & size]);
    __m128 wsn = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));

    x = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one, a), ws), _mm_mul_ps(a, wsn));

    return x;
}

template <int R1, int R2> inline __m128 dcBlock(QuadFilterWaveshaperState *__restrict s, __m128 x)
{
    // https://www.dsprelated.com/freebooks/filters/DC_Blocker.html
    // y_n = x_n - x_n-1 + R y_n-1
    const auto fac = _mm_set1_ps(0.9999);
    auto dx = _mm_sub_ps(x, s->R[R1]);
    auto filtval = _mm_add_ps(dx, _mm_mul_ps(fac, s->R[R2]));
    s->R[R1] = x;
    s->R[R2] = filtval;
    s->init = _mm_setzero_ps();
    return filtval;
}

// Given a table of size N+1, N a power of 2, representing data between -1 and 1, interp
template <int N> __m128 WS_PM1_LUT(const float *table, __m128 in)
{
    static const __m128 one = _mm_set1_ps(1.f);
    static const __m128 dx = _mm_set1_ps(N / 2.f);
    static const __m128 oodx = _mm_set1_ps(2.f / N);
    static const __m128 ctr = _mm_set1_ps(N / 2.f);
    static const __m128 UB = _mm_set1_ps(N - 1.f);
    static const __m128 zero = _mm_setzero_ps();

    auto x = _mm_add_ps(_mm_mul_ps(in, dx), ctr);
    auto e = _mm_cvtps_epi32(_mm_max_ps(_mm_min_ps(x, UB), zero));
    auto a = _mm_cvtepi32_ps(e);
    auto frac = _mm_sub_ps(x, a);
    e = _mm_packs_epi32(e, e);

    // on PC write to memory & back as XMM -> GPR is slow on K8
    short e4 alignas(16)[8];
    _mm_store_si128((__m128i *)&e4, e);

    __m128 ws1 = _mm_load_ss(&table[e4[0]]);
    __m128 ws2 = _mm_load_ss(&table[e4[1]]);
    __m128 ws3 = _mm_load_ss(&table[e4[2]]);
    __m128 ws4 = _mm_load_ss(&table[e4[3]]);
    __m128 ws = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));
    ws1 = _mm_load_ss(&table[(e4[0] + 1)]);
    ws2 = _mm_load_ss(&table[(e4[1] + 1)]);
    ws3 = _mm_load_ss(&table[(e4[2] + 1)]);
    ws4 = _mm_load_ss(&table[(e4[3] + 1)]);
    __m128 wsn = _mm_movelh_ps(_mm_unpacklo_ps(ws1, ws2), _mm_unpacklo_ps(ws3, ws4));

    auto res = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one, frac), ws), _mm_mul_ps(frac, wsn));

    return res;
}

template <int NP, float F(const float)> struct LUTBase
{
    static constexpr int N = NP;
    static constexpr float dx = 2.0 / N;
    float data[N + 1];

    LUTBase()
    {
        for (int i = 0; i < N + 1; ++i)
        {
            float x = i * dx - 1.0;
            data[i] = F(x);
        }
    }
};

template <int scale> float FuzzTable(const float x)
{
    static auto gen = std::minstd_rand(2112);
    static const float range = 0.1 * scale;
    static auto dist = std::uniform_real_distribution<float>(-range, range);

    auto xadj = x * (1 - range) + dist(gen);
    return xadj;
}

template <int scale, __m128 C(QuadFilterWaveshaperState *__restrict, __m128, __m128)>
__m128 Fuzz(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    static LUTBase<1024, FuzzTable<scale>> table;
    return dcBlock<0, 1>(s, WS_PM1_LUT<1024>(table.data, C(s, x, drive)));
}

float FuzzCtrTable(const float x)
{
    static auto gen = std::minstd_rand(2112);
    static const float range = 0.1;
    static const float b = 20;

    static auto dist = std::uniform_real_distribution<float>(-1.0, 1.0);

    auto g = exp(-x * x * b);
    auto xadj = x + g * dist(gen);
    return xadj;
}

float FuzzEdgeTable(const float x)
{
    static auto gen = std::minstd_rand(2112);
    static const float range = 0.1;
    static const float b = 20;

    static auto dist = std::uniform_real_distribution<float>(-1.0, 1.0);

    auto g = x * x * x * x;
    auto xadj = 0.85 * x + 0.15 * g * dist(gen);
    return xadj;
}

float SinPlusX(const float x) { return x - std::sin(x * M_PI); }

template <int T> float SinNXPlusXBound(const float x)
{
    auto z = 1 - fabs(x);
    auto r = z * std::sin(x * M_PI * T);
    return r + x;
}

template <int T> float SinNX(const float x) { return std::sin(x * M_PI * T); }

template <int T> float SinNXBound(const float x)
{
    auto z = 1 - fabs(x);
    auto r = z * std::sin(x * M_PI * T);
    return r;
}

template <float F(float), int N,
          __m128 C(QuadFilterWaveshaperState *__restrict, __m128, __m128) = CLIP, bool block = true>
__m128 TableEval(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    static LUTBase<N, F> table;
    if (block)
        return dcBlock<0, 1>(s, WS_PM1_LUT<N>(table.data, C(s, x, drive)));
    else
        return WS_PM1_LUT<N>(table.data, C(s, x, drive));
};

template <__m128 (*K)(__m128), bool useDCBlock>
__m128 CHEBY_CORE(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    static const auto m1 = _mm_set1_ps(-1.0f);
    static const auto p1 = _mm_set1_ps(1.0f);

    auto bound = K(_mm_max_ps(_mm_min_ps(x, p1), m1));

    if (useDCBlock)
    {
        bound = dcBlock<0, 1>(s, bound);
    }
    return TANH(s, bound, drive);
}

__m128 cheb2_kernel(__m128 x) // 2 x^2 - 1
{
    static const auto m1 = _mm_set1_ps(1);
    static const auto m2 = _mm_set1_ps(2);
    return _mm_sub_ps(_mm_mul_ps(m2, _mm_mul_ps(x, x)), m1);
}

__m128 cheb3_kernel(__m128 x) // 4 x^3 - 3 x
{
    static const auto m4 = _mm_set1_ps(4);
    static const auto m3 = _mm_set1_ps(3);
    auto x2 = _mm_mul_ps(x, x);
    auto v4x2m3 = _mm_sub_ps(_mm_mul_ps(m4, x2), m3);
    return _mm_mul_ps(v4x2m3, x);
}

__m128 cheb4_kernel(__m128 x) // 8 x^4 - 8 x^2 + 1
{
    static const auto m1 = _mm_set1_ps(1);
    static const auto m8 = _mm_set1_ps(8);
    auto x2 = _mm_mul_ps(x, x);
    auto x4mx2 = _mm_mul_ps(x2, _mm_sub_ps(x2, m1)); // x^2 * ( x^2 - 1)
    return _mm_add_ps(_mm_mul_ps(m8, x4mx2), m1);
}

__m128 cheb5_kernel(__m128 x) // 16 x^5 - 20 x^3 + 5 x
{
    static const auto m16 = _mm_set1_ps(16);
    static const auto mn20 = _mm_set1_ps(-20);
    static const auto m5 = _mm_set1_ps(5);

    auto x2 = _mm_mul_ps(x, x);
    auto x3 = _mm_mul_ps(x2, x);
    auto x5 = _mm_mul_ps(x3, x2);
    auto t1 = _mm_mul_ps(m16, x5);
    auto t2 = _mm_mul_ps(mn20, x3);
    auto t3 = _mm_mul_ps(m5, x);
    return _mm_add_ps(t1, _mm_add_ps(t2, t3));
}

// Implement sum_n w_i T_i
template <int len> struct ChebSeries
{
    ChebSeries(const std::initializer_list<float> &f)
    {
        auto q = f.begin();
        auto idx = 0;
        for (int i = 0; i < len; ++i)
            weights[i] = _mm_setzero_ps();
        while (idx < len && q != f.end())
        {
            weights[idx] = _mm_set1_ps(*q);
            ++idx;
            ++q;
        }
    }
    __m128 weights[len];
    __m128 eval(__m128 x) const // x bound on 0,1
    {
        static const auto p2 = _mm_set1_ps(2.0);
        auto Tm1 = _mm_set1_ps(1.0);
        auto Tn = x;
        auto res = _mm_add_ps(weights[0], _mm_mul_ps(weights[1], x));
        for (int idx = 2; idx < len; ++idx)
        {
            auto nxt = _mm_sub_ps(_mm_mul_ps(_mm_mul_ps(p2, Tn), x), Tm1);
            Tm1 = Tn;
            Tn = nxt;
            res = _mm_add_ps(res, _mm_mul_ps(weights[idx], Tn));
        }
        return res;
    }
};

__m128 Plus12(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    static const auto ser = ChebSeries<3>({0, 0.5, 0.5});
    static const auto scale = _mm_set1_ps(0.66);
    return dcBlock<0, 1>(s, ser.eval(TANH(s, _mm_mul_ps(in, scale), drive)));
}

__m128 Plus13(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    static const auto ser = ChebSeries<4>({0, 0.5, 0, 0.5});
    static const auto scale = _mm_set1_ps(0.66);
    return ser.eval(TANH(s, _mm_mul_ps(in, scale), drive));
}

__m128 Plus14(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    static const auto ser = ChebSeries<5>({0, 0.5, 0, 0, 0.5});
    static const auto scale = _mm_set1_ps(0.66);
    return dcBlock<0, 1>(s, ser.eval(TANH(s, _mm_mul_ps(in, scale), drive)));
}

__m128 Plus15(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    static const auto ser = ChebSeries<6>({0, 0.5, 0, 0, 0, 0.5});
    static const auto scale = _mm_set1_ps(0.66);
    return ser.eval(TANH(s, _mm_mul_ps(in, scale), drive));
}

__m128 Plus12345(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    static const auto ser = ChebSeries<6>({0, 0.2, 0.2, 0.2, 0.2, 0.2});
    static const auto scale = _mm_set1_ps(0.66);
    return dcBlock<0, 1>(s, ser.eval(TANH(s, _mm_mul_ps(in, scale), drive)));
}

__m128 PlusSaw3(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    static const float fac = 0.9f / (1.f + 0.5 + 0.25);
    static const auto ser = ChebSeries<4>({0, -fac, fac * 0.5f, -fac * 0.25f});
    static const auto scale = _mm_set1_ps(-0.66); // flip direction
    return dcBlock<0, 1>(s, ser.eval(TANH(s, _mm_mul_ps(in, scale), drive)));
}

__m128 PlusSqr3(QuadFilterWaveshaperState *__restrict s, __m128 in, __m128 drive)
{
    static const float fac = 0.9f / (1.f - 0.25 + 1.0 / 16.0);
    static const auto ser = ChebSeries<6>({0, fac, 0, -fac * 0.25f, 0, fac / 16.f});
    static const auto scale = _mm_set1_ps(0.66);
    return ser.eval(TANH(s, _mm_mul_ps(in, scale), drive));
}

/*
 * Given a function which is from x -> (F, adF) and two registers, do the ADAA
 * Set updateInit to false if you are going to wrap this in a dcBlocker (which
 * resets init itself) or other init adjusting function.
 */
template <void FandADF(__m128, __m128 &, __m128 &), int xR, int aR, bool updateInit = true>
__m128 ADAA(QuadFilterWaveshaperState *__restrict s, __m128 x)
{
    auto xPrior = s->R[xR];
    auto adPrior = s->R[aR];

    __m128 f, ad;
    FandADF(x, f, ad);

    auto dx = _mm_sub_ps(x, xPrior);
    auto dad = _mm_sub_ps(ad, adPrior);

    const static auto tolF = 0.0001;
    const static auto tol = _mm_set1_ps(tolF), ntol = _mm_set1_ps(-tolF);
    auto ltt = _mm_and_ps(_mm_cmplt_ps(dx, tol), _mm_cmpgt_ps(dx, ntol)); // dx < tol && dx > -tol
    ltt = _mm_or_ps(ltt, s->init);
    auto dxDiv = _mm_rcp_ps(_mm_add_ps(_mm_and_ps(ltt, tol), _mm_andnot_ps(ltt, dx)));

    auto fFromAD = _mm_mul_ps(dad, dxDiv);
    auto r = _mm_add_ps(_mm_and_ps(ltt, f), _mm_andnot_ps(ltt, fFromAD));

    s->R[xR] = x;
    s->R[aR] = ad;
    if (updateInit)
    {
        s->init = _mm_setzero_ps();
    }

    return r;
}

void posrect_kernel(__m128 x, __m128 &f, __m128 &adF)
{
    /*
     * F   : x > 0 ? x : 0
     * adF : x > 0 ? x^2/2 : 0
     *
     * observe that adF = F^2/2 in all cases
     */
    static const auto p5 = _mm_set1_ps(0.5f);
    auto gz = _mm_cmpge_ps(x, _mm_setzero_ps());
    f = _mm_and_ps(gz, x);
    adF = _mm_mul_ps(p5, _mm_mul_ps(f, f));
}

template <int R1, int R2>
__m128 ADAA_POS_WAVE(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    x = CLIP(s, x, drive);
    return ADAA<posrect_kernel, R1, R2>(s, x);
}

void negrect_kernel(__m128 x, __m128 &f, __m128 &adF)
{
    /*
     * F   : x < 0 ? x : 0
     * adF : x < 0 ? x^2/2 : 0
     *
     * observe that adF = F^2/2 in all cases
     */
    static const auto p5 = _mm_set1_ps(0.5f);
    auto gz = _mm_cmple_ps(x, _mm_setzero_ps());
    f = _mm_and_ps(gz, x);
    adF = _mm_mul_ps(p5, _mm_mul_ps(f, f));
}

template <int R1, int R2>
__m128 ADAA_NEG_WAVE(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    x = CLIP(s, x, drive);
    return ADAA<negrect_kernel, R1, R2>(s, x);
}

void fwrect_kernel(__m128 x, __m128 &F, __m128 &adF)
{
    /*
     * F   : x > 0 ? x : -x  = sgn(x) * x
     * adF : x > 0 ? x^2 / 2 : -x^2 / 2 = sgn(x) * x^2 / 2
     */
    static const auto p1 = _mm_set1_ps(1.f);
    static const auto p05 = _mm_set1_ps(0.5f);
    auto gz = _mm_cmpge_ps(x, _mm_setzero_ps());
    auto sgn = _mm_sub_ps(_mm_and_ps(gz, p1), _mm_andnot_ps(gz, p1));

    F = _mm_mul_ps(sgn, x);
    adF = _mm_mul_ps(F, _mm_mul_ps(x, p05)); // sgn * x * x * 0.5
}

__m128 ADAA_FULL_WAVE(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    x = CLIP(s, x, drive);
    return ADAA<fwrect_kernel, 0, 1>(s, x);
}

void softrect_kernel(__m128 x, __m128 &F, __m128 &adF)
{
    /*
     * F   : x > 0 ? 2x-1 : -2x - 1   = 2 * ( sgn(x) * x ) - 1
     * adF : x > 0 ? x^2-x : -x^2 - x = sgn(x) * x^2 - x
     */
    static const auto p2 = _mm_set1_ps(2.f);
    static const auto p1 = _mm_set1_ps(1.f);
    static const auto p05 = _mm_set1_ps(0.5f);
    auto gz = _mm_cmpge_ps(x, _mm_setzero_ps());
    auto sgn = _mm_sub_ps(_mm_and_ps(gz, p1), _mm_andnot_ps(gz, p1));

    F = _mm_sub_ps(_mm_mul_ps(p2, _mm_mul_ps(sgn, x)), p1);
    adF = _mm_sub_ps(_mm_mul_ps(sgn, _mm_mul_ps(x, x)), x);
}

__m128 ADAA_SOFTRECT_WAVE(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    return TANH(s, dcBlock<2, 3>(s, ADAA<softrect_kernel, 0, 1, false>(s, x)), drive);
}

template <int pts> struct FolderADAA
{
    FolderADAA(std::initializer_list<float> xi, std::initializer_list<float> yi)
    {
        auto xiv = xi.begin();
        auto yiv = yi.begin();
        for (int i = 0; i < pts; ++i)
        {
            xs[i] = *xiv;
            ys[i] = *yiv;

            xiv++;
            yiv++;
        }

        slopes[pts - 1] = 0;
        dxs[pts - 1] = 0;

        intercepts[0] = -xs[0] * ys[0];
        for (int i = 0; i < pts - 1; ++i)
        {
            dxs[i] = xs[i + 1] - xs[i];
            slopes[i] = (ys[i + 1] - ys[i]) / dxs[i];
            auto vLeft = slopes[i] * dxs[i] * dxs[i] / 2 + ys[i] * xs[i + 1] + intercepts[i];
            auto vRight = ys[i + 1] * xs[i + 1];
            intercepts[i + 1] = -vRight + vLeft;
        }

        for (int i = 0; i < pts; ++i)
        {
            xS[i] = _mm_set1_ps(xs[i]);
            yS[i] = _mm_set1_ps(ys[i]);
            mS[i] = _mm_set1_ps(slopes[i]);
            cS[i] = _mm_set1_ps(intercepts[i]);
        }
    }

    inline void evaluate(__m128 x, __m128 &f, __m128 &adf)
    {
        static const auto p05 = _mm_set1_ps(0.5f);
        __m128 rangeMask[pts - 1], val[pts - 1], adVal[pts - 1];

        for (int i = 0; i < pts - 1; ++i)
        {
            rangeMask[i] = _mm_and_ps(_mm_cmpge_ps(x, xS[i]), _mm_cmplt_ps(x, xS[i + 1]));
            auto ox = _mm_sub_ps(x, xS[i]);
            val[i] = _mm_add_ps(_mm_mul_ps(mS[i], ox), yS[i]);
            adVal[i] = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(ox, ox), _mm_mul_ps(mS[i], p05)),
                                  _mm_add_ps(_mm_mul_ps(yS[i], x), cS[i]));
#if DEBUG_WITH_PRINT
            if (rangeMask[i][0] != 0)
                std::cout << _D(x[0]) << _D(rangeMask[i][0]) << _D(xS[i][0]) << _D(xS[i + 1][0])
                          << _D(ox[0]) << _D(cS[i][0]) << _D(mS[i][0]) << _D(yS[i][0])
                          << _D(val[i][0]) << _D(adVal[i][0]) << std::endl;
#endif
        }
        auto res = _mm_and_ps(rangeMask[0], val[0]);
        auto adres = _mm_and_ps(rangeMask[0], adVal[0]);
        for (int i = 1; i < pts - 1; ++i)
        {
            res = _mm_add_ps(res, _mm_and_ps(rangeMask[i], val[i]));
            adres = _mm_add_ps(adres, _mm_and_ps(rangeMask[i], adVal[i]));
        }
        f = res;
        adf = adres;
    }
    float xs[pts], ys[pts], dxs[pts], slopes[pts], intercepts[pts];

    __m128 xS[pts], yS[pts], dxS[pts], mS[pts], cS[pts];
};

void singleFoldADAA(__m128 x, __m128 &f, __m128 &adf)
{
    static auto folder = FolderADAA<4>({-10, -0.7, 0.7, 10}, {-1, 1, -1, 1});
    folder.evaluate(x, f, adf);
}

void dualFoldADAA(__m128 x, __m128 &f, __m128 &adf)
{
    static auto folder =
        FolderADAA<8>({-10, -3, -1, -0.3, 0.3, 1, 3, 10}, {-1, -0.9, 1, -1, 1, -1, 0.9, 1});
    folder.evaluate(x, f, adf);
}

void westCoastFoldADAA(__m128 x, __m128 &f, __m128 &adf)
{
    // Factors based on
    // DAFx-17 DAFX-194 Virtual Analog Buchla 259 Wavefolder
    // by Sequeda, Pontynen, Valimaki and Parker
    static auto folder = FolderADAA<14>(
        {-10, -2, -1.0919091909190919, -0.815881588158816, -0.5986598659865987, -0.3598359835983597,
         -0.11981198119811971, 0.11981198119811971, 0.3598359835983597, 0.5986598659865987,
         0.8158815881588157, 1.0919091909190919, 2, 10},
        {1, 0.9, -0.679765619488133, 0.5309659972270625, -0.6255506631744251, 0.5991799179917987,
         -0.5990599059905986, 0.5990599059905986, -0.5991799179917987, 0.6255506631744251,
         -0.5309659972270642, 0.679765619488133, -0.9, -1});
    folder.evaluate(x, f, adf);
}

template <void F(__m128, __m128 &, __m128 &)>
__m128 WAVEFOLDER(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    x = _mm_mul_ps(x, drive);
    return ADAA<F, 0, 1>(s, x);
}

__m128 ZAMSAT(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    x = CLIP(s, x, drive);

    // 2x * ( 1 - |x| )
    static const auto p1 = _mm_set1_ps(1.f);
    static const auto p2 = _mm_set1_ps(2.f);

    auto gz = _mm_cmpge_ps(x, _mm_setzero_ps());
    auto sgn = _mm_sub_ps(_mm_and_ps(gz, p1), _mm_andnot_ps(gz, p1));

    auto twox = _mm_mul_ps(p2, x);
    auto F = _mm_sub_ps(twox, _mm_mul_ps(sgn, _mm_mul_ps(x, x)));
    return F;
}

__m128 OJD(QuadFilterWaveshaperState *__restrict s, __m128 x, __m128 drive)
{
    x = _mm_mul_ps(x, drive);

    static const auto pm17 = _mm_set1_ps(-1.7f);
    static const auto p11 = _mm_set1_ps(1.1f);
    static const auto pm03 = _mm_set1_ps(-0.3f);
    static const auto p09 = _mm_set1_ps(0.9f);

    static const auto denLow = _mm_set1_ps(1.f / (4 * (1 - 0.3f)));
    static const auto denHigh = _mm_set1_ps(1.f / (4 * (1 - 0.9f)));

    auto maskNeg = _mm_cmple_ps(x, pm17);                         // in <= -1.7f
    auto maskPos = _mm_cmpge_ps(x, p11);                          // in > 1.1f
    auto maskLow = _mm_andnot_ps(maskNeg, _mm_cmplt_ps(x, pm03)); // in > -1.7 && in < =0.3
    auto maskHigh = _mm_andnot_ps(maskPos, _mm_cmpgt_ps(x, p09)); // in > 0.9 && in < 1.1
    auto maskMid = _mm_and_ps(_mm_cmpge_ps(x, pm03), _mm_cmple_ps(x, p09)); // the middle

    static const auto vNeg = _mm_set1_ps(-1.0);
    static const auto vPos = _mm_set1_ps(1.0);
    auto vMid = x;

    auto xlow = _mm_sub_ps(x, pm03);
    auto vLow = _mm_add_ps(xlow, _mm_mul_ps(denLow, _mm_mul_ps(xlow, xlow)));
    vLow = _mm_add_ps(vLow, pm03);

    auto xhi = _mm_sub_ps(x, p09);
    auto vHi = _mm_sub_ps(xhi, _mm_mul_ps(denHigh, _mm_mul_ps(xhi, xhi)));
    vHi = _mm_add_ps(vHi, p09);

    auto res =
        _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_and_ps(maskNeg, vNeg), _mm_and_ps(maskLow, vLow)),
                              _mm_add_ps(_mm_and_ps(maskHigh, vHi), _mm_and_ps(maskPos, vPos))),
                   _mm_and_ps(maskMid, vMid));

    return res;
}
WaveshaperQFPtr GetQFPtrWaveshaper(int type)
{
    switch (type)
    {
    case wst_soft:
        return TANH;
    case wst_hard:
        return CLIP;
    case wst_asym:
        // return ASYM_SSE2;
        return [](QuadFilterWaveshaperState *__restrict s, auto a, auto b) {
            return WS_LUT<32, 512, 0x3ff>(s, waveshapers[wst_asym], a, b);
        };
    case wst_sine:
        return SINUS_SSE2;
    case wst_digital:
        return DIGI_SSE2;
    case wst_cheby2:
        return CHEBY_CORE<cheb2_kernel, true>;
    case wst_cheby3:
        return CHEBY_CORE<cheb3_kernel, false>;
    case wst_cheby4:
        return CHEBY_CORE<cheb4_kernel, true>;
    case wst_cheby5:
        return CHEBY_CORE<cheb5_kernel, false>;
    case wst_fwrectify:
        return ADAA_FULL_WAVE;
    case wst_softrect:
        return ADAA_SOFTRECT_WAVE;
    case wst_poswav:
        return ADAA_POS_WAVE<0, 1>;
    case wst_negwav:
        return ADAA_NEG_WAVE<0, 1>;
    case wst_singlefold:
        return WAVEFOLDER<singleFoldADAA>;
    case wst_dualfold:
        return WAVEFOLDER<dualFoldADAA>;
    case wst_westfold:
        return WAVEFOLDER<westCoastFoldADAA>;
    case wst_add12:
        return Plus12;
    case wst_add13:
        return Plus13;
    case wst_add14:
        return Plus14;
    case wst_add15:
        return Plus15;
    case wst_add12345:
        return Plus12345;
    case wst_addsaw3:
        return PlusSaw3;
    case wst_addsqr3:
        return PlusSqr3;
    case wst_fuzz:
        return TableEval<FuzzTable<1>, 1024>;
    case wst_fuzzsoft:
        return TableEval<FuzzTable<1>, 1024, TANH>;
    case wst_fuzzheavy:
        return TableEval<FuzzTable<3>, 1024>;
    case wst_fuzzctr:
        return TableEval<FuzzCtrTable, 2048, TANH>;
    case wst_fuzzsoftedge:
        return TableEval<FuzzEdgeTable, 2048, TANH>;

    case wst_sinpx:
        return TableEval<SinPlusX, 1024, CLIP, false>;

    case wst_sin2xpb:
        return TableEval<SinNXPlusXBound<2>, 2048, CLIP, false>;
    case wst_sin3xpb:
        return TableEval<SinNXPlusXBound<3>, 2048, CLIP, false>;
    case wst_sin7xpb:
        return TableEval<SinNXPlusXBound<7>, 2048, CLIP, false>;
    case wst_sin10xpb:
        return TableEval<SinNXPlusXBound<10>, 2048, CLIP, false>;

    case wst_2cyc:
        return TableEval<SinNX<2>, 2048, CLIP, false>;
    case wst_7cyc:
        return TableEval<SinNX<7>, 2048, CLIP, false>;
    case wst_10cyc:
        return TableEval<SinNX<10>, 2048, CLIP, false>;
    case wst_2cycbound:
        return TableEval<SinNXBound<2>, 2048, CLIP, false>;
    case wst_7cycbound:
        return TableEval<SinNXBound<7>, 2048, CLIP, false>;
    case wst_10cycbound:
        return TableEval<SinNXBound<10>, 2048, CLIP, false>;

    case wst_zamsat:
        return ZAMSAT;
    case wst_ojd:
        return OJD;
    }
    return 0;
}

void initializeWaveshaperRegister(int type, float R[n_waveshaper_registers])
{
    switch (type)
    {
    default:
    {
        for (int i = 0; i < n_waveshaper_registers; ++i)
            R[i] = 0.f;
    }
    break;
    }
    return;
}
