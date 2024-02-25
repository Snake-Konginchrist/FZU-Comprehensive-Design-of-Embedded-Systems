#include <iostream>
#include <cstdio>
#include "videodec_ffmpeg.h"

using namespace std;

extern char progressbar_icon[];

int main() {
    // 替换为实际的输出文件名和输入文件名
    std::string output_file = "output.yuv";
    std::string input_file = "input.mp4";

    // 打开输出文件
    FILE *fp_yuv = fopen(output_file.data(), "wb+");

    // 设置日志级别为调试级别
    av_log_set_level(AV_LOG_DEBUG);

    // 使用FFMPEG的AVFormatContext结构体读取输入文件的格式信息
    VideoDec_FFMPEG reader;
    reader.openDec(input_file.data(), 1, "h264_bm", 100, 60, 0, 0);

    AVFormatContext *pFormatCtx = reader.ifmt_ctx;
    AVPacket *packet = av_packet_alloc();
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameYUV = av_frame_alloc();

    // 打印输入文件的格式信息
    av_dump_format(pFormatCtx, 0, input_file.data(), 0);

    // 获取图像转换上下文
    SwsContext *img_convert_ctx = sws_getContext(
        reader.video_dec_ctx->width, reader.video_dec_ctx->height, reader.video_dec_ctx->pix_fmt,
        reader.video_dec_ctx->width, reader.video_dec_ctx->height, AV_PIX_FMT_YUV420P,
        SWS_BICUBIC, NULL, NULL, NULL);

    long long framecount = 0;

    // 读取一帧压缩数据
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        // 判断是否为视频流
        if (packet->stream_index == reader.video_stream_idx) {
            // 解码一帧压缩数据
            int ret = avcodec_receive_frame(reader.video_dec_ctx, pFrame);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                printf("解码错误。\n");
                return -1;
            }

            if (ret >= 0) {
                // 图像转换
                sws_scale(img_convert_ctx, (const uint8_t *const *)pFrame->data, pFrame->linesize, 0, reader.video_dec_ctx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                int y_size = reader.video_dec_ctx->width * reader.video_dec_ctx->height;

                // 将YUV数据写入文件
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);     // Y
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv); // U
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv); // V

                printf("\r已完成 %lld 帧 [%c].", ++framecount, progressbar_icon[framecount % 12]);
                fflush(stdout);
            }
        }
        av_packet_unref(packet);
    }

    // 刷新解码器
    while (1) {
        int ret = avcodec_receive_frame(reader.video_dec_ctx, pFrame);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            break;

        if (ret >= 0) {
            // 图像转换
            sws_scale(img_convert_ctx, (const uint8_t *const *)pFrame->data, pFrame->linesize, 0, reader.video_dec_ctx->height,
                      pFrameYUV->data, pFrameYUV->linesize);
            int y_size = reader.video_dec_ctx->width * reader.video_dec_ctx->height;

            // 将YUV数据写入文件
            fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);     // Y
            fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv); // U
            fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv); // V

            printf("\r已完成 %lld 帧 [%c].", ++framecount, progressbar_icon[framecount % 12]);
            fflush(stdout);
        }
    }

    // 释放资源
    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);
    cout << "总共解码 " << framecount << " 帧" << endl;
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);

    return 0;
}
