#ifndef VIDEOENC_FFMPEG_H
#define VIDEOENC_FFMPEG_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#define STEP_ALIGNMENT 32

class VideoEnc_FFMPEG
{
public:
    // 构造函数
    VideoEnc_FFMPEG();
    
    // 析构函数
    ~VideoEnc_FFMPEG();

    // 打开编码器
    int openEnc(const char* filename, int soc_idx, int codecId, int framerate,
                int width, int height, int inputformat, int bitrate);
    
    // 关闭编码器
    void closeEnc();

    // 写入帧数据
    int writeFrame(const uint8_t* data, int step, int width, int height);

    // 刷新编码器
    int flush_encoder();

private:
    AVFormatContext* ofmt_ctx;    // 输出格式上下文
    AVCodecContext* enc_ctx;      // 编码器上下文
    AVFrame* picture;             // 编码前的图像帧
    AVFrame* input_picture;       // 输入的图像帧
    AVStream* out_stream;         // 输出流
    uint8_t* aligned_input;       // 对齐的输入数据
    int frame_width;              // 视频帧宽度
    int frame_height;             // 视频帧高度
    int frame_idx;                // 帧计数

    // 查找硬件视频编码器
    AVCodec* find_hw_video_encoder(int codecId)
    {
        AVCodec* encoder = NULL;
        switch (codecId)
        {
        case AV_CODEC_ID_H264:
            encoder = avcodec_find_encoder_by_name("h264_bm");
            break;
        case AV_CODEC_ID_H265:
            encoder = avcodec_find_encoder_by_name("h265_bm");
            break;
        default:
            break;
        }
        return encoder;
    }
};

#endif // VIDEOENC_FFMPEG_H
