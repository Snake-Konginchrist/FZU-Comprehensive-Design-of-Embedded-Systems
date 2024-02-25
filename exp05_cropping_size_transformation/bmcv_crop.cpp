#include <iostream>
#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include <memory>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 2) {
        std::cerr << "Usage: "<< argv[0] << "<image_filename>" << std::endl;
        return -1;
    }

    const char* input_image_file = argv[1];

    // 请求Sophon设备
    int dev_id = 0;
    bm_handle_t handle;
    bm_status_t dev_ret = bm_dev_request(&handle, dev_id);
    if (dev_ret != BM_SUCCESS) {
        std::cerr << "Error requesting Sophon device." << std::endl;
        return -1;
    }

    // 加载图像
    cv::Mat input_image = cv::imread(input_image_file);
    if (input_image.empty()) {
        std::cerr << "Error reading input image." << std::endl;
        bm_dev_free(handle);
        return -1;
    }

    // 将图像数据拷贝到Sophon设备
    bm_image input_bmcv_image;
    bm_image_create(handle, 
    	input_image.rows, 
    	input_image.cols, 
    	FORMAT_RGB_PLANAR, 
    	DATA_TYPE_EXT_1N_BYTE, 
    	&input_bmcv_image);
    bm_image_alloc_dev_mem(input_bmcv_image);
    bm_image_copy_host_to_device(input_bmcv_image, (void**)&input_image.data);

    // 使用Sophon SDK进行图像处理（示例：裁剪图像）
    bmcv_rect_t crop_attr;
    crop_attr.start_x = 0;
    crop_attr.start_y = 0;
    crop_attr.crop_w = 100;
    crop_attr.crop_h = 100;

    bm_image output_bmcv_image;
    bm_image_create(handle, 
    	crop_attr.crop_h, 
    	crop_attr.crop_w, 
    	FORMAT_RGB_PLANAR, 
    	DATA_TYPE_EXT_1N_BYTE, 
    	&output_bmcv_image);
    bm_image_alloc_dev_mem(output_bmcv_image);

    if (BM_SUCCESS != bmcv_image_crop(handle, 1, &crop_attr, input_bmcv_image, &output_bmcv_image)) {
        std::cout << "Error in bmcv_image_crop." << std::endl;
        bm_image_destroy(input_bmcv_image);
        bm_image_destroy(output_bmcv_image);
        bm_dev_free(handle);
        return -1;
    }

    // 将处理后的图像数据从Sophon设备拷贝回主机
    cv::Mat output_image(output_bmcv_image.height, output_bmcv_image.width, CV_8UC3);
    bm_image_copy_device_to_host(output_bmcv_image, (void**)&output_image.data);

    // 保存处理后的图像
    cv::imwrite("out.jpg", output_image);

    // 释放资源
    bm_image_destroy(input_bmcv_image);
    bm_image_destroy(output_bmcv_image);
    bm_dev_free(handle);

    return 0;
}
