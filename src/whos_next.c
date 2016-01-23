#include <pebble.h>

#include "T3Window.h"
#ifdef PBL_COLOR
#include "color_sel_lib.h"
#endif
 
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
#define NUM_MODES 8
#else
#define NUM_MODES 6
#endif

static Window *window;

static TextLayer *text_layer;
//static TextLayer *debug_layer;
static Layer *list_layer[MAX_NUM_NAMES];
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

static char *name_text[] = {
  "Name 1    ",
  "Name 2    ",
  "Name 3    ",
  "Name 4    ",
  "Name 5    ",
  "Name 6    ",
};
int last_int[] = {0, 0, 0, 0, 0, 0};
static char *last_text[] = {
  "       ",
  "       ",
  "       ",
  "       ",
  "       ",
  "       ",
};
#ifdef PBL_COLOR
static int name_col[MAX_NUM_NAMES];
#endif

static char *MODES[] = {
  "SELECT",
#ifdef PBL_COLOR
  "VOICE EDIT",
#endif
  "EDIT",
#ifdef PBL_COLOR
  "COLOR",
#endif
  "ADD",
  "DELETE",
  "RESET",
  "DATE FORMAT",
};

#ifdef PBL_COLOR
static DictationSession *dictation_session;
static char dict_text[512];
#endif

static int power(int n) {
  int buf = 1;
  for (int i = 0; i < n; i++) {
    buf = buf * 10;
  }
  return buf;
}

static void draw_update_proc(Layer *layer, GContext *context) {
  graphics_context_set_stroke_color(context, GColorWhite);
  graphics_draw_rect(context, (GRect) {
    .origin = { x[selectname] - 5, y[selectname] + 7}, 
    .size = { x_size, y_size - 2} 
    });
}

static void write_names() {
  APP_LOG(APP_LOG_LEVEL_INFO, "write_names");
  cookie = 0;
  for (int i = 0; i < num_names; i++) {
    cookie += order[i] * power(num_names - 1 - i);
    
    layer_mark_dirty(list_layer[i]);
  }
}

static void name_select() {
  APP_LOG(APP_LOG_LEVEL_INFO, "name_select");
  int swap;

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);

  int month = current_time->tm_mon + 1;
  int mday = current_time->tm_mday;
//  mday =21;
  last_int[order[selectname]] = month * 100 + mday;
  
  for (int j = selectname; j < num_names - 1; j++) {
    swap = order[j];
    order[j] = order[j + 1];
    order[j + 1] = swap;
  }

  write_names();
}

#ifdef PBL_COLOR
/******************************* Dictation API ********************************/

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "dictation_session_callback");
  APP_LOG(APP_LOG_LEVEL_INFO, "%s", transcription);
  editname = order[selectname];

  if(status == DictationSessionStatusSuccess) {
    // Display the dictated text
    snprintf(dict_text, sizeof(dict_text), "%s", transcription);
    strncpy(name_text[editname], dict_text, 10);
    text_layer_set_text(text_layer, MODES[mode]);
//  } else {
    // Display the reason for any error
//    static char s_failed_buff[128];
//    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);  
//    text_layer_set_text(s_output_layer, s_failed_buff);
  }
}
#endif

void handle_T3_close_edit(const char * text) {
  APP_LOG(APP_LOG_LEVEL_INFO, "handle_T3_close_edit");
  APP_LOG(APP_LOG_LEVEL_INFO, "%s", text);
  
  if (strncmp(text, "\0", 1) != 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "blank");
    strncpy(name_text[editname], text, 10);
    text_layer_set_text(text_layer, MODES[mode]);
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
  }
}

static void setting_reset() {
  APP_LOG(APP_LOG_LEVEL_INFO, "setting_reset");
  cookie = 0;
  for (int i = 0; i < num_names; i++) {
    order[i] = i;
    cookie += order[i] * power(num_names -1 - i);
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
  int j = 0;
  for (int i = 0; i < num_names; i++) {
    if (order[i] == num_names) j = 1;
    order[i] = order[i + j];
  }
  cookie = 0;
  for (int i = 0; i < num_names; i++) {
    cookie += order[i] * power(num_names - 1 - i);
  }
  layer_mark_dirty(draw_layer);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {

  mode++;
  if (mode > (NUM_MODES - 1)) mode = 0;
  APP_LOG(APP_LOG_LEVEL_INFO, "NUM: %d", NUM_MODES);
  APP_LOG(APP_LOG_LEVEL_INFO, "mode: %d", mode);
  text_layer_set_text(text_layer, MODES[mode]);
}

static void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  
  APP_LOG(APP_LOG_LEVEL_INFO, "mode: %d", mode);

  switch (mode) {
    case 0: //SELECT
      name_select();
    break;
    
#ifdef PBL_COLOR
    case 1: //VOICE
      dictation_session_start(dictation_session);
    break;
#endif
    
    case PBL_IF_COLOR_ELSE(2, 1): //EDIT
      name_edit();
    break;

#ifdef PBL_COLOR
    case 3: //COLOR
      color_select();
    break;
#endif

    case PBL_IF_COLOR_ELSE(4,2): //ADD
      name_add();
    break;
    
    case PBL_IF_COLOR_ELSE(5,3): //DELETE
      name_delete();
    break;
    
    case PBL_IF_COLOR_ELSE(6,4): //RESET
      setting_reset();
    break;
    
    case PBL_IF_COLOR_ELSE(7,5): //DATE
      date_format = (date_format == 0) ? 1 : 0;
      write_names();
    break;
    
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

static void list_update_proc(Layer *layer, GContext *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "list_update_proc");
  GRect bounds = layer_get_bounds(layer);
  for (int i = 0; i < num_names; i++) {
    if (layer == list_layer[i]) {
      APP_LOG(APP_LOG_LEVEL_INFO, "i: %d", i);
      APP_LOG(APP_LOG_LEVEL_INFO, "%d", last_int[order[i]]);
#ifdef PBL_COLOR
      graphics_context_set_fill_color(context, (GColor){.argb = name_col[order[i]]});
#else
      graphics_context_set_fill_color(context, GColorBlack);
#endif
      graphics_fill_rect(context, bounds, 0, GCornerNone);
      graphics_context_set_text_color(context, GColorWhite);
      graphics_draw_text(context, name_text[order[i]], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(PBL_IF_RECT_ELSE(10, 30),0,PBL_IF_RECT_ELSE(124, 120),29), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
      
      APP_LOG(APP_LOG_LEVEL_INFO, "update: %d, %d, %d, %s", i, order[i], last_int[order[i]], last_text[order[i]]);
      
      if (last_int[order[i]] > 0) {
        int last_m = last_int[order[i]] / 100;
        int last_d = last_int[order[i]] - last_m * 100;
        APP_LOG(APP_LOG_LEVEL_INFO, "d m: %d %d", last_d, last_m);
        if (date_format == 0) {
          snprintf(last_text[order[i]], 6, "%d/%d", last_d, last_m);
        } else {
          snprintf(last_text[order[i]], 6, "%d/%d", last_m, last_d);
        }
        graphics_draw_text(context, last_text[order[i]], fonts_get_system_font(FONT_KEY_GOTHIC_24), GRect(PBL_IF_RECT_ELSE(10, 30),0,PBL_IF_RECT_ELSE(124, 120),29), GTextOverflowModeFill, GTextAlignmentRight, NULL);
      }
    }
  }
}

static void window_load(Window *window) {
  
  APP_LOG(APP_LOG_LEVEL_INFO, "mode1: %d", mode);

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
    }
//    last_int[i] = 110 + i;
    
#ifdef PBL_COLOR
    name_col[i] = persist_exists(COLOR_KEY + i) ? persist_read_int(COLOR_KEY + i) : COLOR_DEFAULT;
//    name_col[i] = COLOR_DEFAULT;
#endif
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "mode2 : %d", mode);

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

  x[0] = PBL_IF_ROUND_ELSE(30, 10);
  y[0] = y_size;
  for (int i = 1; i < MAX_NUM_NAMES; i++) {
    x[i] = x[i-1];
    y[i] = y[i-1] + y_size;
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO, "mode3: %d", mode);
  
  for (int i = MAX_NUM_NAMES-1; i >= 0; i--) {
    list_layer[i] = layer_create((GRect){.origin = {0, y[i] }, .size = {bounds.size.w, 29}});
    layer_set_update_proc(list_layer[i], list_update_proc);
    layer_add_child(window_layer, list_layer[i]);
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO, "mode4: %d", mode);
  
  text_layer = text_layer_create((GRect){.origin = {0, 0}, .size = {bounds.size.w, 29}});
  text_layer_set_text(text_layer, MODES[mode]);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_text_color(text_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  APP_LOG(APP_LOG_LEVEL_INFO, "mode5:%d", mode);
  
  draw_layer = layer_create(bounds);//0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(draw_layer, draw_update_proc);
  layer_add_child(window_layer, draw_layer);
  
  write_names();
}

void window_unload(Window *window) {
  
  APP_LOG(APP_LOG_LEVEL_INFO, "unload1");
  persist_write_int(COOKIE_KEY, cookie);
  APP_LOG(APP_LOG_LEVEL_INFO, "unload2");
  persist_write_int(NUM_KEY, num_names);
  APP_LOG(APP_LOG_LEVEL_INFO, "unload3");
  persist_write_int(DATE_KEY, date_format);
  APP_LOG(APP_LOG_LEVEL_INFO, "unload4");
  for (int i = 0; i < num_names; i++) {
    APP_LOG(APP_LOG_LEVEL_INFO, "unload:%d", i);
    
    persist_write_string(NAME_KEY + i, name_text[i]);
    if (last_int[i] > 0) persist_write_int(LAST_KEY + i, last_int[i]);
#ifdef PBL_COLOR
    persist_write_int(COLOR_KEY + i, name_col[i]);
#endif
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "unload4.5");
  
  
  text_layer_destroy(text_layer);
  APP_LOG(APP_LOG_LEVEL_INFO, "unload6");
  for (int i = 0; i < MAX_NUM_NAMES; i++) {
    layer_destroy(list_layer[i]);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "unload7");
  layer_destroy(draw_layer);
  APP_LOG(APP_LOG_LEVEL_INFO, "unload8");

}

void init(void) {
  window = window_create();
#ifdef PBL_BW
//  window_set_fullscreen(window, true);
#endif
  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true /* Animated */);  
  
#ifdef PBL_COLOR
// Create new dictation session
dictation_session = dictation_session_create(sizeof(dict_text), 
                                               dictation_session_callback, NULL);
#endif

}

void deinit(void) {
#ifdef PBL_COLOR
    dictation_session_destroy(dictation_session);
#endif

  window_destroy(window);
  APP_LOG(APP_LOG_LEVEL_INFO, "finished");
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}