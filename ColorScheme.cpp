#include "ColorScheme.h"

#include <cmath>
#include <initializer_list>

ColorScheme ColorScheme::Fire({Qt::black, Qt::red, QColor(255, 128, 0), Qt::yellow, Qt::white});
ColorScheme ColorScheme::Ice({QColor(0, 0, 40), QColor(0, 0, 80), Qt::blue, QColor(0, 128, 255), Qt::cyan, Qt::white});
ColorScheme ColorScheme::Rainbow({Qt::red, Qt::yellow, Qt::green, Qt::cyan, Qt::blue, Qt::magenta});
ColorScheme ColorScheme::YellowBlue;
ColorScheme ColorScheme::GreenYellow;
ColorScheme ColorScheme::Grey({Qt::black, Qt::white});

QColor ColorScheme::calculateColor ( double index, int maxIterations ) const {
    //double mappedIndex = m_logarithmic ? std::exp(index) - 1 : index;
    //int intIndex = m_cycleColors ? (int) index % m_colors.size() : (int) (index / maxIterations);

    if (index < 0.0) {
        return m_interiorColor;
    }

    //TODO: implement color cycling
    //TODO: implement logarithmic color distribution
    //TODO: implement mandelbrot set color

    double mappedIndex = index / (double) (maxIterations - 1) * (double) (m_colors.size() - 1) * 5;

    double intpart;
    //Add 0.5 for unbiased rounding
    double indexFrac = std::modf(mappedIndex, &intpart);

    int intIndex = ((int) intpart) % m_colors.size();

    if (intIndex == m_colors.size() - 1) {
        return m_colors[intIndex];
    }

    double one_minus_indexFrac = 1.0 - indexFrac;

    QColor color1 = m_colors[intIndex];
    QColor color2 = m_colors[intIndex + 1];

    //TODO: verify rounding
    double red   = color1.red()   * one_minus_indexFrac + color2.red()   * indexFrac;
    double green = color1.green() * one_minus_indexFrac + color2.green() * indexFrac;
    double blue  = color1.blue()  * one_minus_indexFrac + color2.blue()  * indexFrac;

    return QColor((int) red, (int) green, (int) blue);
}

