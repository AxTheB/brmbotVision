#include "cv.h"
#include "highgui.h"
#include "stdio.h"
#include "o2c3conv.h"
#include "imgutil.h"

int g_slider_position = 0;
CvCapture* capture = NULL;

int find_way(IplImage* in_img, IplImage* out_img, int pxdelta){
    //Najde cestu metodou porovnavani se stredovym pixelem 
    //hleda v celem obrazku/roi
    imgdim in_img_dim;
    in_img_dim = get_dimensions(in_img);

    int refy, last_refy, worky1, worky2, workx, tmppxdiff;
    long total = 0;
    int tcount = 0;
    int middle = (in_img_dim.ystop - in_img_dim.ystart)/2;
    uchar *refpixel, *workpixel;

    refy = middle;

    last_refy = refy;

    //projizdi pracovni oblast zprava doleva
    for (workx=in_img_dim.xstop; workx >= in_img_dim.xstart; workx--) {

        //hleda rozdilny pixel smerem nahoru (k nule)
        worky1 = refy;
        worky2 = refy;
        refpixel = px_pos(in_img,workx, refy); 
        //nahoru
        while (worky1 > in_img_dim.ystart){
            tmppxdiff = pxdiff(refpixel, px_pos(in_img, workx, worky1));
            if (tmppxdiff > pxdelta) {
                break;
            }
            worky1--;
        }
        //dolu
        while (worky2 < (in_img_dim.ystop-1)){
            if (pxdiff(refpixel, px_pos(in_img, workx, worky2)) > pxdelta) {
                break;
            }
            worky2++;
        }
        last_refy = refy;
        total = total + refy;
        tcount++;
        refy = (worky1 + worky2) / 2;

        workpixel = px_pos(out_img, workx, (refy+last_refy)/2);
        workpixel[0] = 0;
        workpixel[1] = 255;
        workpixel[2] = 128;

        workpixel = px_pos(out_img, workx, worky1);
        workpixel[0] = 128;
        workpixel[1] = 0;
        workpixel[2] = 128;

        workpixel = px_pos(out_img, workx, worky2);
        workpixel[0] = 128;
        workpixel[1] = 0;
        workpixel[2] = 128;

    }
    int direction = (total/tcount) - (in_img_dim.ystop - in_img_dim.ystart)/2;
    cvLine(out_img, cvPoint(0, middle + direction), cvPoint(tcount, middle+direction),
            cvScalar(0, 0, 255, 0), 1, 8, 0);
    return direction;
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
    int fw_pxdiff = 20;
    int kam, kam2;

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
        to_c1c2c3(frame, newframe2, lookup_table);

        cvRectangle(frame, cvPoint(0,0), cvPoint(roi_width-1, roi_height-1),
                cvScalar(0, 255, 0, 0), 1, 8, 0);
        kam = find_way(newframe, newframe, fw_pxdiff);
        kam2 = find_way(newframe2, newframe2, fw_pxdiff);
        cvLine(frame, cvPoint(0,roi_height/2), cvPoint(roi_width,roi_height/2),
                cvScalar(0, 255, 255, 0), 1, 8, 0);
        cvLine(frame, cvPoint(0,(roi_height+kam+kam2)/2), cvPoint(roi_width,(roi_height+kam+kam2)/2),
                cvScalar(0, 0, 255, 0), 1, 8, 0);
        cvResetImageROI(frame);
        printf("kam %d kam2 %d \n", kam, kam2);
        cvShowImage("Display", frame);
        cvShowImage("o1o2", newframe);
        cvShowImage("c1c2c3", newframe2);
        char c = cvWaitKey(1);
        if (c == 27) break;
        if (c > 0) {
            switch(c){
                //hjkl pro pohyb roi, se shiftem meni velikost
                case 104:
                             roi_x--; printf( "roi_x: %d", roi_x); break;
                case 108:
                             roi_x++; printf( "roi_x: %d", roi_x); break;
                case 106:
                             roi_y++; printf( "roi_y: %d", roi_y); break;
                case 107:
                             roi_y--; printf( "roi_y: %d", roi_y); break;
                case 72:
                            roi_width--; printf( "roi_width: %d", roi_width); break;
                case 76:
                            roi_width++; printf( "roi_width: %d", roi_width); break;
                case 75:
                            roi_height--; printf( "roi_height: %d", roi_height); break;
                case 74:
                            roi_height++; 
                            printf( "roi_height: %d", roi_height);
                            break;
                case 32:
                            cvWaitKey(0); break;
                case 111:
                            fw_pxdiff--; 
                            printf( "fw_pxdiff: %d", fw_pxdiff);
                            break;
                case 112:
                            fw_pxdiff++; 
                            printf( "fw_pxdiff: %d", fw_pxdiff);
                            break;
            }

            printf("key %d \n", c); 
        }
    }
    cvReleaseCapture( &capture);
    cvDestroyWindow( "Display");
    cvDestroyWindow( "o1o2");
    cvDestroyWindow( "c1c2c3");
}
