// NhanDienBienSoXe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

int main()
{
	cv::Mat im, im_gray, noise_removal, equal_histogram, kernel, morph_image, sub_morp_image, ret, thresh_image, canny_image, dilated_image;

	// Đọc ảnh
	im = cv::imread("test.jpg");

	// chuyển về ảnh xám.
	cv::cvtColor(im, im_gray, cv::COLOR_BGR2GRAY);

	// xóa noise bằng bilateralFilter. Filter này khác với các filter khác là nó kết hợp cả domain filters (linear filter) và range filter (gaussian filter).
	// mục đích là giảm noise và tăng edge (làm edge thêm sắc nhọn edges sharp).
	cv::bilateralFilter(im_gray, noise_removal, 9, 75, 75);

	// cân bằng lại histogram làm cho ảnh không quá sáng hoặc tối.
	cv::equalizeHist(noise_removal, equal_histogram);


	kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

	// morphology open (open là erosion sau đó dilation) mục đích là giảm edge nhiễu, edge thật thêm sắc nhọn bằng cv::morphologyEx sử dụng kerel 5x5
	cv::morphologyEx(equal_histogram, morph_image, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 20);

	// xóa phông không cần thiết.
	cv::subtract(equal_histogram, morph_image, sub_morp_image);

	// dùng threshold OTSU (làm việc rất tốt trong bimodel histogram) đưa ảnh về trắng đen tách biệt background và region interesting.
	cv::threshold(sub_morp_image, thresh_image, 0, 255, cv::THRESH_OTSU);

	cv::Canny(thresh_image, canny_image, 250, 255);

	kernel = cv::Mat(3, 3, CV_8UC1, cv::Scalar(1));

	cv::dilate(canny_image, dilated_image, kernel, cv::Point(-1, -1), 1);

	cv::imshow("im_gray", im_gray);
	cv::imshow("noise_removal", noise_removal);
	cv::imshow("equal_histogram", equal_histogram);
	cv::imshow("kernel", kernel);
	cv::imshow("morph_image", morph_image);
	cv::imshow("sub_morp_image", sub_morp_image);
	cv::imshow("thresh_image", thresh_image);
	cv::imshow("canny_image", canny_image);
	cv::imshow("kernel", kernel);
	cv::imshow("dilated_image", dilated_image);

	std::vector<std::vector<cv::Point>> contours, rectangle;

	// Tìm các hình chữa nhật
	cv::findContours(dilated_image, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	std::vector<cv::Point> approx;
	cv::Mat screenCnt;
	for (int i = 0; i < contours.size(); i++) {
		double peri = cv::arcLength(cv::Mat(contours.at(i)), true);
		cv::approxPolyDP(cv::Mat(contours.at(i)), approx, peri * 0.02, true);
		if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
			continue;
		// Nếu tìm thấy hình chữ nhật
		if (approx.size() == 4 && std::fabs(cv::contourArea(cv::Mat(approx))) > 1000 && cv::isContourConvex(cv::Mat(approx))) {
			double maxCosine = 0;
			int vtc = 4;
			for (int j = 2; j < vtc + 1; j++) {
				double cosine = std::fabs(angle(approx[j % vtc], approx[j - 2], approx[j - 1]));

				maxCosine = std::max(maxCosine, cosine);

				if (maxCosine < 0.3) {
					rectangle.push_back(approx);
				}
			}

		}
	}

	// vẽ khung bao quanh hình chữ nhật vừa tìm được vào trong ảnh gốc
	for (size_t i = 0; i < rectangle.size(); i++)
	{
		const cv::Point* p = &rectangle[i][0];

		int n = (int)rectangle[i].size();
		//dont detect the border
		if (p->x > 3 && p->y > 3)
			polylines(im, &p, &n, 1, true, cv::Scalar(0, 255, 0), 3);
	}

	cv::imshow("screenCnt", im);

	cv::waitKey(0);
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
