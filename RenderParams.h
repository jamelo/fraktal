#ifndef RenderParams_H
#define RenderParams_H

class RenderParams
{
    private:
        ColorScheme m_colors;
        ZoomRegion m_region;
        int m_antialiasing;

    public:
        RenderParams(ZoomRegion region, ColorScheme colors, int antialiasing = 1) :
            m_colors(colors),
            m_region(region),
            m_antialiasing(antialiasing)
        { }

        const ColorScheme& colorScheme() const { return m_colors; }
        const ZoomRegion& zoomRegion() const { return m_region; }
        int antialiasing() const { return m_antialiasing; }
};

#endif
