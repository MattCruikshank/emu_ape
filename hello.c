#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>

// sudo /opt/cosmo/bin/fatcosmocc --update

// /opt/cosmo/bin/cosmocc -o server hello.c mongoose.c && zip server index.html && ./server
// Mako streaming html <await>

// make run
// http://debianvm.tail4c18a.ts.net:6711/

// 67... 11...

static const char *s_listen_url = "http://0.0.0.0:6711";
static char *s_html = NULL;
static char *s_css = NULL;

static char *load_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Failed to open %s\n", path);
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buf = malloc(len + 1);
  if (buf) {
    fread(buf, 1, len, f);
    buf[len] = '\0';
  }
  fclose(f);
  return buf;
}

static void handle_websocket_message(struct mg_connection *c, struct mg_ws_message *wm) {
  // Echo the message back with a prefix
  mg_ws_printf(c, WEBSOCKET_OP_TEXT, "Echo: %.*s", (int)wm->data.len, wm->data.buf);
}


static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    printf("Request: %.*s\n", (int)hm->uri.len, hm->uri.buf);

    if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
      // Upgrade to WebSocket
      mg_ws_upgrade(c, hm, NULL);
    } else if (mg_match(hm->uri, mg_str("/index.css"), NULL)) {
      // Serve CSS from embedded zip
      if (s_css) {
        mg_http_reply(c, 200, "Content-Type: text/css\r\n", "%s", s_css);
      } else {
        mg_http_reply(c, 500, "", "CSS not loaded\n");
      }
    } else if (mg_match(hm->uri, mg_str("/"), NULL)) {
      // Serve HTML from embedded zip
      if (s_html) {
        mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s", s_html);
      } else {
        mg_http_reply(c, 500, "", "HTML not loaded\n");
      }
    } else {
      mg_http_reply(c, 404, "", "Not Found\n");
    }
  } else if (ev == MG_EV_WS_MSG) {
    handle_websocket_message(c, (struct mg_ws_message *)ev_data);
  }
}

int main(void) {
  struct mg_mgr mgr;

  s_html = load_file("/zip/index.html");
  s_css = load_file("/zip/index.css");

  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, s_listen_url, event_handler, NULL);

  printf("Starting server at %s\n", s_listen_url);
  printf("Open http://localhost:6711 in your browser\n");

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }

  free(s_html);
  free(s_css);
  mg_mgr_free(&mgr);
  return 0;
}
