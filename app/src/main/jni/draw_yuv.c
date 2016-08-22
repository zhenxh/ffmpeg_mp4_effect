#include <stdio.h>
#include <time.h>
#include <wchar.h>

#define __STDC_CONSTANT_MACROS

#include "libavfilter/avfilter.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"

#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)  printf("(>_<) " format "\n", ##__VA_ARGS__)
#define LOGI(format, ...)  printf("(^_^) " format "\n", ##__VA_ARGS__)
#endif

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_ffmpeg_test_cxh_MainActivity_drawyuv(JNIEnv *env, jobject obj, jobject pMainAct)
{
	int ret;
    AVFrame *frame_in;
    AVFrame *frame_out;
    unsigned char *frame_buffer_in;
    unsigned char *frame_buffer_out;

    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
    static int video_stream_index = -1;

    int in_width=640;
    int in_height=360;

    //Input YUV
    FILE *fp_in=fopen("/storage/emulated/0/old_in.yuv","rb+");
    if(fp_in==NULL){
        printf("Error open input file.\n");
        return -1;
    }

    //Output YUV
    FILE *fp_out=fopen("/storage/emulated/0/new_out.yuv","wb+");
    if(fp_out==NULL){
        printf("Error open output file.\n");
        return -1;
    }

    //const char *filter_descr = "lutyuv='u=128:v=128'";
    //const char *filter_descr = "boxblur";
    //const char *filter_descr = "hflip";
    //const char *filter_descr = "hue='h=60:s=-3'";
    //const char *filter_descr = "crop=2/3*in_w:2/3*in_h";
    const char *filter_descr = "drawbox=x=100:y=100:w=100:h=100:color=pink@0.5";
    //const char *filter_descr = "drawtext=fontfile=arial.ttf:fontcolor=green:fontsize=30:text='Lei Xiaohua'";
    //const char *filter_descr = "drawtext=fontcolor=green:fontsize=30:text='Lei Xiaohua'";
    //const char *filter_descr = "drawtext=fontfile=/system/fonts/DroidSans.ttf:fontcolor=green:fontsize=30:text='Lei Xiaohua'";
    //const char *filter_descr = "drawtext=fontfile=/storage/emulated/0/system/fonts/DroidSans.ttf:fontcolor=green:fontsize=30:text='Lei Xiaohua'";
    //const char *filter_descr = "drawtext=text='Xiaohua':x=100:y=100";
    //const char *filter_descr = "drawtext=fontfile=/system/fonts/DroidSans.ttf: text='this is awesome':x=(w-text_w)/2:y=H-60 :fontcolor=white :box=1:boxcolor=0x00000000@1";
    //const char *filter_descr = "movie=/storage/emulated/0/my_logo.png[wm];[in][wm]overlay=5:5[out]";
    //const char *filter_descr ="drawtext=fontsize=60:fontfile=/system/fonts/DroidSans.ttf:fontcolor=green:text=AAAA:x=(w-max_glyph_w)/2:y=h/2-ascent";

    avfilter_register_all();

    char args[512];
    AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
    AVBufferSinkParams *buffersink_params;

    filter_graph = avfilter_graph_alloc();

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        in_width,in_height,AV_PIX_FMT_YUV420P,
        1, 25,1,1);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
        args, NULL, filter_graph);
    if (ret < 0) {
        printf("Cannot create buffer source\n");
        return ret;
    }

    /* buffer video sink: to terminate the filter chain. */
    buffersink_params = av_buffersink_params_alloc();
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
        NULL, buffersink_params, filter_graph);
    av_free(buffersink_params);
    if (ret < 0) {
        LOGE("avfilter_graph_create_filter : %d", ret);
        return ret;
    }

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,
        &inputs, &outputs, NULL)) < 0)
    {
        LOGE("start10 avfilter_graph_parse_ptr %d" , ret);
        return ret;
    }

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        return ret;

    frame_in=av_frame_alloc();
    frame_buffer_in=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width,in_height,1));
    av_image_fill_arrays(frame_in->data, frame_in->linesize,frame_buffer_in,
        AV_PIX_FMT_YUV420P,in_width, in_height,1);

    frame_out=av_frame_alloc();
    frame_buffer_out=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width,in_height,1));
    av_image_fill_arrays(frame_out->data, frame_out->linesize,frame_buffer_out,
        AV_PIX_FMT_YUV420P,in_width, in_height,1);

    frame_in->width=in_width;
    frame_in->height=in_height;
    frame_in->format=AV_PIX_FMT_YUV420P;

    while (1) {
        if(fread(frame_buffer_in, 1, in_width*in_height*3/2, fp_in)!= in_width*in_height*3/2){
            break;
        }
        //input Y,U,V
        frame_in->data[0]=frame_buffer_in;
        frame_in->data[1]=frame_buffer_in+in_width*in_height;
        frame_in->data[2]=frame_buffer_in+in_width*in_height*5/4;

        if (av_buffersrc_add_frame(buffersrc_ctx, frame_in) < 0) {
            printf( "Error while add frame.\n");
            break;
        }

        /* pull filtered pictures from the filtergraph */
        ret = av_buffersink_get_frame(buffersink_ctx, frame_out);
        if (ret < 0)
            break;

        //output Y,U,V
        if(frame_out->format==AV_PIX_FMT_YUV420P){
            int i = 0;
            for(i=0;i<frame_out->height;i++){
                fwrite(frame_out->data[0]+frame_out->linesize[0]*i,1,frame_out->width,fp_out);
            }
            i = 0;
            for(i=0;i<frame_out->height/2;i++){
                fwrite(frame_out->data[1]+frame_out->linesize[1]*i,1,frame_out->width/2,fp_out);
            }
            i = 0;
            for(i=0;i<frame_out->height/2;i++){
                fwrite(frame_out->data[2]+frame_out->linesize[2]*i,1,frame_out->width/2,fp_out);
            }
        }
        printf("Process 1 frame!\n");
    }

    fclose(fp_in);
    fclose(fp_out);

    av_frame_free(&frame_in);
    av_frame_free(&frame_out);
    avfilter_graph_free(&filter_graph);
    return 0;
}
#ifdef __cplusplus
}
#endif


