#include <iostream>
#include <opencv2/opencv.hpp>
#include "bmcv_api.h"
#include "bmcv_api_ext.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_image_file>" << std::endl;
        return -1;
    }

    const char* input_image_file = argv[1];
    const char* output_image_file = "out.jpg";

    // 初始化Sophon SDK
    bm_handle_t handle;  // 请根据实际SDK提供的接口修改
    
    // 这里应该是Sophon SDK的初始化函数，请根据实际接口修改
    // bm_status_t init_status = bmcv_init(&handle);
    // if (init_status != BM_SUCCESS) {
    //     std::cerr << "Error initializing Sophon SDK." << std::endl;
    //     return -1;
    // }

    // 读取输入图像
    cv::Mat input_image = cv::imread(input_image_file);
    if (input_image.empty()) {
        std::cerr << "Error reading input image." << std::endl;
        // bmcv_release(&handle);  // 请根据实际SDK提供的接口修改
        return -1;
    }

    // 图像尺寸变化参数
    int crop_w = 711, crop_h = 400, resize_w = 711, resize_h = 400;
    bmcv_resize_image resize_attr;
    bmcv_resize_t resize_img_attr;

    resize_img_attr.start_x = 0;
    resize_img_attr.start_y = 0;
    resize_img_attr.in_width = crop_w;
    resize_img_attr.in_height = crop_h;
    resize_img_attr.out_width = resize_w;
    resize_img_attr.out_height = resize_h;

    resize_attr.resize_img_attr = &resize_img_attr;
    resize_attr.roi_num = 1;
    resize_attr.stretch_fit = 1;
    resize_attr.interpolation = BMCV_INTER_NEAREST;

    bm_image input, output;

    // 创建输入图像对象
    int input_data_type = DATA_TYPE_EXT_1N_BYTE;
    bm_image_create(handle,
                    input_image.rows,
                    input_image.cols,
                    FORMAT_BGR_PLANAR,
                    (bm_image_data_format_ext)input_data_type,
                    &input);
    bm_image_alloc_contiguous_mem(1, &input, 1);
    unsigned char* input_img_data = input_image.data;
    bm_image_copy_host_to_device(input, (void**)&input_img_data);

    // 创建输出图像对象
    int output_data_type = DATA_TYPE_EXT_1N_BYTE;
    bm_image_create(handle,
                    resize_h,
                    resize_w,
                    FORMAT_BGR_PLANAR,
                    (bm_image_data_format_ext)output_data_type,
                    &output);
    bm_image_alloc_contiguous_mem(1, &output, 1);

    // 图像尺寸变化
    bmcv_image_resize(handle, 1, &resize_attr, &input, &output);

    // 将处理后的图像数据从Sophon设备拷贝回主机
    cv::Mat output_image(resize_h, resize_w, CV_8UC3);
    unsigned char* res_img_data = output_image.data;
    bm_image_copy_device_to_host(output, (void**)&res_img_data);

    // 保存处理后的图像
    cv::imwrite(output_image_file, output_image);

    // 释放资源
    bm_image_free_contiguous_mem(1, &input);
    bm_image_free_contiguous_mem(1, &output);
    bm_image_destroy(input);
    bm_image_destroy(output);

    return 0;
}
