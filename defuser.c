#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

static const uint32_t p5_f8[12] = {
    2436, 2953, 1369, 1096, 335, 94, 817, 1655, 1907, 2664, 2169, 618
};
static const uint32_t p5_f12[12] = {
    49538, 23437, 62754, 60776, 41444, 55881, 31896, 2508, 8051, 39630, 47924, 38702
};

static uint32_t p6_op_0(uint32_t x) { return (x + 0x155eu) ^ (x >> 2); }
static uint32_t p6_op_1(uint32_t x) { return (x ^ 0xb107u) + 4u * x; }
static uint32_t p6_op_2(uint32_t x) { return ((x >> 2) ^ x) + 0xb201u; }
static uint32_t p6_op_3(uint32_t x) { return (8u * x + 0x55c3u) ^ (5u * x); }
static uint32_t p6_op_4(uint32_t x) { return (x >> 1) | (x << 31); }
static uint32_t p6_op_5(uint32_t x) { return ((x << 7) | (x >> 25)) ^ 0xada3u; }

static uint32_t (*const p6_ops[8])(uint32_t) = {
    p6_op_4, p6_op_1, p6_op_0, p6_op_4, p6_op_1, p6_op_3, p6_op_5, p6_op_2
};

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t lo  = (uint32_t)challenge;
    uint32_t hi  = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);

    uint32_t r0 = (hi ^ 0x3a76u) + 2u * lo;
    uint32_t r1 = (mid + 0xb483u) ^ hi;
    uint32_t r2 = 3u * r1 ^ (lo + 0x5e53u);

    values[0] = r2 ^ r0;
    count = 1;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    values[0] = (uint32_t)((challenge >> 38) & 0x1f);

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t bits = (uint32_t)((challenge >> (5 * i)) & 0x3f);
        values[i] = (bits + i) ^ (3u * values[i - 1]);
    }
    count = 6;

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t idx = (uint32_t)challenge & 7u;
    uint32_t v = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    uint32_t y = 0;

    switch (idx) {
        case 0: y = 5u * v + 0x2188bu; break;         
        case 1: y = (2u * v) ^ 0x8ea0u; break;
        case 2: y = 3u * v + 0x8ccfu; break;  
        case 3: y = (v ^ 0x5401u) + (v >> 2); break;
        case 4: y = 5u * v + 0x44976u; break;
        case 5: y = (4u * v) ^ 0xa229u; break; 
        case 6: y = (v ^ 0x505au) + (v >> 2); break; 
        case 7: y = 3u * v + 0x1f03u; break;
    }

    values[0] = idx;
    values[1] = y;
    count = 2;

    return write_answer(out, cap, values, count);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t a = (uint32_t)challenge & 0x7fu;
    uint32_t k = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);

    uint32_t mids[8];
    uint32_t dirs[8];
    size_t depth = 0;
    uint32_t lo = 0u, hi = 0x7fu, mid;

    for (;;) {
        mid = lo + (hi - lo) / 2u;
        if (mid == a) break;
        mids[depth] = mid + k;
        dirs[depth] = (a < mid) ? 1u : 0u;
        if (a < mid) {
            hi = mid - 1u;
        } else {
            lo = mid + 1u;
        }
        ++depth;
    }

    uint32_t result = (mid + 0x88u) ^ k;
    while (depth > 0) {
        --depth;
        result = 3u * result ^ mids[depth] ^ (dirs[depth] ? 0x16u : 0x1fu);
    }

    values[0] = result;
    count = 1;

    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t idx   = (uint32_t)challenge & 7u;
    uint32_t steps = (uint32_t)(challenge >> 3) & 3u;

    uint32_t v = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32)
               ^ p5_f12[idx] ^ 0xb500u;
    v += p5_f8[idx];
    uint32_t last = p5_f8[idx];

    uint32_t pos = idx;
    for (uint32_t i = 0; i < steps; ++i) {
        ++pos;
        v ^= p5_f12[pos];
        v += p5_f8[pos];
        last = p5_f8[pos];
    }

    values[0] = v;
    values[1] = last;
    count = 2;

    return write_answer(out, cap, values, count);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t v = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);

    uint32_t r = p6_ops[(uint32_t)challenge & 7u](v);
    uint32_t s = p6_ops[((uint32_t)challenge >> 3) & 7u](r ^ 0xef67u);

    values[0] = s + 0xf232u;
    count = 1;

    return write_answer(out, cap, values, count);
}

static uint32_t secret_mix(uint32_t x) {
    return (7u * x + 0x7cddu) ^ 0xf4f5u;
}
int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t c1 = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t c2 = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t x0 = (uint32_t)context & 3u;
    uint32_t r9 = 0x7cddu * x0 ^ (uint32_t)(context >> 16);

    uint32_t x1 = secret_mix(c1 ^ c2) ^ r9;
    uint32_t edx = 0xf4f5u + c2 + x0 + (c1 >> 3);
    uint32_t x2 = x1 ^ edx;

    int n = snprintf(out, cap,
        "0123456789abcdef0123456789abcdef" 
        "OPEN" 
        " %" PRIu32 " %" PRIu32 " %" PRIu32,
        x0, x1, x2);
    return n < 0 || (size_t)n >= cap;
}
