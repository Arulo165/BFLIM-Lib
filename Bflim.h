#pragma once
#include <string>
#include <vector>
#include <types.h>
#include <BflimFormats.h>
#include <ninTexUtils/gx2/gx2Surface.h>

class Bflim
{
public:
    bool isValid(const std::vector<u8>& data);
    void parseImageInformation(const std::vector<u8>& data);
    void parseBinary(const std::vector<u8>& data);

    std::vector<u8> deswizzleLinear(const std::vector<u8>& data , u32 Bpp);
    std::vector<u8> deswizzleMicroTiled(const std::vector<u8>& data, u32 Bpp);
    std::vector<u8> deswizzleMacroTiled(const std::vector<u8>& data, u32 Bpp);

    std::vector<u8> getDeswizzledRGBA(const std::vector<u8>& data);
    GX2SurfaceFormat bflimFormatToGX2(u8 bflimFormat)const; 

    std::vector<u8> decodeRGBA8(const std::vector<u8>& data);
    std::vector<u8> decodeBC1(const std::vector<u8>& data);
    std::vector<u8> decodeBC2(const std::vector<u8>& data);
    std::vector<u8> decodeBC3(const std::vector<u8>& data);
    std::vector<u8> decodeBC4L(const std::vector<u8>& data);
    std::vector<u8> decodeBC4A(const std::vector<u8>& data);
    std::vector<u8> decodeBC5(const std::vector<u8>& data);
    std::vector<u8> decodeL8(const std::vector<u8>& data);
    std::vector<u8> decodeLA8(const std::vector<u8>& data);

    GX2Surface createGX2Surface() const;

    void fillBlock(const std::vector<u8>& rgbaData, u8* block, u32 startX, u32 startY);
    
    bool replaceWithRGBA(const std::vector<u8>& rgbaData);

    std::vector<u8> swizzleMacroTiled(const std::vector<u8>& linearData);
    std::vector<u8> encodeRGBA8(const std::vector<u8>& rgbaData);

    std::vector<u8> encodeBC3(const std::vector<u8>& rgbaData);
    std::vector<u8> encodeBC1(const std::vector<u8>& rgbaData);

    void updateRawData();

    u16 getImageWidth()
    {
        return mImageWidth;
    }

    u16 getImageHeight()
    {
        return mImageHeight;
    }

    Format getImageFormat()
    {
        return mImageFormat;
    }

    std::vector<u8> getImageData()
    {
        return mImageData;
    }

    u8 getTileMode()
    {
        return mTileMode;
    }

    std::vector<u8> getRawData()
    {
        return mRawData;
    }

private:
    std::vector<u8> mImageData;
    u16 mImageWidth;
    u16 mImageHeight;
    Format mImageFormat;
    u8 mTileMode;
    u8 mSwizzle;
    u8 mPipeSwizzle; 
    u8 mBankSwizzle; 
    std::vector<u8> mRawData;
};
