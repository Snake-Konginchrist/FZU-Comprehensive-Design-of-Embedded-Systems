#include <iostream>
#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <图像路径>" << std::endl;
        return -1;
    }

    cv::Mat image = cv::imread(argv[1]);
    if (image.empty()) {
        std::cerr << "错误: 无法读取图像。" << std::endl;
        return -1;
    }

    int H = image.rows;
    int W = image.cols;
    int C = image.channels();
    int dim = 3;
    int channels[3] = {0, 1, 2};
    int histSizes[] = {15000, 32, 32};
    float ranges[] = {0, 1000000, 0, 256, 0, 256};
    int totalHists = 1;
    for (int i = 0; i < dim; ++i)
        totalHists *= histSizes[i];

    bm_handle_t handle = nullptr;
    bm_status_t ret = bm_dev_request(&handle, 0);

    float* inputHost = new float[C * H * W];
    float* outputHost = new float[totalHists];

    // 将图像数据转换为浮点数
    for (int i = 0; i < C; ++i)
        for (int j = 0; j < H * W; ++j)
            inputHost[i * H * W + j] = static_cast<float>(image.at<cv::Vec3b>(j / W, j % W)[i]);

    if (ret != BM_SUCCESS) {
        std::cerr << "bm_dev_request 失败，ret = " << ret << std::endl;
        exit(-1);
    }

    bm_device_mem_t input, output;
    ret = bm_malloc_device_byte(handle, &input, C * H * W * 4);
    if (ret != BM_SUCCESS) {
        std::cerr << "bm_malloc_device_byte 失败，ret = " << ret << std::endl;
        exit(-1);
    }

    ret = bm_memcpy_s2d(handle, input, inputHost);
    if (ret != BM_SUCCESS) {
        std::cerr << "bm_memcpy_s2d 失败，ret = " << ret << std::endl;
        exit(-1);
    }

    ret = bm_malloc_device_byte(handle, &output, totalHists * 4);
    if (ret != BM_SUCCESS) {
        std::cerr << "bm_malloc_device_byte 失败，ret = " << ret << std::endl;
        exit(-1);
    }

    ret = bmcv_calc_hist(handle, input, output, C, H, W, channels, dim, histSizes, ranges, 0);
    if (ret != BM_SUCCESS) {
        std::cerr << "bmcv_calc_hist 失败，ret = " << ret << std::endl;
        exit(-1);
    }

    ret = bm_memcpy_d2s(handle, outputHost, output);
    if (ret != BM_SUCCESS) {
        std::cerr << "bm_memcpy_d2s 失败，ret = " << ret << std::endl;
        exit(-1);
    }

    // 将输出保存为图像（假设 outputHost 表示像素值）
    cv::Mat outputImage(histSizes[1], histSizes[2], CV_32F, outputHost);
    cv::normalize(outputImage, outputImage, 0, 255, cv::NORM_MINMAX);
    cv::imwrite("out.jpg", outputImage);

    bm_free_device(handle, input);
    bm_free_device(handle, output);
    bm_dev_free(handle);

    delete[] inputHost;
    delete[] outputHost;

    return 0;
}
