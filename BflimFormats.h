#pragma once
#include <vector>
#include <string>
#include <types.h> 
#include <ninTexUtils/gx2/gx2Enum.h>

struct Format 
{
    u8 mId;
    std::string mName;
    std::string mType;
    u8 mBPP;
    std::string mSuffix;
};

namespace BflimConstants 
{
    inline const std::vector<Format> SupportedFormats = 
    {
    { 0x00,  "L8_UNORM",              "Luminance",              8,    "^c"      },
    { 0x01,  "A8_UNORM",              "Alpha",                  8,    "^d"      },
    { 0x02,  "LA4_UNORM",             "Luminance Alpha",        8,    "^e"      },
    { 0x03,  "LA8_UNORM",             "Luminance Alpha",        16,   "^f"      },
    { 0x04,  "HILO8",                 "High-Low",               16,   "^g"      },
    { 0x05,  "RGB565_UNORM",          "Color",                  16,   "^h"      },
    { 0x06,  "RGBX8_UNORM",           "Color",                  32,   "^i"      },
    { 0x07,  "RGB5A1_UNORM",          "Color Alpha",            16,   "^j"      },
    { 0x08,  "RGBA4_UNORM",           "Color Alpha",            16,   "^k"      },
    { 0x09,  "RGBA8_UNORM",           "Color Alpha",            32,   "^l"      },
   { 0x0A,  "ETC1_UNORM",            "ETC1",                   4,    "^m"      },
   { 0x0B,  "ETC1A4_UNORM",          "ETC1 Alpha 4",           8,    "^n"      },
   { 0x0C,  "BC1_UNORM",             "BC1 (DXT1)",             4,    "^o"      },
   { 0x0D,  "BC2_UNORM",             "BC2 (DXT3)",             8,    "^p"      },
   { 0x0E,  "BC3_UNORM",             "BC3 (DXT5)",             8,    "^q"      },
   { 0x0F,  "BC4L_UNORM",            "BC4 (Luminance)",        4,    "^r"      },
   { 0x10,  "BC4A_UNORM",            "BC4 (Alpha)",            4,    "^s"      },
   { 0x11,  "BC5_UNORM",             "BC5 (RG)",               8,    "^t"      },
   { 0x12,  "L4_UNORM",              "Luminance",              4,    "^u"      },
   { 0x13,  "A4_UNORM",              "Alpha",                  4,    "^v"      },
   { 0x14,  "RGBA8_SRGB",            "Color Alpha (sRGB)",     32,   "^w"      },
   { 0x15,  "BC1_SRGB",              "BC1 (sRGB)",             4,    "^x"      },
   { 0x16,  "BC2_SRGB",              "BC2 (sRGB)",             8,    "^y"      },
   { 0x17,  "BC3_SRGB",              "BC3 (sRGB)",             8,    "^z"      },
   { 0x18,  "RGB10A2_UNORM",         "Color Alpha (10-bit)",   32,   "unk"     },
   { 0x19,  "RGB565_INDIRECT_UNORM", "Color (Indirect)",       16,   "unk"     }
    };
}
