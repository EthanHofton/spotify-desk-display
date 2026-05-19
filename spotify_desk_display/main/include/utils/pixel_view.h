#pragma once

#include "pixel.h"

#include <span>

struct PixelView {
    struct RowIterator {
        const Pixel* row_start;
        uint16_t     row_width;   // columns in the region
        uint16_t     buf_width;   // full buffer stride

        std::span<const Pixel> operator*() const {
            return {row_start, row_width};
        }
        RowIterator& operator++() { row_start += buf_width; return *this; }
        bool operator!=(const RowIterator& o) const { return row_start != o.row_start; }
    };

    RowIterator begin_it, end_it;
    RowIterator begin() const { return begin_it; }
    RowIterator end()   const { return end_it; }
};
