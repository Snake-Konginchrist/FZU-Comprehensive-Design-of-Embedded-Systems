#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    }

    // 读取图像
    Mat gray = imread(argv[1], IMREAD_GRAYSCALE);
    if (gray.empty()) {
        std::cerr << "Error: Unable to read the image." << std::endl;
        return -1;
    }

    // 需要计算的图像的通道，灰度图像为0，BGR图像需要指定B,G,R
    const int channels[] = {0};
    Mat hist; // 定义输出Mat类型
    int dims = 1; // 设置直方图维度
    const int histSize[] = {256}; // 直方图每一个维度划分的柱条的数目
    float pranges[] = {0, 255}; // 取值区间
    const float* ranges[] = {pranges};

    // 计算直方图
    calcHist(&gray, 1, channels, Mat(), hist, dims, histSize, ranges, true, false);

    // 归一化直方图
    normalize(hist, hist, 0, 255, NORM_MINMAX);

    // 创建绘制直方图的图像
    int scale = 2;
    int hist_height = 256;
    Mat hist_img = Mat::zeros(hist_height, 256 * scale, CV_8UC3);

    // 找到直方图中的最大值
    double max_val;
    minMaxLoc(hist, 0, &max_val, 0, 0);

    // 绘制直方图
    for (int i = 0; i < 256; i++) {
        float bin_val = hist.at<float>(i);
        int intensity = cvRound(bin_val * hist_height / max_val);

        // 使用矩形绘制直方图柱条
        rectangle(hist_img, Point(i * scale, hist_height - 1), Point((i + 1) * scale - 1, hist_height - intensity), Scalar(255, 255, 255));
    }

    // 保存绘制好的直方图图像
    imwrite("opencv_calc_hist_out.jpg", hist_img);

    return 0;
}
