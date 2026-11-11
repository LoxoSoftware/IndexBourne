#ifndef ALGO_H
#define ALGO_H

#include <QImage>

#define __ALGO__SETPIXEL(dest, x, y, col) \
    { if (col>=0) dest->setPixel(x, y, col); else dest->setPixel(x, y, 1+(x+y&1)); }
    //^ draws the selection checkerboard if col is <0

void FillScanline(unsigned char* sl, int xleft, int xright, unsigned char col)
{
    for (int ix=xleft; ix<xright; ix++)
        sl[ix]= col;
}

void DrawEllipseToSurface(QImage* dest, int x0, int y0, int x1, int y1, bool fill, int col)
{
    //Credit: https://stackoverflow.com/a/54488533
    int xb, yb, xc, yc;

    // Calculate height
    yb = yc = (y0 + y1) / 2;
    int qb = (y0 < y1) ? (y1 - y0) : (y0 - y1);
    int qy = qb;
    int dy = qb / 2;
    if (qb % 2 != 0)
        // Bounding box has even pixel height
        yc++;

    // Calculate width
    xb = xc = (x0 + x1) / 2;
    int qa = (x0 < x1) ? (x1 - x0) : (x0 - x1);
    int qx = qa % 2;
    int dx = 0;
    long qt = (long)qa*qa + (long)qb*qb -2L*qa*qa*qb;
    if (qx != 0)
    {
        // Bounding box has even pixel width
        xc++;
        qt += 3L*qb*qb;
    }

    // Start at (dx, dy) = (0, b) and iterate until (a, 0) is reached
    while (qy >= 0 && qx <= qa)
    {
        // Draw the new points
        if (!fill)
        {
            __ALGO__SETPIXEL(dest, xb-dx, yb-dy, col)
            if (dx != 0 || xb != xc)
            {
                __ALGO__SETPIXEL(dest, xc+dx, yb-dy, col)
                if (dy != 0 || yb != yc)
                    __ALGO__SETPIXEL(dest, xc+dx, yc+dy, col)
            }
            if (dy != 0 || yb != yc)
                __ALGO__SETPIXEL(dest, xb-dx, yc+dy, col)
        }

        // If a (+1, 0) step stays inside the ellipse, do it
        if (qt + 2L*qb*qb*qx + 3L*qb*qb <= 0L ||
            qt + 2L*qa*qa*qy - (long)qa*qa <= 0L)
        {
            qt += 8L*qb*qb + 4L*qb*qb*qx;
            dx++;
            qx += 2;
            // If a (0, -1) step stays outside the ellipse, do it
        }
        else if (qt - 2L*qa*qa*qy + 3L*qa*qa > 0L)
        {
            if (fill)
            {
                FillScanline(dest->scanLine(yc+dy), xb-dx, xc+dx, col);
                if (dy != 0 || yb != yc)
                    FillScanline(dest->scanLine(yb-dy), xb-dx, xc+dx, col);
            }
            qt += 8L*qa*qa - 4L*qa*qa*qy;
            dy--;
            qy -= 2;
            // Else step (+1, -1)
        } else
        {
            if (fill)
            {
                FillScanline(dest->scanLine(yc+dy), xb-dx, xc+dx, col);
                if (dy != 0 || yb != yc)
                    FillScanline(dest->scanLine(yb-dy), xb-dx, xc+dx, col);
            }
            qt += 8L*qb*qb + 4L*qb*qb*qx + 8L*qa*qa - 4L*qa*qa*qy;
            dx++;
            qx += 2;
            dy--;
            qy -= 2;
        }
    }   // End of while loop
    return;
}

#endif // ALGO_H
