/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     errcode.h
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-04 01:02:03
 *
 */

#ifndef _PROTON_ERROR_CODE_H_
#define _PROTON_ERROR_CODE_H_

#define RC_SUCCESS 0
#define RC_ERROR_JSON_FORMAT 100
#define RC_ERROR_NOT_IMPLEMENT 200
#define RC_ERROR_COROUTINUE_CANCELED 300
#define RC_ERROR_UNKOWN_HOST 1000
#define RC_ERROR_CREATE_SOCKET 1001

#define RC_ERROR_CRYPT_INIT 1102
#define RC_ERROR_DECRYPT 1103
#define RC_ERROR_ENCRYPT 1104

#define RC_ERROR_MQTT_SUBSCRIBE 1200
#define RC_ERROR_MQTT_PUBLISH 1201
#define RC_ERROR_MQTT_SKIP_CONN 1202
#define RC_ERROR_MQTT_CONNECT 1203

#endif