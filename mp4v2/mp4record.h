//
//  mp4record.h
//  RTSP_Player
//
//  Created by apple on 15/4/7.
//  Copyright (c) 2015年 thinker. All rights reserved.
//

#ifndef __RTSP_Player__mp4record__
#define __RTSP_Player__mp4record__

#include "mp4v2.h"

#define  _NALU_SPS_  0
#define  _NALU_PPS_  1
#define  _NALU_I_    2
#define  _NALU_P_    3


int initMp4Encoder(const char * filename,int width,int height);
int mp4VEncode(uint8_t * data ,int len);
int mp4AEncode(uint8_t * data ,int len);
void closeMp4Encoder();


#endif /* defined(__RTSP_Player__mp4record__) */
