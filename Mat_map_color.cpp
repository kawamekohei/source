#include "C:/opencv/sources/include/opencv/cv.h"
#include "C:/opencv/sources/include/opencv/highgui.h"
#include "C:/opencv/sources/include/opencv2/opencv.hpp"
#include <iostream>
#include <stdio.h>

#define TRUE 1
#define FALSE 0

// �F���o�p�͈̔͐ݒ�l
static int h_max;
static int h_min;
static int s_max;
static int s_min;
static int v_max;
static int v_min;

int b, g, r;

static int click_num = 0;
static int color_num = 0;
static int mouse_x, mouse_y;
static int flag = FALSE;
static int init_flag = FALSE;

void OnMouse(int event, int x, int y, int flags, void *param);
void HSVset(cv::Mat src_img);
void ContoursDraw(cv::Mat maskImage, cv::Mat *dst);
void TimeCounter(int *flag);

// �F���o�֐�
void cv_ColorExtraction(cv::Mat src, cv::Mat dst,
	int code,
	int ch1Lower, int ch1Upper,
	int ch2Lower, int ch2Upper,
	int ch3Lower, int ch3Upper)
{
	cv::Mat colorImage;
	int lower[3];
	int upper[3];
	int find_contour;

	cv::Mat lut = cv::Mat(256, 1, CV_8UC3);

	cv::cvtColor(src, colorImage, code);

	lower[0] = ch1Lower;
	lower[1] = ch2Lower;
	lower[2] = ch3Lower;
	upper[0] = ch1Upper;
	upper[1] = ch2Upper;
	upper[2] = ch3Upper;

	for (int i = 0; i < 256; i++){
		for (int k = 0; k < 3; k++){
			if (lower[k] <= upper[k]){
				if ((lower[k] <= i) && (i <= upper[k])){
					lut.data[i*lut.step + k] = 255;
				}
				else{
					lut.data[i*lut.step + k] = 0;
				}
			}
			else{
				if ((i <= upper[k]) || (lower[k] <= i)){
					lut.data[i*lut.step + k] = 255;
				}
				else{
					lut.data[i*lut.step + k] = 0;
				}
			}
		}
	}

	//LUT���g�p���ē�l��
	cv::LUT(colorImage, lut, colorImage);

	//Channel���ɕ���
	std::vector<cv::Mat> planes;
	cv::split(colorImage, planes);

	//�}�X�N���쐬
	cv::Mat maskImage;
	cv::bitwise_and(planes[0], planes[1], maskImage);
	cv::bitwise_and(maskImage, planes[2], maskImage);

	// �����摜�J���[�ݒ�

	//�o��
	//printf_s("%d,%d,%d\n", b, g, r);
	cv::Mat maskColor(src.rows, src.cols, CV_8UC3, cv::Scalar(b, g, r));
	maskColor.copyTo(dst, maskImage);

	// �֊s�`��
	ContoursDraw(maskImage, &dst);
}

int main()
{
	clock_t start = clock();
	int sw_flag = FALSE;

	h_min = h_max = s_min = s_max = v_min = v_max = 0;

	// �E�B���h�E���쐬����
	cvNamedWindow("window", CV_WINDOW_AUTOSIZE);

	cv::Mat src_img = cv::imread("map.jpg");
	cv::Mat out_img = src_img.clone();

	// �����L�[�����������܂ŁA���[�v������Ԃ�
	while (cvWaitKey(1) == -1)
	{
		//�}�E�X�C�x���g�̎擾
		cvSetMouseCallback("window", OnMouse);
		//�N���b�N���W�̉�f�l
		if (flag == TRUE){
			HSVset(src_img);
			click_num++;
			flag = FALSE;
		}
		// �o�͉摜�̏�����
		if (init_flag == TRUE){
			h_min = h_max = s_min = s_max = v_min = v_max = 0;
			click_num = 0;
			out_img = src_img.clone();
			init_flag = FALSE;
		}

		// �_�ŗp�^�C���J�E���^
		if (clock() - start >= 900){
			TimeCounter(&sw_flag);
			start = clock();
		}

		// �F���o�֐�
		cv_ColorExtraction(src_img, out_img, CV_BGR2HSV, h_min, h_max, s_min, s_max, v_min, v_max); 

		// ��ʕ\��
		cv::imshow("window", out_img);
	}

	// ���
	src_img.release();
	out_img.release();
	// �E�B���h�E��j������
	cvDestroyWindow("window");

	return 0;
}

void OnMouse(int event, int x, int y, int flags, void *param)
{
	// �N���b�N���W�̎擾
	if (event == CV_EVENT_LBUTTONUP){
		mouse_x = x;
		mouse_y = y;
		flag = TRUE;
	}

	if (event == CV_EVENT_RBUTTONUP){
		init_flag = TRUE;
	}
}

void HSVset(cv::Mat src_img)
{
	cv::Mat hsv_img = src_img.clone();

	cvtColor(src_img, hsv_img, CV_BGR2HSV);

	cv::Vec3b pixel = hsv_img.at<cv::Vec3b>(mouse_y, mouse_x);
	printf_s("HSV(%3d,%3d,%3d)\n", pixel[0], pixel[1], pixel[2]);

	h_min = pixel[0] - 2;
	h_max = pixel[0] + 2;
	s_min = pixel[1] - 60;
	s_max = pixel[1] + 60;
	v_min = pixel[2] - 60;
	v_max = pixel[2] + 60;
	if (h_min < 0){
		h_min += 180;
	}
	if (h_max > 180){
		h_max -= 180;
	}

	hsv_img.release();
}

void ContoursDraw(cv::Mat maskImage, cv::Mat *dst)
{
	//�֊s�̍��W���X�g
	CvMemStorage *storage = cvCreateMemStorage(0);
	//�֊s�擾
	std::vector< std::vector< cv::Point > > contours;
	cv::findContours(maskImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	// ���o���ꂽ�֊s����΂ŕ`��
	for (auto contour = contours.begin(); contour != contours.end(); contour++){
		cv::polylines(*dst, *contour, true, cv::Scalar(255, 0, 0), 1);
	}
}

void TimeCounter(int *flag)
{
	if (*flag == FALSE){
		b = 255;
		g = r = 0;
		*flag = TRUE;
	}
	else{
		b = r = g = 0;
		*flag = FALSE;
	}
}