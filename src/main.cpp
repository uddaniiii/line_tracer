#include "opencv2/opencv.hpp"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "dxl.hpp"
using namespace cv;
using namespace std;
bool ctrl_c_pressed;
void ctrlc(int)
{
    ctrl_c_pressed = true;
}
int main()
{
    string src = "nvarguscamerasrc sensor-id=0 ! video/x-raw(memory:NVMM), width=(int)640, \
    height=(int)360, format=(string)NV12 ! \
    nvvidconv flip-method=0 ! video/x-raw, width=(int)640, height=(int)360, \
    format=(string)BGRx ! videoconvert ! \
    video/x-raw, format=(string)BGR !appsink";
    string dst = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! nvvidconv ! nvv4l2h264enc \
    insert-sps-pps=true ! h264parse ! rtph264pay pt=96 ! udpsink host=203.234.58.167 \
    port=8001 sync=false";
    string dst2 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! nvvidconv ! nvv4l2h264enc \
    insert-sps-pps=true ! h264parse ! rtph264pay pt=96 ! udpsink host=203.234.58.167 \
    port=8002 sync=false";
    // host는 영상을 수신하는 컴퓨터의 ip주소를 적어준다.

    struct timeval start,end2;
    // double diff1,diff2,diff3;

    VideoCapture source(src,CAP_GSTREAMER);
    // VideoCapture source("linetracer50rpm.mp4"); // 영상 불러오기
    if (!source.isOpened()) { cerr << "Video error" << endl; return -1; } // 에러메세지 출력

    VideoWriter writer(dst, 0, (double)30, cv::Size(640, 360), true);
    if (!writer.isOpened()) { cerr << "Writer open failed!" << endl; return -1;}
    
    VideoWriter writer2(dst2, 0, (double)30, cv::Size(640, 90), true);
    if (!writer2.isOpened()) { cerr << "Writer open failed!" << endl; return -1;}

    Dxl mx;

    signal(SIGINT, ctrlc);
    if(!mx.open()) { cout << "dynamixel open error"<<endl; return -1; } //장치열기
    
    Mat frame;
    Mat  gray;
    Mat labels, stats, centroids;

    int line_area = 0, line_label = 0, error,mode=false;

    Point line_pt;
    Point line_oldPt(360, 180);

    while (true) {
        gettimeofday(&start,NULL);
        source >> frame; // 프레임 받아오기
        if (frame.empty()) { cerr << "frame empty!" << endl; break; } // 에러 메시지 출력

        // cout<<frame.rows<<frame.cols<<endl;
        //ROI 선정
        Mat roi = frame(Rect(Point(0, frame.rows * 3 / 4), Point(frame.cols, frame.rows))); // 입력영상의 하단 1/4 관심영역 설정

        // 그레이스케일 변환
        cvtColor(roi, gray, COLOR_BGR2GRAY);

        // 밝기 보정
        Scalar avgPixel = mean(gray); // 입력영상의 평균 밝기
        Mat pixel = gray + (85 - avgPixel[0]); //결과 영상 픽셀 값
        //cout << "mean: " << pixel;

        Mat th;
        threshold(pixel, th, 170, 255, THRESH_BINARY); // 이진화 170

        int cnt = connectedComponentsWithStats(th, labels, stats, centroids); //영상 레이블링
        cvtColor(th, th, COLOR_GRAY2BGR);

        if (cnt > 1) {
            int* p = stats.ptr<int>(1);
            line_area = p[4];
            line_label = 1;
            for (int i = 2; i < cnt; i++) {
                int* p = stats.ptr<int>(i);
                if (p[4] >= line_area) {
                    line_area = p[4];
                    line_label = i;
                }
            }
            double* line_center = centroids.ptr<double>(line_label);
            line_pt = Point2d(line_center[0], line_center[1]);

            if (abs(line_oldPt.x - line_pt.x) > 150) {
                line_pt = line_oldPt;
            }
        }
        else {
            line_pt = line_oldPt;
        }
        line_oldPt = line_pt;

        for (int i = 1; i < cnt; i++) {
            if (line_area > 50) {
                if (i == line_label) {
                    circle(th, line_pt, 3, Scalar(0, 0, 255), -1);
                    rectangle(th, Rect(stats.ptr<int>(i)[0], stats.ptr<int>(i)[1], stats.ptr<int>(i)[2], stats.ptr<int>(i)[3]), Scalar(0, 0, 255));
                }
                else {
                    circle(th, Point(centroids.ptr<double>(i)[0], centroids.ptr<double>(i)[1]), 3, Scalar(255, 0, 0), -1);
                    rectangle(th, Rect(stats.ptr<int>(i)[0], stats.ptr<int>(i)[1], stats.ptr<int>(i)[2], stats.ptr<int>(i)[3]), Scalar(255, 0, 0));
                }
            }
        }

        error = (th.cols / 2 - line_pt.x) / 2;
        cout << "현재 error: " << error << '\t' << endl;

        writer << frame;
        writer2<<th;
        
        if (ctrl_c_pressed) {
            break;
        }

        int l_speed = 150, r_speed = 150;
        int l_p = 150, r_p = 150;
        l_speed = l_p + (error * 0.5);
        r_speed = r_p - (error * 0.5);

        if(mx.kbhit()){
            char c=mx.getch();
            if (c=='s') mode=true;
        }

        if(mode){
        mx.setVelocity(r_speed, -l_speed); //전진 속도명령 전송
        }
        
        gettimeofday(&end2,NULL);

        // imshow("gray", gray);
        // imshow("th", th);

        if (waitKey(1) == 27)
            break;
    }
    mx.close(); //장치닫기
    return 0;
}