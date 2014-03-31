#include <iostream>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;

/// Global variables

Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void CannyThreshold(int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

  /// Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);

  src.copyTo( dst, detected_edges);
  imshow( window_name, detected_edges );
 }

void edge_detection()
{
	/// Load an image
	src = imread("hello_world.jpg");

	if( !src.data )
	{
		return;
	}

	/// Create a matrix of the same type and size as src (for dst)
	dst.create( src.size(), src.type() );

	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );

	/// Create a window
	namedWindow( window_name, CV_WINDOW_AUTOSIZE );

	/// Create a Trackbar for user to enter threshold
	createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

	/// Show the image
	CannyThreshold(0, 0);
}

void grab_cut(int x, int y, int width, int height)
{
	cv::Mat image= cv::imread("hello_world.jpg");
	cv::namedWindow("Original Image");
	cv::imshow("Original Image",image);

	// �Է� ���� ���� �κ� ����/��� ���̺��� �����ϴ� ���
	// ���� ��ü ���θ� �����ϴ� ���� ���簢���� ����
	cv::Rect rectangle(x, y, width, height);
	// ��� ���簢�� ����
	// ���簢�� ���� ȭ�Ҵ� ������� ���̺�

	// �Է� ����� ��ü ���� ���� �ܿ� cv::grabCut �Լ��� ȣ���� ��
	// �� �˰��� ���� ���� ���� �����ϴ� �� ����� ���ǰ� �ʿ�
	cv::Mat result; // ���� (4�ڱ� ������ ��)
	cv::Mat bgModel, fgModel; // �� (�ʱ� ���)
	cv::grabCut (image,    // �Է� ����
		result,    // ���� ���
		rectangle,   // ������ �����ϴ� ���簢��
		bgModel, fgModel, // ��
		5,     // �ݺ� Ƚ��
		cv::GC_INIT_WITH_RECT); // ���簢�� ���
	// cv::CC_INT_WITH_RECT �÷��׸� �̿��� ��� ���簢�� ��带 ����ϵ��� ����

	// cv::GC_PR_FGD ���濡 ���� ���� �ִ� ȭ��(���簢�� ������ ȭ�� �ʱⰪ)
	// cv::GC_PR_FGD�� ������ ���� ���� ȭ�Ҹ� ������ ������ ���� ������ ����
	cv::compare(result, cv::GC_PR_FGD, result, cv::CMP_EQ);
	// ������ ���ɼ��� �ִ� ȭ�Ҹ� ��ũ�� ���� ��������
	cv::Mat foreground(image.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	// ��� ���� ����
	image.copyTo(foreground, result);
	// ��� ȭ�Ҵ� ������� ����

	cv::namedWindow("Result");
	cv::imshow("Result", result);

	cv::namedWindow("Foreground");
	cv::imshow("Foreground", foreground);
}

int main()
{
	const char *classifer = "C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt_tree.xml";

	CvHaarClassifierCascade* cascade = 0;
	cascade = (CvHaarClassifierCascade*) cvLoad(classifer, 0, 0, 0 );

	if(!cascade)
	{
		std::cerr<<"error: cascade error!!"<<std::endl;
		return -1;
	}

	CvMemStorage* storage = 0;
	storage = cvCreateMemStorage(0);

	if(!storage)
	{
		std::cerr<<"error: storage error!!"<<std::endl;
		return -2;
	}

	//edge detection ����
	edge_detection();

	//�� �Ʒ��� �� �ν�
	
	//�̹����� �ε�
	IplImage *frame = cvLoadImage("hello_world.jpg",CV_LOAD_IMAGE_COLOR);
	
	//�� ����
	CvSeq *faces = cvHaarDetectObjects(frame, cascade, storage, 1.4 , 1, 0);

	
	//����� �� Rectangle �׸���
	for(int i=0; i<faces->total; i++)//�� �νĵ� ������ faces->total�� ��. ���⼭ �츮�� �� ������ �� �Ѱ��ۿ� �����Ƿ� �״� ��� ������
	{
		CvRect *r = 0;
		r = (CvRect*) cvGetSeqElem(faces, i);
		cvRectangle(frame, cvPoint(r->x, r->y), cvPoint(r->x+r->width, r->y+r->height), cvScalar(0,255,0), 3, CV_AA, 0);

		grab_cut(r->x, r->y + (0.1)*(r->height), r->width, r->height);//�� ũ���ϱ�
	}

	//Window�� frame ���
	cvShowImage("haar example (exit = esc)",frame);
	cvWaitKey(0);

	//�ڿ� ����
	cvReleaseMemStorage(&storage);
	cvReleaseHaarClassifierCascade(&cascade);
	cvDestroyWindow("haar example (exit = esc)");
	return 0;
}