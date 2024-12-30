#pragma once

#include "cvi_venc.h"
#include "sample_comm.h"

#include "defines.h"

class CviVideoEncoder {
  public:
    CviVideoEncoder();
    ~CviVideoEncoder();

  public:
    int32_t init_cviSdk(const char *codeName, VideoInfo *pvideo);
    int32_t venc_start();

    int32_t getStream(EncoderCallback *pCallback);
    int32_t sendkeyframe();
    int32_t setChnParam(uint32_t chgBitrate, int chgFramerate = 0);

  private:
    int32_t venc_stop();
    void exit_cviSdk();
    int32_t getStreamInternal(PAYLOAD_TYPE_E enType, VENC_STREAM_S *pstStream,
                              EncoderCallback *pCallback);

    int32_t _SAMPLE_COMM_VI_DefaultConfig();
    int32_t _SAMPLE_VENC_INIT_CHANNEL();

  private:
    commonInputCfg *m_pCommonIc;
    vencChnCtx *m_pChnCtx;
    VENC_CHN m_chnNum = 0;
    SAMPLE_VI_CONFIG_S m_stViConfig;
};