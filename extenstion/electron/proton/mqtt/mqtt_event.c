/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     mqtt_event.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-02-16 11:46:47
 *
 */

#include "mqtt.h"
#include "proton/coroutine/channel.h"

typedef struct _mqtt_event_t {
  proton_mqtt_event_t type;

  char *topicName;
  int topicLength;
  MQTTClient_message *message;

  list_link_t link;
} mqtt_event_t;

void new_event(proton_mqtt_client_t *mqtt, mqtt_event_t *event) {
  pthread_mutex_lock(&mqtt->mevent);
  LL_insert(&event->link, mqtt->msg_head.prev);
  pthread_mutex_unlock(&mqtt->mevent);
  uv_async_send(&mqtt->notify);
}

void connlost(void *context, char *cause) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)context;

  int clen = cause == NULL ? 0 : strlen(cause);
  mqtt_event_t *event = (mqtt_event_t *)malloc(sizeof(mqtt_event_t) + clen + 2);

  event->type = PME_DISCONNECT;

  event->topicName = NULL;
  event->topicLength = 0;

  event->message = (MQTTClient_message *)malloc(sizeof(MQTTClient_message));
  memset(event->message, 0, sizeof(MQTTClient_message));

  ((char *)event->message->payload)[clen] = '\0';
  event->message->payloadlen = clen;
  if (clen != 0) {
    memcpy(event->message->payload, cause, clen);
  }

  new_event(mqtt, event);
}

int msgarrvd(void *context, char *topicName, int topicLen,
             MQTTClient_message *message) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)context;
  mqtt_event_t *event = (mqtt_event_t *)emalloc(sizeof(mqtt_event_t));
  event->type = 0;
  event->topicName = topicName;
  event->topicLength = 0;
  event->message = message;
  PLOG_DEBUG("mqtt(%p) recv topic(%s), topiclen(%d)\n", mqtt, topicName,
             topicLen);
  new_event(mqtt, event);

  return 1;
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
  PLOG_DEBUG("Message with token value %d delivery confirmed", dt);
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)context;
  mqtt_event_t *event = (mqtt_event_t *)emalloc(sizeof(mqtt_event_t) + 20);
  event->type = PME_MSG_DELIVERED;
  event->topicName = (char *)(&event[1]);
  event->topicLength = snprintf(event->topicName, 20, "%d", (int)dt);
  event->message = NULL;

  new_event(mqtt, event);
}

void handle_mqtt_event(proton_mqtt_client_t *mqtt, mqtt_event_t *event) {
  switch (event->type) {
  case PME_DISCONNECT:
    if (mqtt->status_channel != NULL) {
      zval message;
      ZVAL_STRINGL(&message, event->message->payload,
                   event->message->payloadlen);
      proton_channel_push(mqtt->status_channel, &message);
      Z_TRY_DELREF(message);
    }
    break;
  case PME_MSG_DELIVERED: {
    any_t item = NULL;
    proton_mqtt_publish_watcher_t *pw = NULL;
    pthread_mutex_lock(&mqtt->mwatcher);
    if (hashmap_get(mqtt->publish_watchers, event->topicName, &item) ==
        MAP_OK) {
      // found the publish
      pw = (proton_mqtt_publish_watcher_t *)item;
      // remove it
      hashmap_remove(mqtt->publish_watchers, event->topicName);
    }
    pthread_mutex_unlock(&mqtt->mwatcher);
    if (pw != NULL) {
      proton_coroutine_wakeup(mqtt->runtime, &pw->pw_publish.wq_work, NULL);
    }
  } break;
  case PME_NEW_MESSAGE:
    break;
  }
}

void mqtt_notify_to_php(uv_async_t *req) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;
  int max_count = 1000;

  for (int i = 0; i < max_count; ++i) {
    mqtt_event_t *event = NULL;
    pthread_mutex_lock(&mqtt->mevent);

    if (!LL_isspin(&mqtt->msg_head)) {
      list_link_t *p = mqtt->msg_head.next;

      event = container_of(p, mqtt_event_t, link);
      LL_remove(p);
    }

    pthread_mutex_unlock(&mqtt->mevent);

    if (event == NULL) {
      break;
    }

    handle_mqtt_event(mqtt, event);

    if (event->message != NULL) {
      MQTTClient_freeMessage(&event->message);
    }
    if (event->topicName != NULL) {
      MQTTClient_free(event->topicName);
    }
    free(event);
  }
}