#include "videoenc_ffmpeg.h"

// 构造函数
VideoEnc_FFMPEG::VideoEnc_FFMPEG()
    : ofmt_ctx(nullptr), enc_ctx(nullptr), picture(nullptr),
      input_picture(nullptr), out_stream(nullptr), aligned_input(nullptr),
      frame_width(0), frame_height(0), frame_idx(0)
{
}

// 析构函数
VideoEnc_FFMPEG::~VideoEnc_FFMPEG()
{
    closeEnc();
}

// 打开编码器
int VideoEnc_FFMPEG::openEnc(const char* filename, int soc_idx, int codecId, int framerate, int width, int height, int inputformat, int bitrate)
{
    int ret = 0;
    AVCodec* encoder;
    AVDictionary* dict = nullptr;
    frame_idx = 0;
    frame_width = width;
    frame_height = height;

    // 初始化输出格式上下文
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename);
    if (!ofmt_ctx)
    {
        av_log(nullptr, AV_LOG_ERROR, "无法创建输出上下文\n");
        return AVERROR_UNKNOWN;
    }

    // 查找硬件视频编码器
    encoder = find_hw_video_encoder(codecId);
    if (!encoder)
    {
        av_log(nullptr, AV_LOG_FATAL, "未找到硬件视频编码器\n");
        return AVERROR_INVALIDDATA;
    }

    // 分配编码器上下文
    enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx)
    {
        av_log(nullptr, AV_LOG_FATAL, "无法分配编码器上下文\n");
        return AVERROR(ENOMEM);
    }

    // 设置编码器参数
    enc_ctx->codec_id = (AVCodecID)codecId;
    enc_ctx->width = width;
    enc_ctx->height = height;
    enc_ctx->pix_fmt = (AVPixelFormat)inputformat;
    enc_ctx->bit_rate_tolerance = bitrate;
    enc_ctx->bit_rate = (int64_t)bitrate;
    enc_ctx->gop_size = 32;
    enc_ctx->time_base.num = 1;
    enc_ctx->time_base.den = framerate;
    enc_ctx->framerate.num = framerate;
    enc_ctx->framerate.den = 1;

    // 创建输出流
    out_stream = avformat_new_stream(ofmt_ctx, encoder);
    out_stream->time_base = enc_ctx->time_base;
    out_stream->avg_frame_rate = enc_ctx->framerate;
    out_stream->r_frame_rate = out_stream->avg_frame_rate;

    // 设置自定义参数
    av_dict_set_int(&dict, "sophon_idx", soc_idx, 0);
    av_dict_set_int(&dict, "gop_preset", 8, 0);
    av_dict_set_int(&dict, "is_dma_buffer", 0, 0);
    av_dict_set_int(&dict, "qp", 25, 0);

    // 打开编码器
    ret = avcodec_open2(enc_ctx, encoder, &dict);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "无法打开视频编码器\n");
        return ret;
    }

    // 将编码器参数拷贝到输出流
    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "无法拷贝编码器参数到输出流\n");
        return ret;
    }

    // 如果不是无文件格式，则打开输出文件
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            av_log(nullptr, AV_LOG_ERROR, "无法打开输出文件 '%s'\n", filename);
            return ret;
        }
    }

    // 初始化复用器，写入文件头
    ret = avformat_write_header(ofmt_ctx, nullptr);
    if (ret < 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "打开输出文件时出现错误\n");
        return ret;
    }

    // 分配编码前图像帧
    picture = av_frame_alloc();
    picture->format = enc_ctx->pix_fmt;
    picture->width = width;
    picture->height = height;

    return 0;
}

// 写入帧数据
int VideoEnc_FFMPEG::writeFrame(const uint8_t* data, int step, int width, int height)
{
    int ret = 0;
    int got_output = 0;

    // 检查步长是否与对齐要求一致
    if (step % STEP_ALIGNMENT != 0)
    {
        av_log(nullptr, AV_LOG_ERROR, "输入步长必须与 STEP_ALIGNMENT 对齐\n");
        return -1;
    }

    // 帧数增加
    frame_idx++;

    // 填充图像帧数据
    av_image_fill_arrays(picture->data, picture->linesize, (uint8_t*)data, enc_ctx->pix_fmt, width, height, 1);
    picture->linesize[0] = step;
    picture->pts = frame_idx;

    // 编码帧
    AVPacket enc_pkt;
    enc_pkt.data = nullptr;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);
    ret = avcodec_encode_video2(enc_ctx, &enc_pkt, picture, &got_output);
    if (ret < 0)
        return ret;

    if (got_output == 0)
    {
        av_log(nullptr, AV_LOG_WARNING, "编码器未输出数据\n");
        return -1;
    }

    // 重定时编码帧的时间戳
    av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base, out_stream->time_base);

    // 复用器写入编码后帧
    ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
    return ret;
}

// 关闭编码器
void VideoEnc_FFMPEG::closeEnc()
{
    // 刷新编码器
    flush_encoder();

    // 写入尾部
    av_write_trailer(ofmt_ctx);

    // 释放图像帧内存
    av_frame_free(&picture);

    // 释放输入图像帧内存
    if (input_picture)
        av_free(input_picture);

    // 释放编码器上下文
    avcodec_free_context(&enc_ctx);

    // 如果不是无文件格式，则关闭文件
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);

    // 释放输出格式上下文
    avformat_free_context(ofmt_ctx);
}

// 刷新编码器
int VideoEnc_FFMPEG::flush_encoder()
{
    int ret;
    int got_frame = 0;

    // 如果编码器不支持延迟，直接返回
    if (!(enc_ctx->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;

    // 循环刷新编码器
    while (1)
    {
        av_log(nullptr, AV_LOG_INFO, "刷新视频编码器\n");
        AVPacket enc_pkt;
        enc_pkt.data = nullptr;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);

        // 编码一帧
        ret = avcodec_encode_video2(enc_ctx, &enc_pkt, nullptr, &got_frame);
        if (ret < 0)
            return ret;

        // 如果没有帧输出，退出循环
        if (!got_frame)
            break;

        // 重定时编码帧的时间戳
        av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base, out_stream->time_base);

        // 复用器写入编码后帧
        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }

    return ret;
}
