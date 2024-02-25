#include <iostream>
#include <vector>
#include "bmcv_api.h"
#include "common.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <memory>
#include <opencv2/opencv.hpp>

// 避免在全局命名空间中使用using namespace
// 通过使用cv::来限定OpenCV相关标识符
// 避免潜在的命名冲突
using namespace cv;
using namespace std;
 
int main(int argc, char *argv[]) {
	// 检查命令行参数
    	if (argc < 2) {
        	std::cerr << "Usage: " << argv[0] << " <image_file_path>" << std::endl;
        	return EXIT_FAILURE;
    	}
    	
    	// 获取BM句柄
	bm_handle_t handle;
	bm_dev_request(&handle, 0);
	
	//定义图片数据 
	int width =  600;		
	int height = 600;
	cv::Mat Input,Out,Test; 				   
	Input = cv::imread(argv[1], 0); //opencv读取图片，通过命令行参数传入
		
	// 智能指针获取分配内存数据 
	std::unique_ptr<unsigned char[]> src_data(new unsigned char[width * height]);
	std::unique_ptr<unsigned char[]> res_data(new unsigned char[width * height]);
 
	// BMCV处理 
	bm_image input, output;
	bm_image_create(handle,height,width,FORMAT_GRAY, DATA_TYPE_EXT_1N_BYTE,&input);
	bm_image_alloc_contiguous_mem(1, &input, 1); 	// 分配device memory 
	unsigned char * input_img_data = src_data.get();
	bm_image_copy_host_to_device(input, (void **)&input_img_data);
    	bm_image_create(handle,height,width,FORMAT_GRAY,DATA_TYPE_EXT_1N_BYTE,&output);
	bm_image_alloc_contiguous_mem(1, &output, 1);	
		
	cv::bmcv::toBMI(Input,&input);                  //自动进行内存同步
    	// BMCV图像处理：ca
	if (BM_SUCCESS != bmcv_image_sobel(handle, input, output, 0, 1)) {
		std::cout << "bmcv sobel error !!!" << std::endl;
		// 释放资源
		bm_image_destroy(input);
		bm_image_destroy(output);
		bm_dev_free(handle);
		return -1;
	}
	
	// 将输出结果转成Mat数据并保存 
	cv::bmcv::toMAT(&output, Out);
	cv::imwrite("out.jpg", Out);
		
	// 释放资源
	bm_image_free_contiguous_mem(1, &input);
	bm_image_free_contiguous_mem(1, &output);
	bm_image_destroy(input);
	bm_image_destroy(output);
	bm_dev_free(handle);
	
   	return EXIT_SUCCESS; 
}
