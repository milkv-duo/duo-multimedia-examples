#pragma once

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
}

#include "cvi_sys.h"
#include "sample_comm.h"

#define vbMaxFrmNum 8
#define MAX_VDEC_NUM 2

#define VDEC_WIDTH 1920
#define VDEC_HEIGHT 1080
#define VPSS_WIDTH 800
#define VPSS_HEIGHT 1280
#define VO_WIDTH 800
#define VO_HEIGHT 1280

typedef struct _SAMPLE_VDEC_PARAM_S {
    VDEC_CHN VdecChn;
    VDEC_CHN_ATTR_S stChnAttr;
    CVI_CHAR decode_file_name[64];
    CVI_BOOL stop_thread;
    pthread_t vdec_thread;
    RECT_S stDispRect;
    CVI_U32 bind_mode;
    VB_SOURCE_E vdec_vb_source;
    PIXEL_FORMAT_E vdec_pixel_format;
} SAMPLE_VDEC_PARAM_S;

typedef struct _SAMPLE_VDEC_CONFIG_S {
    CVI_S32 s32ChnNum;
    SAMPLE_VDEC_PARAM_S astVdecParam[MAX_VDEC_NUM];
} SAMPLE_VDEC_CONFIG_S;

class CviVdecVO {
  public:
    CviVdecVO();
    ~CviVdecVO();

    int init_cviSdk();
    int32_t sendStreamToVenc(const AVPacket *pkt, CVI_S32 s32MilliSec = -1);
    // Poco::Runnable
    // Do whatever the thread needs to do.
    void run();

  private:
    CVI_S32 vdec_init_vb_pool(VDEC_CHN ChnIndex,
                              SAMPLE_VDEC_ATTR *pastSampleVdec,
                              CVI_BOOL is_user);
    CVI_VOID vdec_exit_vb_pool(CVI_VOID);
    int32_t start_vdec(SAMPLE_VDEC_PARAM_S *pVdecParam);
    CVI_VOID stop_vdec(SAMPLE_VDEC_PARAM_S *pVdecParam);

  private:
    bool _running = true;

    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = VPSS_CHN0;
    CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM];
    SAMPLE_VO_CONFIG_S stVoConfig;
    VO_DEV VoDev = 0;
    VO_CHN VoChn = 0;

    SAMPLE_VDEC_CONFIG_S stVdecCfg;
    SAMPLE_VDEC_PARAM_S *pVdecChn[MAX_VDEC_NUM];
    VB_POOL g_ahLocalPicVbPool[MAX_VDEC_NUM];
};