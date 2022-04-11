/*
 * **************************************************************************
 *
 *  Copyright (c) 2022 aproton.tech, Inc. All Rights Reserved
 *
 * **************************************************************************
 *
 *  @file     group.c
 *  @author   kuper - <kuper@aproton.tech>
 *  @data     2022-03-29 10:49:01
 *
 */

#include "group.h"

#include <sys/types.h>
#include <sys/wait.h>

PROTON_TYPE_WHOAMI_DEFINE(_process_group_get_type, "processgroup")

static proton_value_type_t __proton_processgroup_type = {
    .construct = NULL,
    .destruct = proton_process_group_uninit,
    .whoami = _process_group_get_type};

typedef struct _pg_client_t {
  list_link_t link;
  union {
    uv_tcp_t tcp;
  };
} pg_client_t;

void _pg_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                      uv_buf_t *buf) {
  *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

void _on_pg_master_client_close(uv_handle_t *handle) {
  pg_client_t *client = (pg_client_t *)handle->data;
  LL_remove(&client->link);
  free(client);
}

void _pg_on_worker_new_connection(uv_stream_t *q, ssize_t nread,
                                  const uv_buf_t *buf) {
  PLOG_DEBUG("pg-worker new connection %ld", nread);
  proton_process_group_t *group = (proton_process_group_t *)q->data;
  if (nread < 0) {
    if (nread != UV_EOF) {
      PLOG_DEBUG("worker Read error %s", uv_err_name(nread));
    }
    uv_close((uv_handle_t *)q, NULL);
    return;
  }

  uv_pipe_t *pipe = (uv_pipe_t *)q;
  if (!uv_pipe_pending_count(pipe)) {
    PLOG_WARN("No pending count");
    return;
  }

  uv_handle_type pending = uv_pipe_pending_type(pipe);
  assert(pending == UV_TCP);

  if (group != NULL && group->on_new_client != NULL) {
    group->on_new_client(group->server, q);
  }
}

int _init_worker_by_fork(proton_process_group_t *group,
                         proton_process_worker_t *worker) {
  int rwpipe[2];
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, rwpipe) != 0) {
    PLOG_WARN("[PG] create pipe failed");
    return RC_ERROR_NEW_WORKER;
  }

  CHECK_UV_RESULT(
      uv_pipe_init(RUNTIME_UV_LOOP(group->runtime), &worker->pipe, 1),
      RC_ERROR_NEW_WORKER);
  worker->pipe.data = group;
  LL_init(&worker->connections);

  pid_t pid = fork();
  if (pid == -1) {
    PLOG_ERROR("[PG] fork new worker failed");
    close(rwpipe[0]);
    close(rwpipe[1]);
    return RC_ERROR_NEW_WORKER;
  } else if (pid == 0) { // worker process
    worker->pid = getpid();
    worker->is_running = 1;
    group->type = PROCESS_WORKER;
    group->current = worker;

    CHECK_UV_RESULT(uv_loop_fork(RUNTIME_UV_LOOP(group->runtime)),
                    RC_ERROR_NEW_WORKER);
    close(rwpipe[1]);
    CHECK_UV_RESULT(uv_pipe_open(&worker->pipe, rwpipe[0]),
                    RC_ERROR_NEW_WORKER);

    CHECK_UV_RESULT(uv_read_start((uv_stream_t *)&worker->pipe,
                                  _pg_alloc_buffer,
                                  _pg_on_worker_new_connection),
                    RC_ERROR_NEW_WORKER);
    PLOG_NOTICE("[PG] worker(%d) started", worker->pid);
    return 0;
  } else { // master process
    worker->pid = pid;
    worker->is_running = 1;
    close(rwpipe[0]);
    CHECK_UV_RESULT(uv_pipe_open(&worker->pipe, rwpipe[1]),
                    RC_ERROR_NEW_WORKER);
  }

  return 0;
}

void child_signal_cb(uv_signal_t *handle, int signum) {
  int status;
  pid_t pid;
  proton_process_group_t *group = (proton_process_group_t *)handle->data;

  while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
    if (WIFEXITED(status)) {
      PLOG_DEBUG("child %d exit with %d", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      PLOG_DEBUG("child %d killed by the %dth signal", pid, WTERMSIG(status));
    }

    for (int i = 0; i < group->worker_count; ++i) {
      proton_process_worker_t *worker = &(group->workers[i]);
      if (worker->pid == pid) {
        worker->is_running = 0;
        uv_close((uv_handle_t *)(&worker->pipe), NULL);

        list_link_t *p = worker->connections.next;
        while (p != &worker->connections) {
          pg_client_t *client = container_of(p, pg_client_t, link);
          p = p->next;
          uv_close((uv_handle_t *)&client->tcp, _on_pg_master_client_close);
        }
        LL_init(&worker->connections);
      }
    }
  }
}

int proton_process_group_init(proton_private_value_t *value,
                              proton_coroutine_runtime *runtime,
                              proton_process_group_config_t *config) {
  MAKESURE_PTR_NOT_NULL(value);
  MAKESURE_PTR_NOT_NULL(runtime);
  MAKESURE_PTR_NOT_NULL(config);
  proton_process_group_t *group = (proton_process_group_t *)value;
  group->value.type = &__proton_processgroup_type;
  group->type = PROCESS_MASTER;
  group->status = PG_STATUS_INITED;
  group->runtime = runtime;
  group->worker_count = 0;
  group->workers = NULL;
  group->current = NULL;
  group->server = NULL;
  group->on_new_client = NULL;
  group->config = *config;

  uv_signal_init(RUNTIME_UV_LOOP(runtime), &group->signal_child);
  group->signal_child.data = group;
  return 0;
}

int proton_process_group_get_type(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_process_group_t *group = (proton_process_group_t *)value;
  return group->type;
}

int proton_process_group_uninit(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_process_group_t *group = (proton_process_group_t *)value;
  proton_process_group_stop(value);

  uv_signal_stop(&group->signal_child);

  return 0;
}

int proton_process_group_bind_server(proton_private_value_t *value,
                                     proton_private_value_t *server,
                                     on_worker_new_client callback) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_process_group_t *group = (proton_process_group_t *)value;
  if (group->status != PG_STATUS_RUNNING) {
    PLOG_WARN("[PG] process group is not running, can't bind server");
    return RC_ERROR_INVALIDATE_STATUS;
  }
  group->server = server;
  group->on_new_client = callback;
  return 0;
}

int proton_process_group_start(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_process_group_t *group = (proton_process_group_t *)value;

  if (RUNTIME_MAIN_COROUTINE(group->runtime) !=
      RUNTIME_CURRENT_COROUTINE(group->runtime)) {
    PLOG_WARN("proton_process_group_start must run on main corotinue");
    return RC_ERROR_NOT_MAIN_COROUTINE;
  }

  if (group->status == PG_STATUS_RUNNING) {
    PLOG_WARN("[PG] process group had started, can't start again");
    return RC_ERROR_INVALIDATE_STATUS;
  }

  int ncount = group->config.static_worker_count;
  if (ncount <= 0) {
    uv_cpu_info_t *info;
    uv_cpu_info(&info, &ncount);
    uv_free_cpu_info(info, ncount);
  }

  PLOG_INFO("[PG] try to start %d workers", ncount);

  proton_process_worker_t *workers = (proton_process_worker_t *)malloc(
      sizeof(proton_process_worker_t) * ncount);
  for (int i = 0; i < ncount; ++i) {
    int rwpipe[2];
    proton_process_worker_t *worker = &workers[i];

    if (_init_worker_by_fork(group, worker) != 0) {
      PLOG_ERROR("[PG] start worker[%d] failed", i);
      return RC_ERROR_NEW_WORKER;
    }

    if (group->type == PROCESS_WORKER) {
      group->status = PG_STATUS_RUNNING;
      group->current = worker;
      free(workers);
      return 0;
    }
  }

  group->workers = workers;
  group->worker_count = ncount;
  group->status = PG_STATUS_RUNNING;

  uv_signal_start(&group->signal_child, child_signal_cb, SIGCHLD);
  return 0;
}

int proton_process_group_stop(proton_private_value_t *value) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_process_group_t *group = (proton_process_group_t *)value;

  if (group->type == PROCESS_MASTER) {
    for (int i = 0; i < group->worker_count; ++i) {
      if (group->workers[i].is_running) {
        uv_close((uv_handle_t *)(&group->workers[i].pipe), NULL);
        uv_kill(group->workers[i].pid, SIGKILL);
        group->workers[i].is_running = 0;
      }
    }
  }

  group->status = PG_STATUS_STOPED;
  return 0;
}

void on_pg_master_write(uv_write_t *req, int status) {
  PLOG_DEBUG("[PG] master write status=%d, error=%s", status,
             status == 0 ? "success" : uv_err_name(status));
  free(req);
}

int proton_process_new_tcpclient(proton_private_value_t *value,
                                 proton_private_value_t *server,
                                 uv_stream_t *server_stream) {
  MAKESURE_PTR_NOT_NULL(value);
  proton_process_group_t *group = (proton_process_group_t *)value;
  int idx = rand() % group->worker_count;
  proton_process_worker_t *worker = &(group->workers[idx]);

  int rc;
  pg_client_t *client = (pg_client_t *)malloc(sizeof(pg_client_t));
  uv_tcp_init(RUNTIME_UV_LOOP(group->runtime), &client->tcp);
  client->tcp.data = client;
  LL_init(&client->link);
  if ((rc = uv_accept(server_stream, (uv_stream_t *)&client->tcp)) != 0) {
    PLOG_WARN("[PG] accept failed with %d, error(%s)", rc, uv_err_name(rc));
    uv_close((uv_handle_t *)&client->tcp, _on_pg_master_client_close);
    return RC_ERROR_CREATE_SOCKET;
  }

  proton_once_write_t *req =
      (proton_once_write_t *)malloc(sizeof(proton_once_write_t));
  req->buf = uv_buf_init("a", 1);
  req->write.data = client;

  PLOG_INFO("[PG] assign connect(%d) to worker[%d](%d)", client->tcp.accepted_fd,
            idx, worker->pid);
  if ((rc = uv_write2(&req->write, (uv_stream_t *)&worker->pipe, &req->buf, 1,
                      (uv_stream_t *)client, on_pg_master_write)) != 0) {
    PLOG_WARN("[PG] write to worker failed with %d, error(%s)", rc,
              uv_err_name(rc));
    uv_close((uv_handle_t *)&client->tcp, _on_pg_master_client_close);
    return RC_ERROR_IO_WRITE;
  }

  LL_insert(&client->link, worker->connections.prev);

  return 0;
}