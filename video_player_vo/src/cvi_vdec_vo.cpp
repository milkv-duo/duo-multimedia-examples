#include "cvi_vdec_vo.h"

#include "log.h"
#include "util.h"

CviVdecVO::CviVdecVO() {
    for (int i = 0; i < VPSS_MAX_PHY_CHN_NUM; ++i) {
        abChnEnable[i] = 0;
    }
    for (int i = 0; i < MAX_VDEC_NUM; ++i) {
        g_ahLocalPicVbPool[i] = VB_INVALID_POOLID;
    }
}

CviVdecVO::~CviVdecVO() {

    for (int i = 0; i < stVdecCfg.s32ChnNum; i++) {
        stop_vdec(pVdecChn[i]);
    }
    SAMPLE_COMM_VO_StopVO(&stVoConfig);
    SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);

    vdec_exit_vb_pool();

    SAMPLE_COMM_SYS_Exit();
}

CVI_S32 CviVdecVO::vdec_init_vb_pool(VDEC_CHN ChnIndex,
                                     SAMPLE_VDEC_ATTR *pastSampleVdec,
                                     CVI_BOOL is_user) {
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U32 u32BlkSize;
    VB_CONFIG_S stVbConf;

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 1;

    u32BlkSize = VDEC_GetPicBufferSize(
        pastSampleVdec->enType, pastSampleVdec->u32Width,
        pastSampleVdec->u32Height, pastSampleVdec->enPixelFormat,
        DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = pastSampleVdec->u32FrameBufCnt;
    stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_NONE;
    INFO("VDec Init Pool[VdecChn_{0}], u32BlkSize = {1}, u32BlkCnt = {2}",
         ChnIndex, stVbConf.astCommPool[0].u32BlkSize,
         stVbConf.astCommPool[0].u32BlkCnt);

    if (!is_user) {
        if (stVbConf.u32MaxPoolCnt == 0 &&
            stVbConf.astCommPool[0].u32BlkSize == 0) {
            CVI_SYS_Exit();
            s32Ret = CVI_SYS_Init();
            if (s32Ret != CVI_SUCCESS) {
                ERROR("CVI_SYS_Init, {0}", intToHex(s32Ret));
                return CVI_FAILURE;
            }
        } else {
            s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
            if (s32Ret != CVI_SUCCESS) {
                ERROR("SAMPLE_COMM_SYS_Init, {0}", intToHex(s32Ret));
                return CVI_FAILURE;
            }
        }
    } else {
        for (CVI_U32 i = 0; i < stVbConf.u32MaxPoolCnt; i++) {
            g_ahLocalPicVbPool[ChnIndex] =
                CVI_VB_CreatePool(&stVbConf.astCommPool[0]);

            if (g_ahLocalPicVbPool[ChnIndex] == VB_INVALID_POOLID) {
                ERROR("CVI_VB_CreatePool Fail!");
                return CVI_FAILURE;
            }
        }
        INFO("CVI_VB_CreatePool : {0}, u32BlkSize= {1}, u32BlkCnt= {2}",
             g_ahLocalPicVbPool[ChnIndex], stVbConf.astCommPool[0].u32BlkSize,
             stVbConf.astCommPool[0].u32BlkCnt);
    }

    return s32Ret;
}

CVI_VOID CviVdecVO::vdec_exit_vb_pool(CVI_VOID) {
    CVI_S32 i, s32Ret;
    VDEC_MOD_PARAM_S stModParam;
    VB_SOURCE_E vb_source;
    CVI_VDEC_GetModParam(&stModParam);
    vb_source = stModParam.enVdecVBSource;
    if (vb_source != VB_SOURCE_USER)
        return;

    for (i = MAX_VDEC_NUM - 1; i >= 0; i--) {
        if (g_ahLocalPicVbPool[i] != VB_INVALID_POOLID) {
            INFO("CVI_VB_DestroyPool : {0}", g_ahLocalPicVbPool[i]);

            s32Ret = CVI_VB_DestroyPool(g_ahLocalPicVbPool[i]);
            if (s32Ret != CVI_SUCCESS) {
                ERROR("CVI_VB_DestroyPool : {0} fail!", g_ahLocalPicVbPool[i]);
            }

            g_ahLocalPicVbPool[i] = VB_INVALID_POOLID;
        }
    }
}

int32_t CviVdecVO::start_vdec(SAMPLE_VDEC_PARAM_S *pVdecParam) {
    VDEC_CHN_PARAM_S stChnParam;
    VDEC_MOD_PARAM_S stModParam;
    CVI_S32 s32Ret = CVI_SUCCESS;
    VDEC_CHN VdecChn = pVdecParam->VdecChn;

    INFO("VdecChn = {0}", VdecChn);

    CVI_VDEC_GetModParam(&stModParam);
    stModParam.enVdecVBSource = pVdecParam->vdec_vb_source;
    CVI_VDEC_SetModParam(&stModParam);

    s32Ret = CVI_VDEC_CreateChn(VdecChn, &pVdecParam->stChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("CVI_VDEC_CreateChn chn {0} failed for {1}!", pVdecParam->VdecChn,
              intToHex(s32Ret));
        return s32Ret;
    }

    if (pVdecParam->vdec_vb_source == VB_SOURCE_USER) {
        VDEC_CHN_POOL_S stPool;

        stPool.hPicVbPool = g_ahLocalPicVbPool[VdecChn];
        stPool.hTmvVbPool = VB_INVALID_POOLID;

        s32Ret = CVI_VDEC_AttachVbPool(VdecChn, &stPool);
        if (s32Ret != CVI_SUCCESS) {
            ERROR("CVI_VDEC_AttachVbPool chn {0} failed for {1} !",
                  pVdecParam->VdecChn, intToHex(s32Ret));
            return s32Ret;
        }
    }

    s32Ret = CVI_VDEC_GetChnParam(VdecChn, &stChnParam);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("CVI_VDEC_GetChnParam chn {0} failed for {1} !",
              pVdecParam->VdecChn, intToHex(s32Ret));
        return s32Ret;
    }
    stChnParam.enPixelFormat = pVdecParam->vdec_pixel_format;
    stChnParam.u32DisplayFrameNum = (pVdecParam->stChnAttr.enType == PT_JPEG ||
                                     pVdecParam->stChnAttr.enType == PT_MJPEG)
                                        ? 0
                                        : 2;
    s32Ret = CVI_VDEC_SetChnParam(VdecChn, &stChnParam);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("CVI_VDEC_SetChnParam chn {0} failed for {1}!",
              pVdecParam->VdecChn, intToHex(s32Ret));
        return s32Ret;
    }

    s32Ret = CVI_VDEC_StartRecvStream(VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("CVI_VDEC_StartRecvStream chn {0} failed for {1}!",
              pVdecParam->VdecChn, intToHex(s32Ret));
        return s32Ret;
    }

    return CVI_SUCCESS;
}

CVI_VOID CviVdecVO::stop_vdec(SAMPLE_VDEC_PARAM_S *pVdecParam) {
    CVI_S32 s32Ret = CVI_SUCCESS;

    s32Ret = CVI_VDEC_StopRecvStream(pVdecParam->VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("CVI_VDEC_StopRecvStream chn {0} failed for {1}!",
              pVdecParam->VdecChn, intToHex(s32Ret));
    }

    if (pVdecParam->vdec_vb_source == VB_SOURCE_USER) {
        INFO("detach in user mode");
        s32Ret = CVI_VDEC_DetachVbPool(pVdecParam->VdecChn);
        if (s32Ret != CVI_SUCCESS) {
            ERROR("CVI_VDEC_DetachVbPool chn {0} failed for {1}!",
                  pVdecParam->VdecChn, intToHex(s32Ret));
        }
    }

    s32Ret = CVI_VDEC_ResetChn(pVdecParam->VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("CVI_VDEC_ResetChn chn {0} failed for {1}!", pVdecParam->VdecChn,
              intToHex(s32Ret));
    }

    s32Ret = CVI_VDEC_DestroyChn(pVdecParam->VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("CVI_VDEC_DestroyChn chn {0} failed for {1} !",
              pVdecParam->VdecChn, intToHex(s32Ret));
    }
}

int CviVdecVO::init_cviSdk() {

    COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_NONE;
    VB_CONFIG_S stVbConf;
    CVI_U32 u32BlkSize;
    SIZE_S stSize;
    CVI_S32 s32Ret = CVI_SUCCESS;

    stSize.u32Width = (VDEC_WIDTH < VPSS_WIDTH) ? VPSS_WIDTH : VDEC_WIDTH;
    stSize.u32Height = (VDEC_HEIGHT < VPSS_HEIGHT) ? VPSS_HEIGHT : VDEC_HEIGHT;

    /************************************************
     * step1:  Init SYS and common VB
     ************************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 1;

    u32BlkSize = COMMON_GetPicBufferSize(
        stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_YUV_PLANAR_420,
        DATA_BITWIDTH_8, enCompressMode, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 5;
    stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_NONE;
    INFO("common pool[0] BlkSize {0}", u32BlkSize);

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("system init failed with {0}", intToHex(s32Ret));
        return -1;
    }

    /************************************************
     * step2:  Init VPSS
     ************************************************/
    VpssGrp = 0;
    VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
    VpssChn = VPSS_CHN0;
    // CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
    stVpssGrpAttr.u32MaxW = VDEC_WIDTH;
    stVpssGrpAttr.u32MaxH = VDEC_HEIGHT;
    stVpssGrpAttr.u8VpssDev = 0;

    astVpssChnAttr[VpssChn].u32Width = VPSS_WIDTH;
    astVpssChnAttr[VpssChn].u32Height = VPSS_HEIGHT;
    astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
    astVpssChnAttr[VpssChn].enPixelFormat = SAMPLE_PIXEL_FORMAT;
    astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
    astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
    astVpssChnAttr[VpssChn].u32Depth = 1;
    astVpssChnAttr[VpssChn].bMirror = CVI_FALSE;
    astVpssChnAttr[VpssChn].bFlip = CVI_FALSE;
    astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_NONE;

    abChnEnable[0] = CVI_TRUE;
    s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr,
                                   astVpssChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("init vpss group failed. s32Ret: {0} !", intToHex(s32Ret));
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr,
                                    astVpssChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("start vpss group failed. s32Ret: {0} !", intToHex(s32Ret));
        return s32Ret;
    }

    /************************************************
     * step3:  Init VO
     ************************************************/
    RECT_S stDefDispRect = {0, 0, VO_WIDTH, VO_HEIGHT};
    SIZE_S stDefImageSize = {VO_WIDTH, VO_HEIGHT};
    VoDev = 0;
    VoChn = 0;

    s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("SAMPLE_COMM_VO_GetDefConfig failed with {0} !",
              intToHex(s32Ret));
        return s32Ret;
    }

    stVoConfig.VoDev = VoDev;
    stVoConfig.stVoPubAttr.enIntfType = VO_INTF_MIPI;
    stVoConfig.stVoPubAttr.enIntfSync = VO_OUTPUT_720x1280_60;
    stVoConfig.stDispRect = stDefDispRect;
    stVoConfig.stImageSize = stDefImageSize;
    stVoConfig.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stVoConfig.enVoMode = VO_MODE_1MUX;

    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
    if (s32Ret != CVI_SUCCESS) {
        ERROR("SAMPLE_COMM_VO_StartVO failed with {0} !", intToHex(s32Ret));
        return s32Ret;
    }

    // CVI_VO_SetChnRotation(VoDev, VoChn, ROTATION_90);

    /************************************************
     * step4:  Init VDEC
     ************************************************/
    stVdecCfg.s32ChnNum = 1;

    for (int i = 0; i < stVdecCfg.s32ChnNum; i++) {
        pVdecChn[i] = &stVdecCfg.astVdecParam[i];
        pVdecChn[i]->VdecChn = i;
        pVdecChn[i]->stop_thread = CVI_FALSE;
        pVdecChn[i]->bind_mode = VDEC_BIND_VPSS_VO;
        pVdecChn[i]->stChnAttr.enType = PT_H264;
        pVdecChn[i]->stChnAttr.enMode = VIDEO_MODE_FRAME;
        pVdecChn[i]->stChnAttr.u32PicWidth = VDEC_WIDTH;
        pVdecChn[i]->stChnAttr.u32PicHeight = VDEC_HEIGHT;
        pVdecChn[i]->stChnAttr.u32StreamBufSize = VDEC_WIDTH * VDEC_HEIGHT;
        pVdecChn[i]->stChnAttr.u32FrameBufCnt = 1;
        if (pVdecChn[i]->stChnAttr.enType == PT_JPEG ||
            pVdecChn[i]->stChnAttr.enType == PT_MJPEG) {
            pVdecChn[i]->stChnAttr.u32FrameBufSize =
                VDEC_GetPicBufferSize(pVdecChn[i]->stChnAttr.enType,
                                      pVdecChn[i]->stChnAttr.u32PicWidth,
                                      pVdecChn[i]->stChnAttr.u32PicHeight,
                                      PIXEL_FORMAT_YUV_PLANAR_444,
                                      DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
        }
        pVdecChn[i]->stDispRect.s32X =
            (VPSS_WIDTH >> (stVdecCfg.s32ChnNum - 1)) * i;
        pVdecChn[i]->stDispRect.s32Y = 0;
        pVdecChn[i]->stDispRect.u32Width =
            (VPSS_WIDTH >> (stVdecCfg.s32ChnNum - 1));
        pVdecChn[i]->stDispRect.u32Height = 720;
        pVdecChn[i]->vdec_vb_source = VB_SOURCE_USER;
        pVdecChn[i]->vdec_pixel_format = PIXEL_FORMAT_YUV_PLANAR_420;
    }

    ////////////////////////////////////////////////////
    // init VB(for VDEC)
    ////////////////////////////////////////////////////
    SAMPLE_VDEC_ATTR astSampleVdec[VDEC_MAX_CHN_NUM];

    for (int i = 0; i < stVdecCfg.s32ChnNum; i++) {
        astSampleVdec[i].enType = pVdecChn[i]->stChnAttr.enType;
        astSampleVdec[i].u32Width = VDEC_WIDTH;
        astSampleVdec[i].u32Height = VDEC_HEIGHT;

        astSampleVdec[i].enMode = VIDEO_MODE_FRAME;
        astSampleVdec[i].stSampleVdecVideo.enDecMode = VIDEO_DEC_MODE_IP;
        astSampleVdec[i].stSampleVdecVideo.enBitWidth = DATA_BITWIDTH_8;
        astSampleVdec[i].stSampleVdecVideo.u32RefFrameNum = 2;
        astSampleVdec[i].u32DisplayFrameNum =
            (astSampleVdec[i].enType == PT_JPEG ||
             astSampleVdec[i].enType == PT_MJPEG)
                ? 0
                : 2;
        astSampleVdec[i].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
        astSampleVdec[i].u32FrameBufCnt = (astSampleVdec[i].enType == PT_JPEG ||
                                           astSampleVdec[i].enType == PT_MJPEG)
                                              ? 1
                                              : vbMaxFrmNum;

        s32Ret = vdec_init_vb_pool(i, &astSampleVdec[i], CVI_TRUE);
        if (s32Ret != CVI_SUCCESS) {
            ERROR("SAMPLE_COMM_VDEC_InitVBPool fail {0}!", intToHex(s32Ret));
        }
    }

    for (int i = 0; i < stVdecCfg.s32ChnNum; i++) {
        start_vdec(pVdecChn[i]);
    }

    return 0;
}

int32_t CviVdecVO::sendStreamToVenc(const AVPacket *pkt, CVI_S32 s32MilliSec) {

    CVI_S32 s32Ret = CVI_SUCCESS;
    VDEC_STREAM_S stStream = {0};
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));

    stStream.u64PTS = (CVI_U64)pkt->pts;
    stStream.pu8Addr = (CVI_U8 *)(pkt->data);
    stStream.u32Len = (CVI_U32)pkt->size;
    stStream.bEndOfFrame = CVI_TRUE;
    stStream.bEndOfStream = (pkt->pts == -1) ? CVI_TRUE : CVI_FALSE;
    stStream.bDisplay = 1;

    for (int attempt = 1; attempt <= 3; ++attempt) {
        s32Ret = CVI_VDEC_SendStream(pVdecChn[0]->VdecChn, &stStream, -1);
        if (s32Ret == CVI_SUCCESS) {
            break;
        } else if (s32Ret == CVI_ERR_VDEC_BUSY) {
            WARN("vdec send stream is busying, try again...");
        } else if (s32Ret == CVI_ERR_VDEC_BUF_FULL) {
            WARN("vdec buffer is full, try again...");
        } else if (s32Ret != CVI_SUCCESS) {
            ERROR("Could not send stream! {0}", intToHex(s32Ret));
        }
    }

    return 0;
}

void CviVdecVO::run() {
    CVI_S32 s32Ret = CVI_SUCCESS;
    VIDEO_FRAME_INFO_S stVdecFrame = {0};
    VIDEO_FRAME_INFO_S stOverlayFrame = {0};
    SAMPLE_VDEC_PARAM_S *pstVdecChn = pVdecChn[0];

    do {

        s32Ret = CVI_VDEC_GetFrame(pstVdecChn->VdecChn, &stVdecFrame, 1000);

        if (s32Ret != CVI_SUCCESS) {
            if (s32Ret == CVI_ERR_VDEC_BUSY ||
                s32Ret == CVI_ERR_VDEC_ERR_INVALID_RET) {
                WARN("vdec busy retry...");
                continue;
            }
        }

        s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVdecFrame, 1000);

        if (s32Ret != CVI_SUCCESS) {
            CVI_VDEC_ReleaseFrame(pstVdecChn->VdecChn, &stVdecFrame);
            continue;
        }

        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stOverlayFrame, 1000);
        CVI_VDEC_ReleaseFrame(pstVdecChn->VdecChn, &stVdecFrame);
        if (s32Ret != CVI_SUCCESS) {
            CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame fail, grp:%d\n",
                          VpssGrp);
            continue;
        }

        s32Ret = CVI_VO_SendFrame(VoDev, VoChn, &stOverlayFrame, 1000);
        if (s32Ret != CVI_SUCCESS) {
            ERROR("CVI_VO_SendFrame fail {0}", intToHex(s32Ret));
        }

        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stOverlayFrame);
        if (s32Ret != CVI_SUCCESS) {
            ERROR("CVI_VPSS_ReleaseChnFrame fail {0}", intToHex(s32Ret));
        }
    } while (false);
}