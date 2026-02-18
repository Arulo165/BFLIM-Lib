# Bflim-Lib

A high-performance C++ library for parsing, de-swizzling, and re-encoding **BFLIM** (Binary Layout File for Images) files. This tool is specifically designed to handle textures from Nintendo Wii U and 3DS titles, supporting complex hardware-specific tiling modes.

## Table of Contents
* [Features](#features)
* [Dependencies](#dependencies)
* [Supported Formats](#supported-formats)
* [Installation](#installation)
* [Usage](#usage)
* [Technical Overview](#technical-overview)

---

## Features

- **Header Validation:** Safely detects the `FLIM` magic header (stored at the end of the file).
- **Advanced Deswizzling:** Supports Linear, Tiled, and Macro-Tiled (Wii U GX2) memory layouts.
- **In-place Injection:** Replace existing textures with new RGBA8 data; the tool handles the encoding and swizzling automatically.
- **Memory Efficient:** Uses `std::vector` for safe memory management and direct buffer manipulation.

## Dependencies

To build this library, you will need the following dependencies:

| Dependency | Purpose |
| :--- | :--- |
| **[ninTexUtils](https://github.com/aboood40091/ninTexUtils/tree/cpp)** | Core GX2 surface management and hardware swizzling logic. |
| **[stb_dxt](https://github.com/nothings/stb/blob/master/stb_dxt.h)** | Real-time DXT1/DXT5 (BC1/BC3) compression for texture replacement. |
| **[BinaryUtils](https://github.com/Arulo165/BinaryUtils)** | Internal project headers for Big-Endian/Little-Endian data handling. |

## Supported Formats

| Category | Formats |
| :--- | :--- |
| **Uncompressed** | L8, LA8, RGB565, RGBA4, RGBA8 |
| **Compressed** | BC1 (DXT1), BC2 (DXT3), BC3 (DXT5), BC4 (L/A), BC5 (RG) |
| **Color Space** | Linear and sRGB variants |

---
