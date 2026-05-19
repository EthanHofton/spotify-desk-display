#pragma once

#include <cstdint>
struct Pixel {
    
    Pixel(uint8_t t_r, uint8_t t_g, uint8_t t_b) {
        m_r = t_r;
        m_g = t_g;
        m_b = t_b;
    }

    Pixel(uint16_t t_rgb565) {
        // undo the byte-swap
        uint16_t v = (t_rgb565 >> 8) | (t_rgb565 << 8);

        // extract each channel and scale back to 8-bit
        m_r = ((v >> 11) & 0x1F) << 3;
        m_g = ((v >>  5) & 0x3F) << 2;
        m_b = ((v >>  0) & 0x1F) << 3;
    }

    static Pixel zero() {
        return Pixel(0,0,0);
    }

    uint8_t r() const { return m_r; }
    uint8_t g() const { return m_g; }
    uint8_t b() const { return m_b; }

    Pixel operator +(const Pixel& p) const {
        return Pixel(m_r + p.m_r, m_g + p.m_g, m_b + p.m_b);
    }

    Pixel operator -(const Pixel& p) const {
        return Pixel(m_r - p.m_r, m_g - p.m_g, m_b - p.m_b);
    }

    bool operator ==(const Pixel& p) const {
        return m_r == p.m_r && m_g == p.m_g && m_b == p.m_b;
    }

    bool operator !=(const Pixel& p) const {
        return !(*(this) == p);
    }

    uint16_t rgb565() const {
        /* ILI9341 expects big-endian RGB565 over SPI */
        uint16_t v = ((m_r >> 3) << 11) | ((m_g >> 2) << 5) | (m_b >> 3);
        return (v >> 8) | (v << 8);   /* swap bytes for big-endian wire format */
    }

private:

    uint8_t m_r;
    uint8_t m_g;
    uint8_t m_b;
};
