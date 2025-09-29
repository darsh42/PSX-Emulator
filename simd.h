#ifndef __SIMD_H__
#define __SIMD_H__

#if   defined(__AVX512F__)
#include "simd_avx512.h"
#elif defined(__AVX2__)
#include "simd_avx256.h"
#elif defined(__SSE__)
#include "simd_sse128.h"
#else
#error "Un-supported currently!"
#endif

#endif // __SIMD_H__
