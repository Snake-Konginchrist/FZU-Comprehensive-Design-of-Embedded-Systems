#include <iostream>
#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include <opencv2/opencv.hpp>

int main() {
    int channel = 3;
    int width = 1920;
    int height = 1080;
    int dev_id = 0;
    bm_handle_t handle;
    
    // 请求设备
    bm_status_t dev_ret = bm_dev_request(&handle, dev_id);

    // 读取两张输入图片（假设为RGB格式）
    cv::Mat src1 = cv::imread("cutecat.jpg");
    cv::Mat src2 = cv::imread("cutedog.jpg");

    // 检查图片是否成功读取
    if (src1.empty() || src2.empty()) {
        std::cerr << "无法读取输入图片！" << std::endl;
        return -1;
    }

    // 创建并分配内存给输入和输出图像
    bm_image input1, input2, output;
    bm_image_create(handle, height, width, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &input1);
    bm_image_create(handle, height, width, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &input2);
    bm_image_create(handle, height, width, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &output);

    bm_image_alloc_dev_mem(input1);
    bm_image_alloc_dev_mem(input2);
    bm_image_alloc_dev_mem(output);

    // 将OpenCV图像数据复制到设备内存
    bm_image_copy_host_to_device(input1, (void**)&src1.data);
    bm_image_copy_host_to_device(input2, (void**)&src2.data);

    // 执行加权融合操作
    if (BM_SUCCESS != bmcv_image_add_weighted(handle, input1, 0.5, input2, 0.5, 0, output)) {
        std::cout << "bmcv add_weighted 出错！" << std::endl;
        bm_image_destroy(input1);
        bm_image_destroy(input2);
        bm_image_destroy(output);
        bm_dev_free(handle);
        exit(-1);
    }

    // 将结果从设备复制到主机内存
    cv::Mat result_mat(height, width, CV_8UC3);
    bm_image_copy_device_to_host(output, (void**)&result_mat.data);

    // 保存结果为 "out.jpg"
    cv::imwrite("out.jpg", result_mat);

    // 释放资源
    bm_image_destroy(input1);
    bm_image_destroy(input2);
    bm_image_destroy(output);
    bm_dev_free(handle);

    return 0;
}
