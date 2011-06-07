#include "cv.h"

typedef struct {
    int xstart, ystart, xstop, ystop;
} imgdim;

int pxdiff(uchar* px1, uchar* px2) {
    return abs(px1[0] - px2[0]) + abs(px1[1] - px1[1]) + abs(px1[2] - px2[2]);
}

uchar* px_pos(IplImage* img, int x, int y){
    uchar* ptr = (uchar*) (
            img->imageData + y * img->widthStep + img->nChannels * x
            );
    return ptr;
}


imgdim get_dimensions(IplImage* in_img) {
    // returns image dimensions in imgdim struct

    imgdim toret;

    if (in_img->roi) {
        toret.ystart = in_img->roi->yOffset;
        toret.xstart = in_img->roi->xOffset;
        toret.ystop = MIN(toret.ystart + in_img->roi->height, in_img->height);
        toret.xstop = MIN(toret.xstart + in_img->roi->width, in_img->width);
    } else {
        toret.xstart=0;
        toret.ystart=0;
        toret.ystop=in_img->height;
        toret.xstop=in_img->width;
    }
    return toret;
}


