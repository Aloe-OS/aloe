#pragma once

#include <stdint.h>

typedef struct [[gnu::packed]] {
    uint32_t rsv0;
    uint32_t rsp0_lower;
    uint32_t rsp0_upper;
    uint32_t rsp1_lower;
    uint32_t rsp1_upper;
    uint32_t rsp2_lower;
    uint32_t rsp2_upper;
    uint32_t rsv1;
    uint32_t rsv2;
    struct {
        uint32_t addr_lower;
        uint32_t addr_upper;
    } ists[7];
    uint32_t rsv3;
    uint32_t rsv4;
    uint16_t rsv5;
    uint16_t iomap_base;
} Tss;

void tss_set_rsp0(Tss* tss, uintptr_t rsp);
