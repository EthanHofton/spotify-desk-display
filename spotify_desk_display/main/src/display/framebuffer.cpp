#include "display/framebuffer.h"

#include <cstring>
#include <algorithm> 

FrameBuffer::FrameBuffer(uint16_t t_width, uint16_t t_height) 
    : m_width(t_width), m_height(t_height) {
    m_buffer = std::vector<Pixel>(m_width * m_height, Pixel::zero());
}

void FrameBuffer::set_pixel(const Point& t_pos, const Pixel&t_pixel) {
    m_buffer[t_pos.y() * m_width + t_pos.x()] = t_pixel;
}

const Pixel& FrameBuffer::get_pixel(const Point& t_pos) const {
    return m_buffer[t_pos.y() * m_width + t_pos.x()];
}

PixelView FrameBuffer::get_pixels(const Point& t_start, const Point& t_end) const {
    const uint16_t cols = t_end.x() - t_start.x() + 1;
    const uint16_t rows = t_end.y() - t_start.y() + 1;
    const Pixel* base   = m_buffer.data() + t_start.y() * m_width + t_start.x();

    return PixelView{
        {base,                        cols, m_width},
        {base + rows * m_width,       cols, m_width}
    };
}

void FrameBuffer::set_pixels(const Point& t_dst_start, const Point& t_dst_end, 
                              const FrameBuffer& t_src, const Point& t_src_start) {
    const uint16_t cols = t_dst_end.x() - t_dst_start.x() + 1;
    const uint16_t rows = t_dst_end.y() - t_dst_start.y() + 1;

    for (uint16_t y = 0; y < rows; ++y) {
        const std::size_t dst_offset = (t_dst_start.y() + y) * m_width       + t_dst_start.x();
        const std::size_t src_offset = (t_src_start.y() + y) * t_src.m_width + t_src_start.x();
        memcpy(m_buffer.data() + dst_offset, t_src.m_buffer.data() + src_offset, cols * sizeof(Pixel));
    }
}

void FrameBuffer::fill(const Pixel& t_color) {
    std::fill(m_buffer.begin(), m_buffer.end(), t_color);
}

void FrameBuffer::fill_region(const Point& t_start, const Point& t_end, const Pixel& t_color) {
    const uint16_t cols = t_end.x() - t_start.x() + 1;
    const uint16_t rows = t_end.y() - t_start.y() + 1;

    for (uint16_t y = 0; y < rows; ++y) {
        Pixel* row = m_buffer.data() + (t_start.y() + y) * m_width + t_start.x();
        std::fill(row, row + cols, t_color);
    }
}

