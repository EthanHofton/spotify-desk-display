#pragma once

#include <cstdint>

struct Point {

    Point(uint16_t t_x, uint16_t t_y) { m_x = t_x; m_y = t_y; }

    uint16_t x() const { return m_x; }
    uint16_t y() const { return m_y; }

    Point operator+ (const Point &p) const {
        return Point(m_x + p.m_x, m_y + p.m_y);
    }

    Point operator- (const Point &p) const {
        return Point(m_x - p.m_x, m_y - p.m_y);
    }

    bool operator==(const Point &p) const {
        return m_x == p.m_x && m_y == p.m_y;
    }

    bool operator !=(const Point &p) const {
        return !(*(this) == p);
    }
 
private:
    uint16_t m_x;
    uint16_t m_y;
};
