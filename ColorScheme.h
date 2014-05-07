#ifndef ColorScheme_H
#define ColorScheme_H

#include <vector>
#include <QColor>

class ColorScheme
{
    private:
        std::vector<QColor> m_colors;
        QColor m_interiorColor;
        bool m_logarithmic;
        bool m_cycleColors;

    public:
        ColorScheme() {}

        ColorScheme(std::vector<QColor> colors, QColor interiorColor = Qt::black, bool logarithmic = false, bool cycleColors = false) :
            m_colors(colors),
            m_interiorColor(interiorColor),
            m_logarithmic(logarithmic),
            m_cycleColors(cycleColors)
        { }

        QColor calculateColor(double index, int maxIterations) const;

        static ColorScheme Fire;
        static ColorScheme Ice;
        static ColorScheme Rainbow;
        static ColorScheme YellowBlue;
        static ColorScheme GreenYellow;
        static ColorScheme Grey;
};

#endif
