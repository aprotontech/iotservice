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

  event->topicName = (char *)(&event[1]);
  event->topicName[0] = '\0';
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
  /*uv_mqtt_client_t* mqtt = (uv_mqtt_client_t*)context;
  mqtt_job_t* job = (mqtt_job_t*)emalloc(sizeof(mqtt_job_t));
  job->type = 0;
  job->topicName = topicName;
  job->topicLength = topicLen == 0 ? strlen(topicName) : topicLen;
  job->message = message;
  UV_LOGGER("mqtt(%p) recv topic(%s), topiclen(%d)\n", mqtt, topicName,
  topicLen);

  pthread_mutex_lock(&mqtt->mobject);
  job->next = mqtt->head.next;
  mqtt->head.next = job;
  pthread_mutex_unlock(&mqtt->mobject);
  uv_async_send(&mqtt->notify);*/

  return 1;
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
  /*UV_LOGGER("Message with token value %d delivery confirmed\n", dt);
  uv_mqtt_client_t* mqtt = (uv_mqtt_client_t*)context;
  mqtt_job_t* job = (mqtt_job_t*)emalloc(sizeof(mqtt_job_t));
  job->type = 1;
  job->topicLength = dt;

  pthread_mutex_lock(&mqtt->mobject);
  job->next = mqtt->head.next;
  mqtt->head.next = job;
  pthread_mutex_unlock(&mqtt->mobject);
  uv_async_send(&mqtt->notify);*/
}

void mqtt_notify_to_php(uv_async_t *req) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->data;
  int max_count = 100;
  list_link_t *p = mqtt->msg_head.next;
  for (int i = 0; i < max_count && p != &mqtt->msg_head; ++i) {
    mqtt_event_t *event = container_of(p, mqtt_event_t, link);
    p = LL_remove(p);
    if (event->type >= 0 && event->type < PME_EVENT_COUNT &&
        mqtt->events[event->type] != NULL) {
      mqttclient_on_event callback = mqtt->events[event->type];
      MQTTClient_message *msg = event->message;
      callback(event->type, event->topicName, (const char *)msg->payload,
               msg->msgid, msg->qos, msg->retained, msg->dup);
    }
    MQTTClient_freeMessage(&event->message);
    free(event);
  }
}