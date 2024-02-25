#ifndef VIDEODEC_FFMPEG_H
#define VIDEODEC_FFMPEG_H

// 引入 C 语言的头文件，用于使用 FFmpeg 库
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

// VideoDec_FFMPEG 类的声明
class VideoDec_FFMPEG {
public:
    // 构造函数
    VideoDec_FFMPEG();

    // 析构函数
    ~VideoDec_FFMPEG();

    // 打开解码器和文件
    int openDec(const char *filename, int codec_name_flag, const char *coder_name, int output_format_mode,
                int extra_frame_buffer_num, int sophon_idx, int pcie_no_copyback);

    // 获取一帧解码结果
    AVFrame *grabFrame();

    // 关闭解码器和文件
    void closeDec();

public:
    AVFormatContext *ifmt_ctx;        // 输入文件格式上下文
    AVCodecContext *video_dec_ctx;    // 视频解码器上下文
    AVCodecParameters *video_dec_par; // 视频解码器参数
    AVCodec *decoder;                  // 解码器
    int width;                         // 视频宽度
    int height;                        // 视频高度
    int pix_fmt;                       // 视频像素格式
    int video_stream_idx;              // 视频流索引
    int refcount;                      // 是否使用引用计数

    AVPacket pkt;                      // FFmpeg 数据包
    AVFrame *frame;                    // FFmpeg 帧
    // 其他可能需要的成员变量

    // 打开指定流的解码器上下文
    int openCodecContext(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                         enum AVMediaType type, int codec_name_flag, const char *coder_name,
                         int output_format_mode, int extra_frame_buffer_num, int sophon_idx, int pcie_no_copyback);
};

#endif // VIDEODEC_FFMPEG_H
