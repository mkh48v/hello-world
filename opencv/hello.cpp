#include <opencv/cv.h>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace cv;

/// Global variables

Mat src, src_gray;
Mat detected_edges;

int ratio = 3;
int kernel_size = 3;

int eye_upper_bound;

enum front_hair_style
{
	no_front_hair,
	short_front_hair,
	long_front_hair
};
enum side_hair_style
{
	short_left_hair,
	long_left_hair,
	tied_hair
};

void CannyThreshold(int, void*)
{
	/// Reduce noise with a kernel 3x3
	blur( src_gray, detected_edges, Size(3,3) );

	int threshold_value = 60;
	Canny( detected_edges, detected_edges, threshold_value, threshold_value*ratio, kernel_size );

	//cv::namedWindow("Result");
	//cv::imshow("Result", detected_edges);

}

void edge_detection(char* argv)
{
	/// Load an image
	src = imread(argv);
	
	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );

	/// Show the image
	CannyThreshold(0, 0);
}

class rgb
{
	public:
		int redsum;
		int greensum;
		int bluesum;
		double redmean;
		double greenmean;
		double bluemean;
};
int front_hair_analysis(cv::Mat color_pic, int vertical_standard_line, int horizontal_lower_bound, rgb* hair_color, int* hair_upper_bound) 
{
	cv::namedWindow("hello");
	cv::imshow("hello", color_pic);
	
	int count = horizontal_lower_bound;

	rgb first_part_rgb;
		
	first_part_rgb.redsum = 0;
	first_part_rgb.greensum = 0;
	first_part_rgb.bluesum = 0;
	
	
	//처음 3개 평균내고 시작하기
	while(count > horizontal_lower_bound - 3)
	{
		int b = color_pic.at<cv::Vec3b>(count,vertical_standard_line)[0];
		int g = color_pic.at<cv::Vec3b>(count,vertical_standard_line)[1];
		int r = color_pic.at<cv::Vec3b>(count,vertical_standard_line)[2];

		first_part_rgb.bluesum += b;
		first_part_rgb.greensum += g;
		first_part_rgb.redsum += r;
		count--;
	}
	first_part_rgb.redmean = first_part_rgb.redsum / (double)(horizontal_lower_bound - count);
	first_part_rgb.greenmean = first_part_rgb.greensum / (double)(horizontal_lower_bound - count);
	first_part_rgb.bluemean = first_part_rgb.bluesum / (double)(horizontal_lower_bound - count);


	while(count > 0)
	{
		if(
			(color_pic.at<cv::Vec3b>(count,vertical_standard_line)[0] < first_part_rgb.bluemean - 50 || color_pic.at<cv::Vec3b>(count,vertical_standard_line)[0] > first_part_rgb.bluemean + 50)
			+(color_pic.at<cv::Vec3b>(count,vertical_standard_line)[1] < first_part_rgb.redmean - 50 || color_pic.at<cv::Vec3b>(count,vertical_standard_line)[1] > first_part_rgb.redmean + 50)
			+(color_pic.at<cv::Vec3b>(count,vertical_standard_line)[2] < first_part_rgb.greenmean - 50 || color_pic.at<cv::Vec3b>(count,vertical_standard_line)[2] > first_part_rgb.greenmean + 50)
			>1
			)
		{
			break;
		}
		else
		{
			int b = color_pic.at<cv::Vec3b>(count,vertical_standard_line)[0];
			int g = color_pic.at<cv::Vec3b>(count,vertical_standard_line)[1];
			int r = color_pic.at<cv::Vec3b>(count,vertical_standard_line)[2];

			first_part_rgb.bluesum += b;
			first_part_rgb.greensum += g;
			first_part_rgb.redsum += r;

			count--;
			first_part_rgb.redmean = first_part_rgb.redsum / (double)(horizontal_lower_bound - count);
			first_part_rgb.greenmean = first_part_rgb.greensum / (double)(horizontal_lower_bound - count);
			first_part_rgb.bluemean = first_part_rgb.bluesum / (double)(horizontal_lower_bound - count);
		}
	}
	if(count == 0)
	{
		//0이 윗머리 꼭대기이고
		//앞머리는 매우 긴 것임
		*hair_upper_bound = 0;
		hair_color->redmean = first_part_rgb.redmean;
		hair_color->greenmean = first_part_rgb.greenmean;
		hair_color->bluemean = first_part_rgb.bluemean;
		return horizontal_lower_bound;
	}

	int returnvalue = count;

	rgb second_part_rgb;
	second_part_rgb.redsum = 0;
	second_part_rgb.greensum = 0;
	second_part_rgb.bluesum = 0;
	
	int hair_analysis_count = count;
	while (hair_analysis_count > count - 3)
	{
		int b = color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[0];
		int g = color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[1];
		int r = color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[2];

		second_part_rgb.bluesum += b;
		second_part_rgb.greensum += g;
		second_part_rgb.redsum += r;
		hair_analysis_count--;
	}
	second_part_rgb.redmean = second_part_rgb.redsum / (double)(count - hair_analysis_count);
	second_part_rgb.greenmean = second_part_rgb.greensum / (double)(count - hair_analysis_count);
	second_part_rgb.bluemean = second_part_rgb.bluesum / (double)(count - hair_analysis_count);

	while(hair_analysis_count > 0)
	{
		if(
			(color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[0] < second_part_rgb.bluemean - 50 || color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[0] > second_part_rgb.bluemean + 50)
			+(color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[1] < second_part_rgb.redmean - 50 || color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[1] > second_part_rgb.redmean + 50)
			+(color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[2] < second_part_rgb.greenmean - 50 || color_pic.at<cv::Vec3b>(hair_analysis_count,vertical_standard_line)[2] > second_part_rgb.greenmean + 50)
			>1
			)
		{
			break;
		}
		else
		{
			int b = color_pic.at<cv::Vec3b>(count + hair_analysis_count,vertical_standard_line)[0];
			int g = color_pic.at<cv::Vec3b>(count + hair_analysis_count,vertical_standard_line)[1];
			int r = color_pic.at<cv::Vec3b>(count + hair_analysis_count,vertical_standard_line)[2];

			second_part_rgb.bluesum += b;
			second_part_rgb.greensum += g;
			second_part_rgb.redsum += r;

			hair_analysis_count--;
			second_part_rgb.redmean = second_part_rgb.redsum / (double)(count - hair_analysis_count);
			second_part_rgb.greenmean = second_part_rgb.greensum / (double)(count - hair_analysis_count);
			second_part_rgb.bluemean = second_part_rgb.bluesum / (double)(count - hair_analysis_count);
		}
	}
	if(hair_analysis_count == 0)
	{
		//머리가 긴 것임
		hair_color->redmean = first_part_rgb.redmean;
		hair_color->greenmean = first_part_rgb.greenmean;
		hair_color->bluemean = first_part_rgb.bluemean;

		*hair_upper_bound = count;
		return horizontal_lower_bound;
	}
	else
	{
		hair_color->redmean = second_part_rgb.redmean;
		hair_color->greenmean = second_part_rgb.greenmean;
		hair_color->bluemean = second_part_rgb.bluemean;
	
		*hair_upper_bound = hair_analysis_count;
		return returnvalue;
	}
}

int get_chin_line_bound(CvRect* face_rect, cv::Mat gray_scale_mat) 
{
	int chin_line_bound = (face_rect->y + face_rect->height + 1);
	
	int count = 0;
	int color_sum = 0;
	double color_mean;
	while(count < 3)
	{
		color_sum += gray_scale_mat.at<uchar>( chin_line_bound + count, (int)( face_rect->x + (face_rect->width/2.0) ) );
		count++;
	}

	color_mean = color_sum/(double)count;


	while( chin_line_bound + count < (int)(face_rect->y + face_rect->height * (5/4.0)) )
	{
		if(gray_scale_mat.at<uchar>( chin_line_bound + count, (int)( face_rect->x + (face_rect->width/2.0) ) ) < color_mean - 35)
		{
			break;
		}
		else
		{
			color_sum += gray_scale_mat.at<uchar>( chin_line_bound + count, (int)( face_rect->x + (face_rect->width/2.0) ) );
			count++;
			color_mean = color_sum/(double)count;
		}
	}
	return chin_line_bound + count;
}



int get_left_hair_lower_bound(int left_hair_lower_bound, int chin_line_bound, int face_left_boundary, int detection_bound) 
{
	//초기화하는 부분
	int face_left_boundary_save = face_left_boundary;
	while(left_hair_lower_bound < chin_line_bound)
	{
		while(face_left_boundary > detection_bound)
		{
			if(detected_edges.at<uchar>(left_hair_lower_bound,face_left_boundary) == 255)
			{
				break;
			}
			else
			{
				face_left_boundary--;
			}
		}
		if(face_left_boundary == detection_bound)
		{
			face_left_boundary = face_left_boundary_save;
			left_hair_lower_bound++;
			continue;
		}
		else
		{
			break;
		}
	}
	int hair_left_boundary = face_left_boundary - 7;
	


	//왼쪽 머리 아랫쪽 경계 찾기
	int hair_left_bound_save = hair_left_boundary;
	while(left_hair_lower_bound < chin_line_bound)
	{
		//머리 왼쪽 경계 찾는 부분
		while(hair_left_boundary>detection_bound)
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
		if(hair_left_boundary == detection_bound)
		{
			hair_left_boundary = hair_left_bound_save;
			left_hair_lower_bound++;
			continue;
		}
		else
		{
			hair_left_bound_save = hair_left_boundary;
		}



		int left_hair_width = face_left_boundary - hair_left_boundary;
		if(left_hair_width < 2)
		{
			break;
		}
		else//왼쪽머리 오른쪽 경계 찾는 부분
		{
			left_hair_lower_bound++;//아랫 픽셀 라인 체크 시작
			face_left_boundary_save = face_left_boundary;
			face_left_boundary = hair_left_boundary + 1;
			while(face_left_boundary < detected_edges.cols)
			{
				if(detected_edges.at<uchar>(left_hair_lower_bound,face_left_boundary) == 255)
				{
					break;
				}
				else
				{
					face_left_boundary++;
				}
			}

			if(face_left_boundary == detected_edges.cols)
			{
				face_left_boundary = face_left_boundary_save;
				hair_left_boundary = face_left_boundary - 1;
				left_hair_lower_bound++;
				continue;
			}

			int left_hair_width = face_left_boundary - hair_left_boundary;
			if(left_hair_width < 2)
			{
				break;
			}
			else
			{
				hair_left_boundary = face_left_boundary-1;
			}
		}
	}
	//루프 끝



	return left_hair_lower_bound;
}



void show_recommended_hairstyle( front_hair_style detected_front_hair, side_hair_style detected_side_hair ) 
{
	//앞머리 긴 여자
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
	
	//앞머리 없는 여자
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
}

side_hair_style judge_side_hairstyle( int left_hair_lower_bound, int chin_line_bound, int hair_upper_boundary_row) 
{
	side_hair_style detected_side_hair;
	//옆머리 기장 판정
	if( left_hair_lower_bound < chin_line_bound && left_hair_lower_bound >= hair_upper_boundary_row + (chin_line_bound - hair_upper_boundary_row)*(4.0/5.0) )
	{
		//옆머리 짧다
		detected_side_hair = short_left_hair;
	}
	else if(left_hair_lower_bound >= chin_line_bound)
	{
		//옆머리 길다
		detected_side_hair = long_left_hair;
	}
	else
	{
		detected_side_hair = tied_hair;
	}
	return detected_side_hair;
}

front_hair_style judge_front_hairstyle(int front_hair_lower_bound, int hair_upper_boundary_row) 
{
	front_hair_style detected_front_hair;
	//앞머리 기장 판정
	if( front_hair_lower_bound > hair_upper_boundary_row + 2*(eye_upper_bound-hair_upper_boundary_row)/3.0 )
	{
		//앞머리 길다
		detected_front_hair = long_front_hair;
	}
	else
	{
		//앞머리 없다
		detected_front_hair =  no_front_hair;
	}
	return detected_front_hair;
}

int determine_front_hair_bound(int mid_front, int left_front, int right_front)
{
	int judged_value = 0;
	return judged_value;
}

int main(int argc, char* argv[])
{
	const char *faceclassifer = "haarcascade_frontalface_alt.xml";
	CvHaarClassifierCascade* facecascade = 0;
	facecascade = (CvHaarClassifierCascade*) cvLoad(faceclassifer, 0, 0, 0 );

	const char *eyeclassifer = "haarcascade_eye.xml";
	CvHaarClassifierCascade* eyecascade = 0;
	eyecascade = (CvHaarClassifierCascade*) cvLoad(eyeclassifer, 0, 0, 0 );

	CvMemStorage* storage = 0;
	storage = cvCreateMemStorage(0);


	//이미지를 로드
	IplImage *pic = cvLoadImage("hello_world.jpg",CV_LOAD_IMAGE_COLOR);
	if(pic == NULL)
	{
		printf("사진 파일을 찾을 수 없습니다. Enter키를 누르시면 종료합니다.\n");
		while(1)
		{
			if (std::cin.get() == '\n')
			{
				return 0;
			}
		}
	}


	//edge detection 먼저
	edge_detection("hello_world.jpg");

	//이 아래는 얼굴 인식
	
	//얼굴 검출
	CvSeq *detected_face = cvHaarDetectObjects(pic, facecascade, storage, 1.4 , 1, 0);
	if(detected_face == NULL)
	{
		printf("사진 파일에서 얼굴을 찾을 수 없습니다. Enter키를 누르시면 종료합니다.\n");
		while(1)
		{
			if (std::cin.get() == '\n')
			{
				return 0;
			}
		}
	}
	//검출된 얼굴 Rectangle 받아오기(얼굴은 하나만 입력되고 인식됐다고 가정)
	CvRect *face_rect = 0;
	face_rect = (CvRect*) cvGetSeqElem(detected_face, 0);

	//얼굴 이미지에서 눈을 찾아야 할 것 같음
	cv::Mat face_mat= cv::imread("hello_world.jpg");

	cv::Mat pic_gray;
	cvtColor( face_mat, pic_gray, CV_BGR2GRAY );


	// Transform it into the C++ cv::Mat format
	cv::Mat image(face_mat); 


	CvRect left_quarter_of_face;
	left_quarter_of_face.x = face_rect -> x;
	left_quarter_of_face.y = face_rect -> y;
	left_quarter_of_face.width = (face_rect -> width)*0.5;
	left_quarter_of_face.height = (face_rect -> height)*0.5;

	CvRect right_quarter_of_face;
	right_quarter_of_face.x = face_rect -> x + (face_rect -> width)*0.5;
	right_quarter_of_face.y = face_rect -> y;
	right_quarter_of_face.width = (face_rect -> width)*0.5;
	right_quarter_of_face.height = (face_rect -> height)*0.5;

	cv::Mat cropped_left_face = image(left_quarter_of_face);
	cv::Mat cropped_right_face = image(right_quarter_of_face);

	//cropped_face를 IplImage로 변환해야 함
	IplImage* left_face_iplimage = &IplImage(cropped_left_face);
	IplImage* right_face_iplimage = &IplImage(cropped_right_face);

	//face_iplimage에서 눈깔을 찾는다
	CvSeq* left_eye = cvHaarDetectObjects(left_face_iplimage, eyecascade, storage, 1.4 , 1, 0);
	if(left_eye == NULL)
	{
		printf("사진에서 왼쪽 눈을 찾을 수 없습니다. Enter키를 누르시면 종료합니다.\n");
		while(1)
		{
			if (std::cin.get() == '\n')
			{
				return 0;
			}
		}
	}
	CvSeq* right_eye = cvHaarDetectObjects(right_face_iplimage, eyecascade, storage, 1.4 , 1, 0);
	if(right_eye == NULL)
	{
		printf("사진에서 오른쪽 눈을 찾을 수 없습니다. Enter키를 누르시면 종료합니다.\n");
		while(1)
		{
			if (std::cin.get() == '\n')
			{
				return 0;
			}
		}
	}

	//검출된 눈깔 Rectangle 받아오기(눈알은 하나만 입력되고 인식됐다고 가정)
	CvRect *left_eye_rect = 0;
	left_eye_rect = (CvRect*) cvGetSeqElem(left_eye, 0);

	CvRect *right_eye_rect = 0;
	right_eye_rect = (CvRect*) cvGetSeqElem(right_eye, 0);


	//eye_rect의 좌표를 기억해놓아서 앞머리의 상대적 길이를 판정할 때 도움을 받아야 함 
	//cvRectangle(pic, cvPoint( face_rect->x + left_eye_rect->x, (face_rect->y)+(left_eye_rect->y) ), cvPoint(face_rect->x + left_eye_rect->x+left_eye_rect->width, (face_rect->y)+(left_eye_rect->y)+left_eye_rect->height), cvScalar(0,255,0), 3, CV_AA, 0);

	//cvRectangle(pic, cvPoint(face_rect->x, face_rect->y), cvPoint(face_rect->x+face_rect->width, face_rect->y+face_rect->height), cvScalar(0,255,0), 3, CV_AA, 0);



	eye_upper_bound = (face_rect->y) + (left_eye_rect->y);
	
	//윗머리 끝 라인
	int hair_upper_boundary_row = -1;
	rgb hair_color;

	
	//스펙트럼을 이용한 앞머리 경계 판정
	
	int front_hair_left_lower_bound = front_hair_analysis(face_mat, face_rect->x + left_eye_rect->x + left_eye_rect->width, eye_upper_bound, &hair_color, &hair_upper_boundary_row);
	int front_hair_right_lower_bound = front_hair_analysis(face_mat, face_rect->x + (int)(face_rect->width / 2.0) + right_eye_rect->x, eye_upper_bound, &hair_color, &hair_upper_boundary_row);
	int mid_front_hair_lower_bound = front_hair_analysis(face_mat, face_rect->x + (int)(face_rect->width / 2.0), eye_upper_bound, &hair_color, &hair_upper_boundary_row);

	int chin_line_bound = get_chin_line_bound(face_rect, pic_gray);
	
	int left_hair_lower_bound = get_left_hair_lower_bound((int) ( eye_upper_bound + (chin_line_bound - eye_upper_bound)*(3.0/5.0) - 1 ), chin_line_bound, face_rect->x + left_eye_rect->x + (left_eye_rect->width/2.0), face_rect->x+3);

	side_hair_style detected_side_hair = judge_side_hairstyle(left_hair_lower_bound, chin_line_bound, hair_upper_boundary_row);

	front_hair_style mid_front_hair_style = judge_front_hairstyle(mid_front_hair_lower_bound, hair_upper_boundary_row);
	front_hair_style left_front_hair_style = judge_front_hairstyle(front_hair_left_lower_bound, hair_upper_boundary_row);
	front_hair_style right_front_hair_style = judge_front_hairstyle(front_hair_right_lower_bound, hair_upper_boundary_row);

	show_recommended_hairstyle(mid_front_hair_style, detected_side_hair);

	cvShowImage("original image (exit = esc)",pic);
	cvWaitKey(0);

	cvDestroyWindow("original image (exit = esc)");

	return 0;
}