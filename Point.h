#ifndef Point_H
#define Point_H

class Point
{
    private:
        double m_x;
        double m_y;

    public:
        Point() :
            m_x(0.0),
            m_y(0.0)
        { }
        
        Point(double x, double y) :
            m_x(x),
            m_y(y)
        { }

        double x() const { return m_x; }
        double y() const { return m_y; }
};

#endif
