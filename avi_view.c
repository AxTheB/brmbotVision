#include "cv.h"
#include "highgui.h"
#include "stdio.h"

int g_slider_position = 0;
CvCapture* capture = NULL;

void to_o1o2(IplImage* in_img, IplImage* o1o2) {
    //prevede frame z RGB na o1o2
    //pro jednoduchost pracuje s 3 plane obrazem s tretim planem prazdnym
    int o1, o2, r, g, b;
    int x, y;
    int ystart, ystop, xstart, xstop;

    int o1orig;
    int omin = 0;
    int omax = 0;

    // pokud je v in_img definovany region of interest, tak ho pouzijeme
    //  - neni treba prevadet cely obraz z kamery, jen cast.
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
                o1o2->imageData + (y - ystart) * o1o2->widthStep
                );
        for (x = xstart; x < xstop; x++) {
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



            ptr_out[3 * (x - xstart)] = (unsigned char) MAX( MIN(o1, 255), 0);
            ptr_out[3 * (x - xstart) + 1] = (unsigned char) MAX( MIN(o2, 255), 0);
            ptr_out[3 * (x - xstart) + 2] = 0;
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

void find_way(IplImage* image, int reg_x, int reg_y, int reg_width, int reg_height){
    int start_x, start_y, _start_x, _start_y;
    start_x = reg_x + reg_width;
    start_y = reg_y + reg_height/2;
    //TODO ;)
}


void onTrackbarSlide(int pos){
    cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, pos);
}


int main(int argc, char** argv){

    int lookup_table[256][256];
    int x,y;

    //prostor v kterem se dela detekce
    // pro avicka z robotur
    // cvRect(70,140, 130, 200)
    int roi_x = 80;
    int roi_y = 170;
    int roi_width = 110;
    int roi_height = 200;

    // priprava lookup array pro c1c2c3
    for (x=0; x<= 255; x++){
        for(y=0; y<=255; y++) {
            lookup_table[x][y] = (int) (cvFastArctan(x,y) * 255 / 90);
        }
    }

    cvNamedWindow( "Display", CV_WINDOW_AUTOSIZE);
    cvNamedWindow( "o1o2", CV_WINDOW_AUTOSIZE);
    cvNamedWindow( "c1c2c3", CV_WINDOW_AUTOSIZE);
    capture = cvCreateFileCapture(argv[1]);
    int frames = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
    if (frames != 0){
        cvCreateTrackbar("Position", "Display", &g_slider_position, frames, onTrackbarSlide);
    }
    IplImage* frame;

    while(1){
        frame = cvQueryFrame(capture);
        if (!frame)
            break;
        // ROI omezuje konvertovanou oblast na tu kterou opravdu zkoumame
        cvSetImageROI(frame, cvRect(roi_x,roi_y, roi_width, roi_height));

        IplImage* newframe  = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
        IplImage* newframe2 = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
        to_o1o2(frame,newframe);
        //do_laplace(newframe, newframe);
        to_c1c2c3(frame, newframe2, lookup_table);

        cvRectangle(frame, cvPoint(0,0), cvPoint(roi_width-1, roi_height-1),
                cvScalar(0, 255, 0, 0), 1, 8, 0);
        cvLine(frame, cvPoint(0,roi_height/2), cvPoint(roi_width,roi_height/2),
                cvScalar(0, 0, 255, 0), 1, 8, 0);
        cvResetImageROI(frame);
        cvShowImage("Display", frame);
        cvShowImage("o1o2", newframe);
        cvShowImage("c1c2c3", newframe2);
        char c = cvWaitKey(1);
        if (c == 27) break;
        if (c > 0) {
            switch(c){
                //hjkl pro pohyb roi, se shiftem meni velikost
                case 104:
                             roi_x--; break;
                case 108:
                             roi_x++; break;
                case 106:
                             roi_y++; break;
                case 107:
                             roi_y--; break;
                case 72:
                            roi_width--; break;
                case 76:
                            roi_width++; break;
                case 75:
                            roi_height--; break;
                case 74:
                            roi_height++; break;
                case 32:
                            cvWaitKey(0); break;
            }

            printf("key %d \n", c); 
        }
    }
    cvReleaseCapture( &capture);
    cvDestroyWindow( "Display");
    cvDestroyWindow( "o1o2");
    cvDestroyWindow( "c1c2c3");
}
