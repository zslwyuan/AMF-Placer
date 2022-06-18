#ifndef _TEST_CONFIG_H
#define _TEST_CONFIG_H

#define CORE_NUM 16
#define L2_CACHE_SIZE 2048  // L2 Cache 的大小 (in Bytes)
#define MEMORY_SIZE MEMORY_MEM_SIZE               // RAM 的大小 (in Bytes)

#define MEMORY_WR_SIZE (L2_CACHE_SIZE * 10)        // 小范围测试
#define MEMORY_WR_SIZE_BIT (L2_CACHE_SIZE * 4)

// #define MEMORY_WR_SIZE MEMORY_SIZE 
#define DEFINED_TYPE uint8_t                      // 访存最小单位的类型 (u_int8_t, u_int16_t, etc.)
#define TYPE_WIDTH (sizeof(DEFINED_TYPE) << 3)    // 计算出访存最小单位的宽度 (in bits) (e.g. 1 Byte = 8 bits)
#define TYPE_RANGE (1 << TYPE_WIDTH)              // 计算出访存最小单位的范围 (e.g. 1 Byte 的范围为 256)

#define OUTPUT_SHIFT 37                           // 在输出扫描的地址时，每 OUTPUT_SHIFT 个地址输出一次

#endif
