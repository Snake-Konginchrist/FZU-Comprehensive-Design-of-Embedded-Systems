#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "用法：" << argv[0] << " <视频文件>" << endl;
        return -1;
    }

    // 初始化VideoCapture类
    VideoCapture cap;

    // 打开视频文件
    cap.open(argv[1], CAP_FFMPEG);

    // 检查视频是否成功打开
    if (!cap.isOpened()) {
        cerr << "错误：无法打开视频文件。" << endl;
        return -1;
    }

    // 设置输出的高和宽
    int h = 480; // 设置期望的高度
    int w = 640; // 设置期望的宽度
    cap.set(CAP_PROP_FRAME_HEIGHT, (double)h);
    cap.set(CAP_PROP_FRAME_WIDTH, (double)w);

    // 设置输出为YUV数据格式
    cap.set(CAP_PROP_OUTPUT_YUV, PROP_TRUE);
    
    // 设置输出为YUV数据格式（如果这是一个有效的设置）
    // 注意：cap.set(CAP_PROP_OUTPUT_YUV, PROP_TRUE); 这一行可能不是必需的，取决于OpenCV版本和你的需求。

    // 读取并处理视频帧
    Mat image;
    
    // 假设'dumpfile'是一个用于写入YUV数据的FILE指针
    FILE* dumpfile = fopen("output.yuv", "wb");
    
    while (cap.read(image)) {
        // 内存同步到CPU
        // 注意：以下代码假设'bmcv::downloadMat'函数在其他地方定义
        bmcv::downloadMat(image);

        // 假设'dumpfile'是一个用于写入YUV数据的FILE指针
        for (int i = 0; i < image.avRows(); i++) {
            fwrite((char*)image.avAddr(0) + i * image.avStep(0), 1, image.avCols(), dumpfile);
        }

        for (int i = 0; i < image.avRows() / 2; i++) {
            fwrite((char*)image.avAddr(1) + i * image.avStep(1), 1, image.avCols() / 2, dumpfile);
        }

        for (int i = 0; i < image.avRows() / 2; i++) {
            fwrite((char*)image.avAddr(2) + i * image.avStep(2), 1, image.avCols() / 2, dumpfile);
        }

        // 在此处添加处理逻辑

        // 如果用户按下'Esc'键，则中断循环
        if (waitKey(30) == 27) {
            cout << "用户按下了Esc键。停止播放视频。" << endl;
            break;
        }
    }

    // 释放VideoCapture对象并关闭dumpfile
    cap.release();
    fclose(dumpfile);

    return 0;
}
