/*
 * *************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * *************************************************************************
 *
 * *
 *  @file     options.c
 *  @author   huxiaolong - kuper@aproton.tech
 *  @data     2022-10-18 10:45:10
 *  @version  0
 *  @brief
 *
 * *
 */

#include "mqtt_client.h"

#define GET_LONG_OPTION(dest, source, name, must_exists)                       \
  {                                                                            \
    HashTable *arr_hash = Z_ARRVAL_P(source);                                  \
    zval *val = zend_hash_str_find(arr_hash, #name, sizeof(#name) - 1);        \
    if (val != NULL) {                                                         \
      dest = Z_LVAL_P(val);                                                    \
    } else if (must_exists) {                                                  \
      PLOG_WARN("key %s not found in option", #name);                          \
      return -1;                                                               \
    }                                                                          \
  }

#define GET_STRING_OPTION(dest, source, name, must_exists)                     \
  {                                                                            \
    HashTable *arr_hash = Z_ARRVAL_P(source);                                  \
    zval *val = zend_hash_str_find(arr_hash, #name, sizeof(#name) - 1);        \
    if (val != NULL) {                                                         \
      dest = ZSTR_VAL(Z_STR_P(val));                                           \
    } else if (must_exists) {                                                  \
      PLOG_WARN("key %s not found in option", #name);                          \
      return -1;                                                               \
    }                                                                          \
  }

int parse_mqttclient_options(zval *options,
                             proton_mqttclient_connect_options_t *output) {
  if (output != NULL) {
    memset(output, 0, sizeof(proton_mqttclient_connect_options_t));
    GET_STRING_OPTION(output->client_id, options, clientId, 1);
    GET_STRING_OPTION(output->user_name, options, userName, 0);
    GET_STRING_OPTION(output->password, options, password, 1);
    GET_STRING_OPTION(output->will_topic, options, willTopic, 0);
    GET_LONG_OPTION(output->keep_alive_interval, options, keepAliveInterval, 1);
    GET_LONG_OPTION(output->clean_session, options, cleanSession, 0);
    GET_LONG_OPTION(output->reliable, options, reliable, 0);

    return 0;
  }

  return -1;
}