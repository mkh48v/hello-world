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

cv::Mat result;

enum classified_hairstyle
{

};

void CannyThreshold(int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  int threshold_value = 60;
  Canny( detected_edges, detected_edges, threshold_value, threshold_value*ratio, kernel_size );

  /// Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);
  
  src.copyTo( dst, detected_edges);
  imshow( window_name, detected_edges );
 }

void edge_detection()
{
	/// Load an image
	src = imread("hello_world.jpg");

	/// Create a matrix of the same type and size as src (for dst)
	dst.create( src.size(), src.type() );

	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );

	/// Create a window
	namedWindow( window_name, CV_WINDOW_AUTOSIZE );

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
	
	cv::Mat bgModel, fgModel; // �� (�ʱ� ���)
	cv::grabCut (image,    // �Է� �̹���
		result,    // ���� ���
		rectangle,   // ������ �����ϴ� ���簢��
		bgModel, fgModel, // ��
		3,     // �ݺ� Ƚ��
		cv::GC_INIT_WITH_RECT); // ���簢�� ���

	// cv::GC_PR_FGD ���濡 ���� ���� �ִ� ȭ��(���簢�� ������ ȭ�� �ʱⰪ)
	// cv::GC_PR_FGD�� ������ ���� ���� ȭ�Ҹ� ������ ������ ���� ������ ����
	cv::compare(result, cv::GC_PR_FGD, result, cv::CMP_EQ);
	// ������ ���ɼ��� �ִ� ȭ�Ҹ� ��ũ�� ���� ��������
	cv::Mat foreground(image.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	// ��� �̹��� ����
	image.copyTo(foreground, result);
	// ��� ȭ�Ҵ� ������� ����
	

	int front_hair_rower_bound_line, front_hair_rower_bound_y;
	for(int i = 0; i < result.rows; i++)
	{
		int j=0;
		while(j < result.cols)
		{
			if(result.at<uchar>(i,j) == 255)
			{
				//���⼭���� ���Ӹ� ��� ���ö����� startingx���� ������ ���̴�.(����� �ոӸ� �Ʒ��� ���)
				front_hair_rower_bound_line=i;
				front_hair_rower_bound_y=j;
				break;
			}
			else
			{
				j++;
			}
		}
		if(j != result.cols)
		{
			break;
		}
	}

	//���Ӹ� �� ��ǥ
	int hair_upper_edge_row=0;
	int hair_upper_edge_col=0;

	for(int i = 0; i < detected_edges.rows; i++)
	{
		int j=0;
		while(j < detected_edges.cols)
		{
			if(detected_edges.at<uchar>(i,j) == 255)
			{
				//���Ⱑ �Ӹ� ���� ���
				hair_upper_edge_row=i;
				hair_upper_edge_col=j;
				break;
			}
			else
			{
				j++;
			}
		}
		if(hair_upper_edge_col!=0 && hair_upper_edge_row!=0)
		{
			break;
		}
	}

	//�� ��� ã�ƾ� ��
	int chin_line_bound = front_hair_rower_bound_line+1;
	
	while(chin_line_bound < result.rows)
	{
		int j=0;
		while(j < result.cols)
		{
			if(result.at<uchar>(chin_line_bound,j) == 255)
			{
				break;
			}
			else
			{
				j++;
			}
		}
		if(j == result.cols)
		{
			//���Ⱑ �� ����
			chin_line_bound -= 1;
			break;
		}
		else
			chin_line_bound++;
	}

	//���Ӹ� �Ʒ��� ��踦 ã�ƾ� ��
	//������ ���Ӹ��� �β��� ���������� �������� �����س����� 0�� �Ǵ� ���� �װ��� row, column ��ǥ�� ����Ѵ�
	int left_hair_width_warning=0;
	int left_hair_lower_bound=front_hair_rower_bound_line;
	while(left_hair_lower_bound < chin_line_bound)
	{
		//�켱 result�� col ��ǥ�� ã�ƾ���
		int face_left_boundary=0;
		while(face_left_boundary < result.cols)
		{
			if(result.at<uchar>(left_hair_lower_bound,face_left_boundary) == 255)
			{
				break;
			}
			else
				face_left_boundary++;
		}
		//ã�� col ���� ��迡������ detected_edges�� �ִ� �� �������� �Ÿ��� �����ؾ� ��
		int hair_left_boundary = face_left_boundary;
		while(hair_left_boundary>0)
		{
			if(detected_edges.at<uchar>(left_hair_lower_bound,hair_left_boundary) == 255)
			{
				break;
			}
			else
				hair_left_boundary--;
		}
		int left_hair_width = face_left_boundary - hair_left_boundary;
		if(left_hair_width < 2)
		{
			break;
		}

		left_hair_lower_bound++;
	}

	//���Ӹ� ���� ����
	if( left_hair_lower_bound < chin_line_bound && left_hair_lower_bound >= hair_upper_edge_row + (chin_line_bound - hair_upper_edge_row)*(2.0/3.0) )
	{
		//���Ӹ� ª��
	}
	else if(left_hair_lower_bound >= chin_line_bound)
	{
		//���Ӹ� ���
	}
	else
	{
		//���Ӹ� ����
	}

	//�ոӸ� ���� ����
		

	//���ʿ��� �Ƹ� result �� ����̰� detected_edges������ ����� �κ��� ã�´ٸ� �� �̹����� �ռ��Ͽ� �Ӹ� ��踦 ã�Ƴ��� ���� ���̴�
	for(int i = 0; i < detected_edges.rows; i++)
	{
		int j=0;
		while(j < detected_edges.cols)
		{
			if(result.at<uchar>(i,j) != 255 && detected_edges.at<uchar>(i,j) == 255)
				result.at<uchar>(i,j) = 255;
			
			j++;
			// do something with BGR values...
		}
	}
	cv::namedWindow("Result");
	cv::imshow("Result", result);
	

}

int main()
{
	const char *faceclassifer = "C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt_tree.xml";
	CvHaarClassifierCascade* facecascade = 0;
	facecascade = (CvHaarClassifierCascade*) cvLoad(faceclassifer, 0, 0, 0 );

	const char *eyeclassifer = "C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye.xml";
	CvHaarClassifierCascade* eyecascade = 0;
	eyecascade = (CvHaarClassifierCascade*) cvLoad(eyeclassifer, 0, 0, 0 );

	CvMemStorage* storage = 0;
	storage = cvCreateMemStorage(0);

	//edge detection ����
	edge_detection();

	//�� �Ʒ��� �� �ν�
	
	//�̹����� �ε�
	IplImage *frame = cvLoadImage("hello_world.jpg",CV_LOAD_IMAGE_COLOR);
	
	//�� ����
	CvSeq *faces = cvHaarDetectObjects(frame, facecascade, storage, 1.4 , 1, 0);

	//����� �� Rectangle �޾ƿ���(���� �ϳ��� �Էµǰ� �νĵƴٰ� ����)
	CvRect *face_rect = 0;
	face_rect = (CvRect*) cvGetSeqElem(faces, 0);


	//�� �̹������� ���� ã�ƾ� �� �� ����
	cv::Mat face_mat= cv::imread("hello_world.jpg");

	// Transform it into the C++ cv::Mat format
	cv::Mat image(face_mat); 

	// Crop the full image to that image contained by the rectangle myROI
	// Note that this doesn't copy the data
	CvRect quarter_of_face;
	quarter_of_face.x = face_rect -> x;
	quarter_of_face.y = face_rect -> y;
	quarter_of_face.width = (face_rect -> width)*0.5;
	quarter_of_face.height = (face_rect -> height)*0.5;


	cv::Mat cropped_face = image(quarter_of_face);

	//cropped_face�� IplImage�� ��ȯ�ؾ� ��
	IplImage* face_iplimage = &IplImage(cropped_face);

	//face_iplimage���� ������ ã�´�
	CvSeq *eye = cvHaarDetectObjects(face_iplimage, eyecascade, storage, 1.4 , 1, 0);

	//����� ���� Rectangle �޾ƿ���(������ �ϳ��� �Էµǰ� �νĵƴٰ� ����)
	CvRect *eye_rect = 0;
	eye_rect = (CvRect*) cvGetSeqElem(eye, 0);

	//eye_rect�� ��ǥ�� ����س��Ƽ� �ոӸ��� ����� ���̸� ������ �� ������ �޾ƾ� �� 
	cvRectangle(frame, cvPoint( face_rect->x + eye_rect->x, (face_rect->y)+(eye_rect->y) ), cvPoint(face_rect->x + eye_rect->x+eye_rect->width, (face_rect->y)+(eye_rect->y)+eye_rect->height), cvScalar(0,255,0), 3, CV_AA, 0);
	

	
	
	cvRectangle(frame, cvPoint(face_rect->x, face_rect->y), cvPoint(face_rect->x+face_rect->width, face_rect->y+face_rect->height), cvScalar(0,255,0), 3, CV_AA, 0);
	grab_cut(face_rect->x, face_rect->y + (0.1)*(face_rect->height), face_rect->width, face_rect->height);//�� ũ���ϱ�



	//Window�� frame ���
	cvShowImage("haar example (exit = esc)",frame);
	cvWaitKey(0);


	//�ڿ� ����
	cvReleaseMemStorage(&storage);
	cvReleaseHaarClassifierCascade(&facecascade);
	cvDestroyWindow("haar example (exit = esc)");
	
	return 0;
}