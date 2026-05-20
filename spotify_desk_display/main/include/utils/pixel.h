#pragma once
#include <cstdint>
#include <algorithm>

struct Pixel {
    static constexpr uint8_t R_MAX = 31;
    static constexpr uint8_t G_MAX = 63;
    static constexpr uint8_t B_MAX = 31;

    // r: 0-31, g: 0-63, b: 0-31
    Pixel(uint8_t t_r, uint8_t t_g, uint8_t t_b) {
        m_rgb565 = pack(t_r, t_g, t_b);
    }

    // Accepts the byte-swapped (big-endian wire) format produced by rgb565()
    explicit Pixel(uint16_t t_rgb565) {
        m_rgb565 = swap_bytes(t_rgb565);
    }

    static Pixel zero() { return Pixel(uint8_t{0}, uint8_t{0}, uint8_t{0}); }

    // t_r/g/b in [0.0, 1.0] → mapped to native channel depth
    static Pixel from_normalised(float t_r, float t_g, float t_b) {
        t_r = std::clamp(t_r, 0.0f, 1.0f);
        t_g = std::clamp(t_g, 0.0f, 1.0f);
        t_b = std::clamp(t_b, 0.0f, 1.0f);
        return Pixel(
            static_cast<uint8_t>(t_r * R_MAX),
            static_cast<uint8_t>(t_g * G_MAX),
            static_cast<uint8_t>(t_b * B_MAX)
        );
    }

    // Accessors return native channel depth: r/b in 0-31, g in 0-63
    uint8_t r() const { return static_cast<uint8_t>((m_rgb565 >> 11) & 0x1F); }
    uint8_t g() const { return static_cast<uint8_t>((m_rgb565 >>  5) & 0x3F); }
    uint8_t b() const { return static_cast<uint8_t>((m_rgb565 >>  0) & 0x1F); }

    Pixel operator+(const Pixel& p) const {
        return Pixel(
            static_cast<uint8_t>(r() + p.r()),
            static_cast<uint8_t>(g() + p.g()),
            static_cast<uint8_t>(b() + p.b())
        );
    }
    Pixel operator-(const Pixel& p) const {
        return Pixel(
            static_cast<uint8_t>(r() - p.r()),
            static_cast<uint8_t>(g() - p.g()),
            static_cast<uint8_t>(b() - p.b())
        );
    }

    bool operator==(const Pixel& p) const { return m_rgb565 == p.m_rgb565; }
    bool operator!=(const Pixel& p) const { return m_rgb565 != p.m_rgb565; }

    // Returns big-endian (byte-swapped) RGB565 for ILI9341 SPI wire format
    uint16_t rgb565() const { return swap_bytes(m_rgb565); }

private:
    uint16_t m_rgb565;   // native host byte order: [R4:R0 G5:G0 B4:B0]

    static uint16_t pack(uint8_t r5, uint8_t g6, uint8_t b5) {
        return static_cast<uint16_t>(r5 & 0x1F) << 11
             | static_cast<uint16_t>(g6 & 0x3F) <<  5
             | (b5 & 0x1F);
    }
    static uint16_t swap_bytes(uint16_t v) { return (v >> 8) | (v << 8); }
};
