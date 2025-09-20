// #include <cstddef>
// #include <opencv2/opencv.hpp>
// #include <iostream>
// #include <sys/stat.h>  

// const std::string OUT_DIR = "/home/danny/opencv_project/resources/result_img2";


// void prepareDir() {
// #ifdef _WIN32
//     _mkdir(OUT_DIR.c_str());
// #else
//     mkdir(OUT_DIR.c_str(), 0755);  
// #endif
// }

// void saveShow(const cv::Mat& img, const std::string& name, int idx) {
//     std::string fname = OUT_DIR + "/" + std::to_string(idx) + "_" + name + ".png";
//     if (!cv::imwrite(fname, img)) {
//         std::cerr << "Error: Unable to save image to " << fname << std::endl;
//         return;
//     }
//     std::cout << "[INFO] Saved " << fname << std::endl;
//     cv::imshow(name, img);                      
// }

// int main(int argc, char** argv) {
//     prepareDir();  

//     std::string imgPath = (argc > 1) ? argv[1] : "/home/danny/opencv_project/resources/test_image.png";
//     cv::Mat img = cv::imread(imgPath);
//     if (img.empty()) { 
//         std::cerr << "Cannot read image: " << imgPath << std::endl; 
//         return -1; 
//     }
//   //  cv::imshow("origin", img); 
//    // cv::waitKey(200);

//     // 2. 颜色空间转换：灰度 + HSV
//     cv::Mat gray, hsv;
//     cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);   // 灰度化，单通道，降低计算量
//     cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);     // HSV 把颜色与亮度分开，方便颜色过滤
//   //  saveShow(hsv, "hsv", 1);


//     // 5. 高亮区域二值化 + 形态学开运算
//     cv::Mat bin, morph;
//     cv::threshold(gray, bin, 128, 255, cv::THRESH_BINARY);  // 阈值 180：提取高亮
//   //  saveShow(bin, "bright_binary", 9);
//     cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
//     cv::morphologyEx(bin, morph, cv::MORPH_OPEN, kernel);  // 开运算：先腐蚀再膨胀，去小噪点
//   //  saveShow(morph, "morph_open", 10);

//     cv::Mat edges;
//     cv::Canny(morph, edges, 100, 200);  // Canny 边缘检测
//  //   saveShow(edges, "canny_edges", 11);

//     cv::Mat morph2;
//     cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
//     morphologyEx(edges, morph2, cv::MORPH_CLOSE, kernel2);  // 闭运算：先膨胀再腐蚀，连通边缘
//     saveShow(morph2, "morph_close", 12);

//     std::vector<std::vector<cv::Point>> contours;
//     std::vector<cv::Vec4i> hierarchy;
//     cv::findContours(morph2 ,contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
//     cv:: Mat contourImg = cv::Mat::zeros(img.size(), CV_8UC3);
//     for(size_t i = 0; i < contours.size(); i++) {
//         cv::drawContours(contourImg, contours, (int)i, cv::Scalar(0, 255, 0), 2);
//     }
//     saveShow(contourImg, "lunkuo", 16);
//     // 绘制轮廓

//     for (size_t i = 0; i < contours.size(); i++) {
//             double area = contourArea(contours[i]);
//                 if (area < 2000 || area > 6000) continue; //面积过小的剔除
//                 cv::Rect bbox = boundingRect(contours[i]);
//             double aspectRatio = static_cast<double>(bbox.width) / bbox.height;
//         if (aspectRatio < 0.75 || aspectRatio > 1) continue; // 筛选长宽比不符合的轮廓
//                 rectangle(img, bbox, cv::Scalar(0, 0, 255), 2);
//     } 
//     saveShow(img, "lunkuoshaixuan", 17);
//     std::cout << "[INFO] All results saved to " << OUT_DIR << std::endl;
//     std::cout << "[INFO] Press any key to exit..." << std::endl;
//     cv::waitKey(0);  
//     return 0;
// }