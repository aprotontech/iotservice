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

int reset_mqtt_read_buffer(proton_mqtt_client_t *mqtt, int new_length) {
  if (mqtt->client.recv_buffer.mem_size <= MQTT_MESSAGE_MAX_LENGTH &&
      new_length > mqtt->client.recv_buffer.mem_size) {
    char *new_ptr = (char *)qmalloc(new_length);
    if (new_ptr != NULL) {
      memmove(new_ptr, mqtt->client.recv_buffer.mem_start,
              mqtt->client.recv_buffer.mem_size);

      qfree(mqtt->client.recv_buffer.mem_start);

      mqtt->client.recv_buffer.curr =
          new_ptr + mqtt->client.recv_buffer.curr_sz;
      mqtt->client.recv_buffer.curr_sz +=
          new_length - mqtt->client.recv_buffer.mem_size;

      mqtt->client.recv_buffer.mem_start = new_ptr;
      mqtt->client.recv_buffer.mem_size = new_length;

      return 0;
    }
  }

  return -1;
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
  if (IS_REAL_COROUTINE(RUNTIME_CURRENT_COROUTINE(mqtt->runtime))) {
    proton_write_t write;
    write.writer.data = &write;
    PROTON_WAIT_OBJECT_INIT(write.wq_write);
    uv_buf_t buff = uv_buf_init((char *)buf, len);
    int rc = uv_write(&write.writer, (uv_stream_t *)&mqtt->tcp, &buff, 1,
                      mqttclient_on_write);
    if (rc == 0) {
      proton_coroutine_waitfor(mqtt->runtime, &write.wq_write, NULL);
    } else { // write failed
      PLOG_WARN("[MQTT] send data failed with %d, err=%s", rc, uv_err_name(rc));
      _mqttclient_close_all(mqtt);
      return 0;
    }
  } else { // on main coroution, can't wait send result
    proton_once_write_t *ptr =
        (proton_once_write_t *)malloc(sizeof(proton_once_write_t) + len);
    ptr->write.data = ptr;
    ptr->buf.base = (char *)&ptr[1];
    ptr->buf.len = len;
    memcpy(ptr->buf.base, buf, len);
    uv_write(&ptr->write, (uv_stream_t *)&mqtt->tcp, &ptr->buf, 1,
             mqttclient_on_once_write);
  }

  return len;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void *buf, size_t bufsz,
                         int flags) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)fd;
  int nread = 0;
  if (mqtt->cur_recved_size > 0) {
    nread = mqtt->cur_recved_size;
    mqtt->cur_recved_size = 0;
  }
  return nread;
}

void mqttclient_on_closed(uv_handle_t *handle) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)handle->data;
  PLOG_DEBUG("mqtt(%p) client closed", mqtt);

  mqtt->status = MQTT_CLIENT_DISCONNECTED;

  // cancel wait msg queue
  proton_coroutine_cancel(mqtt->runtime, &mqtt->wq_callback, NULL);
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
  *buf = uv_buf_init(mqtt->client.recv_buffer.curr,
                     mqtt->client.recv_buffer.curr_sz);
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

  // used in mqtt_pal_recvall
  mqtt->cur_recved_size = nread;

  int ret = mqtt_sync(&mqtt->client);
  if (ret == MQTT_ERROR_RECV_BUFFER_TOO_SMALL) {
    // no more message and buffer is too small
    if (reset_mqtt_read_buffer(mqtt, mqtt->client.recv_buffer.mem_size * 2) !=
        0) {
      PLOG_WARN("message is too long, can't hold it. current buffer=%lu. alloc "
                "new buffer failed",
                mqtt->client.recv_buffer.mem_size);
      _mqttclient_close_all(mqtt);
    }
  }
}

void mqttclient_on_tick(uv_timer_t *timer) {
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)timer->data;

  // send buffered data and ping
  mqtt->cur_recved_size = 0;
  mqtt_sync(&mqtt->client);
}

void publish_callback(void **ctx, struct mqtt_response_publish *published) {
  // send message to queue
  proton_mqtt_client_t *mqtt = (proton_mqtt_client_t *)*ctx;
  if (mqtt->is_looping) {

    proton_mqttclient_callback_t *cbmsg =
        (proton_mqttclient_callback_t *)qmalloc(
            sizeof(proton_mqttclient_callback_t));
    cbmsg->type = 1;
    // build message object(array)
    ZVAL_NEW_ARR(&cbmsg->message);
    zend_hash_init(Z_ARRVAL(cbmsg->message), 10, NULL, ZVAL_PTR_DTOR, 0);

    zval content;
    ZVAL_STRINGL(&content, published->application_message,
                 published->application_message_size);

    zend_hash_add(Z_ARRVAL(cbmsg->message),
                  zend_string_init("content", sizeof("content") - 1, 0),
                  &content);

    zval topic;
    ZVAL_STRINGL(&topic, published->topic_name, published->topic_name_size);
    zend_hash_add(Z_ARRVAL(cbmsg->message),
                  zend_string_init("topic", sizeof("topic") - 1, 0), &topic);

    LL_insert(&cbmsg->link, mqtt->msg_head.prev);

    proton_coroutine_wakeup(mqtt->runtime, &mqtt->wq_callback, NULL);

  } else {
    PLOG_WARN("mqtt(%p) is not loop, so skip the message", mqtt);
  }
}

void mqttclient_on_timer_closed(uv_handle_t *handle) {}

int _mqttclient_close_all(proton_mqtt_client_t *mqtt) {
  if (!uv_is_closing((uv_handle_t *)&mqtt->tcp)) {
    uv_close((uv_handle_t *)&mqtt->tcp, mqttclient_on_closed);
  }
  if (!uv_is_closing((uv_handle_t *)&mqtt->timer)) {
    uv_close((uv_handle_t *)&mqtt->timer, mqttclient_on_timer_closed);
  }
  return 0;
}
