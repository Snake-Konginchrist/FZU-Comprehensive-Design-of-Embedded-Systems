#include <iostream>
#include "bmcv_api.h"
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    int resize_w = 400, resize_h = 400;

    bmcv_resize_image resize_attr;
    bmcv_resize_t resize_img_attr;

    resize_img_attr.start_x = 0;
    resize_img_attr.start_y = 0;
    resize_img_attr.in_width = resize_w;
    resize_img_attr.in_height = resize_h;
    resize_img_attr.out_width = resize_w;
    resize_img_attr.out_height = resize_h;

    resize_attr.resize_img_attr = &resize_img_attr;
    resize_attr.roi_num = 1;
    resize_attr.stretch_fit = 1;
    resize_attr.interpolation = BMCV_INTER_NEAREST;

    bm_image input, output;

    // 读取jpg图片
    cv::Mat Input = cv::imread(argv[1]);
    if (Input.empty()) {
        std::cerr << "Error reading input image." << std::endl;
        return -1;
    }

    // 初始化输入图像
    input = bmcv_image_create(Input.cols, Input.rows, BMCV_U8C3);  // 假设图像是RGB格式

    // 将OpenCV的Mat对象复制到BMI图像对象中
    bmcv_image_copy_from_mat(input, Input);

    // 创建输出图像对象
    output = bmcv_image_create(resize_w, resize_h, BMCV_U8C3);

    // 缩放图像
    bm_status_t resize_status = bmcv_image_resize(handle, 1, &resize_attr, &input, &output);
    if (resize_status != BM_SUCCESS) {
        std::cerr << "Error resizing image." << std::endl;
        bmcv_image_release(input);
        return -1;
    }

    // 保存缩放后的图像
    bm_status_t save_status = bmcv_image_write(output, "out.jpg");
    if (save_status != BM_SUCCESS) {
        std::cerr << "Error saving resized image." << std::endl;
    }

    // 释放内存
    bmcv_image_release(input);
    bmcv_image_release(output);

    return 0;
}
