#pragma description ZISO 
#pragma MIME application/x-zso
#pragma magic [ 5A 49 53 4F ] @ 0x00
#pragma array_limit 5000000
#pragma pattern_limit 10000000

import std.io;

struct ZsoHeader {
    char magic[4] [[name("Magic")]];
    u32 header_size [[name("Header Size")]];
    u64 total_bytes [[name("Total Bytes")]];
    u32 block_size [[name("Block Size")]];
    u8 ver [[name("Version")]];
    u8 align [[name("Alignement")]];
    u8 rsv[2] [[hidden]];
};

ZsoHeader zso @ 0x00 [[name("ZISO Header")]];

struct ZsoIndex
{
    u32 raw_value [[hidden]];
    bool is_compressed = (raw_value & 0x80000000) == 0 [[export, name("Is Compressed")]];
    u32 address = (raw_value & 0x7FFFFFFF) << zso.align [[export, name("Address")]];
};

u32 block_count = zso.total_bytes / zso.block_size;
ZsoIndex indices[block_count] @ zso.header_size  [[name("Indices")]];
u32 data_start @ zso.header_size + block_count * 4;