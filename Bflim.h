#ifndef BFLIM_READER_H
#define BFLIM_READER_H

#include <vector>
#include <string>
#include <cstdint>

namespace BflimLib {

    // Formate laut Wexos's Wiki 
    enum class TextureFormat : uint8_t {
        L8 = 0x00,
        RGB565 = 0x05,
        RGBX8 = 0x06,
        RGBA8 = 0x09,
        BC1 = 0x0C, // DXT1
        BC3 = 0x0E, // DXT5
        RGBA8_SRGB = 0x14
    };

    struct BflimHeader {
        char magic[4];       // "FLIM" [cite: 16]
        uint16_t bom;        // Byte Order Mark (0xFEFF für Wii U) [cite: 16]
        uint16_t headerSize; // 0x14 [cite: 16]
        uint32_t version;    // [cite: 16]
        uint32_t fileSize;   // [cite: 16]
        uint16_t blockCount; // Immer 1 [cite: 16]
    };

    struct ImagBlock {
        char magic[4];       // "imag" 
        uint32_t blockSize;  // 
        uint16_t width;      // 
        uint16_t height;     // 
        uint16_t alignment;  // 
        uint8_t format;      // TextureFormat 
        uint8_t swizzleTile; // SSSTTTTT 
        uint32_t dataSize;   // 
    };

    class BflimReader {
    public:
        bool load(const std::string& filePath);
        std::vector<uint8_t> getLinearPixels();
        
    private:
        BflimHeader header;
        ImagBlock imag;
        std::vector<uint8_t> rawData;
        
        uint32_t computeSwizzleAddr(uint32_t x, uint32_t y, uint32_t width, uint32_t bpp, uint8_t tileMode);
        uint16_t swap16(uint16_t val);
        uint32_t swap32(uint32_t val);
    };
}

#endif
