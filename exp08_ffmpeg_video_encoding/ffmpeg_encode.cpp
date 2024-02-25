#include <iostream>
#include "videoenc_ffmpeg.h"

// 引入 FFmpeg 库的头文件
extern "C" {
    #include "libavcodec/avcodec.h"          // 视频编解码器
    #include "libswscale/swscale.h"          // 视频像素格式转换
    #include "libavutil/imgutils.h"          // 图像工具函数
    #include "libavformat/avformat.h"        // 封装格式处理
    #include "libavfilter/buffersink.h"      // 缓冲池接收端
    #include "libavfilter/buffersrc.h"       // 缓冲池发送端
    #include "libavutil/opt.h"               // AVOption 选项设置
    #include "libavutil/pixdesc.h"           // 像素格式描述
}
 
#define STEP_ALIGNMENT 32  // 步进对齐值，可根据需要调整

int main(int argc, char **argv)
{
    // 设置参数和变量
    int soc_idx      = 0;                              // 视频流索引
    int enc_id       = AV_CODEC_ID_H264;               // 编码器ID，可以改为 AV_CODEC_ID_H265
    int inputformat  = AV_PIX_FMT_YUV420P;             // 输入视频像素格式
    int framerate    = 30;                             // 视频帧率
    int width        = 1920;                           // 视频宽度
    int height       = 1080;                           // 视频高度
    int bitrate      = 1000000;                        // 比特率，单位：比特每秒
    char *input_file = "1080p.yuv";                    // 输入 YUV 文件名
    char *output_file= "test.mp4";                     // 输出 MP4 文件名
    int ret;
 
    // 设置日志级别为调试
    av_log_set_level(AV_LOG_DEBUG);
 
    // 计算对齐参数
    int stride = (width + STEP_ALIGNMENT - 1) & ~(STEP_ALIGNMENT - 1);
    int aligned_input_size = stride * height * 3 / 2;
 
    // 分配对齐内存
    uint8_t *aligned_input = (uint8_t*)av_mallocz(aligned_input_size);
    if (aligned_input == NULL) {
        av_log(NULL, AV_LOG_ERROR, "av_mallocz failed\n");
        return -1;
    }
 
    // 打开输入文件
    FILE *in_file = fopen(input_file, "rb");
    if (in_file == NULL) {
        fprintf(stderr, "Failed to open input file\n");
        return -1;
    }
 
    bool isFileEnd = false;
    VideoEnc_FFMPEG writer;
    // 打开视频编码器
    ret = writer.openEnc(output_file, soc_idx, enc_id, framerate, width, height, inputformat, bitrate);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "writer.openEnc failed\n");
        return -1;
    }
 
    // 读取原始数据并进行编码
    while (1) {
        for (int y = 0; y < height * 3 / 2; y++) {
            ret = fread(aligned_input + y * stride, 1, width, in_file);
            if (ret < width) {
                if (ferror(in_file))
                    av_log(NULL, AV_LOG_ERROR, "Failed to read raw data!\n");
                else if (feof(in_file))
                    av_log(NULL, AV_LOG_INFO, "The end of file!\n");
                isFileEnd = true;
                break;
            }
        }
        if (isFileEnd)
            break;
 
        // 写入帧数据
        writer.writeFrame(aligned_input, stride, width, height);
    }
 
    // 关闭编码器
    writer.closeEnc();
 
    // 释放内存和关闭文件
    av_free(aligned_input);
    fclose(in_file);
 
    av_log(NULL, AV_LOG_INFO, "encode finish! \n");
    return 0;
}
