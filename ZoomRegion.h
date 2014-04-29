#ifndef ZoomRegion_H
#define ZoomRegion_H

#include "Point.h"

class ZoomRegion
{
    private:
        double m_x1;
        double m_y1;
        double m_x2;
        double m_y2;

    public:
        ZoomRegion(double x1, double y1, double x2, double y2) :
            m_x1(x1),
            m_y1(y1),
            m_x2(x2),
            m_y2(y2)
        { }

        ZoomRegion(const Point& center, double width, double height) :
            m_x1(center.x() - width * 0.5),
            m_y1(center.y() - height * 0.5),
            m_x2(center.x() + width * 0.5),
            m_y2(center.y() + height * 0.5)
        { }

        Point center() const     { return Point((m_x1 + m_x2) * 0.5, (m_y1 + m_y2) * 0.5); }
        Point location() const   { return Point(m_x1, m_y1); }
        double width() const     { return m_x2 - m_x1; }
        double height() const    { return m_y2 - m_y1; }
};

#endif
