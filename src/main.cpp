#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/stat.h>   // 用于跨平台创建文件夹

// 结果保存根目录（用绝对路径，避免运行时工作目录不同导致找不到）
const std::string OUT_DIR = "/home/danny/opencv_project/resources/result_img";

/* -------------------- 工具函数 -------------------- */
// 创建输出目录（Linux 下 mkdir；Windows 用 _mkdir）
void prepareDir() {
#ifdef _WIN32
    _mkdir(OUT_DIR.c_str());
#else
    mkdir(OUT_DIR.c_str(), 0755);   // 0755 表示用户可读写执行，组和其他只读执行
#endif
}

// 保存、显示、打印一条龙
// idx 仅用来给文件名加序号，方便查看顺序
void saveShow(const cv::Mat& img, const std::string& name, int idx) {
    std::string fname = OUT_DIR + "/" + std::to_string(idx) + "_" + name + ".png";
    if (!cv::imwrite(fname, img)) {
        std::cerr << "Error: Unable to save image to " << fname << std::endl;
        return;
    }
    std::cout << "[INFO] Saved " << fname << std::endl;
    cv::imshow(name, img);                       // 弹出窗口显示
}

/* -------------------- main 函数 -------------------- */
int main(int argc, char** argv) {
    prepareDir();   // 先确保输出目录存在

    // 1. 读取原图（支持命令行传入任意图片路径，默认用培训给的测试图）
    std::string imgPath = (argc > 1) ? argv[1] : "/home/danny/opencv_project/resources/test_image_1.png";
    cv::Mat img = cv::imread(imgPath);
    if (img.empty()) { 
        std::cerr << "Cannot read image: " << imgPath << std::endl; 
        return -1; 
    }
    cv::imshow("origin", img); 
    cv::waitKey(200);

    // 2. 颜色空间转换：灰度 + HSV
    cv::Mat gray, hsv;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);   // 灰度化，单通道，降低计算量
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);     // HSV 把颜色与亮度分开，方便颜色过滤
    saveShow(gray, "gray", 1);
    saveShow(hsv, "hsv", 2);

    // 3. 三种滤波：均值、高斯、中值
    cv::Mat blurImg, gauImg, medImg;
    cv::blur(img, blurImg, cv::Size(5, 5));                    // 均值滤波：简单平滑
    cv::GaussianBlur(img, gauImg, cv::Size(5, 5), 1.5);       // 高斯滤波：加权平滑，保留边缘更好
    cv::medianBlur(img, medImg, 5);                            // 中值滤波：去椒盐噪点
    saveShow(blurImg, "blur", 3);
    saveShow(gauImg, "gaussian", 4);
    saveShow(medImg, "median", 5);

    // 4. 提取红色区域 → 轮廓 → 外接矩形 → 面积
    cv::Mat redMask;
    // HSV 红色有两个区间：0-10 和 170-180，合并结果
    cv::inRange(hsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), redMask);
    // cv::Mat tmp;
    // cv::inRange(hsv, cv::Scalar(170, 120, 70), cv::Scalar(180, 255, 255), tmp);
//    redMask += tmp;   // 掩膜相加，得到完整红色区域
    saveShow(redMask, "red_mask", 6);

    cv::Mat morph;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(redMask, morph, cv::MORPH_CLOSE,kernel);


    // 找轮廓：只检测最外层，压缩水平/垂直段
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(redMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat contourImg = img.clone();
    cv::drawContours(contourImg, contours, -1, cv::Scalar(0, 255, 0), 2);  // 绿色画轮廓
    saveShow(contourImg, "red_contour", 7);

    
    // 对每个轮廓计算面积并画最小外接矩形
    cv::Mat bboxImg = img.clone();
    for (const auto& c : contours) {
        double area = cv::contourArea(c);   // 计算面积
        if (area < 100) continue;           // 过滤小噪声
        cv::Rect r = cv::boundingRect(c);   // 最小外接矩形（正矩形）
        cv::rectangle(bboxImg, r, cv::Scalar(0, 0, 255), 2);  // 红色框
        std::cout << "[INFO] Red bbox area: " << area << " px\n";
    }
    saveShow(bboxImg, "red_bbox", 8);

    // 5. 高亮区域二值化 + 形态学开运算
    cv::Mat bin, morph_2;
    cv::threshold(gray, bin, 128, 255, cv::THRESH_BINARY);  // 阈值 180：提取高亮
    saveShow(bin, "bright_binary", 9);
    cv::Mat kernel_2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(bin, morph_2, cv::MORPH_OPEN, kernel_2);  // 开运算：先腐蚀再膨胀，去小噪点
    saveShow(morph_2, "morph_open", 10);

    // 6. 旋转 35°
    cv::Point2f center(img.cols / 2.f, img.rows / 2.f);
    cv::Mat rotM = cv::getRotationMatrix2D(center, 35, 1.0);  // 角度 35，不缩放
    cv::Mat rotated;
    cv::warpAffine(img, rotated, rotM, img.size());
    saveShow(rotated, "rotate35", 11);

    // 7. 裁剪左上角 1/4
    cv::Rect roi(0, 0, img.cols / 2, img.rows / 2);  // 左上角 1/4 区域
    cv::Mat crop = img(roi).clone();                  // 深拷贝，防止后续修改影响原图
    saveShow(crop, "crop_tl", 12);

    // 8. 额外绘制：圆、矩形、文字
    cv::Mat draw = img.clone();
    cv::circle(draw, cv::Point(100, 100), 50, cv::Scalar(255, 0, 0), 3);  // 蓝色圆
    cv::rectangle(draw, cv::Point(200, 50), cv::Point(400, 200), cv::Scalar(0, 255, 0), 2);  // 绿色矩形
    cv::putText(draw, "Hello DX-RMV", cv::Point(50, 400), cv::FONT_HERSHEY_SIMPLEX,
                1.2, cv::Scalar(0, 0, 255), 2);  // 红色文字
    saveShow(draw, "drawing", 13);

    std::cout << "[INFO] All results saved to " << OUT_DIR << std::endl;
    std::cout << "[INFO] Press any key to exit..." << std::endl;
    cv::waitKey(0);  // 等待用户按键结束
    return 0;
}