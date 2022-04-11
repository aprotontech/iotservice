
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <uv.h>

#define WORKER_COUNT 2

#define CHECK_UV_RESULT(expr)                                                  \
  {                                                                            \
    int r = expr;                                                              \
    if (r != 0) {                                                              \
      printf("%s error %d, %s\n", #expr, r, uv_err_name(r));                   \
    }                                                                          \
  }

uv_loop_t _loop;

struct child_worker {
  uv_process_t req;
  uv_process_options_t options;
  uv_pipe_t pipe;
};

struct write_req {
  uv_write_t write;
  uv_buf_t buf;
};

int _round_robin_counter = 0;
struct child_worker _workers[WORKER_COUNT];

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

void on_close(uv_handle_t *handle) {
  printf("pid=%d, handle closed\n", getpid());
}

void on_write(uv_write_t *req, int status) {
  printf("pid=%d, write status=%d, error=%s\n", getpid(), status,
         status == 0 ? "success" : uv_err_name(status));
  free(req);
}

void echo_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *)stream, on_close);
    return;
  }

  struct write_req *req =
      (struct write_req *)malloc(sizeof(struct write_req) + nread);
  req->buf = uv_buf_init((char *)&req[1], nread);
  memcpy(&req[1], buf->base, nread);

  uv_write(&req->write, stream, &req->buf, 1, on_write);

  free(buf->base);
}

void on_master_new_connection(uv_stream_t *server, int status) {
  if (status == -1) {
    printf("master error\n");
    // error!
    return;
  }

  printf("master new connection\n");

  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(&_loop, client);
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    struct write_req *req =
        (struct write_req *)malloc(sizeof(struct write_req) + 10);
    req->buf = uv_buf_init("a", 1);

    printf("[master] assign to worker %d\n", _round_robin_counter);
    struct child_worker *worker = &_workers[_round_robin_counter];
    CHECK_UV_RESULT(uv_write2(&req->write, (uv_stream_t *)&worker->pipe,
                              &req->buf, 1, (uv_stream_t *)client, on_write));

    _round_robin_counter = (_round_robin_counter + 1) % WORKER_COUNT;
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

void on_worker_new_connection(uv_stream_t *q, ssize_t nread,
                              const uv_buf_t *buf) {
  printf("on_worker_new_connection %ld\n", nread);
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "worker Read error %s\n", uv_err_name(nread));
    }
    uv_close((uv_handle_t *)q, NULL);
    return;
  }

  uv_pipe_t *pipe = (uv_pipe_t *)q;
  if (!uv_pipe_pending_count(pipe)) {
    fprintf(stderr, "No pending count\n");
    return;
  }

  uv_handle_type pending = uv_pipe_pending_type(pipe);
  assert(pending == UV_TCP);

  printf("worker[%d] new connection\n", getpid());

  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(&_loop, client);
  if (uv_accept(q, (uv_stream_t *)client) == 0) {
    uv_os_fd_t fd;
    uv_fileno((const uv_handle_t *)client, &fd);
    fprintf(stderr, "Worker %d: Accepted fd %d\n", getpid(), fd);
    CHECK_UV_RESULT(
        uv_read_start((uv_stream_t *)client, alloc_buffer, echo_read));
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

struct child_worker *setup_workers() {
  int count = WORKER_COUNT;
  while (count--) {
    int rwpipe[2];
    struct child_worker *worker = &_workers[count];
    uv_pipe_init(&_loop, &worker->pipe, 1);
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, rwpipe) != 0) {
      printf("[master] create pipe failed\n");
      return NULL;
    }
    printf("pipe, read=%d,write=%d\n", rwpipe[0], rwpipe[1]);

    pid_t pid = fork();
    if (pid == -1) {
      printf("[master] fork new process failed\n");
      return NULL;
    } else if (pid == 0) { // worker process
      // uv_loop_fork(&_loop);

      close(rwpipe[1]);
      // fcntl(rwpipe[0], F_SETFL, O_NONBLOCK);
      CHECK_UV_RESULT(uv_pipe_open(&worker->pipe, rwpipe[0]));
      fprintf(stderr, "Started worker %d\n", getpid());
      return worker;
    } else { // master process
      close(rwpipe[0]);

      // fcntl(rwpipe[1], F_SETFL, O_NONBLOCK);
      CHECK_UV_RESULT(uv_pipe_open(&worker->pipe, rwpipe[1]));
    }
  }

  return NULL;
}

int main(int argc, const char *argv[]) {

  uv_tcp_t server;
  struct sockaddr_in addr;

  uv_loop_init(&_loop);

  struct child_worker *worker = setup_workers();
  if (worker != NULL) { // worker
    printf("worker started\n");
    uv_loop_fork(&_loop);
    CHECK_UV_RESULT(uv_read_start((uv_stream_t *)&worker->pipe, alloc_buffer,
                                  on_worker_new_connection));
  } else { // master
    printf("master start to listen\n");
    uv_tcp_init(&_loop, &server);
    uv_ip4_addr("0.0.0.0", 18888, &addr);
    uv_tcp_bind(&server, (struct sockaddr *)&addr, 0);
    uv_listen((uv_stream_t *)&server, 1, on_master_new_connection);
  }

  return uv_run(&_loop, UV_RUN_DEFAULT);
}