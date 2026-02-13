#include "BflimReader.h"
#include <fstream>
#include <iostream>

namespace BflimLib {

uint16_t BflimReader::swap16(uint16_t val) { return (val << 8) | (val >> 8); }
uint32_t BflimReader::swap32(uint32_t val) {
    return ((val >> 24) & 0xff) | ((val << 8) & 0xff0000) | ((val >> 8) & 0xff00) | ((val << 24) & 0xff000000);
}

bool BflimReader::load(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return false;

    // 1. Header am Ende lesen (0x14 Bytes) [cite: 14, 15]
    file.seekg(-20, std::ios::end);
    file.read(reinterpret_cast<char*>(&header), sizeof(BflimHeader));

    if (std::string(header.magic, 4) != "FLIM") return false;

    // Wii U Check: Wenn BOM 0xFEFF, müssen wir swappen [cite: 16]
    bool needsSwap = (header.bom == 0xFEFF);

    // 2. Imag-Block direkt davor lesen (0x14 Bytes) 
    file.seekg(-40, std::ios::end);
    file.read(reinterpret_cast<char*>(&imag), sizeof(ImagBlock));

    if (needsSwap) {
        imag.width = swap16(imag.width);
        imag.height = swap16(imag.height);
        imag.dataSize = swap32(imag.dataSize);
    }

    // 3. Rohdaten vom Anfang laden 
    rawData.resize(imag.dataSize);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(rawData.data()), rawData.size());

    return true;
}

// Vereinfachte GX2-Adressierung (Wii U spezifisch)
uint32_t BflimReader::computeSwizzleAddr(uint32_t x, uint32_t y, uint32_t width, uint32_t bpp, uint8_t tileMode) {
    // Standard Wii U Tiling nutzt 8x8 Tiles
    uint32_t tileX = x / 8;
    uint32_t tileY = y / 8;
    uint32_t inTileX = x % 8;
    uint32_t inTileY = y % 8;

    uint32_t pitch = (width + 7) / 8; // Tiles pro Zeile
    uint32_t tileIdx = (tileY * pitch) + tileX;
    uint32_t pixelIdx = (inTileY * 8) + inTileX;

    return (tileIdx * 64 + pixelIdx) * (bpp / 8);
}

std::vector<uint8_t> BflimReader::getLinearPixels() {
    std::vector<uint8_t> linear(imag.width * imag.height * 4); // Ziel: RGBA8
    uint8_t tileMode = imag.swizzleTile & 0x1F; // TTTTT 

    for (uint32_t y = 0; y < imag.height; ++y) {
        for (uint32_t x = 0; x < imag.width; ++x) {
            // Beispiel für RGBA8 (Format 0x09) 
            uint32_t swizzledAddr = computeSwizzleAddr(x, y, imag.width, 32, tileMode);
            uint32_t linearAddr = (y * imag.width + x) * 4;

            if (swizzledAddr + 3 < rawData.size()) {
                linear[linearAddr + 0] = rawData[swizzledAddr + 0];
                linear[linearAddr + 1] = rawData[swizzledAddr + 1];
                linear[linearAddr + 2] = rawData[swizzledAddr + 2];
                linear[linearAddr + 3] = rawData[swizzledAddr + 3];
            }
        }
    }
    return linear;
}

} // namespace BflimLib
