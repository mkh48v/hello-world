#include <iostream>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;

/// Global variables

Mat src, src_gray;
Mat detected_edges;

int ratio = 3;
int kernel_size = 3;

int eye_upper_bound;

cv::Mat result;

enum front_hair_style
{
	no_front_hair,
	short_front_hair,
	long_front_hair
};
enum side_hair_style
{
	tied_hair,
	short_left_hair,
	long_left_hair
};

void CannyThreshold(int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  int threshold_value = 60;
  Canny( detected_edges, detected_edges, threshold_value, threshold_value*ratio, kernel_size );
 }

void edge_detection()
{
	/// Load an image
	src = imread("hello_world.jpg");
	
	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );

	/// Show the image
	CannyThreshold(0, 0);
}

void hair_style_detection(int x, int y, int width, int height)
{
	cv::Mat image= cv::imread("hello_world.jpg");

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
	

	int front_hair_lower_bound;
	for(int i = 0; i < result.rows; i++)
	{
		int j=0;
		while(j < result.cols)
		{
			if(result.at<uchar>(i,j) == 255)
			{
				//���⼭���� ���Ӹ� ��� ���ö����� startingx���� ������ ���̴�.(����� �ոӸ� �Ʒ��� ���)
				front_hair_lower_bound=i;
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
	int hair_upper_boundary_row=0;
	int hair_upper_edge_col=0;
	for(int i = 0; i < detected_edges.rows; i++)
	{
		int j=0;
		while(j < detected_edges.cols)
		{
			if(detected_edges.at<uchar>(i,j) == 255)
			{
				//���Ⱑ �Ӹ� ���� ���
				hair_upper_boundary_row=i;
				hair_upper_edge_col=j;
				break;
			}
			else
			{
				j++;
			}
		}
		if(hair_upper_edge_col!=0 && hair_upper_boundary_row!=0)
		{
			break;
		}
	}

	//�� ��� ã�ƾ� ��
	int chin_line_bound = front_hair_lower_bound+1;
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
			chin_line_bound -= 1;//��� �� ���� ������ ��� �κ����� bound �� �ٲ�
			break;
		}
		else
		{
			chin_line_bound++;
		}
	}

	//���Ӹ� �Ʒ��� ��踦 ã�ƾ� ��
	//������ ���Ӹ��� �β��� ���������� �������� �����س����� 0�� �Ǵ� ���� �װ��� row, column ��ǥ�� ����Ѵ�
	int left_hair_lower_bound=hair_upper_boundary_row + (chin_line_bound - hair_upper_boundary_row)*(1.0/2.0);
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
			{
				face_left_boundary++;
			}
		}
		//ã�� col ���� ��迡������ detected_edges�� �ִ� �� �������� �Ÿ��� �����ؾ� ��
		int hair_left_boundary = face_left_boundary-2;
		while(hair_left_boundary>0)
		{
			if(detected_edges.at<uchar>(left_hair_lower_bound,hair_left_boundary) == 255)
			{
				break;
			}
			else
			{
				hair_left_boundary--;
			}
		}
		int left_hair_width = face_left_boundary - hair_left_boundary;
		if(left_hair_width <= 2)
		{
			break;
		}
		else
		{
			left_hair_lower_bound++;
		}
	}


	side_hair_style detected_side_hair;
	//���Ӹ� ���� ����
	if( left_hair_lower_bound < chin_line_bound && left_hair_lower_bound >= hair_upper_boundary_row + (chin_line_bound - hair_upper_boundary_row)*(2.0/3.0) )
	{
		//���Ӹ� ª��
		detected_side_hair = short_left_hair;
	}
	else if(left_hair_lower_bound >= chin_line_bound)
	{
		//���Ӹ� ���
		detected_side_hair = long_left_hair;
	}
	else
	{
		//���Ӹ� ����(������)
		detected_side_hair = tied_hair;
	}

	front_hair_style detected_front_hair;
	//�ոӸ� ���� ����
	if( front_hair_lower_bound > hair_upper_boundary_row + 2*(eye_upper_bound-hair_upper_boundary_row)/3.0 )
	{
		//�ոӸ� ���
		detected_front_hair = long_front_hair;
	}
	else if(
		front_hair_lower_bound <= hair_upper_boundary_row + 2*(eye_upper_bound-hair_upper_boundary_row)/3.0
		&& front_hair_lower_bound > hair_upper_boundary_row + (eye_upper_bound-hair_upper_boundary_row)/2.0
		)
	{
		//�ոӸ� ª��
		detected_front_hair = short_front_hair;
	}
	else
	{
		//�ոӸ� ����
		detected_front_hair =  no_front_hair;
	}
	
	//�ոӸ� �� ����
	if(detected_front_hair == long_front_hair && detected_side_hair == long_left_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontlong_sidelong.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}
	else if(detected_front_hair == long_front_hair && detected_side_hair == short_left_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontlong_sidemedium.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}
	else if(detected_front_hair == long_front_hair && detected_side_hair == tied_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontlong_sideshort.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}

	//�ոӸ� ª�� ����
	else if(detected_front_hair == short_front_hair && detected_side_hair == long_left_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontmedium_sidelong.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}
	else if(detected_front_hair == short_front_hair && detected_side_hair == short_left_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontmedium_sidemedium.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}
	else if(detected_front_hair == short_front_hair && detected_side_hair == tied_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontmedium_sideshort.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}

	//�ոӸ� ���� ����
	else if(detected_front_hair == no_front_hair && detected_side_hair == long_left_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontshort_sidelong.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}
	else if(detected_front_hair == no_front_hair && detected_side_hair == short_left_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontshort_sidemedium.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}
	else if(detected_front_hair == no_front_hair && detected_side_hair == tied_hair)
	{
		cv::Mat hairtyle_image= cv::imread("frontshort_sideshort.png");
		cv::namedWindow("recommended hairstyle");
		cv::imshow("recommended hairstyle",hairtyle_image);
	}

	//���ʿ��� �Ƹ� result �� ����̰� detected_edges������ ����� �κ��� ã�´ٸ� �� �̹����� �ռ��Ͽ� �Ӹ� ��踦 ã�Ƴ��� ���� ���̴�
	for(int i = 0; i < detected_edges.rows; i++)
	{
		int j=0;
		while(j < detected_edges.cols)
		{
			if(result.at<uchar>(i,j) != 255 && detected_edges.at<uchar>(i,j) == 255)
				result.at<uchar>(i,j) = 255;
			
			j++;
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
	CvSeq *detected_face = cvHaarDetectObjects(frame, facecascade, storage, 1.4 , 1, 0);

	//����� �� Rectangle �޾ƿ���(���� �ϳ��� �Էµǰ� �νĵƴٰ� ����)
	CvRect *face_rect = 0;
	face_rect = (CvRect*) cvGetSeqElem(detected_face, 0);


	//�� �̹������� ���� ã�ƾ� �� �� ����
	cv::Mat face_mat= cv::imread("hello_world.jpg");

	// Transform it into the C++ cv::Mat format
	cv::Mat image(face_mat); 

	//���� ���� ��(�� 2��и�)�� �簢���� ���� �̹����� ��ȯ�Ѵ�.
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

	eye_upper_bound = (face_rect->x) + (eye_rect->x);
	
	hair_style_detection(face_rect->x, face_rect->y + (0.1)*(face_rect->height), face_rect->width, face_rect->height);//�Ӹ� ��� ã��
		
	cvShowImage("haar example (exit = esc)",frame);
	cvWaitKey(0);

	cvDestroyWindow("haar example (exit = esc)");

	return 0;
}