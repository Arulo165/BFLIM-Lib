#include <Bflim.h>
#include <BflimFormats.h>
#include <BinaryUtils.h>
#include <iostream>
#include <algorithm>
#include <ninTexUtils/gx2/gx2Surface.h>
#include <memory>
#include <bcdec.h>
#include <ninTexUtils/bcn/decompress.h>

bool Bflim::isValid(const std::vector<u8>& data)
{
    if (data.size() < 0x28) return false;

    u32 headerOffset = data.size() - 0x28; 

    return (data[headerOffset] == 'F' && 
            data[headerOffset + 1] == 'L' && 
            data[headerOffset + 2] == 'I' && 
            data[headerOffset + 3] == 'M');
}


void Bflim::parseBinary(const std::vector<u8>& data) {
    
    if (!isValid(data)) {
        return;
    }
    
    mRawData = data;
    
    parseImageInformation(data);
    
    size_t headerSize = 0x28; 
    if (data.size() > headerSize)
    {
        size_t imageDataSize = data.size() - headerSize;
        mImageData.assign(data.begin(), data.begin() + imageDataSize);
    }
}

void Bflim::parseImageInformation(const std::vector<u8>& data) {
    if (data.size() < 0x28) {
        return;
    }
    
    size_t imagOffset = data.size() - 0x28;
    
    for (size_t i = data.size() - 0x50; i < data.size() - 0x10; i++) {
        if (data[i] == 'i' && data[i+1] == 'm' && 
            data[i+2] == 'a' && data[i+3] == 'g') {
            imagOffset = i;
            break;
        }
    }
    
    mImageWidth = Read16(&data[imagOffset + 0x08]);
    mImageHeight = Read16(&data[imagOffset + 0x0A]);

    u16 FormatID = data[imagOffset + 0x0E];

    for(u32 i=0; i<BflimConstants::SupportedFormats.size(); i++)
    {
        if(BflimConstants::SupportedFormats[i].mId == FormatID)
        {
            mImageFormat = BflimConstants::SupportedFormats[i];
            break;
        }
    }

    u8 tileModeSwizzle = data[imagOffset + 0x0F];
    mTileMode = tileModeSwizzle & 0x1F;
    mSwizzle = (tileModeSwizzle >> 5) & 0x07;

    mPipeSwizzle = (mSwizzle >> 0) & 0x01;  
    mBankSwizzle = (mSwizzle >> 1) & 0x03;  
}

std::vector<u8> Bflim::deswizzleLinear(const std::vector<u8>& data , u32 Bpp)
{
    u16 pitch;
    u32 bytesPerPixel = Bpp / 8;

    if(mTileMode == 0)
    {
        pitch = mImageWidth;
    }
    else if(mTileMode ==1)
    {
        u32 pitchAlign = std::max(64u, 2048u / Bpp);
        pitch = alignTo(mImageWidth * bytesPerPixel, pitchAlign) / bytesPerPixel;
    }

    u32 vectorSize = mImageWidth * mImageHeight * bytesPerPixel;
    std::vector<u8> newData(vectorSize);

    for(u16 y=0; y < mImageHeight; y++)
    {
        for(u16 x=0; x < mImageWidth; x++)
        {
            u32 inputOffset = (y * pitch + x) * bytesPerPixel;
            u32 outputOffset = (y * mImageWidth + x) * bytesPerPixel;
            memcpy(&newData[outputOffset], &data[inputOffset], bytesPerPixel);
        }
    }

    return newData;
}

std::vector<u8> Bflim::deswizzleMacroTiled(const std::vector<u8>& data, u32 Bpp) {
    
    // Source Surface (geswizzelt)
    GX2Surface srcSurf = createGX2Surface();
    GX2CalcSurfaceSizeAndAlignment(&srcSurf);
    srcSurf.imagePtr.set(const_cast<u8*>(data.data()));
    
    // Dest Surface (linear)
    GX2Surface dstSurf;
    std::memset(&dstSurf, 0, sizeof(GX2Surface));
    dstSurf.dim = GX2_SURFACE_DIM_2D;
    dstSurf.width = mImageWidth;
    dstSurf.height = mImageHeight;
    dstSurf.depth = 1;
    dstSurf.numMips = 1;
    dstSurf.format = srcSurf.format;
    dstSurf.aa = GX2_AA_MODE_1X;
    dstSurf.use = GX2_SURFACE_USE_TEXTURE;
    dstSurf.tileMode = GX2_TILE_MODE_LINEAR_SPECIAL;  // ← LINEAR!
    dstSurf.swizzle = 0;
    GX2CalcSurfaceSizeAndAlignment(&dstSurf);
    
    // Output Buffer
    std::vector<u8> output(dstSurf.imageSize);
    dstSurf.imagePtr.set(output.data());
    dstSurf.mipPtr.set(nullptr);
    
    // Deswizzle!
    GX2CopySurface(&srcSurf, 0, 0, &dstSurf, 0, 0);
    
    return output;
}

GX2SurfaceFormat Bflim::bflimFormatToGX2(u8 bflimFormat) const {
    switch (bflimFormat) {
        case 0x00: return GX2_SURFACE_FORMAT_UNORM_R8;      // L8
        case 0x01: return GX2_SURFACE_FORMAT_UNORM_R8;      // A8
        case 0x03: return GX2_SURFACE_FORMAT_UNORM_RG8;     // LA8
        case 0x05: return GX2_SURFACE_FORMAT_UNORM_RGB565;  // RGB565
        case 0x07: return GX2_SURFACE_FORMAT_UNORM_RGB5A1;  // RGB5A1
        case 0x08: return GX2_SURFACE_FORMAT_UNORM_RGBA4;   // RGBA4
        case 0x09: return GX2_SURFACE_FORMAT_UNORM_RGBA8;   // RGBA8
        case 0x0C: return GX2_SURFACE_FORMAT_UNORM_BC1;     // BC1
        case 0x0D: return GX2_SURFACE_FORMAT_UNORM_BC2;     // BC2
        case 0x0E: return GX2_SURFACE_FORMAT_UNORM_BC3;     // BC3
        case 0x0F: return GX2_SURFACE_FORMAT_UNORM_BC4;     // BC4L
        case 0x10: return GX2_SURFACE_FORMAT_UNORM_BC4;     // BC4A
        case 0x11: return GX2_SURFACE_FORMAT_UNORM_BC5;     // BC5
        case 0x14: return GX2_SURFACE_FORMAT_SRGB_RGBA8;    // RGBA8 sRGB
        case 0x15: return GX2_SURFACE_FORMAT_SRGB_BC1;      // BC1 sRGB
        case 0x16: return GX2_SURFACE_FORMAT_SRGB_BC2;      // BC2 sRGB
        case 0x17: return GX2_SURFACE_FORMAT_SRGB_BC3;      // BC3 sRGB
        default:   return GX2_SURFACE_FORMAT_UNORM_RGBA8;
    }
}

GX2Surface Bflim::createGX2Surface() const {
    GX2Surface surf;
    std::memset(&surf, 0, sizeof(GX2Surface));
    
    surf.dim = GX2_SURFACE_DIM_2D;
    surf.width = mImageWidth;
    surf.height = mImageHeight;
    surf.depth = 1;
    surf.numMips = 1;
    surf.format = bflimFormatToGX2(mImageFormat.mId); 
    surf.aa = GX2_AA_MODE_1X;
    surf.use = GX2_SURFACE_USE_TEXTURE;
    surf.tileMode = (GX2TileMode)(mTileMode); 
    
    u8 pipeSwizzle = (mSwizzle >> 0) & 0x01;
    u8 bankSwizzle = (mSwizzle >> 1) & 0x03;
    surf.swizzle = ((bankSwizzle << 1) | pipeSwizzle) << 8;
    surf.swizzle |= 0xD0000;  


    
    return surf;
}

std::vector<u8> Bflim::decodeRGBA8(const std::vector<u8>& data) {
    std::vector<u8> output(data.size());
    
    for (size_t i = 0; i < data.size(); i += 4) {
        u32 pixel = Read32(&data[i]);
        
        output[i + 0] = (pixel >> 24) & 0xFF;  // R
        output[i + 1] = (pixel >> 16) & 0xFF;  // G
        output[i + 2] = (pixel >> 8) & 0xFF;   // B
        output[i + 3] = pixel & 0xFF;           // A
    }
    
    return output;
}

std::vector<u8> Bflim::decodeBC1(const std::vector<u8>& data) {
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    BCn_DecompressBC1(mImageWidth, mImageHeight, data.data(), output.data());
    return output;
}

std::vector<u8> Bflim::decodeBC2(const std::vector<u8>& data) {
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    BCn_DecompressBC2(mImageWidth, mImageHeight, data.data(), output.data());
    return output;
}

std::vector<u8> Bflim::decodeBC3(const std::vector<u8>& data) {
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    BCn_DecompressBC3(mImageWidth, mImageHeight, data.data(), output.data());
    return output;
}

std::vector<u8> Bflim::decodeBC4L(const std::vector<u8>& data) {
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    BCn_DecompressBC4U(mImageWidth, mImageHeight, data.data(), output.data());
    
    for (size_t i = 0; i < mImageWidth * mImageHeight; i++) {
        u8 r = output[i * 4 + 0];
        output[i * 4 + 0] = r;  // R
        output[i * 4 + 1] = r;  // G = R
        output[i * 4 + 2] = r;  // B = R
        output[i * 4 + 3] = 255; // A = voll
    }
    return output;
}

std::vector<u8> Bflim::decodeBC4A(const std::vector<u8>& data) {
    std::vector<u8> raw(mImageWidth * mImageHeight * 4);
    BCn_DecompressBC4U(mImageWidth, mImageHeight, data.data(), raw.data());
    
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    for (size_t i = 0; i < mImageWidth * mImageHeight; i++) {
        u8 r = raw[i * 4 + 0];  // Alpha-Wert
        output[i * 4 + 0] = r;  // R = Alpha
        output[i * 4 + 1] = r;  // G = Alpha
        output[i * 4 + 2] = r;  // B = Alpha
        output[i * 4 + 3] = 255; // A = voll sichtbar
    }
    return output;
}

std::vector<u8> Bflim::decodeBC5(const std::vector<u8>& data) {
    std::vector<u8> raw(mImageWidth * mImageHeight * 4);
    BCn_DecompressBC5U(mImageWidth, mImageHeight, data.data(), raw.data());

    std::cout << "BC5 Pixel 0: R=" << (int)raw[0] 
              << " G=" << (int)raw[1]
              << " B=" << (int)raw[2]
              << " A=" << (int)raw[3] << std::endl;
    
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    for (size_t i = 0; i < mImageWidth * mImageHeight; i++) {
        u8 r = raw[i * 4 + 0];
        u8 g = raw[i * 4 + 1];
        output[i * 4 + 0] = r;
        output[i * 4 + 1] = r;   // Grau = R
        output[i * 4 + 2] = r;
        output[i * 4 + 3] = 255;
    }
    return output;
}

std::vector<u8> Bflim::decodeL8(const std::vector<u8>& data) {
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    for (size_t i = 0; i < mImageWidth * mImageHeight; i++) {
        u8 l = data[i];         // Ein Byte pro Pixel!
        output[i * 4 + 0] = l; // R
        output[i * 4 + 1] = l; // G
        output[i * 4 + 2] = l; // B
        output[i * 4 + 3] = 255;// A
    }
    return output;
}

std::vector<u8> Bflim::decodeLA8(const std::vector<u8>& data) {
    std::vector<u8> output(mImageWidth * mImageHeight * 4);
    for (size_t i = 0; i < mImageWidth * mImageHeight; i++) {
        u8 l = data[i * 2 + 0]; // Luminance
        u8 a = data[i * 2 + 1]; // Alpha
        output[i * 4 + 0] = l;  // R
        output[i * 4 + 1] = l;  // G
        output[i * 4 + 2] = l;  // B
        output[i * 4 + 3] = a;  // A
    }
    return output;
}

std::vector<u8> Bflim::getDeswizzledRGBA(const std::vector<u8>& data) {
    
    std::vector<u8> linear;
    
   
    if (mTileMode == 0 || mTileMode == 1) {
        linear = deswizzleLinear(data, mImageFormat.mBPP); 
    }
    else if (mTileMode == 2 || mTileMode == 3) {
        linear = deswizzleMacroTiled(data, mImageFormat.mBPP); 
    }
    else if (mTileMode >= 4 && mTileMode <= 15) {
        linear = deswizzleMacroTiled(data, mImageFormat.mBPP); 
    }
    else {
        std::cerr << "Unknown TileMode: " << (int)mTileMode << std::endl;
        return std::vector<u8>(mImageWidth * mImageHeight * 4, 128);
    }
    
    switch (mImageFormat.mId) {
        case 0x01: return decodeL8(linear);
        case 0x03: return decodeLA8(linear);
        case 0x09: return decodeRGBA8(linear);
        case 0x0C: return decodeBC1(linear);
        case 0x0D: return decodeBC2(linear);
        case 0x0E: return decodeBC3(linear);
        case 0x0F: return decodeBC4L(linear);  // Luminance
        case 0x10: return decodeBC4A(linear);  // Alpha
        case 0x11: return decodeBC5(linear);   // RG
        default:
            std::cerr << "Format 0x" << std::hex << (int)mImageFormat.mId 
                      << " not supported yet!" << std::dec << std::endl;
            return std::vector<u8>(mImageWidth * mImageHeight * 4, 128);
    }
}

std::vector<u8> Bflim::encodeRGBA8(const std::vector<u8>& rgbaData) {
    std::vector<u8> output(rgbaData.size());
    
    for (size_t i = 0; i < rgbaData.size(); i += 4) {
        u8 r = rgbaData[i + 0];
        u8 g = rgbaData[i + 1];
        u8 b = rgbaData[i + 2];
        u8 a = rgbaData[i + 3];
        
        // Direkt Big Endian schreiben (wie es gespeichert ist)
        output[i + 0] = r;
        output[i + 1] = g;
        output[i + 2] = b;
        output[i + 3] = a;
    }
    
    return output;
}

std::vector<u8> Bflim::swizzleMacroTiled(const std::vector<u8>& linearData) {
    
    GX2Surface srcSurf;
    std::memset(&srcSurf, 0, sizeof(GX2Surface));
    srcSurf.dim = GX2_SURFACE_DIM_2D;
    srcSurf.width = mImageWidth;
    srcSurf.height = mImageHeight;
    srcSurf.depth = 1;
    srcSurf.numMips = 1;
    srcSurf.format = bflimFormatToGX2(mImageFormat.mId);
    srcSurf.aa = GX2_AA_MODE_1X;
    srcSurf.use = GX2_SURFACE_USE_TEXTURE;
    srcSurf.tileMode = GX2_TILE_MODE_LINEAR_SPECIAL;  // ← LINEAR!
    srcSurf.swizzle = 0;
    GX2CalcSurfaceSizeAndAlignment(&srcSurf);
    srcSurf.imagePtr.set(const_cast<u8*>(linearData.data()));

    GX2Surface dstSurf = createGX2Surface();
    GX2CalcSurfaceSizeAndAlignment(&dstSurf);
    
    std::vector<u8> output(dstSurf.imageSize);
    dstSurf.imagePtr.set(output.data());
    dstSurf.mipPtr.set(nullptr);
    
    // Swizzle!
    GX2CopySurface(&srcSurf, 0, 0, &dstSurf, 0, 0);
    
    return output;
}

bool Bflim::replaceWithRGBA(const std::vector<u8>& rgbaData) {
    if (mTileMode < 2 || mTileMode > 15) {
        std::cerr << "Only tiled modes (2-15) supported!" << std::endl;
        return false;
    }

    std::vector<u8> encoded;

    switch (mImageFormat.mId) {
        case 0x09: encoded = encodeRGBA8(rgbaData); break;
        case 0x0C: encoded = encodeBC1(rgbaData); break;
        case 0x0E: encoded = encodeBC3(rgbaData); break;
        default:
            std::cerr << "Format 0x" << std::hex << (int)mImageFormat.mId 
                      << " encoding not implemented!" << std::dec << std::endl;
            return false;
    }

    std::vector<u8> swizzled = swizzleMacroTiled(encoded);
    
    mImageData = swizzled;
    updateRawData();
    return true;
}

void Bflim::updateRawData() {
    
    if (mImageData.size() <= mRawData.size()) {
        std::memcpy(mRawData.data(), mImageData.data(), mImageData.size());
    } else {
        std::cerr << "Image data größer als Raw data buffer!" << std::endl;
    }
}

#define STB_DXT_IMPLEMENTATION
#include <stb_dxt.h>

std::vector<u8> Bflim::encodeBC1(const std::vector<u8>& rgbaData) {
    u32 blockWidth = (mImageWidth + 3) / 4;
    u32 blockHeight = (mImageHeight + 3) / 4;
    std::vector<u8> output(blockWidth * blockHeight * 8); // BC1 hat 8 Bytes pro Block

    for (u32 y = 0; y < mImageHeight; y += 4) {
        for (u32 x = 0; x < mImageWidth; x += 4) {
            u8 blockPixels[64]; // 4x4 RGBA
            // Hier Pixel aus rgbaData in den 4x4 Block kopieren (mit Padding für Ränder)
            fillBlock(rgbaData, blockPixels, x, y); 

            u32 blockIdx = ((y/4) * blockWidth + (x/4)) * 8;
            stb_compress_dxt_block(&output[blockIdx], blockPixels, 0, STB_DXT_NORMAL); // 0 = BC1
        }
    }
    return output;
}

std::vector<u8> Bflim::encodeBC3(const std::vector<u8>& rgbaData) {
    u32 blockWidth = (mImageWidth + 3) / 4;
    u32 blockHeight = (mImageHeight + 3) / 4;
    std::vector<u8> output(blockWidth * blockHeight * 16); // BC3 hat 16 Bytes pro Block

    for (u32 y = 0; y < mImageHeight; y += 4) {
        for (u32 x = 0; x < mImageWidth; x += 4) {
            u8 blockPixels[64];
            fillBlock(rgbaData, blockPixels, x, y);

            u32 blockIdx = ((y/4) * blockWidth + (x/4)) * 16;
            stb_compress_dxt_block(&output[blockIdx], blockPixels, 1, STB_DXT_NORMAL); // 1 = BC3
        }
    }
    return output;
}

void Bflim::fillBlock(const std::vector<u8>& rgbaData, u8* block, u32 startX, u32 startY) {
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            u32 currX = std::min(startX + j, (u32)mImageWidth - 1);
            u32 currY = std::min(startY + i, (u32)mImageHeight - 1);
            u32 rgbaIdx = (currY * mImageWidth + currX) * 4;
            u32 blockIdx = (i * 4 + j) * 4;

            block[blockIdx + 0] = rgbaData[rgbaIdx + 0];
            block[blockIdx + 1] = rgbaData[rgbaIdx + 1];
            block[blockIdx + 2] = rgbaData[rgbaIdx + 2];
            block[blockIdx + 3] = rgbaData[rgbaIdx + 3];
        }
    }
}
