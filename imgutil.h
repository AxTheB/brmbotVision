typedef struct {
    int xstart, ystart, xstop, ystop;
} imgdim;

imgdim get_dimensions(IplImage* in_img);
uchar* px_pos(IplImage* img, int x, int y);
int pxdiff(uchar* px1, uchar* px2);
