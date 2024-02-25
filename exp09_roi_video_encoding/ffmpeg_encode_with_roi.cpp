#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

// 模拟 av_frame_get_side_data 函数
AVFrameSideData* av_frame_get_side_data(void* picture, int data_type) {
    // 实际实现应在这里
    return NULL;  // 用实际实现替换
}

// 模拟 AV_CODEC_ID_H264 和 AV_CODEC_ID_H265 的值
#define AV_CODEC_ID_H264 1
// #define AV_CODEC_ID_H265 2

// 模拟 FFmpeg 函数
AVCodecContext* get_encoder_context() {
    // 实际实现应在这里
    return NULL;  // 用实际实现替换
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    
    // 模拟 AVFrame 和 picture 设置
    void* picture = malloc(sizeof(picture));  // 用实际分配替换

    // 模拟 AVCodecContext 设置
    AVCodecContext* enc_ctx = get_encoder_context();

    // 模拟 height 和 width
    int height = 480;
    int width = 640;

    // 模拟 av_frame_get_side_data
    AVFrameSideData* fside = av_frame_get_side_data(picture, AV_FRAME_DATA_BM_ROI_INFO);

    if (fside) {
        AVBMRoiInfo* roiinfo = (AVBMRoiInfo*)fside->data;
        memset(roiinfo, 0, sizeof(AVBMRoiInfo));

        if (enc_ctx->codec_id == AV_CODEC_ID_H264) {
            // H.264 编码
            roiinfo->customRoiMapEnable = 1;
            roiinfo->customModeMapEnable = 0;

            for (int i = 0; i < (height >> 4); i++) {
                for (int j = 0; j < (width >> 4); j++) {
                    // H.264 编码
                    int pos = i * (width >> 4) + j;
                    if (j >= (width >> 4) / 2 && i >= (height >> 4) / 2) {
                        roiinfo->field[pos].H264.mb_qp = 10;
                    } else {
                        roiinfo->field[pos].H264.mb_qp = 40;
                    }
                }
            }
        } else if (enc_ctx->codec_id == AV_CODEC_ID_H265) {
            // H.265 编码
            roiinfo->customRoiMapEnable = 1;
            roiinfo->customModeMapEnable = 0;
            roiinfo->customLambdaMapEnable = 0;
            roiinfo->customCoefDropEnable = 0;

            for (int i = 0; i < (height >> 6); i++) {
                for (int j = 0; j < (width >> 6); j++) {
                    // H.265 编码
                    int pos = i * (width >> 6) + j;
                    if (j > (width >> 6) / 2 && i > (height >> 6) / 2) {
                        roiinfo->field[pos].HEVC.sub_ctu_qp_0 = 10;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_1 = 10;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_2 = 10;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_3 = 10;
                    } else {
                        roiinfo->field[pos].HEVC.sub_ctu_qp_0 = 40;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_1 = 40;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_2 = 40;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_3 = 40;
                    }

                    roiinfo->field[pos].HEVC.ctu_force_mode = 0;
                    roiinfo->field[pos].HEVC.ctu_coeff_drop = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_0 = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_1 = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_2 = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_3 = 0;
                }
            }
        }
    }
    
    // 使用 FFmpeg 进行视频编码和保存
    av_register_all();
    AVFormatContext* format_ctx = avformat_alloc_context();
    avformat_alloc_output_context2(&format_ctx, NULL, NULL, output_file);
    AVStream* stream = avformat_new_stream(format_ctx, NULL);
    AVCodec* codec = avcodec_find_encoder(enc_ctx->codec_id);
    avcodec_open2(stream->codec, codec, NULL);
    avformat_write_header(format_ctx, NULL);
    
    AVPacket pkt;
    av_init_packet(&pkt);
    
    // 转换 AVFrame 到 AVPacket
    if (avcodec_receive_packet(stream->codec, &pkt) == 0) {
    	pkt.stream_index = stream->index;
    	av_write_frame(format_ctx, &pkt);
    	av_packet_unref(&pkt);
    }
    
    av_write_trailer(format_ctx);
    avcodec_close(stream->codec);
    
    // 清理
    free(picture);  // 用实际的释放替换

    return 0;
}
