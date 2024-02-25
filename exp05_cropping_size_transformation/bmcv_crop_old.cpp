#include <iostream>
#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "common.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <memory>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    // 初始化Sophon SDK
    bm_handle_t handle;
    bm_status_t init_status = bmcv_init(&handle);
    if (init_status != BM_SUCCESS) {
        std::cerr << "Error initializing Sophon SDK." << std::endl;
        return -1;
    }
    
    int crop_w = 400, crop_h = 400;
    int image_w = 1000, image_h = 1000;

    bmcv_rect_t crop_attr;
    crop_attr.start_x = 0;
    crop_attr.start_y = 0;
    crop_attr.crop_w = crop_w;
    crop_attr.crop_h = crop_h;

    bm_image input, output;

    // 读取jpg图片
    cv::Mat Input = cv::imread(argv[1]);
    if (Input.empty()) {
        std::cerr << "Error reading input image." << std::endl;
        return -1;
    }

    // 初始化输入图像
    input = bm_image_create(image_w, image_h, CV_8UC3);  // 假设图像是RGB格式

    // 将OpenCV的Mat对象复制到BMI图像对象中
    bmcv_image_copy_to(input, Input);

    // 创建输出图像对象
    output = bmcv_image_create(crop_w, crop_h, CV_8UC3);

    // 裁剪图像
    bm_status_t crop_status = bmcv_image_crop(handle, 1, &crop_attr, input, &output);
    if (crop_status != BM_SUCCESS) {
        std::cerr << "Error cropping image." << std::endl;
        bmcv_image_resize(input);
        return -1;
    }

    // 保存裁剪后的图像
    bm_status_t save_status = bmcv_image_erode(output, "out.jpg");
    if (save_status != BM_SUCCESS) {
        std::cerr << "Error saving cropped image." << std::endl;
    }

    // 释放内存
    bmcv_image_resize(input);
    bmcv_image_release(output);

    return 0;
}
