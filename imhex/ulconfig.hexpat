#pragma description OPL ul.cfg

import std.mem;

enum MediaType : u8 {
    CD = 0x12,
    DVD = 0x14
};

struct UlConfig {
    char name[32] [[name("Game Name")]];
    char image[15] [[name("Image ID")]];
    u8 parts [[name("Parts Count")]];
    MediaType media [[name("Media Type")]];
    padding[15];
};

UlConfig chunks[while($ < std::mem::size())] @ 0x00 [[name("Games")]];
