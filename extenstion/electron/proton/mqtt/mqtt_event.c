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

#include "mqtt_client.h"
#include "proton/coroutine/channel.h"
/*

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
}*/

int reset_mqtt_read_buffer(proton_mqtt_client_t *mqtt, int new_length) {
  proton_buffer_t *ptr =
      (proton_buffer_t *)malloc(new_length + sizeof(proton_buffer_t) + 1);
  ptr->buff.base = (char *)&ptr[1];
  ptr->buff.len = new_length; // 1-eof '\0'
  ptr->buff.base[ptr->buff.len] = '\0';
  ptr->used = 0;
  ptr->need_free = 1;
  LL_init(&ptr->link);

  if (mqtt->read_buffer != NULL) {
    if (mqtt->read_buffer->used != 0) {
      memcpy(ptr->buff.base, mqtt->read_buffer->buff.base,
             mqtt->read_buffer->used);
      ptr->used = mqtt->read_buffer->used;
    }
    free(mqtt->read_buffer);
  }

  mqtt->read_buffer = ptr;
  return 0;
}

void mqttclient_on_write(uv_write_t *req, int status) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)req->handle->data;
  proton_write_t *pw = (proton_write_t *)req->data;
  proton_coroutine_wakeup(mqtt->runtime, &pw->wq_write, NULL);
}

void mqttclient_on_once_write(uv_write_t *req, int status) { free(req->data); }

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void *buf, size_t len,
                         int flags) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)fd;
  if (RUNTIME_CURRENT_COROUTINE(mqtt->runtime) ==
      RUNTIME_MAIN_COROUTINE(mqtt->runtime)) {
    proton_once_write_t *ptr =
        (proton_once_write_t *)malloc(sizeof(proton_once_write_t) + len);
    ptr->write.data = ptr;
    ptr->buf.base = (char *)&ptr[1];
    ptr->buf.len = len;
    memcpy(ptr->buf.base, buf, len);
    uv_write(&ptr->write, (uv_stream_t *)&mqtt->tcp, &ptr->buf, 1,
             mqttclient_on_once_write);
  } else {
    proton_write_t write;
    write.writer.data = &write;
    PROTON_WAIT_OBJECT_INIT(write.wq_write);
    uv_buf_t buff = uv_buf_init((char *)buf, len);
    int rc = uv_write(&write.writer, (uv_stream_t *)&mqtt->tcp, &buff, 1,
                      mqttclient_on_write);
    if (rc == 0) {
      proton_coroutine_waitfor(mqtt->runtime, &write.wq_write, NULL);
    }
  }

  return len;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void *buf, size_t bufsz,
                         int flags) {
  return 0;
}

void mqttclient_on_closed(uv_handle_t *handle) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)handle->data;
  PLOG_DEBUG("mqtt(%p) client closed", mqtt);

  /*
    if (mqtt->status_channel != NULL) {
      zval message;
      ZVAL_STRINGL(&message, "closed", sizeof("closed") - 1);
      proton_channel_push(mqtt->status_channel, &message);
      Z_TRY_DELREF(message);
    }*/
}

void mqttclient_on_connected(uv_connect_t *req, int status) {
  PLOG_DEBUG("connect status=%d", status);
  proton_connect_t *pc = (proton_connect_t *)req->data;
  pc->status = status;

  proton_coroutine_wakeup(UV_HANDLE_RUNTIME(req->handle), &pc->wq_connect,
                          NULL);
}

void mqttclient_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                             uv_buf_t *buf) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)handle->data;
  *buf = uv_buf_init(mqtt->read_buffer->buff.base + mqtt->read_buffer->used,
                     mqtt->read_buffer->buff.len - mqtt->read_buffer->used);
}

void mqttclient_on_read(uv_stream_t *handle, ssize_t nread,
                        const uv_buf_t *buf) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)handle->data;
  PLOG_DEBUG("mqtt(%p) read(%d)", handle, (int)nread);
  if (nread < 0) { // error
    PLOG_WARN("[MQTT] read failed with %d", (int)nread);
    _mqttclient_close_all(mqtt);
    return;
  }

  mqtt->read_buffer->used += nread;

  struct mqtt_client *client = &mqtt->client;

  char *ptr = mqtt->read_buffer->buff.base;
  int total = mqtt->read_buffer->used;
  int offset = 0;

  while (client->error == MQTT_OK && offset < total) {
    struct mqtt_queued_message *msg = NULL;
    struct mqtt_response response;

    ssize_t consumed =
        mqtt_unpack_response(&response, ptr + offset, total - offset);
    PLOG_DEBUG("offset(%d),total(%d),consumed(%d)", offset, total,
               (int)consumed);

    if (consumed < 0) {
      client->error = consumed;
    } else if (consumed == 0) {
      break;
    }

    offset += consumed;

    ssize_t mqtt_recv_ret = __mqtt_handle_message(client, &response);
    switch (response.fixed_header.control_type) {
    case MQTT_CONTROL_CONNACK:
      PLOG_DEBUG("MQTT_CONTROL_CONNACK");
      proton_coroutine_wakeup(mqtt->runtime, &mqtt->wq_connack, NULL);
      break;
    case MQTT_CONTROL_PINGRESP:
      PLOG_DEBUG("MQTT_CONTROL_PINGRESP");
      break;
    }
  }

  if (offset != 0) { // handle some data
    if (total == offset) {
      mqtt->read_buffer->used = 0;
    } else {
      memmove(ptr, ptr + offset, total - offset);
      mqtt->read_buffer->used = total - offset;
      total = mqtt->read_buffer->used;
      offset = 0;
    }
  } else { // no more message and buffer is too small
    if (mqtt->read_buffer->buff.len <= MQTT_MESSAGE_MAX_LENGTH) {
      reset_mqtt_read_buffer(mqtt, mqtt->read_buffer->buff.len * 2);
    } else {
      PLOG_WARN("message is too long, can't hold it. current buffer=%d", total);
      _mqttclient_close_all(mqtt);
      return;
    }
  }
}

void mqttclient_on_tick(uv_timer_t *timer) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)timer->data;
  // send buffered data and ping
  __mqtt_send(&mqtt->client);
}

void publish_callback(void **unused, struct mqtt_response_publish *published) {}

void mqttclient_on_timer_closed(uv_handle_t *handle) {}

int _mqttclient_close_all(proton_mqtt_client_t *mqtt) {
  uv_close((uv_handle_t *)&mqtt->tcp, mqttclient_on_closed);
  uv_close((uv_handle_t *)&mqtt->timer, mqttclient_on_timer_closed);
  return 0;
}