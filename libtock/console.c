#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "console.h"

#define CIRC_BUF_LEN 5

typedef struct putstr_data_elem {
  char* buf;
  int len;
  bool called;
} putstr_data_elem_t;

putstr_data_elem_t putstr_cbuf[CIRC_BUF_LEN];
int cbuf_head = 0;
int cbuf_count = 0;

static void putstr_upcall(int   _x __attribute__ ((unused)),
                          int   _y __attribute__ ((unused)),
                          int   _z __attribute__ ((unused)),
                          void* ud __attribute__ ((unused))) {
    putstr_data_elem_t* data = &putstr_cbuf[cbuf_head];
    data->called = true;
    free(data->buf);

    if (cbuf_count > 0) {
        int ret;
        putstr_data_elem_t* head = &putstr_cbuf[cbuf_head];
        ret = putnstr_async(head->buf, head->len, putstr_upcall, NULL);
        if (ret < 0) {
            // XXX There's no path to report errors currently, so just drop it
            putstr_upcall(0, 0, 0, NULL);
        }
    }

    return;
}

int putnstr(const char *str, size_t len) {
  int ret = RETURNCODE_SUCCESS;

  if (cbuf_count == CIRC_BUF_LEN) return RETURNCODE_ENOMEM;

  putstr_data_elem_t* data = &putstr_cbuf[(cbuf_head + cbuf_count) % CIRC_BUF_LEN];
  cbuf_count++;

  data->len    = len;
  data->called = false;
  data->buf    = (char*)malloc(len * sizeof(char));
  if (data->buf == NULL) {
    ret = RETURNCODE_ENOMEM;
    goto putnstr_fail_buf_alloc;
  }
  strncpy(data->buf, str, len);

  if (cbuf_count == 1) {
    ret = putnstr_async(data->buf, data->len, putstr_upcall, NULL);
    if (ret < 0) goto putnstr_fail_async;
  }

  yield_for(&data->called);

putnstr_fail_async:
  free(data->buf);
putnstr_fail_buf_alloc:
  cbuf_head = (cbuf_head + 1) % CIRC_BUF_LEN;
  cbuf_count--;

  return ret;
}

int putnstr_async(const char *str, size_t len, subscribe_upcall cb, void* userdata) {
#pragma GCC diagnostic push
#pragma GCC diagnostic pop

  allow_ro_return_t ro = allow_readonly(DRIVER_NUM_CONSOLE, 1, str, len);
  if (!ro.success) {
    return tock_status_to_returncode(ro.status);
  }

  subscribe_return_t sub = subscribe(DRIVER_NUM_CONSOLE, 1, cb, userdata);
  if (!sub.success) {
    return tock_status_to_returncode(sub.status);
  }

  syscall_return_t com = command(DRIVER_NUM_CONSOLE, 1, len, 0);
  return tock_command_return_novalue_to_returncode(com);
}

int getnstr_async(char *buf, size_t len, subscribe_upcall cb, void* userdata) {
  allow_rw_return_t rw = allow_readwrite(DRIVER_NUM_CONSOLE, 1, buf, len);
  if (!rw.success) {
    return tock_status_to_returncode(rw.status);
  }

  subscribe_return_t sub = subscribe(DRIVER_NUM_CONSOLE, 2, cb, userdata);
  if (!sub.success) {
    return tock_status_to_returncode(sub.status);
  }

  syscall_return_t com = command(DRIVER_NUM_CONSOLE, 2, len, 0);
  return tock_command_return_novalue_to_returncode(com);
}

typedef struct getnstr_data {
  bool called;
  int result;
} getnstr_data_t;

static getnstr_data_t getnstr_data = { true, 0 };

static void getnstr_upcall(int   result,
                           int   _y __attribute__ ((unused)),
                           int   _z __attribute__ ((unused)),
                           void* ud __attribute__ ((unused))) {
  getnstr_data.result = result;
  getnstr_data.called = true;
}

int getnstr(char *str, size_t len) {
  int ret;

  if (!getnstr_data.called) {
    // A call is already in progress
    return RETURNCODE_EALREADY;
  }
  getnstr_data.called = false;

  ret = getnstr_async(str, len, getnstr_upcall, NULL);
  if (ret < 0) {
    return ret;
  }

  yield_for(&getnstr_data.called);

  return getnstr_data.result;
}

int getch(void) {
  int r;
  char buf[1];

  r = getnstr(buf, 1);
  return (r == RETURNCODE_SUCCESS) ? buf[0] : RETURNCODE_FAIL;
}

int getnstr_abort(void) {
  syscall_return_t com = command(DRIVER_NUM_CONSOLE, 3, 0, 0);
  return tock_command_return_novalue_to_returncode(com);
}
