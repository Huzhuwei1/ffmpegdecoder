#include <jni.h>
#include <string>
#include <android/log.h>


const char *url;
jmethodID jmid_dec_callback;
jobject jobj;

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"



#define  LOG_TAG    "ffmpegdecoder"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

JNIEXPORT jstring JNICALL
Java_com_hzw_ffmpeg_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT void JNICALL
Java_com_hzw_ffmpeg_MainActivity_init(JNIEnv *env, jobject obj, jstring jurl) {
    url = env->GetStringUTFChars(jurl, 0);
    jobj = env->NewGlobalRef(obj);
    jclass  jlz = env->GetObjectClass(jobj);
    if(!jlz) {
        LOGD("find jclass faild");
        return ;
    }
    jmid_dec_callback = env->GetMethodID(jlz, "decCallBack", "(II[B[B[B)V");
}



JNIEXPORT void JNICALL
Java_com_hzw_ffmpeg_MainActivity_start(JNIEnv *env, jobject obj) {
    //1.注册所有组件
    av_register_all();

    //封装格式上下文，统领全局的结构体，保存了视频文件封装格式的相关信息
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        LOGD("%s","无法打开输入视频文件");
        return ;
    }

    //3.获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx,NULL) < 0) {
        LOGD("%s","无法获取视频文件信息");
        return ;
    }

    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    int v_stream_idx = -1;
    int i = 0;
    //number of streams
    for (; i < pFormatCtx->nb_streams; i++) {
        //流的类型
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            v_stream_idx = i;
            break;
        }
    }

    if (v_stream_idx == -1) {
        LOGD("%s","找不到视频流\n");
        return ;
    }

    //只有知道视频的编码方式，才能够根据编码方式去找到解码器
    //获取视频流中的编解码上下文
    AVCodecContext *pCodecCtx = pFormatCtx->streams[v_stream_idx]->codec;
    //4.根据编解码上下文中的编码id查找对应的解码
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGD("%s","找不到解码器\n");
        return ;
    }

    //5.打开解码器
    if (avcodec_open2(pCodecCtx,pCodec,NULL)<0) {
        LOGD("%s","解码器无法打开\n");
        return ;
    }

    //输出视频信息
    LOGD("视频的文件格式：%s",pFormatCtx->iformat->name);
    LOGD("视频时长：%d", (pFormatCtx->duration)/1000000);
    LOGD("视频的宽高：%d,%d",pCodecCtx->width,pCodecCtx->height);
    LOGD("解码器的名称：%s",pCodec->name);

    //准备读取
    //AVPacket用于存储一帧一帧的压缩数据（H264）
    //缓冲区，开辟空间
    AVPacket *packet = (AVPacket*)av_malloc(sizeof(AVPacket));

    //AVFrame用于存储解码后的像素数据(YUV)
    //内存分配
    AVFrame *pFrame = av_frame_alloc();
    //YUV420
    AVFrame *pFrameYUV = av_frame_alloc();
    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

    //用于转码（缩放）的参数，转之前的宽高，转之后的宽高，格式等
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,
                                                pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                                SWS_BICUBIC, NULL, NULL, NULL);
    int got_picture, ret;

    int frame_count = 0;

    //6.一帧一帧的读取压缩数据
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //只要视频压缩数据（根据流的索引位置判断）
        if (packet->stream_index == v_stream_idx) {
            //7.解码一帧视频压缩数据，得到视频像素数据
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                LOGD("%s","解码错误");
                return ;
            }

            //为0说明解码完成，非0正在解码
            if (got_picture) {
                //AVFrame转为像素格式YUV420，宽高
                //2 6输入、输出数据
                //3 7输入、输出画面一行的数据的大小 AVFrame 转换是一行一行转换的
                //4 输入数据第一列要转码的位置 从0开始
                //5 输入画面的高度
                sws_scale(sws_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                //将YUV数据回调给java层
                //data解码后的图像像素数据（音频采样数据）
                //Y 亮度 UV 色度（压缩了） 人对亮度更加敏感
                //U V 个数是Y的1/4
                int width = pFrame -> width;
                int height = pFrame -> height;

                jbyteArray y = env->NewByteArray(width * height);
                env->SetByteArrayRegion(y, 0, width * height, (jbyte*)pFrame->data[0]);

                jbyteArray u = env->NewByteArray(width * height / 4);
                env->SetByteArrayRegion(u, 0, width * height / 4, (jbyte*)pFrame->data[1]);

                jbyteArray v = env->NewByteArray(width * height / 4);
                env->SetByteArrayRegion(v, 0, width * height / 4, (jbyte*)pFrame->data[2]);

                env->CallVoidMethod(jobj, jmid_dec_callback, width, height, y, u, v);
                env->DeleteLocalRef(y);
                env->DeleteLocalRef(u);
                env->DeleteLocalRef(v);

                frame_count++;
                LOGD("解码第%d帧\n",frame_count);
            }
        }

        //释放资源
        av_free_packet(packet);
    }

    av_frame_free(&pFrame);

    avcodec_close(pCodecCtx);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
}

}