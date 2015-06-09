#include <pebble.h>

#include "T3Window.h"
#include "color_sel_lib.h"

const char * keyboardSet1[] = {T3_LAYOUT_UPPERCASE, T3_LAYOUT_LOWERCASE};

#define MAX_NUM_NAMES 6

#define COOKIE_KEY 0
#define COOKIE_DEFAULT 123
#define NUM_KEY 1
#define NUM_DEFAULT 4
#define NAME_KEY 2
#define LAST_KEY 30
#define DATE_KEY 40
#ifdef PBL_COLOR
#define COLOR_KEY 20
#define COLOR_DEFAULT 203
#endif

#ifdef PBL_COLOR
#define NUM_MODES 7
#else
#define NUM_MODES 6
#endif

static Window *window;

// This is a simple menu layer
static TextLayer *text_layer;
//static TextLayer *debug_layer;
static TextLayer *name_layer[MAX_NUM_NAMES];
static TextLayer *last_layer[MAX_NUM_NAMES];
static Layer *draw_layer;

int num_names = NUM_DEFAULT;
int order[MAX_NUM_NAMES];
int cookie = COOKIE_DEFAULT;
int mode = 0;
int editname;
int selectname = 0;
int date_format = 0;
int x[MAX_NUM_NAMES], y[MAX_NUM_NAMES];
const int x_size = 80, y_size = 23;

static char* name_text[] = {
  "Name 1    ",
  "Name 2    ",
  "Name 3    ",
  "Name 4    ",
  "Name 5    ",
  "Name 6    ",
};
int last_int[] = {0, 0, 0, 0, 0, 0};
static char* last_text[] = {
  " X1/XX",
  " X2/XX",
  " X3/XX",
  " X4/XX",
  " X5/XX",
  " X6/XX",
};
#ifdef PBL_COLOR
static int name_col[MAX_NUM_NAMES];
#endif

static char* MODES[] = {
  "SELECT",
  "EDIT",
#ifdef PBL_COLOR
  "COLOR",
#endif
  "ADD",
  "DELETE",
  "RESET",
  "DATE FORMAT",
};

static int power(int n) {
  int buf = 1;
  for (int i = 0; i < n; i++) {
    buf = buf * 10;
  }
  return buf;
}

static void draw_update_proc(Layer *layer, GContext *context) {
  int linewidth = 1;
  for (int i = 0; i < linewidth; i++) {
    graphics_context_set_stroke_color(context, GColorWhite);
    graphics_draw_rect(context, (GRect) {
      .origin = { x[selectname] + i - 5, y[selectname] + i + 7}, 
      .size = { x_size - 2 * i, y_size - 2 * i - 1} 
    });
  }
}

static void write_names() {
  APP_LOG(APP_LOG_LEVEL_INFO, "write_names");
//  char buf[]=" X1/XX";
  cookie = 0;
  for (int i = 0; i < num_names; i++) {
    cookie += order[i] * power(num_names - 1 - i);
    text_layer_set_text(name_layer[i], name_text[order[i]]);
//    snprintf(buf, 6, "%d", last_int[order[i]]);
//    APP_LOG(APP_LOG_LEVEL_INFO, buf);
//    APP_LOG(APP_LOG_LEVEL_INFO, last_text[order[i]]);
    text_layer_set_text(last_layer[i], last_text[order[i]]);
    if (last_int[order[i]] > 0) {
      text_layer_set_text_color(last_layer[i], GColorWhite);
    } else {
#ifdef PBL_COLOR
      text_layer_set_text_color(last_layer[i], (GColor){.argb = name_col[order[i]]});
#else
      text_layer_set_text_color(last_layer[i], GColorBlack);
#endif
    }
//    snprintf(buf, 6, "%d", order[i]);
//    APP_LOG(APP_LOG_LEVEL_INFO, buf);
#ifdef PBL_COLOR
    text_layer_set_background_color(name_layer[i], (GColor){.argb = name_col[order[i]]});
    text_layer_set_background_color(last_layer[i], (GColor){.argb = name_col[order[i]]});
#endif
  }
}

static void write_dates() {
  char buffer[]=" X1/XX";
  APP_LOG(APP_LOG_LEVEL_INFO, "last_int");
  for (int i = 0; i < num_names; i++) {
    snprintf(buffer, 6, "%d", last_int[i]);
    APP_LOG(APP_LOG_LEVEL_INFO, buffer);
    if (last_int[i] > 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "last_int[i] > 0");
      int last_m = last_int[i] / 100;
      int last_d = last_int[i] - last_m * 100;
      snprintf(buffer, 6, "%d", last_m);
      APP_LOG(APP_LOG_LEVEL_INFO, buffer);
      snprintf(buffer, 6, "%d", last_d);
      APP_LOG(APP_LOG_LEVEL_INFO, buffer);
      if (date_format == 0) {
        snprintf(last_text[i], 5, "%d/%d", last_d, last_m);
      } else {
        snprintf(last_text[i], 5, "%d/%d", last_m, last_d);
      }
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "else");
      last_text[i] = "-/-";
      APP_LOG(APP_LOG_LEVEL_INFO, last_text[i]);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, last_text[i]);
  }
}

static void name_select() {
  APP_LOG(APP_LOG_LEVEL_INFO, "name_select");
  int swap;

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);

  int month = current_time->tm_mon + 1;
  int mday = current_time->tm_mday;
  last_int[order[selectname]] = month * 100 + mday;
//  last_text[order[selectname]] = "XX/XX";
  for (int j = selectname; j < num_names - 1; j++) {
    swap = order[j];
    order[j] = order[j + 1];
    order[j + 1] = swap;
  }
  
  write_dates();

  write_names();
}

void handle_T3_close_edit(const char * text) {
  APP_LOG(APP_LOG_LEVEL_INFO, "handle_T3_close_edit");
  APP_LOG(APP_LOG_LEVEL_INFO, text);
//  if (sizeof(name_text[editname]) > sizeof(text)) {
//    strcpy(name_text[editname], text);
//  } else {
//    strcpy(name_text[editname], "");
//    strncat(name_text[editname], text, sizeof(name_text[editname]));
//  }
  if (strncmp(text, "\0", 1) != 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "blank");
    strncpy(name_text[editname], text, 10);
    text_layer_set_text(text_layer, MODES[mode]);
    for (int i = 0; i < num_names; i++) {
      text_layer_set_text(name_layer[i], name_text[order[i]]);
    }
  }
}

static void name_edit() {
  APP_LOG(APP_LOG_LEVEL_INFO, "name_edit");
  editname = order[selectname];
  
  T3Window * myT3Window = t3window_create(
    keyboardSet1, 2,
    NULL, 0,
    NULL, 0,
    (T3CloseHandler)handle_T3_close_edit);
  
  t3window_show(myT3Window, true);
}

#ifdef PBL_COLOR

void handle_CS_close_edit(int color) {
  APP_LOG(APP_LOG_LEVEL_INFO, "handle_CS_close_edit");
  name_col[editname] = color;
//  mode = 0;
  text_layer_set_text(text_layer, MODES[mode]);
  for (int i = 0; i < num_names; i++) {
    text_layer_set_background_color(name_layer[i], (GColor){.argb = name_col[order[i]]});
    text_layer_set_background_color(last_layer[i], (GColor){.argb = name_col[order[i]]});
    if (last_int[order[i]] == 0 ) {
      text_layer_set_text_color(last_layer[i], (GColor){.argb = name_col[order[i]]});
    }
  }
}

static void color_select() {
  APP_LOG(APP_LOG_LEVEL_INFO, "color_select");
  editname = order[selectname];
  
  CSWindow * myCSWindow = cswindow_create(
    0, false, (CSCloseHandler)handle_CS_close_edit);
  
  cswindow_show(myCSWindow, true);
}

#endif

static void name_add() {
  APP_LOG(APP_LOG_LEVEL_INFO, "name_add");
  num_names++;
  order[num_names - 1] = num_names - 1;
  cookie = 0;
  for (int i = 0; i < num_names; i++) {
    cookie += order[i] * power(num_names - 1 - i);
    text_layer_set_text(name_layer[i], name_text[order[i]]);
  }
}

static void setting_reset() {
  APP_LOG(APP_LOG_LEVEL_INFO, "setting_reset");
  cookie = 0;
  for (int i = 0; i < num_names; i++) {
    order[i] = i;
    cookie += order[i] * power(num_names -1 - i);
  }
  for (int i = 0; i < MAX_NUM_NAMES; i++) {
    if (i < num_names) {
      text_layer_set_text(name_layer[i], name_text[order[i]]);
    } else {
      text_layer_set_text(name_layer[i], "");
    }
  }
  selectname = 0;
  layer_mark_dirty(draw_layer);
}

static void name_delete() {
  APP_LOG(APP_LOG_LEVEL_INFO, "name_delete");
  num_names--;
  if (order[selectname] != num_names) {
    strncpy(name_text[order[selectname]], name_text[num_names], 10);
  }
//  text_layer_set_text(name_layer[num_names], "X");
  int j = 0;
  for (int i = 0; i < num_names; i++) {
    if (order[i] == num_names) j = 1;
    order[i] = order[i + j];
  }
  cookie = 0;
  for (int i = 0; i < num_names; i++) {
    cookie += order[i] * power(num_names - 1 - i);
    text_layer_set_text(name_layer[i], name_text[order[i]]);
  }
  for (int i = num_names; i < MAX_NUM_NAMES; i++) {
    text_layer_set_text(name_layer[i], "");
#ifdef PBL_COLOR
    text_layer_set_background_color(name_layer[i], (GColor){.argb = COLOR_DEFAULT});
#endif
  }
//  setting_reset();
  layer_mark_dirty(draw_layer);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
char message[]="XXXXXXXXXX";
  mode++;
  if (mode > (NUM_MODES - 1)) mode = 0;
  snprintf(message, 10, "NUM: %d", NUM_MODES);
  APP_LOG(APP_LOG_LEVEL_INFO, message);
  snprintf(message, 10, "mode: %d", mode);
  APP_LOG(APP_LOG_LEVEL_INFO, message);
  text_layer_set_text(text_layer, MODES[mode]);

}

static void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  char message[]="XXXXXXXXXX";
  snprintf(message, 10, "mode: %d", mode);
  APP_LOG(APP_LOG_LEVEL_INFO, message);

  switch (mode) {
    case 0: //SELECT
      name_select();
    break;
    case 1: //EDIT
      name_edit();
    break;
    
#ifdef PBL_COLOR
    case 3: //ADD
#else
    case 2: //ADD
#endif
      name_add();
    break;
    
#ifdef PBL_COLOR
    case 4: //DELETE
#else
    case 3: //DELETE
#endif
      name_delete();
    break;
    
#ifdef PBL_COLOR
    case 5: //RESET
#else
    case 4: //RESET
#endif
      setting_reset();
    break;
    
#ifdef PBL_COLOR
    case 6: //DATE
#else
    case 5: //DATE
#endif
      date_format = (date_format == 0) ? 1 : 0;
      write_names();
      write_dates();
    break;
  
#ifdef PBL_COLOR
    case 2: //COLOR
      color_select();
    break;
#endif
  }
}

static void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  selectname--;
  if (selectname < 0) selectname = num_names - 1;
  layer_mark_dirty(draw_layer);
}

static void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  selectname++;
  if (selectname == num_names) selectname = 0;
  layer_mark_dirty(draw_layer);
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_single_click_handler);
//	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)select_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_SELECT, 0, (ClickHandler)select_long_click_handler, NULL);
}

static void window_load(Window *window) {
  char message[]="XXXXXXXXXX";
  snprintf(message, 10, "mode1: %d", mode);
  APP_LOG(APP_LOG_LEVEL_INFO, message);

	window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider);

  num_names = persist_exists(NUM_KEY) ? persist_read_int(NUM_KEY) : NUM_DEFAULT;
  cookie = persist_exists(COOKIE_KEY) ? persist_read_int(COOKIE_KEY) : COOKIE_DEFAULT;
  if (persist_exists(DATE_KEY)) date_format = persist_read_int(DATE_KEY);

  
#ifdef PBL_COLOR
  for (int i = 0; i < MAX_NUM_NAMES; i++) {
    name_col[i] = COLOR_DEFAULT;
  }
#endif
  
  for (int i = 0; i < num_names; i++) {
    if (persist_exists(NAME_KEY + i)) {
      persist_read_string(NAME_KEY + i, name_text[i], 10);
    }
    if (persist_exists(LAST_KEY + i)) {
      last_int[i] = persist_read_int(LAST_KEY + i);
//      last_int[i] = 528 + i;
      if (last_int[i] > 0) {
        int last_m = last_int[i] / 100;
        int last_d = last_int[i] - last_m * 100;
        if (date_format == 0) {
          snprintf(last_text[i], 5, "%d/%d", last_d, last_m);
        } else {
          snprintf(last_text[i], 5, "%d/%d", last_m, last_d);
        }
      }
    }
#ifdef PBL_COLOR
    name_col[i] = persist_exists(COLOR_KEY + i) ? persist_read_int(COLOR_KEY + i) : COLOR_DEFAULT;
//    name_col[i] = COLOR_DEFAULT;
#endif
  }
  snprintf(message, 10, "mode2: %d", mode);
  APP_LOG(APP_LOG_LEVEL_INFO, message);

  int cook_temp = cookie;
  for (int i = num_names-1; i >= 0; i--) {
    int highbit = (int)(cook_temp / 10);
    order[i] = (int)(cook_temp - (10 * highbit));
    cook_temp = highbit;
  }
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
#ifdef PBL_COLOR
  window_set_background_color(window, (GColor){.argb = COLOR_DEFAULT});
#else
  window_set_background_color(window, GColorBlack);
#endif

  x[0] = 10;
  y[0] = y_size;
  for (int i = 1; i < MAX_NUM_NAMES; i++) {
    x[i] = x[i-1];
    y[i] = y[i-1] + y_size;
  }
  snprintf(message, 10, "mode3: %d", mode);
  APP_LOG(APP_LOG_LEVEL_INFO, message);
  
  for (int i = MAX_NUM_NAMES-1; i >= 0; i--) {
    name_layer[i] = text_layer_create((GRect){.origin = {x[i], y[i] }, .size = {x_size, 29}});
    last_layer[i] = text_layer_create((GRect){.origin = {0, y[i] }, .size = {144, 29}});
    text_layer_set_text_alignment(name_layer[i], GTextAlignmentLeft);
    text_layer_set_text_alignment(last_layer[i], GTextAlignmentRight);
    text_layer_set_font(name_layer[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_font(last_layer[i], fonts_get_system_font(FONT_KEY_GOTHIC_24));
#ifdef PBL_COLOR
    text_layer_set_background_color(name_layer[i], (GColor){.argb = name_col[order[i]]});
    text_layer_set_background_color(last_layer[i], (GColor){.argb = name_col[order[i]]});
#else
    text_layer_set_background_color(name_layer[i], GColorBlack);
    text_layer_set_background_color(last_layer[i], GColorBlack);
#endif
    text_layer_set_text_color(name_layer[i], GColorWhite);
    if (last_int[order[i]] > 0 ) {
      text_layer_set_text_color(last_layer[i], GColorWhite);
    } else {
#ifdef PBL_COLOR
    text_layer_set_text_color(last_layer[i], (GColor){.argb = name_col[order[i]]});
#else
    text_layer_set_text_color(last_layer[i], GColorBlack);
#endif
    }
    if (i < num_names) {
      text_layer_set_text(name_layer[i], name_text[order[i]]);
      text_layer_set_text(last_layer[i], last_text[order[i]]);
    }
    layer_add_child(window_layer, text_layer_get_layer(last_layer[i]));
    layer_add_child(window_layer, text_layer_get_layer(name_layer[i]));
  }

  snprintf(message, 10, "mode4: %d", mode);
  APP_LOG(APP_LOG_LEVEL_INFO, message);

  text_layer = text_layer_create((GRect){.origin = {0, 0}, .size = {144, 29}});
  text_layer_set_text(text_layer, MODES[mode]);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_text_color(text_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  snprintf(message, 10, "mode5: %d", mode);
  APP_LOG(APP_LOG_LEVEL_INFO, message);


  draw_layer = layer_create(bounds);//0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(draw_layer, draw_update_proc);
  layer_add_child(window_layer, draw_layer);
  
}

void window_unload(Window *window) {
  
  persist_write_int(COOKIE_KEY, cookie);
  persist_write_int(NUM_KEY, num_names);
  persist_write_int(DATE_KEY, date_format);
  for (int i = 0; i < num_names; i++) {
    persist_write_string(NAME_KEY + i, name_text[i]);
    if (last_int[i] > 0) persist_write_int(LAST_KEY + i, last_int[i]);
#ifdef PBL_COLOR
    persist_write_int(COLOR_KEY + i, name_col[i]);
#endif
  }
  
  
  text_layer_destroy(text_layer);
  for (int i = 0; i < MAX_NUM_NAMES; i++) {
    text_layer_destroy(name_layer[i]);
    text_layer_destroy(last_layer[i]);
  }
  layer_destroy(draw_layer);

}

int main(void) {
  window = window_create();
#ifdef PBL_COLOR
#else
  window_set_fullscreen(window, true);
#endif
  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true /* Animated */);

  app_event_loop();

  window_destroy(window);
}