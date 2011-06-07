#include "cv.h"
#include "highgui.h"
#include "stdio.h"
#include "o2c3conv.h"

int g_slider_position = 0;
CvCapture* capture = NULL;


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
