#pragma once

#include "utils/pixel.h"
#include "utils/pixel_view.h"
#include "utils/point.h"
#include <cstdint>
#include <vector> 

class FrameBuffer {
public:

    FrameBuffer(uint16_t t_width, uint16_t t_height);

    void set_pixel(const Point& t_pos, const Pixel& t_pixel);
    void set_pixels(const Point& t_start, const Point& t_end, const FrameBuffer& t_src, const Point& t_src_start);

    const Pixel& get_pixel(const Point& t_pos) const;
    PixelView get_pixels(const Point& t_start, const Point& t_end) const;

    void fill(const Pixel& t_color);
    void fill_region(const Point& t_start, const Point& t_end, const Pixel& t_color);

    const std::vector<Pixel>& get_buffer() const { return m_buffer; }

private:

    const uint16_t m_width;
    const uint16_t m_height;
    std::vector<Pixel> m_buffer;
};
