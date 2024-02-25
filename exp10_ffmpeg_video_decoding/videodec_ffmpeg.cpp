#include "videodec_ffmpeg.h"
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

static AVCodec* findBmDecoder(AVCodecID codec_id, const char* codec_name, int codec_name_flag, AVMediaType media_type);

// 构造函数
VideoDec_FFMPEG::VideoDec_FFMPEG() {
    ifmt_ctx = NULL;
    video_dec_ctx = NULL;
    video_dec_par = NULL;
    decoder = NULL;
    width = 0;
    height = 0;
    pix_fmt = 0;
    video_stream_idx = -1;
    refcount = 1;

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    frame = av_frame_alloc();
}

// 析构函数
VideoDec_FFMPEG::~VideoDec_FFMPEG() {
    closeDec();
    printf("#VideoDec_FFMPEG exit \n");
}

// 打开文件和解码器
int VideoDec_FFMPEG::openDec(const char *filename, int codec_name_flag, const char *coder_name,
                             int output_format_mode, int extra_frame_buffer_num, int sophon_idx, int pcie_no_copyback) {
    int ret = 0;
    AVDictionary *dict = NULL;
    av_dict_set(&dict, "rtsp_flags", "prefer_tcp", 0);

    // 打开媒体流
    ret = avformat_open_input(&ifmt_ctx, filename, NULL, &dict);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "无法打开输入文件\n");
        return ret;
    }

    // 获取媒体信息
    ret = avformat_find_stream_info(ifmt_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "无法获取流信息\n");
        return ret;
    }

    // 打开编码器（二次封装函数）
    ret = openCodecContext(&video_stream_idx, &video_dec_ctx, ifmt_ctx, AVMEDIA_TYPE_VIDEO,
                           codec_name_flag, coder_name, output_format_mode, extra_frame_buffer_num);
    if (ret >= 0) {
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
    }

    av_log(video_dec_ctx, AV_LOG_INFO, "openDec video_stream_idx = %d, pix_fmt = %d\n", video_stream_idx, pix_fmt);
    av_dict_free(&dict);
    return ret;
}

// 打开解码器上下文
int VideoDec_FFMPEG::openCodecContext(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                                      enum AVMediaType type, int codec_name_flag, const char *coder_name,
                                      int output_format_mode, int extra_frame_buffer_num, int sophon_idx, int pcie_no_copyback) {
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "找不到 %s 流\n", av_get_media_type_string(type));
        return ret;
    }

    stream_index = ret;
    st = fmt_ctx->streams[stream_index];

    // 查找流的解码器
    if (codec_name_flag && coder_name)
        decoder = findBmDecoder((AVCodecID)0, coder_name, codec_name_flag, AVMEDIA_TYPE_VIDEO);
    else
        decoder = findBmDecoder(st->codecpar->codec_id);

    if (!decoder) {
        av_log(NULL, AV_LOG_FATAL, "找不到 %s 编解码器\n", av_get_media_type_string(type));
        return AVERROR(EINVAL);
    }

    // 为解码器分配上下文
    *dec_ctx = avcodec_alloc_context3(decoder);
    if (!*dec_ctx) {
        av_log(NULL, AV_LOG_FATAL, "无法分配 %s 解码器上下文\n", av_get_media_type_string(type));
        return AVERROR(ENOMEM);
    }

    // 从输入流拷贝编码器参数到输出编码器上下文
    ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_FATAL, "无法拷贝 %s 编码器参数到解码器上下文\n", av_get_media_type_string(type));
        return ret;
    }

    video_dec_par = st->codecpar;

    // 初始化解码器，带或不带引用计数
    av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
    if (output_format_mode == 101)
        av_dict_set_int(&opts, "output_format", output_format_mode, 18);
    av_dict_set_int(&opts, "extra_frame_buffer_num", extra_frame_buffer_num, 0);
    ret = av

codec_open2(*dec_ctx, dec, &opts);
    if (ret < 0) {
        av_log(NULL, AV_LOG_FATAL, "无法打开 %s 解码器\n", av_get_media_type_string(type));
        return ret;
    }

    *stream_idx = stream_index;

    av_dict_free(&opts);

    return 0;
}

// 解码一帧视频
AVFrame *VideoDec_FFMPEG::grabFrame() {
    int ret = 0;
    int got_frame = 0;
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    while (1) {
        av_packet_unref(&pkt);
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            if (ret == AVERROR(EAGAIN)) {
                gettimeofday(&tv2, NULL);
                if (((tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000) > 1000 * 60) {
                    av_log(video_dec_ctx, AV_LOG_WARNING, "av_read_frame 失败 ret(%d) 重试时间 >60s.\n", ret);
                    break;
                }
                usleep(10 * 1000);
                continue;
            }
            av_log(video_dec_ctx, AV_LOG_ERROR, "av_read_frame ret(%d) 可能是文件结束...\n", ret);
            return NULL; // TODO: 处理文件结束的情况
        }

        if (pkt.stream_index != video_stream_idx) {
            continue;
        }

        if (!frame) {
            av_log(video_dec_ctx, AV_LOG_ERROR, "无法分配帧\n");
            return NULL;
        }

        if (refcount) {
            av_frame_unref(frame);
        }

        gettimeofday(&tv1, NULL);

        ret = avcodec_decode_video2(video_dec_ctx, frame, &got_frame, &pkt);
        if (ret < 0) {
            av_log(video_dec_ctx, AV_LOG_ERROR, "解码视频帧出错 (%d)\n", ret);
            continue; // TODO: 处理解码错误
        }

        if (!got_frame) {
            continue;
        }

        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;

        if (frame->width != width || frame->height != height || frame->format != pix_fmt) {
            av_log(video_dec_ctx, AV_LOG_ERROR,
                   "错误: 在 rawvideo 文件中，宽度、高度和像素格式必须是常量的，但是输入视频的宽度、高度或像素格式发生了变化:\n"
                   "原始: 宽度 = %d, 高度 = %d, 格式 = %s\n"
                   "新的: 宽度 = %d, 高度 = %d, 格式 = %s\n",
                   width, height, av_get_pix_fmt_name((AVPixelFormat)pix_fmt),
                   frame->width, frame->height,
                   av_get_pix_fmt_name((AVPixelFormat)frame->format));
            continue;
        }

        break;
    }

    return frame;
}

// 关闭解码器和文件
void VideoDec_FFMPEG::closeDec() {
    if (video_dec_ctx) {
        avcodec_free_context(&video_dec_ctx);
        video_dec_ctx = NULL;
    }

    if (ifmt_ctx) {
        avformat_close_input(&ifmt_ctx);
        ifmt_ctx = NULL;
    }

    if (frame) {
        av_frame_free(&frame);
        frame = NULL;
    }
}
