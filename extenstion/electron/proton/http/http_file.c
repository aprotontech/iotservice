/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     http_file.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-03-05 01:16:41
 *
 */

#include "http.h"

#define CONTENT_TYPE "content-type"
#define CONTENT_TYPE_LEN (sizeof(CONTENT_TYPE) - 1)

#define UV_BUF_NOT_EMPTY(pbuff) ((pbuff)->base != NULL && (pbuff)->len != 0)

#define ZARR_HASH_ADD(arr, key_value)                                          \
  zend_hash_add(Z_ARRVAL_P(arr),                                               \
                zend_string_init(#key_value, sizeof(#key_value) - 1, 0),       \
                &(key_value))

typedef struct _http_multipart_t {
  uv_buf_t name;
  uv_buf_t filename;
  uv_buf_t content;
  uv_buf_t encode;
  uv_buf_t type;
} http_multipart_t;

int http_multipart_init(http_multipart_t *mp) {
  mp->name = uv_buf_init(NULL, 0);
  mp->filename = uv_buf_init(NULL, 0);
  mp->content = uv_buf_init(NULL, 0);
  mp->encode = uv_buf_init(NULL, 0);
  mp->type = uv_buf_init(NULL, 0);
  return 0;
}

static char *__get_maps() {
  static char _maps[256] = {0};
  if (_maps[1] == 0) {
    for (int i = 0; i < 256; ++i) {
      _maps[i] = i;
    }
    for (int i = 'A'; i <= 'Z'; ++i) {
      _maps[i] = i - 'A' + 'a';
    }
  }

  return _maps;
}

static int __hash_key(const char *str, int len) {
  char *maps = __get_maps();
  int hashkey = 0;
  for (int i = 0; i < len; ++i) {
    hashkey += maps[(unsigned char)str[i]];
  }
  return hashkey;
}

static int __content_type_key() {
  static int _key = 0;
  if (_key == 0) {
    _key = __hash_key(CONTENT_TYPE, CONTENT_TYPE_LEN);
  }
  return _key;
}

static char *_ltrim(char *s, char *t) {
  while (*s == ' ' && s < t)
    ++s;
  return s;
}

static char *_rtrim(char *s, char *t) {
  while (*t == ' ' && s < t)
    --t;
  return t;
}

int is_http_post_multipart(proton_http_message_t *message, uv_buf_t *boundary) {
  int is_post_multipart = 0;

  list_link_t *p = message->headers.next;
  while (p != &message->headers) {
    proton_header_t *header = container_of(p, proton_header_t, link);
    p = p->next;
    if (header->key.len != CONTENT_TYPE_LEN) {
      continue; // not content-type
    }

    // match content-type
    if (__hash_key(header->key.base, header->key.len) == __content_type_key() &&
        strncasecmp(header->key.base, CONTENT_TYPE, CONTENT_TYPE_LEN) == 0) {
      char *offset = (char *)memchr(header->value.base, ';', header->value.len);
      if (offset == NULL) {
        break;
      }

      char *s = _ltrim(header->value.base, offset - 1);
      char *t = _rtrim(header->value.base, offset - 1);

      if (s < t && strncasecmp(s, "multipart/form-data", t - s + 1) == 0) {
        s = _ltrim(offset + 1, header->value.base + header->value.len);
        t = _rtrim(offset + 1, header->value.base + header->value.len);
        if (strncasecmp(s, "boundary=", sizeof("boundary=") - 1) == 0) {
          is_post_multipart = 1;
          boundary->base = s + sizeof("boundary=") - 1;
          boundary->len = t - boundary->base;
        }
      }

      break;
    }
  }

  return is_post_multipart;
}

///////////// HELP-FUNCTION

int _get_dispoition_field(char *start, char *end, const char *key, int key_len,
                          uv_buf_t *field) {
  char *s = strstr(start, key);
  if (s != NULL) {
    field->base = s + key_len;
    char *eof = strchr(field->base, '"');
    if (eof != NULL) {
      field->len = eof - field->base;
    } else {
      field->len = end - field->base;
      PLOG_WARN("invalidate Content-Disposition, %s has no ending \"", key);
      return -1;
    }
  }

  return 0;
}

static void _debug_print_ptr(const char *ptr, int len) {
  zend_string *xx = zend_string_init(ptr, len, 0);
  PLOG_DEBUG("[LINE] %s", ZSTR_VAL(xx));
  zend_string_release(xx);
}

/// response: left body
uv_buf_t _get_multipart_content(uv_buf_t *body, uv_buf_t *boundary,
                                http_multipart_t *mp) {
  char *s = strstr(body->base, boundary->base);
  char *e = body->base + body->len;

  if (s == NULL) {
    return uv_buf_init(NULL, 0);
  }

  s = s + boundary->len;

  if (s[0] == '\r' && s[1] == '\n') {
    s += 2;
  } else if (s[0] == '-' && s[1] == '-') {
    return uv_buf_init(NULL, 0);
  } else {
    PLOG_WARN("invalidate multipart");
    return uv_buf_init(NULL, 0);
  }

  char *t = NULL;

  while (1) {
    // don't care length of s,
    // because input body(zend_string) will ending with '\0'
    t = strstr(s, "\r\n");

    if (t == NULL || s == t) {
      break;
    }

    _debug_print_ptr(s, t - s);

    if (strncasecmp(s, STRING_ARRAY_PARAM("Content-Disposition:")) == 0) {
      const int length = sizeof("Content-Disposition:") - 1;
      char *m = _ltrim(s + length, t);
      char *offset = (char *)memchr(m, ';', t - m);
      if (offset == NULL) {
        PLOG_WARN("invalidate Content-Disposition");
        break;
      }

      *t = '\0'; // mark ending, will revert later

      _get_dispoition_field(offset, e, STRING_ARRAY_PARAM("name=\""),
                            &mp->name);
      _get_dispoition_field(offset, e, STRING_ARRAY_PARAM("filename=\""),
                            &mp->filename);

      *t = '\r'; // revert
    } else if (strncasecmp(s, STRING_ARRAY_PARAM("Content-Type:")) == 0) {
      const int length = sizeof("Content-Type:") - 1;
      char *m = _ltrim(s + length, t);
      mp->type = uv_buf_init(m, t - m);
    } else if (strncasecmp(
                   s, STRING_ARRAY_PARAM("Content-Transfer-Encoding:")) == 0) {
      const int length = sizeof("Content-Transfer-Encoding:") - 1;
      char *m = _ltrim(s + length, t);
      mp->encode = uv_buf_init(s + length, t - s - length);
    }

    s = t + 2;
  }

  if (t == NULL) {
    PLOG_WARN("invlidate multipart");
    return uv_buf_init(NULL, 0);
  }

  char *next = strstr(t, boundary->base);
  if (next != NULL) {
    mp->content.base = t + 2;       // 2-"\r\n"
    mp->content.len = next - t - 4; // 4-2*"\r\n"
  }

  return uv_buf_init(next, e - next);
}

zval *_post_file(zval *zfile, http_multipart_t *mp, const char *tmp_file,
                 long error_code) {
  ZVAL_NEW_ARR(zfile);
  zend_hash_init(Z_ARRVAL_P(zfile), 5, NULL, ZVAL_PTR_DTOR, 0);
  zval name;
  ZVAL_STRINGL(&name, mp->name.base, mp->name.len);
  ZARR_HASH_ADD(zfile, name);

  zval type;
  ZVAL_STRINGL(&type, mp->type.base, mp->type.len);
  ZARR_HASH_ADD(zfile, type);

  zval tmp_name;
  ZVAL_STRING(&tmp_name, tmp_file);
  ZARR_HASH_ADD(zfile, tmp_name);

  zval error;
  ZVAL_LONG(&error, error_code);
  ZARR_HASH_ADD(zfile, error);

  zval size;
  ZVAL_LONG(&size, mp->content.len);
  ZARR_HASH_ADD(zfile, size);

  return zfile;
}

int _save_tmp_file(proton_http_message_t *message, http_multipart_t *mp,
                   const char **pfname) {
  FILE *fp = NULL;
  const int length = sizeof("/tmp/proton") - 1;
  char *filename = proton_link_buffer_alloc(&message->buffers, 30, 1);
  strcpy(filename, "/tmp/proton");
  srand((int)time(NULL) + __hash_key(mp->filename.base, mp->filename.len));
  for (int i = 0; i < 6; ++i) {
    filename[length + i] = (rand() % 26) + 'a';
  }
  filename[length + 6] = '\0';

  fp = fopen(filename, "wb");
  if (fp == NULL) {
    PLOG_WARN("open temp file(%s) failed", filename);
    return -1;
  }
  fwrite(mp->content.base, mp->content.len, 1, fp);

  fclose(fp);

  *pfname = filename;

  return 0;
}

int http_message_get_multipart_content(proton_http_message_t *message,
                                       zval *_post, zval *_file) {

  MAKESURE_PTR_NOT_NULL(message);

  if (message->boundary.base == NULL && message->boundary.len == 0) {
    if (!is_http_post_multipart(message, &message->boundary)) {
      PLOG_WARN("not found boundary");
      return -1; // not found boundary
    }
  }

  zend_string *body = http_message_get_raw_body(message);

  zend_string *bound_search_key =
      zend_string_alloc(message->boundary.len + 2, 0);
  char *q = ZSTR_VAL(bound_search_key);
  strcpy(q, "--");
  memcpy(q + 2, message->boundary.base, message->boundary.len);
  q[message->boundary.len + 2] = '\0';

  uv_buf_t buf_body = uv_buf_init(ZSTR_VAL(body), ZSTR_LEN(body));
  uv_buf_t buf_bound =
      uv_buf_init(ZSTR_VAL(bound_search_key), ZSTR_LEN(bound_search_key));

  while (buf_body.base != NULL && buf_body.len > buf_bound.len) {
    http_multipart_t multipart;
    http_multipart_init(&multipart);
    buf_body = _get_multipart_content(&buf_body, &buf_bound, &multipart);
    if (UV_BUF_NOT_EMPTY(&multipart.name) &&
        UV_BUF_NOT_EMPTY(&multipart.content)) {
      // append file
      if (_file != NULL && UV_BUF_NOT_EMPTY(&multipart.filename)) {
        // First Save Content To File
        const char *tmp_file = NULL;
        int error_code = _save_tmp_file(message, &multipart, &tmp_file);

        zval zfile;
        zend_hash_next_index_insert(
            Z_ARRVAL_P(_file),
            _post_file(&zfile, &multipart, tmp_file, error_code));
      } else if (_post != NULL && !UV_BUF_NOT_EMPTY(&multipart.filename)) {
        // append content
        zval value;
        ZVAL_STRINGL(&value, multipart.content.base, multipart.content.len);
        zend_hash_add(
            Z_ARRVAL_P(_post),
            zend_string_init(multipart.name.base, multipart.name.len, 0),
            &value);
      }
    }
  }

  zend_string_release(bound_search_key);

  return 0;
}