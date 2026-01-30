#include "mongoose.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

static const char *s_listen_url = "http://0.0.0.0:6711";
static const char *s_roms_dir = "roms";

static void handle_websocket_message(struct mg_connection *c, struct mg_ws_message *wm) {
  mg_ws_printf(c, WEBSOCKET_OP_TEXT, "Echo: %.*s", (int)wm->data.len, wm->data.buf);
}

static void handle_api_roms(struct mg_connection *c) {
  DIR *dir = opendir(s_roms_dir);
  if (!dir) {
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "[]");
    return;
  }

  mg_printf(c, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_http_printf_chunk(c, "[");

  struct dirent *entry;
  int first = 1;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] == '.') continue;
    mg_http_printf_chunk(c, "%s\"%s\"", first ? "" : ",", entry->d_name);
    first = 0;
  }
  closedir(dir);

  mg_http_printf_chunk(c, "]");
  mg_http_printf_chunk(c, "");
}

static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    printf("Request: %.*s\n", (int)hm->uri.len, hm->uri.buf);
    if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
      mg_ws_upgrade(c, hm, NULL);
    } else if (mg_match(hm->uri, mg_str("/api/roms"), NULL)) {
      handle_api_roms(c);
    } else if (mg_match(hm->uri, mg_str("/roms/*"), NULL)) {
      struct mg_http_serve_opts opts = {.root_dir = "."};
      mg_http_serve_dir(c, hm, &opts);
    } else {
      struct mg_http_serve_opts opts = {.root_dir = "/zip"};
      mg_http_serve_dir(c, hm, &opts);
    }
  } else if (ev == MG_EV_WS_MSG) {
    handle_websocket_message(c, (struct mg_ws_message *)ev_data);
  }
}

int main(void) {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, s_listen_url, event_handler, NULL);
  printf("Starting server at %s\n", s_listen_url);
  printf("Open http://localhost:6711 in your browser\n");
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);
  return 0;
}
