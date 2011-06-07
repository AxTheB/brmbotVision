#include "cv.h"
#include "highgui.h"
#include "stdio.h"
#include "imgutil.h"

void to_o1o2(IplImage* in_img, IplImage* o1o2) {
    //prevede frame z RGB na o1o2
    //pro jednoduchost pracuje s 3 plane obrazem s tretim planem prazdnym
    int o1, o2, r, g, b;
    int x, y;
    imgdim in_img_dim;

    int o1orig;
    int omin = 0;
    int omax = 0;

    // pokud je v in_img definovany region of interest, tak ho pouzijeme
    //  - neni treba prevadet cely obraz z kamery, jen cast.
    in_img_dim = get_dimensions(in_img);

    for (y = in_img_dim.ystart; y < in_img_dim.ystop; y++) {
        uchar* ptr = (uchar*) (
                in_img->imageData + y * in_img->widthStep
                );
        uchar* ptr_out = (uchar*) (
                o1o2->imageData + (y - in_img_dim.ystart) * o1o2->widthStep
                );
        for (x = in_img_dim.xstart; x < in_img_dim.xstop; x++) {
            b = ptr[3 * x];
            g = ptr[3 * x + 1];
            r = ptr[3 * x + 2];

            // tohle je samotny prevod.
            // Spravne by to melo byt 
            // (r - g) / 2 + 128 a ((r + g) / 4 - b / 2) + 128
            // ale to vraci vyblitej obraz
            o1 = (r - g) + 128;
            o2 = ((r + g) / 2 - b) + 128;

            o1orig = (r + g) /4 - b / 2; 
            omin = MIN(omin, o1orig);
            omax = MAX(omax, o1orig);



//            ptr_out[3 * (x - xstart)] = (unsigned char) MAX( MIN(o1, 255), 0);
//           ptr_out[3 * (x - xstart) + 1] = (unsigned char) MAX( MIN(o2, 255), 0);
//            ptr_out[3 * (x - xstart) + 2] = 0;
            ptr_out[3 * (x - in_img_dim.xstart) + 0] = o1;
            ptr_out[3 * (x - in_img_dim.xstart) + 1] = o2;
            ptr_out[3 * (x - in_img_dim.xstart) + 2] = 0;
        }
    }
}

void to_c1c2c3(IplImage* in_img, IplImage* out_img, int lookup_table[256][256]) {
    int r, g, b, c1, c2, c3;
    int x, y;
    int ystart, ystop, xstart, xstop;
    if (in_img->roi) {
        ystart = in_img->roi->yOffset;
        xstart = in_img->roi->xOffset;
        ystop = MIN(ystart + in_img->roi->height, in_img->height);
        xstop = MIN(xstart + in_img->roi->width, in_img->width);
    } else {
        xstart=0;
        ystart=0;
        ystop=in_img->height;
        xstop=in_img->width;
    }

    for (y = ystart; y < ystop; y++) {
        uchar* ptr = (uchar*) (
                in_img->imageData + y * in_img->widthStep
                );
        uchar* ptr_out = (uchar*) (
                out_img->imageData + (y - ystart) * out_img->widthStep
                );
        for (x = xstart; x < xstop; x++) {
            b = ptr[3 * x];
            g = ptr[3 * x + 1];
            r = ptr[3 * x + 2];
            c1 = lookup_table[r][MAX(g, b)];
            c2 = lookup_table[g][MAX(r, b)];
            c3 = lookup_table[b][MAX(g, r)];

            ptr_out[3 * (x - xstart)] = c1;
            ptr_out[3 * (x - xstart) + 1] = c2;
            ptr_out[3 * (x - xstart) + 2] = c3;
        }
    }
}
