#include "mongoose.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

static const char *s_listen_url = "http://0.0.0.0:6711";
static const char *s_roms_dir = "roms";

static void str_tolower(char *dst, const char *src, size_t len) {
  size_t i;
  for (i = 0; i < len - 1 && src[i]; i++) dst[i] = tolower(src[i]);
  dst[i] = '\0';
}

static void handle_websocket_message(struct mg_connection *c, struct mg_ws_message *wm) {
  mg_ws_printf(c, WEBSOCKET_OP_TEXT, "Echo: %.*s", (int)wm->data.len, wm->data.buf);
}

#define MAX_ROMS 1024

static void handle_api_roms(struct mg_connection *c) {
  // Collect unique rom paths from both sources
  static char paths[MAX_ROMS][512];
  static char names[MAX_ROMS][256];
  static char cores[MAX_ROMS][256];
  int count = 0;

  const char *bases[] = {"/zip/roms", s_roms_dir};
  for (int b = 0; b < 2 && count < MAX_ROMS; b++) {
    DIR *dir = opendir(bases[b]);
    if (!dir) continue;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < MAX_ROMS) {
      if (entry->d_name[0] == '.') continue;

      char subpath[512];
      snprintf(subpath, sizeof(subpath), "%s/%s", bases[b], entry->d_name);
      DIR *subdir = opendir(subpath);
      if (subdir) {
        char core[256];
        str_tolower(core, entry->d_name, sizeof(core));
        struct dirent *rom;
        while ((rom = readdir(subdir)) != NULL && count < MAX_ROMS) {
          if (rom->d_name[0] == '.') continue;
          // Build path as core/romname
          char path[512];
          snprintf(path, sizeof(path), "%s/%s", core, rom->d_name);
          // Check for duplicate
          int dup = 0;
          for (int i = 0; i < count; i++) {
            if (strcmp(paths[i], path) == 0) { dup = 1; break; }
          }
          if (dup) continue;
          snprintf(paths[count], sizeof(paths[0]), "%s", path);
          snprintf(names[count], sizeof(names[0]), "%s", rom->d_name);
          snprintf(cores[count], sizeof(cores[0]), "%s", core);
          count++;
        }
        closedir(subdir);
      } else {
        // Check for duplicate
        int dup = 0;
        for (int i = 0; i < count; i++) {
          if (strcmp(paths[i], entry->d_name) == 0) { dup = 1; break; }
        }
        if (dup) continue;
        snprintf(paths[count], sizeof(paths[0]), "%s", entry->d_name);
        snprintf(names[count], sizeof(names[0]), "%s", entry->d_name);
        cores[count][0] = '\0';
        count++;
      }
    }
    closedir(dir);
  }

  mg_printf(c, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_http_printf_chunk(c, "[");
  for (int i = 0; i < count; i++) {
    if (cores[i][0]) {
      mg_http_printf_chunk(c, "%s{\"name\":\"%s\",\"core\":\"%s\",\"path\":\"%s\"}",
                           i ? "," : "", names[i], cores[i], paths[i]);
    } else {
      mg_http_printf_chunk(c, "%s{\"name\":\"%s\",\"core\":null,\"path\":\"%s\"}",
                           i ? "," : "", names[i], paths[i]);
    }
  }
  mg_http_printf_chunk(c, "]");
  mg_http_printf_chunk(c, "");
}

static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    printf("Request: [%.*s] len=%d\n", (int)hm->uri.len, hm->uri.buf, (int)hm->uri.len);
    if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
      mg_ws_upgrade(c, hm, NULL);
    } else if (mg_strcmp(hm->uri, mg_str("/api/roms")) == 0) {
      printf("Matched /api/roms\n");
      handle_api_roms(c);
    } else if (mg_match(hm->uri, mg_str("/roms/*"), NULL)) {
      // Try filesystem first, fall back to embedded zip
      char fspath[512];
      snprintf(fspath, sizeof(fspath), ".%.*s", (int)hm->uri.len, hm->uri.buf);
      FILE *f = fopen(fspath, "r");
      if (f) {
        fclose(f);
        struct mg_http_serve_opts opts = {.root_dir = "."};
        mg_http_serve_dir(c, hm, &opts);
      } else {
        struct mg_http_serve_opts opts = {.root_dir = "/zip"};
        mg_http_serve_dir(c, hm, &opts);
      }
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
