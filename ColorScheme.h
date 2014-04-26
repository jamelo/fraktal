#ifndef ColorScheme_H
#define ColorScheme_H

#include <vector>
#include <QColor>

class ColorScheme
{
    private:
        std::vector<QColor> m_colors;

    public:
        ColorScheme() {}

        ColorScheme(std::vector<QColor> colors) :
            m_colors(colors)
        { }

        QColor calculateColor(double index);

        static ColorScheme Fire;
        static ColorScheme Ice;
        static ColorScheme Rainbow;
        static ColorScheme YellowBlue;
        static ColorScheme GreenYellow;
        static ColorScheme Grey;
};

#endif
