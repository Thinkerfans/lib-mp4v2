//
//  mp4record.c
//  RTSP_Player
//
//  Created by apple on 15/4/7.
//  Copyright (c) 2015年 thinker. All rights reserved.
//


#include "mp4record.h"
#include <stdlib.h>


typedef struct MP4V2_CONTEXT{
    
    int m_vWidth,m_vHeight,m_vFrateR,m_vTimeScale;
    MP4FileHandle m_mp4FHandle;
    MP4TrackId m_vTrackId,m_aTrackId;
    double m_vFrameDur;
    
} MP4V2_CONTEXT;

struct MP4V2_CONTEXT * recordCtx = NULL;


int initMp4Encoder(const char * filename,int width,int height){
    
    int ret = -1;
    recordCtx = malloc(sizeof(struct MP4V2_CONTEXT));
    if (!recordCtx) {
        printf("error : malloc context \n");
        return ret;
    }
    
    recordCtx->m_vWidth = width;
    recordCtx->m_vHeight = height;
    recordCtx->m_vFrateR = 25;
    recordCtx->m_vTimeScale = 90000;
    recordCtx->m_vFrameDur = 300;
    recordCtx->m_vTrackId = 0;
    recordCtx->m_aTrackId = 0;
    
    recordCtx->m_mp4FHandle = MP4Create(filename,0);
    if (recordCtx->m_mp4FHandle == MP4_INVALID_FILE_HANDLE) {
        printf("error : MP4Create  \n");
        return ret;
    }
     MP4SetTimeScale(recordCtx->m_mp4FHandle, recordCtx->m_vTimeScale);
    //------------------------------------------------------------------------------------- audio track
//    recordCtx->m_aTrackId = MP4AddAudioTrack(recordCtx->m_mp4FHandle, 44100, 1024, MP4_MPEG4_AUDIO_TYPE);
//    if (recordCtx->m_aTrackId == MP4_INVALID_TRACK_ID){
//        printf("error : MP4AddAudioTrack  \n");
//        return ret;
//    }
//
//    MP4SetAudioProfileLevel(recordCtx->m_mp4FHandle, 0x2);
//    uint8_t aacConfig[2] = {18,16};
//    MP4SetTrackESConfiguration(recordCtx->m_mp4FHandle,recordCtx->m_aTrackId,aacConfig,2);
//    printf("ok  : initMp4Encoder file=%s  \n",filename);

    return 0;
}
int mp4VEncode(uint8_t * _naluData ,int _naluSize){
    
    int index = -1;
    
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x67){
        index = _NALU_SPS_;
    }
    
    if(index!=_NALU_SPS_ && recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID){
        return index;
    }
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x68){
        index = _NALU_PPS_;
    }
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x65){
        index = _NALU_I_;
    }
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x41){
        index = _NALU_P_;
    }
    //
    switch(index){
        case _NALU_SPS_:
            if(recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID){
                recordCtx->m_vTrackId = MP4AddH264VideoTrack
                (recordCtx->m_mp4FHandle,
                 recordCtx->m_vTimeScale,
                 recordCtx->m_vTimeScale / recordCtx->m_vFrateR,
                 recordCtx->m_vWidth,     // width
                 recordCtx->m_vHeight,    // height
                 _naluData[5], // sps[1] AVCProfileIndication
                 _naluData[6], // sps[2] profile_compat
                 _naluData[7], // sps[3] AVCLevelIndication
                 3);           // 4 bytes length before each NAL unit
                if (recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID)  {
                    return -1;
                }
                MP4SetVideoProfileLevel(recordCtx->m_mp4FHandle, 0x7F); //  Simple Profile @ Level 3
            }
            MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle,recordCtx->m_vTrackId,_naluData+4,_naluSize-4);
            //
            break;
        case _NALU_PPS_:
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle,recordCtx->m_vTrackId,_naluData+4,_naluSize-4);
            break;
        case _NALU_I_:
        {
            uint8_t * IFrameData = malloc(_naluSize+1);
            //
            IFrameData[0] = (_naluSize-3) >>24;
            IFrameData[1] = (_naluSize-3) >>16;
            IFrameData[2] = (_naluSize-3) >>8;
            IFrameData[3] = (_naluSize-3) &0xff;
    
            memcpy(IFrameData+4,_naluData+3,_naluSize-3);
//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, recordCtx->m_vFrameDur/44100*90000, 0, 1)){
//                return -1;
//            }
//            recordCtx->m_vFrameDur = 0;
            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, MP4_INVALID_DURATION, 0, 1)){
                return -1;
            }
            free(IFrameData);
            //
            break;
        }
        case _NALU_P_:
        {
            _naluData[0] = (_naluSize-4) >>24;  
            _naluData[1] = (_naluSize-4) >>16;  
            _naluData[2] = (_naluSize-4) >>8;  
            _naluData[3] = (_naluSize-4) &0xff;
            
//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, recordCtx->m_vFrameDur/44100*90000, 0, 1)){
//                return -1;
//            }
//            recordCtx->m_vFrameDur = 0;
            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, MP4_INVALID_DURATION, 0, 1)){
                return -1;
            }
            break;
        }
    }
    return 0;
}


int mp4AEncode(uint8_t * data ,int len){
    if(recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID){
        return -1;
    }
    MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_aTrackId, data, len , MP4_INVALID_DURATION, 0, 1);
    recordCtx->m_vFrameDur += 1024;
    return 0;
}

void closeMp4Encoder(){
    if(recordCtx){
        if (recordCtx->m_mp4FHandle != MP4_INVALID_FILE_HANDLE) {
            MP4Close(recordCtx->m_mp4FHandle,0);
            recordCtx->m_mp4FHandle = NULL;
        }
        
        free(recordCtx);
        recordCtx = NULL;
    }
    
    printf("ok  : closeMp4Encoder  \n");

}
