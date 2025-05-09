/*
 *  MMX/3DNow!/SSE/SSE2/SSE3/SSSE3/SSE4/PNI support
 *
 *  Copyright (c) 2005 Fabrice Bellard
 *  Copyright (c) 2008 Intel Corporation  <andrew.zaborowski@intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "crypto/aes.h"

#if SHIFT == 0
#define Reg MMXReg
#define XMM_ONLY(...)
#define B(n) MMX_B(n)
#define W(n) MMX_W(n)
#define L(n) MMX_L(n)
#define Q(n) MMX_Q(n)
#define SUFFIX _mmx
#else
#define Reg ZMMReg
#define XMM_ONLY(...) __VA_ARGS__
#define B(n) ZMM_B(n)
#define W(n) ZMM_W(n)
#define L(n) ZMM_L(n)
#define Q(n) ZMM_Q(n)
#if SHIFT == 1
#define SUFFIX _xmm
#else
#define SUFFIX _ymm
#endif
#endif

#if SHIFT <= 1
void glue(helper_psrlw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 15) {
        d->Q(0) = 0;
#if SHIFT == 1
        d->Q(1) = 0;
#endif
    } else {
        shift = s->B(0);
        d->W(0) >>= shift;
        d->W(1) >>= shift;
        d->W(2) >>= shift;
        d->W(3) >>= shift;
#if SHIFT == 1
        d->W(4) >>= shift;
        d->W(5) >>= shift;
        d->W(6) >>= shift;
        d->W(7) >>= shift;
#endif
    }
}

void glue(helper_psraw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 15) {
        shift = 15;
    } else {
        shift = s->B(0);
    }
    d->W(0) = (int16_t)d->W(0) >> shift;
    d->W(1) = (int16_t)d->W(1) >> shift;
    d->W(2) = (int16_t)d->W(2) >> shift;
    d->W(3) = (int16_t)d->W(3) >> shift;
#if SHIFT == 1
    d->W(4) = (int16_t)d->W(4) >> shift;
    d->W(5) = (int16_t)d->W(5) >> shift;
    d->W(6) = (int16_t)d->W(6) >> shift;
    d->W(7) = (int16_t)d->W(7) >> shift;
#endif
}

void glue(helper_psllw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 15) {
        d->Q(0) = 0;
#if SHIFT == 1
        d->Q(1) = 0;
#endif
    } else {
        shift = s->B(0);
        d->W(0) <<= shift;
        d->W(1) <<= shift;
        d->W(2) <<= shift;
        d->W(3) <<= shift;
#if SHIFT == 1
        d->W(4) <<= shift;
        d->W(5) <<= shift;
        d->W(6) <<= shift;
        d->W(7) <<= shift;
#endif
    }
}

void glue(helper_psrld, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 31) {
        d->Q(0) = 0;
#if SHIFT == 1
        d->Q(1) = 0;
#endif
    } else {
        shift = s->B(0);
        d->L(0) >>= shift;
        d->L(1) >>= shift;
#if SHIFT == 1
        d->L(2) >>= shift;
        d->L(3) >>= shift;
#endif
    }
}

void glue(helper_psrad, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 31) {
        shift = 31;
    } else {
        shift = s->B(0);
    }
    d->L(0) = (int32_t)d->L(0) >> shift;
    d->L(1) = (int32_t)d->L(1) >> shift;
#if SHIFT == 1
    d->L(2) = (int32_t)d->L(2) >> shift;
    d->L(3) = (int32_t)d->L(3) >> shift;
#endif
}

void glue(helper_pslld, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 31) {
        d->Q(0) = 0;
#if SHIFT == 1
        d->Q(1) = 0;
#endif
    } else {
        shift = s->B(0);
        d->L(0) <<= shift;
        d->L(1) <<= shift;
#if SHIFT == 1
        d->L(2) <<= shift;
        d->L(3) <<= shift;
#endif
    }
}

void glue(helper_psrlq, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 63) {
        d->Q(0) = 0;
#if SHIFT == 1
        d->Q(1) = 0;
#endif
    } else {
        shift = s->B(0);
        d->Q(0) >>= shift;
#if SHIFT == 1
        d->Q(1) >>= shift;
#endif
    }
}

void glue(helper_psllq, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift;

    if (s->Q(0) > 63) {
        d->Q(0) = 0;
#if SHIFT == 1
        d->Q(1) = 0;
#endif
    } else {
        shift = s->B(0);
        d->Q(0) <<= shift;
#if SHIFT == 1
        d->Q(1) <<= shift;
#endif
    }
}

#if SHIFT == 1
void glue(helper_psrldq, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift, i;

    shift = s->L(0);
    if (shift > 16) {
        shift = 16;
    }
    for (i = 0; i < 16 - shift; i++) {
        d->B(i) = d->B(i + shift);
    }
    for (i = 16 - shift; i < 16; i++) {
        d->B(i) = 0;
    }
}

void glue(helper_pslldq, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int shift, i;

    shift = s->L(0);
    if (shift > 16) {
        shift = 16;
    }
    for (i = 15; i >= shift; i--) {
        d->B(i) = d->B(i - shift);
    }
    for (i = 0; i < shift; i++) {
        d->B(i) = 0;
    }
}
#endif

#define SSE_HELPER_B(name, F)                                   \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)   \
    {                                                           \
        d->B(0) = F(d->B(0), s->B(0));                          \
        d->B(1) = F(d->B(1), s->B(1));                          \
        d->B(2) = F(d->B(2), s->B(2));                          \
        d->B(3) = F(d->B(3), s->B(3));                          \
        d->B(4) = F(d->B(4), s->B(4));                          \
        d->B(5) = F(d->B(5), s->B(5));                          \
        d->B(6) = F(d->B(6), s->B(6));                          \
        d->B(7) = F(d->B(7), s->B(7));                          \
        XMM_ONLY(                                               \
                 d->B(8) = F(d->B(8), s->B(8));                 \
                 d->B(9) = F(d->B(9), s->B(9));                 \
                 d->B(10) = F(d->B(10), s->B(10));              \
                 d->B(11) = F(d->B(11), s->B(11));              \
                 d->B(12) = F(d->B(12), s->B(12));              \
                 d->B(13) = F(d->B(13), s->B(13));              \
                 d->B(14) = F(d->B(14), s->B(14));              \
                 d->B(15) = F(d->B(15), s->B(15));              \
                                                        )       \
            }

#define SSE_HELPER_W(name, F)                                   \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)   \
    {                                                           \
        d->W(0) = F(d->W(0), s->W(0));                          \
        d->W(1) = F(d->W(1), s->W(1));                          \
        d->W(2) = F(d->W(2), s->W(2));                          \
        d->W(3) = F(d->W(3), s->W(3));                          \
        XMM_ONLY(                                               \
                 d->W(4) = F(d->W(4), s->W(4));                 \
                 d->W(5) = F(d->W(5), s->W(5));                 \
                 d->W(6) = F(d->W(6), s->W(6));                 \
                 d->W(7) = F(d->W(7), s->W(7));                 \
                                                        )       \
            }

#define SSE_HELPER_L(name, F)                                   \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)   \
    {                                                           \
        d->L(0) = F(d->L(0), s->L(0));                          \
        d->L(1) = F(d->L(1), s->L(1));                          \
        XMM_ONLY(                                               \
                 d->L(2) = F(d->L(2), s->L(2));                 \
                 d->L(3) = F(d->L(3), s->L(3));                 \
                                                        )       \
            }

#define SSE_HELPER_Q(name, F)                                   \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)   \
    {                                                           \
        d->Q(0) = F(d->Q(0), s->Q(0));                          \
        XMM_ONLY(                                               \
                 d->Q(1) = F(d->Q(1), s->Q(1));                 \
                                                        )       \
            }

#if SHIFT == 0
static inline int satub(int x)
{
    if (x < 0) {
        return 0;
    } else if (x > 255) {
        return 255;
    } else {
        return x;
    }
}

static inline int satuw(int x)
{
    if (x < 0) {
        return 0;
    } else if (x > 65535) {
        return 65535;
    } else {
        return x;
    }
}

static inline int satsb(int x)
{
    if (x < -128) {
        return -128;
    } else if (x > 127) {
        return 127;
    } else {
        return x;
    }
}

static inline int satsw(int x)
{
    if (x < -32768) {
        return -32768;
    } else if (x > 32767) {
        return 32767;
    } else {
        return x;
    }
}

#define FADD(a, b) ((a) + (b))
#define FADDUB(a, b) satub((a) + (b))
#define FADDUW(a, b) satuw((a) + (b))
#define FADDSB(a, b) satsb((int8_t)(a) + (int8_t)(b))
#define FADDSW(a, b) satsw((int16_t)(a) + (int16_t)(b))

#define FSUB(a, b) ((a) - (b))
#define FSUBUB(a, b) satub((a) - (b))
#define FSUBUW(a, b) satuw((a) - (b))
#define FSUBSB(a, b) satsb((int8_t)(a) - (int8_t)(b))
#define FSUBSW(a, b) satsw((int16_t)(a) - (int16_t)(b))
#define FMINUB(a, b) ((a) < (b)) ? (a) : (b)
#define FMINSW(a, b) ((int16_t)(a) < (int16_t)(b)) ? (a) : (b)
#define FMAXUB(a, b) ((a) > (b)) ? (a) : (b)
#define FMAXSW(a, b) ((int16_t)(a) > (int16_t)(b)) ? (a) : (b)

#define FAND(a, b) ((a) & (b))
#define FANDN(a, b) ((~(a)) & (b))
#define FOR(a, b) ((a) | (b))
#define FXOR(a, b) ((a) ^ (b))

#define FCMPGTB(a, b) ((int8_t)(a) > (int8_t)(b) ? -1 : 0)
#define FCMPGTW(a, b) ((int16_t)(a) > (int16_t)(b) ? -1 : 0)
#define FCMPGTL(a, b) ((int32_t)(a) > (int32_t)(b) ? -1 : 0)
#define FCMPEQ(a, b) ((a) == (b) ? -1 : 0)

#define FMULLW(a, b) ((a) * (b))
#define FMULHRW(a, b) (((int16_t)(a) * (int16_t)(b) + 0x8000) >> 16)
#define FMULHUW(a, b) ((a) * (b) >> 16)
#define FMULHW(a, b) ((int16_t)(a) * (int16_t)(b) >> 16)

#define FAVG(a, b) (((a) + (b) + 1) >> 1)
#endif

SSE_HELPER_B(helper_paddb, FADD)
SSE_HELPER_W(helper_paddw, FADD)
SSE_HELPER_L(helper_paddl, FADD)
SSE_HELPER_Q(helper_paddq, FADD)

SSE_HELPER_B(helper_psubb, FSUB)
SSE_HELPER_W(helper_psubw, FSUB)
SSE_HELPER_L(helper_psubl, FSUB)
SSE_HELPER_Q(helper_psubq, FSUB)

SSE_HELPER_B(helper_paddusb, FADDUB)
SSE_HELPER_B(helper_paddsb, FADDSB)
SSE_HELPER_B(helper_psubusb, FSUBUB)
SSE_HELPER_B(helper_psubsb, FSUBSB)

SSE_HELPER_W(helper_paddusw, FADDUW)
SSE_HELPER_W(helper_paddsw, FADDSW)
SSE_HELPER_W(helper_psubusw, FSUBUW)
SSE_HELPER_W(helper_psubsw, FSUBSW)

SSE_HELPER_B(helper_pminub, FMINUB)
SSE_HELPER_B(helper_pmaxub, FMAXUB)

SSE_HELPER_W(helper_pminsw, FMINSW)
SSE_HELPER_W(helper_pmaxsw, FMAXSW)

SSE_HELPER_Q(helper_pand, FAND)
SSE_HELPER_Q(helper_pandn, FANDN)
SSE_HELPER_Q(helper_por, FOR)
SSE_HELPER_Q(helper_pxor, FXOR)

SSE_HELPER_B(helper_pcmpgtb, FCMPGTB)
SSE_HELPER_W(helper_pcmpgtw, FCMPGTW)
SSE_HELPER_L(helper_pcmpgtl, FCMPGTL)

SSE_HELPER_B(helper_pcmpeqb, FCMPEQ)
SSE_HELPER_W(helper_pcmpeqw, FCMPEQ)
SSE_HELPER_L(helper_pcmpeql, FCMPEQ)

SSE_HELPER_W(helper_pmullw, FMULLW)
#if SHIFT == 0
SSE_HELPER_W(helper_pmulhrw, FMULHRW)
#endif
SSE_HELPER_W(helper_pmulhuw, FMULHUW)
SSE_HELPER_W(helper_pmulhw, FMULHW)

SSE_HELPER_B(helper_pavgb, FAVG)
SSE_HELPER_W(helper_pavgw, FAVG)

void glue(helper_pmuludq, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    d->Q(0) = (uint64_t)s->L(0) * (uint64_t)d->L(0);
#if SHIFT == 1
    d->Q(1) = (uint64_t)s->L(2) * (uint64_t)d->L(2);
#endif
}

void glue(helper_pmaddwd, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;

    for (i = 0; i < (2 << SHIFT); i++) {
        d->L(i) = (int16_t)s->W(2 * i) * (int16_t)d->W(2 * i) +
            (int16_t)s->W(2 * i + 1) * (int16_t)d->W(2 * i + 1);
    }
}

#if SHIFT == 0
static inline int abs1(int a)
{
    if (a < 0) {
        return -a;
    } else {
        return a;
    }
}
#endif
void glue(helper_psadbw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    unsigned int val;

    val = 0;
    val += abs1(d->B(0) - s->B(0));
    val += abs1(d->B(1) - s->B(1));
    val += abs1(d->B(2) - s->B(2));
    val += abs1(d->B(3) - s->B(3));
    val += abs1(d->B(4) - s->B(4));
    val += abs1(d->B(5) - s->B(5));
    val += abs1(d->B(6) - s->B(6));
    val += abs1(d->B(7) - s->B(7));
    d->Q(0) = val;
#if SHIFT == 1
    val = 0;
    val += abs1(d->B(8) - s->B(8));
    val += abs1(d->B(9) - s->B(9));
    val += abs1(d->B(10) - s->B(10));
    val += abs1(d->B(11) - s->B(11));
    val += abs1(d->B(12) - s->B(12));
    val += abs1(d->B(13) - s->B(13));
    val += abs1(d->B(14) - s->B(14));
    val += abs1(d->B(15) - s->B(15));
    d->Q(1) = val;
#endif
}

void glue(helper_maskmov, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                  target_ulong a0)
{
    int i;

    for (i = 0; i < (8 << SHIFT); i++) {
        if (s->B(i) & 0x80) {
            cpu_stb_data_ra(env, a0 + i, d->B(i), GETPC());
        }
    }
}

void glue(helper_movl_mm_T0, SUFFIX)(Reg *d, uint32_t val)
{
    d->L(0) = val;
    d->L(1) = 0;
#if SHIFT == 1
    d->Q(1) = 0;
#endif
}

#ifdef TARGET_X86_64
void glue(helper_movq_mm_T0, SUFFIX)(Reg *d, uint64_t val)
{
    d->Q(0) = val;
#if SHIFT == 1
    d->Q(1) = 0;
#endif
}
#endif

#if SHIFT == 0
void glue(helper_pshufw, SUFFIX)(Reg *d, Reg *s, int order)
{
    Reg r;

    r.W(0) = s->W(order & 3);
    r.W(1) = s->W((order >> 2) & 3);
    r.W(2) = s->W((order >> 4) & 3);
    r.W(3) = s->W((order >> 6) & 3);
    *d = r;
}
#else
void helper_shufps(Reg *d, Reg *s, int order)
{
    Reg r;

    r.L(0) = d->L(order & 3);
    r.L(1) = d->L((order >> 2) & 3);
    r.L(2) = s->L((order >> 4) & 3);
    r.L(3) = s->L((order >> 6) & 3);
    *d = r;
}

void helper_shufpd(Reg *d, Reg *s, int order)
{
    Reg r;

    r.Q(0) = d->Q(order & 1);
    r.Q(1) = s->Q((order >> 1) & 1);
    *d = r;
}

void glue(helper_pshufd, SUFFIX)(Reg *d, Reg *s, int order)
{
    Reg r;

    r.L(0) = s->L(order & 3);
    r.L(1) = s->L((order >> 2) & 3);
    r.L(2) = s->L((order >> 4) & 3);
    r.L(3) = s->L((order >> 6) & 3);
    *d = r;
}

void glue(helper_pshuflw, SUFFIX)(Reg *d, Reg *s, int order)
{
    Reg r;

    r.W(0) = s->W(order & 3);
    r.W(1) = s->W((order >> 2) & 3);
    r.W(2) = s->W((order >> 4) & 3);
    r.W(3) = s->W((order >> 6) & 3);
    r.Q(1) = s->Q(1);
    *d = r;
}

void glue(helper_pshufhw, SUFFIX)(Reg *d, Reg *s, int order)
{
    Reg r;

    r.Q(0) = s->Q(0);
    r.W(4) = s->W(4 + (order & 3));
    r.W(5) = s->W(4 + ((order >> 2) & 3));
    r.W(6) = s->W(4 + ((order >> 4) & 3));
    r.W(7) = s->W(4 + ((order >> 6) & 3));
    *d = r;
}
#endif

#if SHIFT == 1
/* FPU ops */
/* XXX: not accurate */

#define SSE_HELPER_S(name, F)                                           \
    void helper_ ## name ## ps(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_S(0) = F(32, d->ZMM_S(0), s->ZMM_S(0));                  \
        d->ZMM_S(1) = F(32, d->ZMM_S(1), s->ZMM_S(1));                  \
        d->ZMM_S(2) = F(32, d->ZMM_S(2), s->ZMM_S(2));                  \
        d->ZMM_S(3) = F(32, d->ZMM_S(3), s->ZMM_S(3));                  \
    }                                                                   \
                                                                        \
    void helper_ ## name ## ss(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_S(0) = F(32, d->ZMM_S(0), s->ZMM_S(0));                  \
    }                                                                   \
                                                                        \
    void helper_ ## name ## pd(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_D(0) = F(64, d->ZMM_D(0), s->ZMM_D(0));                  \
        d->ZMM_D(1) = F(64, d->ZMM_D(1), s->ZMM_D(1));                  \
    }                                                                   \
                                                                        \
    void helper_ ## name ## sd(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_D(0) = F(64, d->ZMM_D(0), s->ZMM_D(0));                  \
    }

#define FPU_ADD(size, a, b) float ## size ## _add(a, b, &env->sse_status)
#define FPU_SUB(size, a, b) float ## size ## _sub(a, b, &env->sse_status)
#define FPU_MUL(size, a, b) float ## size ## _mul(a, b, &env->sse_status)
#define FPU_DIV(size, a, b) float ## size ## _div(a, b, &env->sse_status)
#define FPU_SQRT(size, a, b) float ## size ## _sqrt(b, &env->sse_status)

/* Note that the choice of comparison op here is important to get the
 * special cases right: for min and max Intel specifies that (-0,0),
 * (NaN, anything) and (anything, NaN) return the second argument.
 */
#define FPU_MIN(size, a, b)                                     \
    (float ## size ## _lt(a, b, &env->sse_status) ? (a) : (b))
#define FPU_MAX(size, a, b)                                     \
    (float ## size ## _lt(b, a, &env->sse_status) ? (a) : (b))

SSE_HELPER_S(add, FPU_ADD)
SSE_HELPER_S(sub, FPU_SUB)
SSE_HELPER_S(mul, FPU_MUL)
SSE_HELPER_S(div, FPU_DIV)
SSE_HELPER_S(min, FPU_MIN)
SSE_HELPER_S(max, FPU_MAX)
SSE_HELPER_S(sqrt, FPU_SQRT)


/* float to float conversions */
void helper_cvtps2pd(CPUX86State *env, Reg *d, Reg *s)
{
    float32 s0, s1;

    s0 = s->ZMM_S(0);
    s1 = s->ZMM_S(1);
    d->ZMM_D(0) = float32_to_float64(s0, &env->sse_status);
    d->ZMM_D(1) = float32_to_float64(s1, &env->sse_status);
}

void helper_cvtpd2ps(CPUX86State *env, Reg *d, Reg *s)
{
    d->ZMM_S(0) = float64_to_float32(s->ZMM_D(0), &env->sse_status);
    d->ZMM_S(1) = float64_to_float32(s->ZMM_D(1), &env->sse_status);
    d->Q(1) = 0;
}

void helper_cvtss2sd(CPUX86State *env, Reg *d, Reg *s)
{
    d->ZMM_D(0) = float32_to_float64(s->ZMM_S(0), &env->sse_status);
}

void helper_cvtsd2ss(CPUX86State *env, Reg *d, Reg *s)
{
    d->ZMM_S(0) = float64_to_float32(s->ZMM_D(0), &env->sse_status);
}

/* integer to float */
void helper_cvtdq2ps(CPUX86State *env, Reg *d, Reg *s)
{
    d->ZMM_S(0) = int32_to_float32(s->ZMM_L(0), &env->sse_status);
    d->ZMM_S(1) = int32_to_float32(s->ZMM_L(1), &env->sse_status);
    d->ZMM_S(2) = int32_to_float32(s->ZMM_L(2), &env->sse_status);
    d->ZMM_S(3) = int32_to_float32(s->ZMM_L(3), &env->sse_status);
}

void helper_cvtdq2pd(CPUX86State *env, Reg *d, Reg *s)
{
    int32_t l0, l1;

    l0 = (int32_t)s->ZMM_L(0);
    l1 = (int32_t)s->ZMM_L(1);
    d->ZMM_D(0) = int32_to_float64(l0, &env->sse_status);
    d->ZMM_D(1) = int32_to_float64(l1, &env->sse_status);
}

void helper_cvtpi2ps(CPUX86State *env, ZMMReg *d, MMXReg *s)
{
    d->ZMM_S(0) = int32_to_float32(s->MMX_L(0), &env->sse_status);
    d->ZMM_S(1) = int32_to_float32(s->MMX_L(1), &env->sse_status);
}

void helper_cvtpi2pd(CPUX86State *env, ZMMReg *d, MMXReg *s)
{
    d->ZMM_D(0) = int32_to_float64(s->MMX_L(0), &env->sse_status);
    d->ZMM_D(1) = int32_to_float64(s->MMX_L(1), &env->sse_status);
}

void helper_cvtsi2ss(CPUX86State *env, ZMMReg *d, uint32_t val)
{
    d->ZMM_S(0) = int32_to_float32(val, &env->sse_status);
}

void helper_cvtsi2sd(CPUX86State *env, ZMMReg *d, uint32_t val)
{
    d->ZMM_D(0) = int32_to_float64(val, &env->sse_status);
}

#ifdef TARGET_X86_64
void helper_cvtsq2ss(CPUX86State *env, ZMMReg *d, uint64_t val)
{
    d->ZMM_S(0) = int64_to_float32(val, &env->sse_status);
}

void helper_cvtsq2sd(CPUX86State *env, ZMMReg *d, uint64_t val)
{
    d->ZMM_D(0) = int64_to_float64(val, &env->sse_status);
}
#endif

/* float to integer */

/*
 * x86 mandates that we return the indefinite integer value for the result
 * of any float-to-integer conversion that raises the 'invalid' exception.
 * Wrap the softfloat functions to get this behaviour.
 */
#define WRAP_FLOATCONV(RETTYPE, FN, FLOATTYPE, INDEFVALUE)              \
    static inline RETTYPE x86_##FN(FLOATTYPE a, float_status *s)        \
    {                                                                   \
        int oldflags, newflags;                                         \
        RETTYPE r;                                                      \
                                                                        \
        oldflags = get_float_exception_flags(s);                        \
        set_float_exception_flags(0, s);                                \
        r = FN(a, s);                                                   \
        newflags = get_float_exception_flags(s);                        \
        if (newflags & float_flag_invalid) {                            \
            r = INDEFVALUE;                                             \
        }                                                               \
        set_float_exception_flags(newflags | oldflags, s);              \
        return r;                                                       \
    }

WRAP_FLOATCONV(int32_t, float32_to_int32, float32, INT32_MIN)
WRAP_FLOATCONV(int32_t, float32_to_int32_round_to_zero, float32, INT32_MIN)
WRAP_FLOATCONV(int32_t, float64_to_int32, float64, INT32_MIN)
WRAP_FLOATCONV(int32_t, float64_to_int32_round_to_zero, float64, INT32_MIN)
WRAP_FLOATCONV(int64_t, float32_to_int64, float32, INT64_MIN)
WRAP_FLOATCONV(int64_t, float32_to_int64_round_to_zero, float32, INT64_MIN)
WRAP_FLOATCONV(int64_t, float64_to_int64, float64, INT64_MIN)
WRAP_FLOATCONV(int64_t, float64_to_int64_round_to_zero, float64, INT64_MIN)

void helper_cvtps2dq(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_L(0) = x86_float32_to_int32(s->ZMM_S(0), &env->sse_status);
    d->ZMM_L(1) = x86_float32_to_int32(s->ZMM_S(1), &env->sse_status);
    d->ZMM_L(2) = x86_float32_to_int32(s->ZMM_S(2), &env->sse_status);
    d->ZMM_L(3) = x86_float32_to_int32(s->ZMM_S(3), &env->sse_status);
}

void helper_cvtpd2dq(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_L(0) = x86_float64_to_int32(s->ZMM_D(0), &env->sse_status);
    d->ZMM_L(1) = x86_float64_to_int32(s->ZMM_D(1), &env->sse_status);
    d->ZMM_Q(1) = 0;
}

void helper_cvtps2pi(CPUX86State *env, MMXReg *d, ZMMReg *s)
{
    d->MMX_L(0) = x86_float32_to_int32(s->ZMM_S(0), &env->sse_status);
    d->MMX_L(1) = x86_float32_to_int32(s->ZMM_S(1), &env->sse_status);
}

void helper_cvtpd2pi(CPUX86State *env, MMXReg *d, ZMMReg *s)
{
    d->MMX_L(0) = x86_float64_to_int32(s->ZMM_D(0), &env->sse_status);
    d->MMX_L(1) = x86_float64_to_int32(s->ZMM_D(1), &env->sse_status);
}

int32_t helper_cvtss2si(CPUX86State *env, ZMMReg *s)
{
    return x86_float32_to_int32(s->ZMM_S(0), &env->sse_status);
}

int32_t helper_cvtsd2si(CPUX86State *env, ZMMReg *s)
{
    return x86_float64_to_int32(s->ZMM_D(0), &env->sse_status);
}

#ifdef TARGET_X86_64
int64_t helper_cvtss2sq(CPUX86State *env, ZMMReg *s)
{
    return x86_float32_to_int64(s->ZMM_S(0), &env->sse_status);
}

int64_t helper_cvtsd2sq(CPUX86State *env, ZMMReg *s)
{
    return x86_float64_to_int64(s->ZMM_D(0), &env->sse_status);
}
#endif

/* float to integer truncated */
void helper_cvttps2dq(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_L(0) = x86_float32_to_int32_round_to_zero(s->ZMM_S(0), &env->sse_status);
    d->ZMM_L(1) = x86_float32_to_int32_round_to_zero(s->ZMM_S(1), &env->sse_status);
    d->ZMM_L(2) = x86_float32_to_int32_round_to_zero(s->ZMM_S(2), &env->sse_status);
    d->ZMM_L(3) = x86_float32_to_int32_round_to_zero(s->ZMM_S(3), &env->sse_status);
}

void helper_cvttpd2dq(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_L(0) = x86_float64_to_int32_round_to_zero(s->ZMM_D(0), &env->sse_status);
    d->ZMM_L(1) = x86_float64_to_int32_round_to_zero(s->ZMM_D(1), &env->sse_status);
    d->ZMM_Q(1) = 0;
}

void helper_cvttps2pi(CPUX86State *env, MMXReg *d, ZMMReg *s)
{
    d->MMX_L(0) = x86_float32_to_int32_round_to_zero(s->ZMM_S(0), &env->sse_status);
    d->MMX_L(1) = x86_float32_to_int32_round_to_zero(s->ZMM_S(1), &env->sse_status);
}

void helper_cvttpd2pi(CPUX86State *env, MMXReg *d, ZMMReg *s)
{
    d->MMX_L(0) = x86_float64_to_int32_round_to_zero(s->ZMM_D(0), &env->sse_status);
    d->MMX_L(1) = x86_float64_to_int32_round_to_zero(s->ZMM_D(1), &env->sse_status);
}

int32_t helper_cvttss2si(CPUX86State *env, ZMMReg *s)
{
    return x86_float32_to_int32_round_to_zero(s->ZMM_S(0), &env->sse_status);
}

int32_t helper_cvttsd2si(CPUX86State *env, ZMMReg *s)
{
    return x86_float64_to_int32_round_to_zero(s->ZMM_D(0), &env->sse_status);
}

#ifdef TARGET_X86_64
int64_t helper_cvttss2sq(CPUX86State *env, ZMMReg *s)
{
    return x86_float32_to_int64_round_to_zero(s->ZMM_S(0), &env->sse_status);
}

int64_t helper_cvttsd2sq(CPUX86State *env, ZMMReg *s)
{
    return x86_float64_to_int64_round_to_zero(s->ZMM_D(0), &env->sse_status);
}
#endif

void helper_rsqrtps(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    d->ZMM_S(0) = float32_div(float32_one,
                              float32_sqrt(s->ZMM_S(0), &env->sse_status),
                              &env->sse_status);
    d->ZMM_S(1) = float32_div(float32_one,
                              float32_sqrt(s->ZMM_S(1), &env->sse_status),
                              &env->sse_status);
    d->ZMM_S(2) = float32_div(float32_one,
                              float32_sqrt(s->ZMM_S(2), &env->sse_status),
                              &env->sse_status);
    d->ZMM_S(3) = float32_div(float32_one,
                              float32_sqrt(s->ZMM_S(3), &env->sse_status),
                              &env->sse_status);
    set_float_exception_flags(old_flags, &env->sse_status);
}

void helper_rsqrtss(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    d->ZMM_S(0) = float32_div(float32_one,
                              float32_sqrt(s->ZMM_S(0), &env->sse_status),
                              &env->sse_status);
    set_float_exception_flags(old_flags, &env->sse_status);
}

void helper_rcpps(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    d->ZMM_S(0) = float32_div(float32_one, s->ZMM_S(0), &env->sse_status);
    d->ZMM_S(1) = float32_div(float32_one, s->ZMM_S(1), &env->sse_status);
    d->ZMM_S(2) = float32_div(float32_one, s->ZMM_S(2), &env->sse_status);
    d->ZMM_S(3) = float32_div(float32_one, s->ZMM_S(3), &env->sse_status);
    set_float_exception_flags(old_flags, &env->sse_status);
}

void helper_rcpss(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    d->ZMM_S(0) = float32_div(float32_one, s->ZMM_S(0), &env->sse_status);
    set_float_exception_flags(old_flags, &env->sse_status);
}

static inline uint64_t helper_extrq(uint64_t src, int shift, int len)
{
    uint64_t mask;

    if (len == 0) {
        mask = ~0LL;
    } else {
        mask = (1ULL << len) - 1;
    }
    return (src >> shift) & mask;
}

void helper_extrq_r(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_Q(0) = helper_extrq(d->ZMM_Q(0), s->ZMM_B(1), s->ZMM_B(0));
}

void helper_extrq_i(CPUX86State *env, ZMMReg *d, int index, int length)
{
    d->ZMM_Q(0) = helper_extrq(d->ZMM_Q(0), index, length);
}

static inline uint64_t helper_insertq(uint64_t src, int shift, int len)
{
    uint64_t mask;

    if (len == 0) {
        mask = ~0ULL;
    } else {
        mask = (1ULL << len) - 1;
    }
    return (src & ~(mask << shift)) | ((src & mask) << shift);
}

void helper_insertq_r(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_Q(0) = helper_insertq(s->ZMM_Q(0), s->ZMM_B(9), s->ZMM_B(8));
}

void helper_insertq_i(CPUX86State *env, ZMMReg *d, int index, int length)
{
    d->ZMM_Q(0) = helper_insertq(d->ZMM_Q(0), index, length);
}

void helper_haddps(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    ZMMReg r;

    r.ZMM_S(0) = float32_add(d->ZMM_S(0), d->ZMM_S(1), &env->sse_status);
    r.ZMM_S(1) = float32_add(d->ZMM_S(2), d->ZMM_S(3), &env->sse_status);
    r.ZMM_S(2) = float32_add(s->ZMM_S(0), s->ZMM_S(1), &env->sse_status);
    r.ZMM_S(3) = float32_add(s->ZMM_S(2), s->ZMM_S(3), &env->sse_status);
    *d = r;
}

void helper_haddpd(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    ZMMReg r;

    r.ZMM_D(0) = float64_add(d->ZMM_D(0), d->ZMM_D(1), &env->sse_status);
    r.ZMM_D(1) = float64_add(s->ZMM_D(0), s->ZMM_D(1), &env->sse_status);
    *d = r;
}

void helper_hsubps(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    ZMMReg r;

    r.ZMM_S(0) = float32_sub(d->ZMM_S(0), d->ZMM_S(1), &env->sse_status);
    r.ZMM_S(1) = float32_sub(d->ZMM_S(2), d->ZMM_S(3), &env->sse_status);
    r.ZMM_S(2) = float32_sub(s->ZMM_S(0), s->ZMM_S(1), &env->sse_status);
    r.ZMM_S(3) = float32_sub(s->ZMM_S(2), s->ZMM_S(3), &env->sse_status);
    *d = r;
}

void helper_hsubpd(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    ZMMReg r;

    r.ZMM_D(0) = float64_sub(d->ZMM_D(0), d->ZMM_D(1), &env->sse_status);
    r.ZMM_D(1) = float64_sub(s->ZMM_D(0), s->ZMM_D(1), &env->sse_status);
    *d = r;
}

void helper_addsubps(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_S(0) = float32_sub(d->ZMM_S(0), s->ZMM_S(0), &env->sse_status);
    d->ZMM_S(1) = float32_add(d->ZMM_S(1), s->ZMM_S(1), &env->sse_status);
    d->ZMM_S(2) = float32_sub(d->ZMM_S(2), s->ZMM_S(2), &env->sse_status);
    d->ZMM_S(3) = float32_add(d->ZMM_S(3), s->ZMM_S(3), &env->sse_status);
}

void helper_addsubpd(CPUX86State *env, ZMMReg *d, ZMMReg *s)
{
    d->ZMM_D(0) = float64_sub(d->ZMM_D(0), s->ZMM_D(0), &env->sse_status);
    d->ZMM_D(1) = float64_add(d->ZMM_D(1), s->ZMM_D(1), &env->sse_status);
}

/* XXX: unordered */
#define SSE_HELPER_CMP(name, F)                                         \
    void helper_ ## name ## ps(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_L(0) = F(32, d->ZMM_S(0), s->ZMM_S(0));                  \
        d->ZMM_L(1) = F(32, d->ZMM_S(1), s->ZMM_S(1));                  \
        d->ZMM_L(2) = F(32, d->ZMM_S(2), s->ZMM_S(2));                  \
        d->ZMM_L(3) = F(32, d->ZMM_S(3), s->ZMM_S(3));                  \
    }                                                                   \
                                                                        \
    void helper_ ## name ## ss(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_L(0) = F(32, d->ZMM_S(0), s->ZMM_S(0));                  \
    }                                                                   \
                                                                        \
    void helper_ ## name ## pd(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_Q(0) = F(64, d->ZMM_D(0), s->ZMM_D(0));                  \
        d->ZMM_Q(1) = F(64, d->ZMM_D(1), s->ZMM_D(1));                  \
    }                                                                   \
                                                                        \
    void helper_ ## name ## sd(CPUX86State *env, Reg *d, Reg *s)        \
    {                                                                   \
        d->ZMM_Q(0) = F(64, d->ZMM_D(0), s->ZMM_D(0));                  \
    }

#define FPU_CMPEQ(size, a, b)                                           \
    (float ## size ## _eq_quiet(a, b, &env->sse_status) ? -1 : 0)
#define FPU_CMPLT(size, a, b)                                           \
    (float ## size ## _lt(a, b, &env->sse_status) ? -1 : 0)
#define FPU_CMPLE(size, a, b)                                           \
    (float ## size ## _le(a, b, &env->sse_status) ? -1 : 0)
#define FPU_CMPUNORD(size, a, b)                                        \
    (float ## size ## _unordered_quiet(a, b, &env->sse_status) ? -1 : 0)
#define FPU_CMPNEQ(size, a, b)                                          \
    (float ## size ## _eq_quiet(a, b, &env->sse_status) ? 0 : -1)
#define FPU_CMPNLT(size, a, b)                                          \
    (float ## size ## _lt(a, b, &env->sse_status) ? 0 : -1)
#define FPU_CMPNLE(size, a, b)                                          \
    (float ## size ## _le(a, b, &env->sse_status) ? 0 : -1)
#define FPU_CMPORD(size, a, b)                                          \
    (float ## size ## _unordered_quiet(a, b, &env->sse_status) ? 0 : -1)

SSE_HELPER_CMP(cmpeq, FPU_CMPEQ)
SSE_HELPER_CMP(cmplt, FPU_CMPLT)
SSE_HELPER_CMP(cmple, FPU_CMPLE)
SSE_HELPER_CMP(cmpunord, FPU_CMPUNORD)
SSE_HELPER_CMP(cmpneq, FPU_CMPNEQ)
SSE_HELPER_CMP(cmpnlt, FPU_CMPNLT)
SSE_HELPER_CMP(cmpnle, FPU_CMPNLE)
SSE_HELPER_CMP(cmpord, FPU_CMPORD)

static const int comis_eflags[4] = {CC_C, CC_Z, 0, CC_Z | CC_P | CC_C};

void helper_ucomiss(CPUX86State *env, Reg *d, Reg *s)
{
    FloatRelation ret;
    float32 s0, s1;

    s0 = d->ZMM_S(0);
    s1 = s->ZMM_S(0);
    ret = float32_compare_quiet(s0, s1, &env->sse_status);
    CC_SRC = comis_eflags[ret + 1];
}

void helper_comiss(CPUX86State *env, Reg *d, Reg *s)
{
    FloatRelation ret;
    float32 s0, s1;

    s0 = d->ZMM_S(0);
    s1 = s->ZMM_S(0);
    ret = float32_compare(s0, s1, &env->sse_status);
    CC_SRC = comis_eflags[ret + 1];
}

void helper_ucomisd(CPUX86State *env, Reg *d, Reg *s)
{
    FloatRelation ret;
    float64 d0, d1;

    d0 = d->ZMM_D(0);
    d1 = s->ZMM_D(0);
    ret = float64_compare_quiet(d0, d1, &env->sse_status);
    CC_SRC = comis_eflags[ret + 1];
}

void helper_comisd(CPUX86State *env, Reg *d, Reg *s)
{
    FloatRelation ret;
    float64 d0, d1;

    d0 = d->ZMM_D(0);
    d1 = s->ZMM_D(0);
    ret = float64_compare(d0, d1, &env->sse_status);
    CC_SRC = comis_eflags[ret + 1];
}

uint32_t helper_movmskps(CPUX86State *env, Reg *s)
{
    int b0, b1, b2, b3;

    b0 = s->ZMM_L(0) >> 31;
    b1 = s->ZMM_L(1) >> 31;
    b2 = s->ZMM_L(2) >> 31;
    b3 = s->ZMM_L(3) >> 31;
    return b0 | (b1 << 1) | (b2 << 2) | (b3 << 3);
}

uint32_t helper_movmskpd(CPUX86State *env, Reg *s)
{
    int b0, b1;

    b0 = s->ZMM_L(1) >> 31;
    b1 = s->ZMM_L(3) >> 31;
    return b0 | (b1 << 1);
}

#endif

uint32_t glue(helper_pmovmskb, SUFFIX)(CPUX86State *env, Reg *s)
{
    uint32_t val;

    val = 0;
    val |= (s->B(0) >> 7);
    val |= (s->B(1) >> 6) & 0x02;
    val |= (s->B(2) >> 5) & 0x04;
    val |= (s->B(3) >> 4) & 0x08;
    val |= (s->B(4) >> 3) & 0x10;
    val |= (s->B(5) >> 2) & 0x20;
    val |= (s->B(6) >> 1) & 0x40;
    val |= (s->B(7)) & 0x80;
#if SHIFT == 1
    val |= (s->B(8) << 1) & 0x0100;
    val |= (s->B(9) << 2) & 0x0200;
    val |= (s->B(10) << 3) & 0x0400;
    val |= (s->B(11) << 4) & 0x0800;
    val |= (s->B(12) << 5) & 0x1000;
    val |= (s->B(13) << 6) & 0x2000;
    val |= (s->B(14) << 7) & 0x4000;
    val |= (s->B(15) << 8) & 0x8000;
#endif
    return val;
}

void glue(helper_packsswb, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    Reg r;

    r.B(0) = satsb((int16_t)d->W(0));
    r.B(1) = satsb((int16_t)d->W(1));
    r.B(2) = satsb((int16_t)d->W(2));
    r.B(3) = satsb((int16_t)d->W(3));
#if SHIFT == 1
    r.B(4) = satsb((int16_t)d->W(4));
    r.B(5) = satsb((int16_t)d->W(5));
    r.B(6) = satsb((int16_t)d->W(6));
    r.B(7) = satsb((int16_t)d->W(7));
#endif
    r.B((4 << SHIFT) + 0) = satsb((int16_t)s->W(0));
    r.B((4 << SHIFT) + 1) = satsb((int16_t)s->W(1));
    r.B((4 << SHIFT) + 2) = satsb((int16_t)s->W(2));
    r.B((4 << SHIFT) + 3) = satsb((int16_t)s->W(3));
#if SHIFT == 1
    r.B(12) = satsb((int16_t)s->W(4));
    r.B(13) = satsb((int16_t)s->W(5));
    r.B(14) = satsb((int16_t)s->W(6));
    r.B(15) = satsb((int16_t)s->W(7));
#endif
    *d = r;
}

void glue(helper_packuswb, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    Reg r;

    r.B(0) = satub((int16_t)d->W(0));
    r.B(1) = satub((int16_t)d->W(1));
    r.B(2) = satub((int16_t)d->W(2));
    r.B(3) = satub((int16_t)d->W(3));
#if SHIFT == 1
    r.B(4) = satub((int16_t)d->W(4));
    r.B(5) = satub((int16_t)d->W(5));
    r.B(6) = satub((int16_t)d->W(6));
    r.B(7) = satub((int16_t)d->W(7));
#endif
    r.B((4 << SHIFT) + 0) = satub((int16_t)s->W(0));
    r.B((4 << SHIFT) + 1) = satub((int16_t)s->W(1));
    r.B((4 << SHIFT) + 2) = satub((int16_t)s->W(2));
    r.B((4 << SHIFT) + 3) = satub((int16_t)s->W(3));
#if SHIFT == 1
    r.B(12) = satub((int16_t)s->W(4));
    r.B(13) = satub((int16_t)s->W(5));
    r.B(14) = satub((int16_t)s->W(6));
    r.B(15) = satub((int16_t)s->W(7));
#endif
    *d = r;
}

void glue(helper_packssdw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    Reg r;

    r.W(0) = satsw(d->L(0));
    r.W(1) = satsw(d->L(1));
#if SHIFT == 1
    r.W(2) = satsw(d->L(2));
    r.W(3) = satsw(d->L(3));
#endif
    r.W((2 << SHIFT) + 0) = satsw(s->L(0));
    r.W((2 << SHIFT) + 1) = satsw(s->L(1));
#if SHIFT == 1
    r.W(6) = satsw(s->L(2));
    r.W(7) = satsw(s->L(3));
#endif
    *d = r;
}

#define UNPCK_OP(base_name, base)                                       \
                                                                        \
    void glue(helper_punpck ## base_name ## bw, SUFFIX)(CPUX86State *env,\
                                                        Reg *d, Reg *s) \
    {                                                                   \
        Reg r;                                                          \
                                                                        \
        r.B(0) = d->B((base << (SHIFT + 2)) + 0);                       \
        r.B(1) = s->B((base << (SHIFT + 2)) + 0);                       \
        r.B(2) = d->B((base << (SHIFT + 2)) + 1);                       \
        r.B(3) = s->B((base << (SHIFT + 2)) + 1);                       \
        r.B(4) = d->B((base << (SHIFT + 2)) + 2);                       \
        r.B(5) = s->B((base << (SHIFT + 2)) + 2);                       \
        r.B(6) = d->B((base << (SHIFT + 2)) + 3);                       \
        r.B(7) = s->B((base << (SHIFT + 2)) + 3);                       \
        XMM_ONLY(                                                       \
                 r.B(8) = d->B((base << (SHIFT + 2)) + 4);              \
                 r.B(9) = s->B((base << (SHIFT + 2)) + 4);              \
                 r.B(10) = d->B((base << (SHIFT + 2)) + 5);             \
                 r.B(11) = s->B((base << (SHIFT + 2)) + 5);             \
                 r.B(12) = d->B((base << (SHIFT + 2)) + 6);             \
                 r.B(13) = s->B((base << (SHIFT + 2)) + 6);             \
                 r.B(14) = d->B((base << (SHIFT + 2)) + 7);             \
                 r.B(15) = s->B((base << (SHIFT + 2)) + 7);             \
                                                                      ) \
            *d = r;                                                     \
    }                                                                   \
                                                                        \
    void glue(helper_punpck ## base_name ## wd, SUFFIX)(CPUX86State *env,\
                                                        Reg *d, Reg *s) \
    {                                                                   \
        Reg r;                                                          \
                                                                        \
        r.W(0) = d->W((base << (SHIFT + 1)) + 0);                       \
        r.W(1) = s->W((base << (SHIFT + 1)) + 0);                       \
        r.W(2) = d->W((base << (SHIFT + 1)) + 1);                       \
        r.W(3) = s->W((base << (SHIFT + 1)) + 1);                       \
        XMM_ONLY(                                                       \
                 r.W(4) = d->W((base << (SHIFT + 1)) + 2);              \
                 r.W(5) = s->W((base << (SHIFT + 1)) + 2);              \
                 r.W(6) = d->W((base << (SHIFT + 1)) + 3);              \
                 r.W(7) = s->W((base << (SHIFT + 1)) + 3);              \
                                                                      ) \
            *d = r;                                                     \
    }                                                                   \
                                                                        \
    void glue(helper_punpck ## base_name ## dq, SUFFIX)(CPUX86State *env,\
                                                        Reg *d, Reg *s) \
    {                                                                   \
        Reg r;                                                          \
                                                                        \
        r.L(0) = d->L((base << SHIFT) + 0);                             \
        r.L(1) = s->L((base << SHIFT) + 0);                             \
        XMM_ONLY(                                                       \
                 r.L(2) = d->L((base << SHIFT) + 1);                    \
                 r.L(3) = s->L((base << SHIFT) + 1);                    \
                                                                      ) \
            *d = r;                                                     \
    }                                                                   \
                                                                        \
    XMM_ONLY(                                                           \
             void glue(helper_punpck ## base_name ## qdq, SUFFIX)(CPUX86State \
                                                                  *env, \
                                                                  Reg *d, \
                                                                  Reg *s) \
             {                                                          \
                 Reg r;                                                 \
                                                                        \
                 r.Q(0) = d->Q(base);                                   \
                 r.Q(1) = s->Q(base);                                   \
                 *d = r;                                                \
             }                                                          \
                                                                        )

UNPCK_OP(l, 0)
UNPCK_OP(h, 1)

/* 3DNow! float ops */
#if SHIFT == 0
void helper_pi2fd(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_S(0) = int32_to_float32(s->MMX_L(0), &env->mmx_status);
    d->MMX_S(1) = int32_to_float32(s->MMX_L(1), &env->mmx_status);
}

void helper_pi2fw(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_S(0) = int32_to_float32((int16_t)s->MMX_W(0), &env->mmx_status);
    d->MMX_S(1) = int32_to_float32((int16_t)s->MMX_W(2), &env->mmx_status);
}

void helper_pf2id(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_L(0) = float32_to_int32_round_to_zero(s->MMX_S(0), &env->mmx_status);
    d->MMX_L(1) = float32_to_int32_round_to_zero(s->MMX_S(1), &env->mmx_status);
}

void helper_pf2iw(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_L(0) = satsw(float32_to_int32_round_to_zero(s->MMX_S(0),
                                                       &env->mmx_status));
    d->MMX_L(1) = satsw(float32_to_int32_round_to_zero(s->MMX_S(1),
                                                       &env->mmx_status));
}

void helper_pfacc(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    MMXReg r;

    r.MMX_S(0) = float32_add(d->MMX_S(0), d->MMX_S(1), &env->mmx_status);
    r.MMX_S(1) = float32_add(s->MMX_S(0), s->MMX_S(1), &env->mmx_status);
    *d = r;
}

void helper_pfadd(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_S(0) = float32_add(d->MMX_S(0), s->MMX_S(0), &env->mmx_status);
    d->MMX_S(1) = float32_add(d->MMX_S(1), s->MMX_S(1), &env->mmx_status);
}

void helper_pfcmpeq(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_L(0) = float32_eq_quiet(d->MMX_S(0), s->MMX_S(0),
                                   &env->mmx_status) ? -1 : 0;
    d->MMX_L(1) = float32_eq_quiet(d->MMX_S(1), s->MMX_S(1),
                                   &env->mmx_status) ? -1 : 0;
}

void helper_pfcmpge(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_L(0) = float32_le(s->MMX_S(0), d->MMX_S(0),
                             &env->mmx_status) ? -1 : 0;
    d->MMX_L(1) = float32_le(s->MMX_S(1), d->MMX_S(1),
                             &env->mmx_status) ? -1 : 0;
}

void helper_pfcmpgt(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_L(0) = float32_lt(s->MMX_S(0), d->MMX_S(0),
                             &env->mmx_status) ? -1 : 0;
    d->MMX_L(1) = float32_lt(s->MMX_S(1), d->MMX_S(1),
                             &env->mmx_status) ? -1 : 0;
}

void helper_pfmax(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    if (float32_lt(d->MMX_S(0), s->MMX_S(0), &env->mmx_status)) {
        d->MMX_S(0) = s->MMX_S(0);
    }
    if (float32_lt(d->MMX_S(1), s->MMX_S(1), &env->mmx_status)) {
        d->MMX_S(1) = s->MMX_S(1);
    }
}

void helper_pfmin(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    if (float32_lt(s->MMX_S(0), d->MMX_S(0), &env->mmx_status)) {
        d->MMX_S(0) = s->MMX_S(0);
    }
    if (float32_lt(s->MMX_S(1), d->MMX_S(1), &env->mmx_status)) {
        d->MMX_S(1) = s->MMX_S(1);
    }
}

void helper_pfmul(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_S(0) = float32_mul(d->MMX_S(0), s->MMX_S(0), &env->mmx_status);
    d->MMX_S(1) = float32_mul(d->MMX_S(1), s->MMX_S(1), &env->mmx_status);
}

void helper_pfnacc(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    MMXReg r;

    r.MMX_S(0) = float32_sub(d->MMX_S(0), d->MMX_S(1), &env->mmx_status);
    r.MMX_S(1) = float32_sub(s->MMX_S(0), s->MMX_S(1), &env->mmx_status);
    *d = r;
}

void helper_pfpnacc(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    MMXReg r;

    r.MMX_S(0) = float32_sub(d->MMX_S(0), d->MMX_S(1), &env->mmx_status);
    r.MMX_S(1) = float32_add(s->MMX_S(0), s->MMX_S(1), &env->mmx_status);
    *d = r;
}

void helper_pfrcp(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_S(0) = float32_div(float32_one, s->MMX_S(0), &env->mmx_status);
    d->MMX_S(1) = d->MMX_S(0);
}

void helper_pfrsqrt(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_L(1) = s->MMX_L(0) & 0x7fffffff;
    d->MMX_S(1) = float32_div(float32_one,
                              float32_sqrt(d->MMX_S(1), &env->mmx_status),
                              &env->mmx_status);
    d->MMX_L(1) |= s->MMX_L(0) & 0x80000000;
    d->MMX_L(0) = d->MMX_L(1);
}

void helper_pfsub(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_S(0) = float32_sub(d->MMX_S(0), s->MMX_S(0), &env->mmx_status);
    d->MMX_S(1) = float32_sub(d->MMX_S(1), s->MMX_S(1), &env->mmx_status);
}

void helper_pfsubr(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    d->MMX_S(0) = float32_sub(s->MMX_S(0), d->MMX_S(0), &env->mmx_status);
    d->MMX_S(1) = float32_sub(s->MMX_S(1), d->MMX_S(1), &env->mmx_status);
}

void helper_pswapd(CPUX86State *env, MMXReg *d, MMXReg *s)
{
    MMXReg r;

    r.MMX_L(0) = s->MMX_L(1);
    r.MMX_L(1) = s->MMX_L(0);
    *d = r;
}
#endif

/* SSSE3 op helpers */
void glue(helper_pshufb, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;
    Reg r;

    for (i = 0; i < (8 << SHIFT); i++) {
        r.B(i) = (s->B(i) & 0x80) ? 0 : (d->B(s->B(i) & ((8 << SHIFT) - 1)));
    }

    *d = r;
}

void glue(helper_phaddw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{

    Reg r;

    r.W(0) = (int16_t)d->W(0) + (int16_t)d->W(1);
    r.W(1) = (int16_t)d->W(2) + (int16_t)d->W(3);
    XMM_ONLY(r.W(2) = (int16_t)d->W(4) + (int16_t)d->W(5));
    XMM_ONLY(r.W(3) = (int16_t)d->W(6) + (int16_t)d->W(7));
    r.W((2 << SHIFT) + 0) = (int16_t)s->W(0) + (int16_t)s->W(1);
    r.W((2 << SHIFT) + 1) = (int16_t)s->W(2) + (int16_t)s->W(3);
    XMM_ONLY(r.W(6) = (int16_t)s->W(4) + (int16_t)s->W(5));
    XMM_ONLY(r.W(7) = (int16_t)s->W(6) + (int16_t)s->W(7));

    *d = r;
}

void glue(helper_phaddd, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    Reg r;

    r.L(0) = (int32_t)d->L(0) + (int32_t)d->L(1);
    XMM_ONLY(r.L(1) = (int32_t)d->L(2) + (int32_t)d->L(3));
    r.L((1 << SHIFT) + 0) = (int32_t)s->L(0) + (int32_t)s->L(1);
    XMM_ONLY(r.L(3) = (int32_t)s->L(2) + (int32_t)s->L(3));

    *d = r;
}

void glue(helper_phaddsw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    Reg r;

    r.W(0) = satsw((int16_t)d->W(0) + (int16_t)d->W(1));
    r.W(1) = satsw((int16_t)d->W(2) + (int16_t)d->W(3));
    XMM_ONLY(r.W(2) = satsw((int16_t)d->W(4) + (int16_t)d->W(5)));
    XMM_ONLY(r.W(3) = satsw((int16_t)d->W(6) + (int16_t)d->W(7)));
    r.W((2 << SHIFT) + 0) = satsw((int16_t)s->W(0) + (int16_t)s->W(1));
    r.W((2 << SHIFT) + 1) = satsw((int16_t)s->W(2) + (int16_t)s->W(3));
    XMM_ONLY(r.W(6) = satsw((int16_t)s->W(4) + (int16_t)s->W(5)));
    XMM_ONLY(r.W(7) = satsw((int16_t)s->W(6) + (int16_t)s->W(7)));

    *d = r;
}

void glue(helper_pmaddubsw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    d->W(0) = satsw((int8_t)s->B(0) * (uint8_t)d->B(0) +
                    (int8_t)s->B(1) * (uint8_t)d->B(1));
    d->W(1) = satsw((int8_t)s->B(2) * (uint8_t)d->B(2) +
                    (int8_t)s->B(3) * (uint8_t)d->B(3));
    d->W(2) = satsw((int8_t)s->B(4) * (uint8_t)d->B(4) +
                    (int8_t)s->B(5) * (uint8_t)d->B(5));
    d->W(3) = satsw((int8_t)s->B(6) * (uint8_t)d->B(6) +
                    (int8_t)s->B(7) * (uint8_t)d->B(7));
#if SHIFT == 1
    d->W(4) = satsw((int8_t)s->B(8) * (uint8_t)d->B(8) +
                    (int8_t)s->B(9) * (uint8_t)d->B(9));
    d->W(5) = satsw((int8_t)s->B(10) * (uint8_t)d->B(10) +
                    (int8_t)s->B(11) * (uint8_t)d->B(11));
    d->W(6) = satsw((int8_t)s->B(12) * (uint8_t)d->B(12) +
                    (int8_t)s->B(13) * (uint8_t)d->B(13));
    d->W(7) = satsw((int8_t)s->B(14) * (uint8_t)d->B(14) +
                    (int8_t)s->B(15) * (uint8_t)d->B(15));
#endif
}

void glue(helper_phsubw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    d->W(0) = (int16_t)d->W(0) - (int16_t)d->W(1);
    d->W(1) = (int16_t)d->W(2) - (int16_t)d->W(3);
    XMM_ONLY(d->W(2) = (int16_t)d->W(4) - (int16_t)d->W(5));
    XMM_ONLY(d->W(3) = (int16_t)d->W(6) - (int16_t)d->W(7));
    d->W((2 << SHIFT) + 0) = (int16_t)s->W(0) - (int16_t)s->W(1);
    d->W((2 << SHIFT) + 1) = (int16_t)s->W(2) - (int16_t)s->W(3);
    XMM_ONLY(d->W(6) = (int16_t)s->W(4) - (int16_t)s->W(5));
    XMM_ONLY(d->W(7) = (int16_t)s->W(6) - (int16_t)s->W(7));
}

void glue(helper_phsubd, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    d->L(0) = (int32_t)d->L(0) - (int32_t)d->L(1);
    XMM_ONLY(d->L(1) = (int32_t)d->L(2) - (int32_t)d->L(3));
    d->L((1 << SHIFT) + 0) = (int32_t)s->L(0) - (int32_t)s->L(1);
    XMM_ONLY(d->L(3) = (int32_t)s->L(2) - (int32_t)s->L(3));
}

void glue(helper_phsubsw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    d->W(0) = satsw((int16_t)d->W(0) - (int16_t)d->W(1));
    d->W(1) = satsw((int16_t)d->W(2) - (int16_t)d->W(3));
    XMM_ONLY(d->W(2) = satsw((int16_t)d->W(4) - (int16_t)d->W(5)));
    XMM_ONLY(d->W(3) = satsw((int16_t)d->W(6) - (int16_t)d->W(7)));
    d->W((2 << SHIFT) + 0) = satsw((int16_t)s->W(0) - (int16_t)s->W(1));
    d->W((2 << SHIFT) + 1) = satsw((int16_t)s->W(2) - (int16_t)s->W(3));
    XMM_ONLY(d->W(6) = satsw((int16_t)s->W(4) - (int16_t)s->W(5)));
    XMM_ONLY(d->W(7) = satsw((int16_t)s->W(6) - (int16_t)s->W(7)));
}

#define FABSB(_, x) (x > INT8_MAX  ? -(int8_t)x : x)
#define FABSW(_, x) (x > INT16_MAX ? -(int16_t)x : x)
#define FABSL(_, x) (x > INT32_MAX ? -(int32_t)x : x)
SSE_HELPER_B(helper_pabsb, FABSB)
SSE_HELPER_W(helper_pabsw, FABSW)
SSE_HELPER_L(helper_pabsd, FABSL)

#define FMULHRSW(d, s) (((int16_t) d * (int16_t)s + 0x4000) >> 15)
SSE_HELPER_W(helper_pmulhrsw, FMULHRSW)

#define FSIGNB(d, s) (s <= INT8_MAX  ? s ? d : 0 : -(int8_t)d)
#define FSIGNW(d, s) (s <= INT16_MAX ? s ? d : 0 : -(int16_t)d)
#define FSIGNL(d, s) (s <= INT32_MAX ? s ? d : 0 : -(int32_t)d)
SSE_HELPER_B(helper_psignb, FSIGNB)
SSE_HELPER_W(helper_psignw, FSIGNW)
SSE_HELPER_L(helper_psignd, FSIGNL)

void glue(helper_palignr, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                  int32_t shift)
{
    Reg r;

    /* XXX could be checked during translation */
    if (shift >= (16 << SHIFT)) {
        r.Q(0) = 0;
        XMM_ONLY(r.Q(1) = 0);
    } else {
        shift <<= 3;
#define SHR(v, i) (i < 64 && i > -64 ? i > 0 ? v >> (i) : (v << -(i)) : 0)
#if SHIFT == 0
        r.Q(0) = SHR(s->Q(0), shift - 0) |
            SHR(d->Q(0), shift -  64);
#else
        r.Q(0) = SHR(s->Q(0), shift - 0) |
            SHR(s->Q(1), shift -  64) |
            SHR(d->Q(0), shift - 128) |
            SHR(d->Q(1), shift - 192);
        r.Q(1) = SHR(s->Q(0), shift + 64) |
            SHR(s->Q(1), shift -   0) |
            SHR(d->Q(0), shift -  64) |
            SHR(d->Q(1), shift - 128);
#endif
#undef SHR
    }

    *d = r;
}


#define XMM0 (env->xmm_regs[0])

#if SHIFT == 1
#define SSE_HELPER_V(name, elem, num, F)                                \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)           \
    {                                                                   \
        d->elem(0) = F(d->elem(0), s->elem(0), XMM0.elem(0));           \
        d->elem(1) = F(d->elem(1), s->elem(1), XMM0.elem(1));           \
        if (num > 2) {                                                  \
            d->elem(2) = F(d->elem(2), s->elem(2), XMM0.elem(2));       \
            d->elem(3) = F(d->elem(3), s->elem(3), XMM0.elem(3));       \
            if (num > 4) {                                              \
                d->elem(4) = F(d->elem(4), s->elem(4), XMM0.elem(4));   \
                d->elem(5) = F(d->elem(5), s->elem(5), XMM0.elem(5));   \
                d->elem(6) = F(d->elem(6), s->elem(6), XMM0.elem(6));   \
                d->elem(7) = F(d->elem(7), s->elem(7), XMM0.elem(7));   \
                if (num > 8) {                                          \
                    d->elem(8) = F(d->elem(8), s->elem(8), XMM0.elem(8)); \
                    d->elem(9) = F(d->elem(9), s->elem(9), XMM0.elem(9)); \
                    d->elem(10) = F(d->elem(10), s->elem(10), XMM0.elem(10)); \
                    d->elem(11) = F(d->elem(11), s->elem(11), XMM0.elem(11)); \
                    d->elem(12) = F(d->elem(12), s->elem(12), XMM0.elem(12)); \
                    d->elem(13) = F(d->elem(13), s->elem(13), XMM0.elem(13)); \
                    d->elem(14) = F(d->elem(14), s->elem(14), XMM0.elem(14)); \
                    d->elem(15) = F(d->elem(15), s->elem(15), XMM0.elem(15)); \
                }                                                       \
            }                                                           \
        }                                                               \
    }

#define SSE_HELPER_I(name, elem, num, F)                                \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *s, uint32_t imm) \
    {                                                                   \
        d->elem(0) = F(d->elem(0), s->elem(0), ((imm >> 0) & 1));       \
        d->elem(1) = F(d->elem(1), s->elem(1), ((imm >> 1) & 1));       \
        if (num > 2) {                                                  \
            d->elem(2) = F(d->elem(2), s->elem(2), ((imm >> 2) & 1));   \
            d->elem(3) = F(d->elem(3), s->elem(3), ((imm >> 3) & 1));   \
            if (num > 4) {                                              \
                d->elem(4) = F(d->elem(4), s->elem(4), ((imm >> 4) & 1)); \
                d->elem(5) = F(d->elem(5), s->elem(5), ((imm >> 5) & 1)); \
                d->elem(6) = F(d->elem(6), s->elem(6), ((imm >> 6) & 1)); \
                d->elem(7) = F(d->elem(7), s->elem(7), ((imm >> 7) & 1)); \
                if (num > 8) {                                          \
                    d->elem(8) = F(d->elem(8), s->elem(8), ((imm >> 8) & 1)); \
                    d->elem(9) = F(d->elem(9), s->elem(9), ((imm >> 9) & 1)); \
                    d->elem(10) = F(d->elem(10), s->elem(10),           \
                                    ((imm >> 10) & 1));                 \
                    d->elem(11) = F(d->elem(11), s->elem(11),           \
                                    ((imm >> 11) & 1));                 \
                    d->elem(12) = F(d->elem(12), s->elem(12),           \
                                    ((imm >> 12) & 1));                 \
                    d->elem(13) = F(d->elem(13), s->elem(13),           \
                                    ((imm >> 13) & 1));                 \
                    d->elem(14) = F(d->elem(14), s->elem(14),           \
                                    ((imm >> 14) & 1));                 \
                    d->elem(15) = F(d->elem(15), s->elem(15),           \
                                    ((imm >> 15) & 1));                 \
                }                                                       \
            }                                                           \
        }                                                               \
    }

/* SSE4.1 op helpers */
#define FBLENDVB(d, s, m) ((m & 0x80) ? s : d)
#define FBLENDVPS(d, s, m) ((m & 0x80000000) ? s : d)
#define FBLENDVPD(d, s, m) ((m & 0x8000000000000000LL) ? s : d)
SSE_HELPER_V(helper_pblendvb, B, 16, FBLENDVB)
SSE_HELPER_V(helper_blendvps, L, 4, FBLENDVPS)
SSE_HELPER_V(helper_blendvpd, Q, 2, FBLENDVPD)

void glue(helper_ptest, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    uint64_t zf = (s->Q(0) &  d->Q(0)) | (s->Q(1) &  d->Q(1));
    uint64_t cf = (s->Q(0) & ~d->Q(0)) | (s->Q(1) & ~d->Q(1));

    CC_SRC = (zf ? 0 : CC_Z) | (cf ? 0 : CC_C);
}

#define SSE_HELPER_F(name, elem, num, F)        \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)     \
    {                                           \
        if (num > 2) {                          \
            if (num > 4) {                      \
                d->elem(7) = F(7);              \
                d->elem(6) = F(6);              \
                d->elem(5) = F(5);              \
                d->elem(4) = F(4);              \
            }                                   \
            d->elem(3) = F(3);                  \
            d->elem(2) = F(2);                  \
        }                                       \
        d->elem(1) = F(1);                      \
        d->elem(0) = F(0);                      \
    }

SSE_HELPER_F(helper_pmovsxbw, W, 8, (int8_t) s->B)
SSE_HELPER_F(helper_pmovsxbd, L, 4, (int8_t) s->B)
SSE_HELPER_F(helper_pmovsxbq, Q, 2, (int8_t) s->B)
SSE_HELPER_F(helper_pmovsxwd, L, 4, (int16_t) s->W)
SSE_HELPER_F(helper_pmovsxwq, Q, 2, (int16_t) s->W)
SSE_HELPER_F(helper_pmovsxdq, Q, 2, (int32_t) s->L)
SSE_HELPER_F(helper_pmovzxbw, W, 8, s->B)
SSE_HELPER_F(helper_pmovzxbd, L, 4, s->B)
SSE_HELPER_F(helper_pmovzxbq, Q, 2, s->B)
SSE_HELPER_F(helper_pmovzxwd, L, 4, s->W)
SSE_HELPER_F(helper_pmovzxwq, Q, 2, s->W)
SSE_HELPER_F(helper_pmovzxdq, Q, 2, s->L)

void glue(helper_pmuldq, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    d->Q(0) = (int64_t)(int32_t) d->L(0) * (int32_t) s->L(0);
    d->Q(1) = (int64_t)(int32_t) d->L(2) * (int32_t) s->L(2);
}

#define FCMPEQQ(d, s) (d == s ? -1 : 0)
SSE_HELPER_Q(helper_pcmpeqq, FCMPEQQ)

void glue(helper_packusdw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    Reg r;

    r.W(0) = satuw((int32_t) d->L(0));
    r.W(1) = satuw((int32_t) d->L(1));
    r.W(2) = satuw((int32_t) d->L(2));
    r.W(3) = satuw((int32_t) d->L(3));
    r.W(4) = satuw((int32_t) s->L(0));
    r.W(5) = satuw((int32_t) s->L(1));
    r.W(6) = satuw((int32_t) s->L(2));
    r.W(7) = satuw((int32_t) s->L(3));
    *d = r;
}

#define FMINSB(d, s) MIN((int8_t)d, (int8_t)s)
#define FMINSD(d, s) MIN((int32_t)d, (int32_t)s)
#define FMAXSB(d, s) MAX((int8_t)d, (int8_t)s)
#define FMAXSD(d, s) MAX((int32_t)d, (int32_t)s)
SSE_HELPER_B(helper_pminsb, FMINSB)
SSE_HELPER_L(helper_pminsd, FMINSD)
SSE_HELPER_W(helper_pminuw, MIN)
SSE_HELPER_L(helper_pminud, MIN)
SSE_HELPER_B(helper_pmaxsb, FMAXSB)
SSE_HELPER_L(helper_pmaxsd, FMAXSD)
SSE_HELPER_W(helper_pmaxuw, MAX)
SSE_HELPER_L(helper_pmaxud, MAX)

#define FMULLD(d, s) ((int32_t)d * (int32_t)s)
SSE_HELPER_L(helper_pmulld, FMULLD)

void glue(helper_phminposuw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int idx = 0;

    if (s->W(1) < s->W(idx)) {
        idx = 1;
    }
    if (s->W(2) < s->W(idx)) {
        idx = 2;
    }
    if (s->W(3) < s->W(idx)) {
        idx = 3;
    }
    if (s->W(4) < s->W(idx)) {
        idx = 4;
    }
    if (s->W(5) < s->W(idx)) {
        idx = 5;
    }
    if (s->W(6) < s->W(idx)) {
        idx = 6;
    }
    if (s->W(7) < s->W(idx)) {
        idx = 7;
    }

    d->W(0) = s->W(idx);
    d->W(1) = idx;
    d->L(1) = 0;
    d->Q(1) = 0;
}

void glue(helper_roundps, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                  uint32_t mode)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    signed char prev_rounding_mode;

    prev_rounding_mode = env->sse_status.float_rounding_mode;
    if (!(mode & (1 << 2))) {
        set_x86_rounding_mode(mode & 3, &env->sse_status);
    }

    d->ZMM_S(0) = float32_round_to_int(s->ZMM_S(0), &env->sse_status);
    d->ZMM_S(1) = float32_round_to_int(s->ZMM_S(1), &env->sse_status);
    d->ZMM_S(2) = float32_round_to_int(s->ZMM_S(2), &env->sse_status);
    d->ZMM_S(3) = float32_round_to_int(s->ZMM_S(3), &env->sse_status);

    if (mode & (1 << 3) && !(old_flags & float_flag_inexact)) {
        set_float_exception_flags(get_float_exception_flags(&env->sse_status) &
                                  ~float_flag_inexact,
                                  &env->sse_status);
    }
    env->sse_status.float_rounding_mode = prev_rounding_mode;
}

void glue(helper_roundpd, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                  uint32_t mode)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    signed char prev_rounding_mode;

    prev_rounding_mode = env->sse_status.float_rounding_mode;
    if (!(mode & (1 << 2))) {
        set_x86_rounding_mode(mode & 3, &env->sse_status);
    }

    d->ZMM_D(0) = float64_round_to_int(s->ZMM_D(0), &env->sse_status);
    d->ZMM_D(1) = float64_round_to_int(s->ZMM_D(1), &env->sse_status);

    if (mode & (1 << 3) && !(old_flags & float_flag_inexact)) {
        set_float_exception_flags(get_float_exception_flags(&env->sse_status) &
                                  ~float_flag_inexact,
                                  &env->sse_status);
    }
    env->sse_status.float_rounding_mode = prev_rounding_mode;
}

void glue(helper_roundss, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                  uint32_t mode)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    signed char prev_rounding_mode;

    prev_rounding_mode = env->sse_status.float_rounding_mode;
    if (!(mode & (1 << 2))) {
        set_x86_rounding_mode(mode & 3, &env->sse_status);
    }

    d->ZMM_S(0) = float32_round_to_int(s->ZMM_S(0), &env->sse_status);

    if (mode & (1 << 3) && !(old_flags & float_flag_inexact)) {
        set_float_exception_flags(get_float_exception_flags(&env->sse_status) &
                                  ~float_flag_inexact,
                                  &env->sse_status);
    }
    env->sse_status.float_rounding_mode = prev_rounding_mode;
}

void glue(helper_roundsd, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                  uint32_t mode)
{
    uint8_t old_flags = get_float_exception_flags(&env->sse_status);
    signed char prev_rounding_mode;

    prev_rounding_mode = env->sse_status.float_rounding_mode;
    if (!(mode & (1 << 2))) {
        set_x86_rounding_mode(mode & 3, &env->sse_status);
    }

    d->ZMM_D(0) = float64_round_to_int(s->ZMM_D(0), &env->sse_status);

    if (mode & (1 << 3) && !(old_flags & float_flag_inexact)) {
        set_float_exception_flags(get_float_exception_flags(&env->sse_status) &
                                  ~float_flag_inexact,
                                  &env->sse_status);
    }
    env->sse_status.float_rounding_mode = prev_rounding_mode;
}

#define FBLENDP(d, s, m) (m ? s : d)
SSE_HELPER_I(helper_blendps, L, 4, FBLENDP)
SSE_HELPER_I(helper_blendpd, Q, 2, FBLENDP)
SSE_HELPER_I(helper_pblendw, W, 8, FBLENDP)

void glue(helper_dpps, SUFFIX)(CPUX86State *env, Reg *d, Reg *s, uint32_t mask)
{
    float32 iresult = float32_zero;

    if (mask & (1 << 4)) {
        iresult = float32_add(iresult,
                              float32_mul(d->ZMM_S(0), s->ZMM_S(0),
                                          &env->sse_status),
                              &env->sse_status);
    }
    if (mask & (1 << 5)) {
        iresult = float32_add(iresult,
                              float32_mul(d->ZMM_S(1), s->ZMM_S(1),
                                          &env->sse_status),
                              &env->sse_status);
    }
    if (mask & (1 << 6)) {
        iresult = float32_add(iresult,
                              float32_mul(d->ZMM_S(2), s->ZMM_S(2),
                                          &env->sse_status),
                              &env->sse_status);
    }
    if (mask & (1 << 7)) {
        iresult = float32_add(iresult,
                              float32_mul(d->ZMM_S(3), s->ZMM_S(3),
                                          &env->sse_status),
                              &env->sse_status);
    }
    d->ZMM_S(0) = (mask & (1 << 0)) ? iresult : float32_zero;
    d->ZMM_S(1) = (mask & (1 << 1)) ? iresult : float32_zero;
    d->ZMM_S(2) = (mask & (1 << 2)) ? iresult : float32_zero;
    d->ZMM_S(3) = (mask & (1 << 3)) ? iresult : float32_zero;
}

void glue(helper_dppd, SUFFIX)(CPUX86State *env, Reg *d, Reg *s, uint32_t mask)
{
    float64 iresult = float64_zero;

    if (mask & (1 << 4)) {
        iresult = float64_add(iresult,
                              float64_mul(d->ZMM_D(0), s->ZMM_D(0),
                                          &env->sse_status),
                              &env->sse_status);
    }
    if (mask & (1 << 5)) {
        iresult = float64_add(iresult,
                              float64_mul(d->ZMM_D(1), s->ZMM_D(1),
                                          &env->sse_status),
                              &env->sse_status);
    }
    d->ZMM_D(0) = (mask & (1 << 0)) ? iresult : float64_zero;
    d->ZMM_D(1) = (mask & (1 << 1)) ? iresult : float64_zero;
}

void glue(helper_mpsadbw, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                  uint32_t offset)
{
    int s0 = (offset & 3) << 2;
    int d0 = (offset & 4) << 0;
    int i;
    Reg r;

    for (i = 0; i < 8; i++, d0++) {
        r.W(i) = 0;
        r.W(i) += abs1(d->B(d0 + 0) - s->B(s0 + 0));
        r.W(i) += abs1(d->B(d0 + 1) - s->B(s0 + 1));
        r.W(i) += abs1(d->B(d0 + 2) - s->B(s0 + 2));
        r.W(i) += abs1(d->B(d0 + 3) - s->B(s0 + 3));
    }

    *d = r;
}

/* SSE4.2 op helpers */
#define FCMPGTQ(d, s) ((int64_t)d > (int64_t)s ? -1 : 0)
SSE_HELPER_Q(helper_pcmpgtq, FCMPGTQ)

static inline int pcmp_elen(CPUX86State *env, int reg, uint32_t ctrl)
{
    target_long val, limit;

    /* Presence of REX.W is indicated by a bit higher than 7 set */
    if (ctrl >> 8) {
        val = (target_long)env->regs[reg];
    } else {
        val = (int32_t)env->regs[reg];
    }
    if (ctrl & 1) {
        limit = 8;
    } else {
        limit = 16;
    }
    if ((val > limit) || (val < -limit)) {
        return limit;
    }
    return abs1(val);
}

static inline int pcmp_ilen(Reg *r, uint8_t ctrl)
{
    int val = 0;

    if (ctrl & 1) {
        while (val < 8 && r->W(val)) {
            val++;
        }
    } else {
        while (val < 16 && r->B(val)) {
            val++;
        }
    }

    return val;
}

static inline int pcmp_val(Reg *r, uint8_t ctrl, int i)
{
    switch ((ctrl >> 0) & 3) {
    case 0:
        return r->B(i);
    case 1:
        return r->W(i);
    case 2:
        return (int8_t)r->B(i);
    case 3:
    default:
        return (int16_t)r->W(i);
    }
}

static inline unsigned pcmpxstrx(CPUX86State *env, Reg *d, Reg *s,
                                 int8_t ctrl, int valids, int validd)
{
    unsigned int res = 0;
    int v;
    int j, i;
    int upper = (ctrl & 1) ? 7 : 15;

    valids--;
    validd--;

    if (valids < upper) {
        env->eflags |= CC_Z;
    } else {
        env->eflags &= (~CC_Z);
    }
    if (validd < upper) {
        env->eflags |= CC_S;
    } else {
        env->eflags &= (~CC_S);
    }

    /*CC_SRC = (valids < upper ? CC_Z : 0) | (validd < upper ? CC_S : 0);*/

    switch ((ctrl >> 2) & 3) {
    case 0:
        for (j = valids; j >= 0; j--) {
            res <<= 1;
            v = pcmp_val(s, ctrl, j);
            for (i = validd; i >= 0; i--) {
                res |= (v == pcmp_val(d, ctrl, i));
            }
        }
        break;
    case 1:
        for (j = valids; j >= 0; j--) {
            res <<= 1;
            v = pcmp_val(s, ctrl, j);
            for (i = ((validd - 1) | 1); i >= 0; i -= 2) {
                res |= (pcmp_val(d, ctrl, i - 0) >= v &&
                        pcmp_val(d, ctrl, i - 1) <= v);
            }
        }
        break;
    case 2:
        res = (1 << (upper - MAX(valids, validd))) - 1;
        res <<= MAX(valids, validd) - MIN(valids, validd);
        for (i = MIN(valids, validd); i >= 0; i--) {
            res <<= 1;
            v = pcmp_val(s, ctrl, i);
            res |= (v == pcmp_val(d, ctrl, i));
        }
        break;
    case 3:
        if (validd == -1) {
            res = (2 << upper) - 1;
            break;
        }
        for (j = valids == upper ? valids : valids - validd; j >= 0; j--) {
            res <<= 1;
            v = 1;
            for (i = MIN(valids - j, validd); i >= 0; i--) {
                v &= (pcmp_val(s, ctrl, i + j) == pcmp_val(d, ctrl, i));
            }
            res |= v;
        }
        break;
    }

    switch ((ctrl >> 4) & 3) {
    case 1:
        res ^= (2 << upper) - 1;
        break;
    case 3:
        res ^= (1 << (valids + 1)) - 1;
        break;
    }

    if (res) {
        env->eflags |= CC_C;
        /*CC_SRC |= CC_C;*/
    } else {
        env->eflags &= (~CC_C);
    }
    if (res & 1) {
        env->eflags |= CC_O;
        /*CC_SRC |= CC_O;*/
    } else {
        env->eflags &= (~CC_O);
    }
    env->eflags &= (~CC_A);
    env->eflags &= (~CC_P);
    return res;
}

void glue(helper_pcmpestri, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                    uint32_t ctrl)
{
    unsigned int res = pcmpxstrx(env, d, s, ctrl,
                                 pcmp_elen(env, R_EDX, ctrl),
                                 pcmp_elen(env, R_EAX, ctrl));

    if (res) {
        env->regs[R_ECX] = (ctrl & (1 << 6)) ? 31 - clz32(res) : ctz32(res);
    } else {
        env->regs[R_ECX] = 16 >> (ctrl & (1 << 0));
    }
}

void glue(helper_pcmpestrm, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                    uint32_t ctrl)
{
    int i;
    unsigned int res = pcmpxstrx(env, d, s, ctrl,
                                 pcmp_elen(env, R_EDX, ctrl),
                                 pcmp_elen(env, R_EAX, ctrl));

    if ((ctrl >> 6) & 1) {
        if (ctrl & 1) {
            for (i = 0; i < 8; i++, res >>= 1) {
                env->xmm_regs[0].W(i) = (res & 1) ? ~0 : 0;
            }
        } else {
            for (i = 0; i < 16; i++, res >>= 1) {
                env->xmm_regs[0].B(i) = (res & 1) ? ~0 : 0;
            }
        }
    } else {
        env->xmm_regs[0].Q(1) = 0;
        env->xmm_regs[0].Q(0) = res;
    }
}

void glue(helper_pcmpistri, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                    uint32_t ctrl)
{
    unsigned int res = pcmpxstrx(env, d, s, ctrl,
                                 pcmp_ilen(s, ctrl),
                                 pcmp_ilen(d, ctrl));

    if (res) {
        env->regs[R_ECX] = (ctrl & (1 << 6)) ? 31 - clz32(res) : ctz32(res);
    } else {
        env->regs[R_ECX] = 16 >> (ctrl & (1 << 0));
    }
}

void glue(helper_pcmpistrm, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                    uint32_t ctrl)
{
    int i;
    unsigned int res = pcmpxstrx(env, d, s, ctrl,
                                 pcmp_ilen(s, ctrl),
                                 pcmp_ilen(d, ctrl));

    if ((ctrl >> 6) & 1) {
        if (ctrl & 1) {
            for (i = 0; i < 8; i++, res >>= 1) {
                env->xmm_regs[0].W(i) = (res & 1) ? ~0 : 0;
            }
        } else {
            for (i = 0; i < 16; i++, res >>= 1) {
                env->xmm_regs[0].B(i) = (res & 1) ? ~0 : 0;
            }
        }
    } else {
        env->xmm_regs[0].Q(1) = 0;
        env->xmm_regs[0].Q(0) = res;
    }
}

#define CRCPOLY        0x1edc6f41
#define CRCPOLY_BITREV 0x82f63b78
target_ulong helper_crc32(uint32_t crc1, target_ulong msg, uint32_t len)
{
    target_ulong crc = (msg & ((target_ulong) -1 >>
                               (TARGET_LONG_BITS - len))) ^ crc1;

    while (len--) {
        crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_BITREV : 0);
    }

    return crc;
}

void glue(helper_pclmulqdq, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                    uint32_t ctrl)
{
    uint64_t ah, al, b, resh, resl;

    ah = 0;
    al = d->Q((ctrl & 1) != 0);
    b = s->Q((ctrl & 16) != 0);
    resh = resl = 0;

    while (b) {
        if (b & 1) {
            resl ^= al;
            resh ^= ah;
        }
        ah = (ah << 1) | (al >> 63);
        al <<= 1;
        b >>= 1;
    }

    d->Q(0) = resl;
    d->Q(1) = resh;
}

void glue(helper_aesdec, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;
    Reg st = *d;
    Reg rk = *s;

    for (i = 0 ; i < 4 ; i++) {
        d->L(i) = rk.L(i) ^ bswap32(AES_Td0[st.B(AES_ishifts[4*i+0])] ^
                                    AES_Td1[st.B(AES_ishifts[4*i+1])] ^
                                    AES_Td2[st.B(AES_ishifts[4*i+2])] ^
                                    AES_Td3[st.B(AES_ishifts[4*i+3])]);
    }
}

void glue(helper_aesdeclast, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;
    Reg st = *d;
    Reg rk = *s;

    for (i = 0; i < 16; i++) {
        d->B(i) = rk.B(i) ^ (AES_isbox[st.B(AES_ishifts[i])]);
    }
}

void glue(helper_aesenc, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;
    Reg st = *d;
    Reg rk = *s;

    for (i = 0 ; i < 4 ; i++) {
        d->L(i) = rk.L(i) ^ bswap32(AES_Te0[st.B(AES_shifts[4*i+0])] ^
                                    AES_Te1[st.B(AES_shifts[4*i+1])] ^
                                    AES_Te2[st.B(AES_shifts[4*i+2])] ^
                                    AES_Te3[st.B(AES_shifts[4*i+3])]);
    }
}

void glue(helper_aesenclast, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;
    Reg st = *d;
    Reg rk = *s;

    for (i = 0; i < 16; i++) {
        d->B(i) = rk.B(i) ^ (AES_sbox[st.B(AES_shifts[i])]);
    }

}

void glue(helper_aesimc, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;
    Reg tmp = *s;

    for (i = 0 ; i < 4 ; i++) {
        d->L(i) = bswap32(AES_imc[tmp.B(4*i+0)][0] ^
                          AES_imc[tmp.B(4*i+1)][1] ^
                          AES_imc[tmp.B(4*i+2)][2] ^
                          AES_imc[tmp.B(4*i+3)][3]);
    }
}

void glue(helper_aeskeygenassist, SUFFIX)(CPUX86State *env, Reg *d, Reg *s,
                                          uint32_t ctrl)
{
    int i;
    Reg tmp = *s;

    for (i = 0 ; i < 4 ; i++) {
        d->B(i) = AES_sbox[tmp.B(i + 4)];
        d->B(i + 8) = AES_sbox[tmp.B(i + 12)];
    }
    d->L(1) = (d->L(0) << 24 | d->L(0) >> 8) ^ ctrl;
    d->L(3) = (d->L(2) << 24 | d->L(2) >> 8) ^ ctrl;
}

#endif
#endif

/* FMA3 op helpers */
#if SHIFT == 1
#define SSE_HELPER_FMAS(name, elem, F)                                         \
    void name(CPUX86State *env, Reg *d, Reg *a, Reg *b, Reg *c, int flags)     \
    {                                                                          \
        d->elem(0) = F(a->elem(0), b->elem(0), c->elem(0), flags, &env->sse_status); \
    }
#define SSE_HELPER_FMAP(name, elem, num, F)                                    \
    void glue(name, SUFFIX)(CPUX86State *env, Reg *d, Reg *a, Reg *b, Reg *c,  \
                            int flags, int flip)                               \
    {                                                                          \
        int i;                                                                 \
        for (i = 0; i < num; i++) {                                            \
            d->elem(i) = F(a->elem(i), b->elem(i), c->elem(i), flags, &env->sse_status); \
            flags ^= flip;                                                     \
        }                                                                      \
    }

SSE_HELPER_FMAS(helper_fma4ss,  ZMM_S, float32_muladd)
SSE_HELPER_FMAS(helper_fma4sd,  ZMM_D, float64_muladd)
#endif

#if SHIFT >= 1
SSE_HELPER_FMAP(helper_fma4ps,  ZMM_S, 2 << SHIFT, float32_muladd)
SSE_HELPER_FMAP(helper_fma4pd,  ZMM_D, 1 << SHIFT, float64_muladd)
#endif

#if SHIFT == 1
#define SSE_HELPER_SHA1RNDS4(name, F, K) \
    void name(CPUX86State *env, Reg *d, Reg *a, Reg *b)                     \
    {                                                                       \
        uint32_t A, B, C, D, E, t, i;                                       \
                                                                            \
        A = a->L(3);                                                        \
        B = a->L(2);                                                        \
        C = a->L(1);                                                        \
        D = a->L(0);                                                        \
        E = 0;                                                              \
                                                                            \
        for (i = 0; i <= 3; i++) {                                          \
            t = F(B, C, D) + rol32(A, 5) + b->L(3 - i) + E + K;             \
            E = D;                                                          \
            D = C;                                                          \
            C = rol32(B, 30);                                               \
            B = A;                                                          \
            A = t;                                                          \
        }                                                                   \
                                                                            \
        d->L(3) = A;                                                        \
        d->L(2) = B;                                                        \
        d->L(1) = C;                                                        \
        d->L(0) = D;                                                        \
    }

#define SHA1_F0(b, c, d) (((b) & (c)) ^ (~(b) & (d)))
#define SHA1_F1(b, c, d) ((b) ^ (c) ^ (d))
#define SHA1_F2(b, c, d) (((b) & (c)) ^ ((b) & (d)) ^ ((c) & (d)))

SSE_HELPER_SHA1RNDS4(helper_sha1rnds4_f0, SHA1_F0, 0x5A827999)
SSE_HELPER_SHA1RNDS4(helper_sha1rnds4_f1, SHA1_F1, 0x6ED9EBA1)
SSE_HELPER_SHA1RNDS4(helper_sha1rnds4_f2, SHA1_F2, 0x8F1BBCDC)
SSE_HELPER_SHA1RNDS4(helper_sha1rnds4_f3, SHA1_F1, 0xCA62C1D6)

void helper_sha1nexte(CPUX86State *env, Reg *d, Reg *a, Reg *b)
{
    d->L(3) = b->L(3) + rol32(a->L(3), 30);
    d->L(2) = b->L(2);
    d->L(1) = b->L(1);
    d->L(0) = b->L(0);
}

void helper_sha1msg1(CPUX86State *env, Reg *d, Reg *a, Reg *b)
{
    /* These could be overwritten by the first two assignments, save them.  */
    uint32_t b3 = b->L(3);
    uint32_t b2 = b->L(2);

    d->L(3) = a->L(3) ^ a->L(1);
    d->L(2) = a->L(2) ^ a->L(0);
    d->L(1) = a->L(1) ^ b3;
    d->L(0) = a->L(0) ^ b2;
}

void helper_sha1msg2(CPUX86State *env, Reg *d, Reg *a, Reg *b)
{
    d->L(3) = rol32(a->L(3) ^ b->L(2), 1);
    d->L(2) = rol32(a->L(2) ^ b->L(1), 1);
    d->L(1) = rol32(a->L(1) ^ b->L(0), 1);
    d->L(0) = rol32(a->L(0) ^ d->L(3), 1);
}

#define SHA256_CH(e, f, g)  (((e) & (f)) ^ (~(e) & (g)))
#define SHA256_MAJ(a, b, c) (((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)))

#define SHA256_RNDS0(w) (ror32((w), 2) ^ ror32((w), 13) ^ ror32((w), 22))
#define SHA256_RNDS1(w) (ror32((w), 6) ^ ror32((w), 11) ^ ror32((w), 25))
#define SHA256_MSGS0(w) (ror32((w), 7) ^ ror32((w), 18) ^ ((w) >> 3))
#define SHA256_MSGS1(w) (ror32((w), 17) ^ ror32((w), 19) ^ ((w) >> 10))

void helper_sha256rnds2(CPUX86State *env, Reg *d, Reg *a, Reg *b, uint32_t wk0, uint32_t wk1)
{
    uint32_t t, AA, EE;

    uint32_t A = b->L(3);
    uint32_t B = b->L(2);
    uint32_t C = a->L(3);
    uint32_t D = a->L(2);
    uint32_t E = b->L(1);
    uint32_t F = b->L(0);
    uint32_t G = a->L(1);
    uint32_t H = a->L(0);

    /* Even round */
    t = SHA256_CH(E, F, G) + SHA256_RNDS1(E) + wk0 + H;
    AA = t + SHA256_MAJ(A, B, C) + SHA256_RNDS0(A);
    EE = t + D;

    /* These will be B and F at the end of the odd round */
    d->L(2) = AA;
    d->L(0) = EE;

    D = C, C = B, B = A, A = AA;
    H = G, G = F, F = E, E = EE;

    /* Odd round */
    t = SHA256_CH(E, F, G) + SHA256_RNDS1(E) + wk1 + H;
    AA = t + SHA256_MAJ(A, B, C) + SHA256_RNDS0(A);
    EE = t + D;

    d->L(3) = AA;
    d->L(1) = EE;
}

void helper_sha256rnds2_xmm0(CPUX86State *env, Reg *d, Reg *a, Reg *b)
{
    uint32_t t, AA, EE;

    uint32_t wk0 = XMM0.L(0);
    uint32_t wk1 = XMM0.L(1);
    uint32_t A = b->L(3);
    uint32_t B = b->L(2);
    uint32_t C = a->L(3);
    uint32_t D = a->L(2);
    uint32_t E = b->L(1);
    uint32_t F = b->L(0);
    uint32_t G = a->L(1);
    uint32_t H = a->L(0);

    /* Even round */
    t = SHA256_CH(E, F, G) + SHA256_RNDS1(E) + wk0 + H;
    AA = t + SHA256_MAJ(A, B, C) + SHA256_RNDS0(A);
    EE = t + D;

    /* These will be B and F at the end of the odd round */
    d->L(2) = AA;
    d->L(0) = EE;

    D = C, C = B, B = A, A = AA;
    H = G, G = F, F = E, E = EE;

    /* Odd round */
    t = SHA256_CH(E, F, G) + SHA256_RNDS1(E) + wk1 + H;
    AA = t + SHA256_MAJ(A, B, C) + SHA256_RNDS0(A);
    EE = t + D;

    d->L(3) = AA;
    d->L(1) = EE;
}

void helper_sha256msg1(CPUX86State *env, Reg *d, Reg *a, Reg *b)
{
    /* b->L(0) could be overwritten by the first assignment, save it.  */
    uint32_t b0 = b->L(0);

    d->L(0) = a->L(0) + SHA256_MSGS0(a->L(1));
    d->L(1) = a->L(1) + SHA256_MSGS0(a->L(2));
    d->L(2) = a->L(2) + SHA256_MSGS0(a->L(3));
    d->L(3) = a->L(3) + SHA256_MSGS0(b0);
}

void helper_sha256msg2(CPUX86State *env, Reg *d, Reg *a, Reg *b)
{
    /* Earlier assignments cannot overwrite any of the two operands.  */
    d->L(0) = a->L(0) + SHA256_MSGS1(b->L(2));
    d->L(1) = a->L(1) + SHA256_MSGS1(b->L(3));
    /* Yes, this reuses the previously computed values.  */
    d->L(2) = a->L(2) + SHA256_MSGS1(d->L(0));
    d->L(3) = a->L(3) + SHA256_MSGS1(d->L(1));
}
#endif

#if SHIFT >= 1
void glue(helper_cvtph2ps, SUFFIX)(CPUX86State *env, Reg *d, Reg *s)
{
    int i;

    for (i = 2 << SHIFT; --i >= 0; ) {
         d->ZMM_S(i) = float16_to_float32(s->ZMM_H(i), true, &env->sse_status);
    }
}

void glue(helper_cvtps2ph, SUFFIX)(CPUX86State *env, Reg *d, Reg *s, int mode)
{
    int i;
    FloatRoundMode prev_rounding_mode = env->sse_status.float_rounding_mode;
    if (!(mode & (1 << 2))) {
        set_x86_rounding_mode(mode & 3, &env->sse_status);
    }

    for (i = 0; i < 2 << SHIFT; i++) {
        d->ZMM_H(i) = float32_to_float16(s->ZMM_S(i), true, &env->sse_status);
    }
    for (i >>= 2; i < 1 << SHIFT; i++) {
        d->Q(i) = 0;
    }

    env->sse_status.float_rounding_mode = prev_rounding_mode;
}
#endif

#undef SHIFT
#undef XMM_ONLY
#undef Reg
#undef B
#undef W
#undef L
#undef Q
#undef SUFFIX
