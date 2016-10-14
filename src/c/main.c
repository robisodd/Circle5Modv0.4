#include "main.h"
/*
20161014 v0.4: Getting rid of redrawing entire screen every minute
Now will save background bitmap for the hour
Will add bluetooth disconenct triangle
Will add options for white background


20161013 v0.3: Replaced GPath's with whole ring bitmaps
Cleaned up a lot of code, renders properly on Aplite-Basalt-Chalk-Diorite

2016xxxx v0.2: Battery animates while charing
Attempted to make rotated bitmaps for each digit and each rotation.

20160920 v0.1: Initial release to appstore


//http://i.imgur.com/Djvy1Jy.png

TODO:

Display ERROR if ever unable to load bitmap
White on black
Bluetooth disconnect triangle
Build static background



*/
#define REBUILD_BACKGROUND true
#define  STATIC_BACKGROUND false

static Window *main_window = NULL;
static Layer *root_layer = NULL;

static Layer *graphics_layer = NULL;

static GRect bounds;
static GPoint center;

static GBitmap *background_bitmap;
static GBitmap *cursor_bitmap;
// static GBitmap *large_bitmap;

GColor *palette;

static GBitmap *battery_bitmap, *date_bitmap, *day_bitmap;
static GBitmap *weekday_bitmap[7];
static GBitmap *datefont_bitmap[10];
static GBitmap *battsegs_bitmap[6];

uint32_t HOURS[13] = {
  RESOURCE_ID_LARGE1,
  RESOURCE_ID_LARGE1,
  RESOURCE_ID_LARGE2,
  RESOURCE_ID_LARGE3,
  RESOURCE_ID_LARGE4,
  RESOURCE_ID_LARGE5,
  RESOURCE_ID_LARGE6,
  RESOURCE_ID_LARGE7,
  RESOURCE_ID_LARGE8,
  RESOURCE_ID_LARGE9,
  RESOURCE_ID_LARGE10,
  RESOURCE_ID_LARGE11,
  RESOURCE_ID_LARGE12,
};

uint32_t TENS[6] = {
  RESOURCE_ID_TENS0,
  RESOURCE_ID_TENS1,
  RESOURCE_ID_TENS2,
  RESOURCE_ID_TENS3,
  RESOURCE_ID_TENS4,
  RESOURCE_ID_TENS5,
};

uint32_t ONES[10] = {
  RESOURCE_ID_ONES0,
  RESOURCE_ID_ONES1,
  RESOURCE_ID_ONES2,
  RESOURCE_ID_ONES3,
  RESOURCE_ID_ONES4,
  RESOURCE_ID_ONES5,
  RESOURCE_ID_ONES6,
  RESOURCE_ID_ONES7,
  RESOURCE_ID_ONES8,
  RESOURCE_ID_ONES9,
};


//bool js_connected=false;
bool phone_connected = true;
// uint8_t phone_battery_level=255;
// bool phone_charging=false;
BatteryChargeState watch_battery;// = {255, false, false};

static void dirty(bool rebuild) {
  //if (rebuild)
    // Delete background, set to NULL (test for null in drawing routine)
  
  if (root_layer)
    layer_mark_dirty(root_layer);
}


// ------------------------------------------------------------------------ //
//  Timer Functions
// ------------------------------------------------------------------------ //
static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  //TODO: If HOUR changed, redo background
  dirty(STATIC_BACKGROUND);
  //layer_mark_dirty(graphics_layer);  // Schedule redraw of screen
}

// ------------------------------------------------------------------------ //
//  Drawing Functions
// ------------------------------------------------------------------------ //



// Fill screen with color.  Note: For Aplite, color should be either 0 or 255. Vertical stripes will appear otherwise.
void fill_screen(GContext *ctx, GColor color) {
  uint32_t *framebuffer = (uint32_t*)*(uintptr_t*)ctx;
  #if defined(PBL_PLATFORM_APLITE)
    memset(framebuffer, color.argb, 20 * 168);
    graphics_release_frame_buffer(ctx, graphics_capture_frame_buffer(ctx));  // Needed on Aplite to force screen to draw
  
  // TODO: Test on Aplite with colors, might not work
  
  #elif defined(PBL_PLATFORM_BASALT)
    memset(framebuffer, color.argb, 144 * 168);
  #elif defined(PBL_PLATFORM_CHALK)
    memset(framebuffer, color.argb, 76 + 25792); // First pixel on PTR doesn't start til framebuffer + 76
  #endif
}

static void bitmap_set_color(GBitmap *bitmap, GColor color) {
//   gbitmap_get_palette(bitmap)[0] = (GColor){.argb=0b00111111};
  GColor *pal;
  switch (gbitmap_get_format(bitmap)) {
 		case GBitmapFormat1Bit:         break;
 		case GBitmapFormat8Bit:         break;
    case GBitmapFormat8BitCircular: break;
    
 		case GBitmapFormat4BitPalette: 
      pal = gbitmap_get_palette(bitmap);
      pal[0] = (GColor){.argb =  0b00111111};
    break;
    
    case GBitmapFormat1BitPalette: 
      pal = gbitmap_get_palette(bitmap);
      pal[0] = (GColor){.argb =  0b00111111};
      pal[1] = (GColor){.argb = (0b11000000 | (color.argb&63))};
    break;
    
		case GBitmapFormat2BitPalette:
      pal = gbitmap_get_palette(bitmap);
      pal[0] = (GColor){.argb =  0b00111111};
      pal[1] = (GColor){.argb = (0b01111111 | (color.argb&63))};
      pal[2] = (GColor){.argb = (0b10111111 | (color.argb&63))};
      pal[3] = (GColor){.argb = (0b11111111 | (color.argb&63))};
    break;
    
    default:
    break;
  }
}



void draw_resource(GContext *ctx, uint32_t resource) {
  GBitmap *bitmap = gbitmap_create_with_resource(resource);
//   GColor     *pal = gbitmap_get_palette(bitmap);
  GSize      size = gbitmap_get_bounds(bitmap).size;
  bitmap_set_color(bitmap, GColorWhite);
  
//   pal[0] = (GColor){.argb=0b00111111};
//   #if defined(PBL_COLOR)
//   pal[1] = (GColor){.argb=0b01111111};
//   pal[2] = (GColor){.argb=0b10111111};
//   pal[3] = (GColor){.argb=0b11111111};
//   #endif
  
  graphics_draw_bitmap_in_rect(ctx, bitmap, GRect(center.x + 33 - (size.w / 2), center.y - (size.h / 2), size.w, size.h));
  gbitmap_destroy(bitmap);
}

static void root_layer_update(Layer *layer, GContext *ctx) {
  int x = center.x; int y = center.y;
  
  time_t now = time(NULL);  // now = number of seconds since 00:00:00 UTC, Jan 1, 1970
  struct tm *local = localtime(&now);  // Current Watch Time
  //int seconds = local->tm_sec;
  //seconds = now;
  int n = now;
  
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_stroke_width(ctx, 1);
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);

  
  //fill_screen(ctx, GColorDukeBlue);
  
  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  // -----------------
  // Background
  // -----------------
  graphics_draw_bitmap_in_rect(ctx, background_bitmap, GRect(center.x - (144 / 2), center.y - (168 / 2), 144, 168));

  
    
  // -----------------
  //  Hour digits
  // -----------------
  n = ((now)%12)+1;
  //n = (local->tm_hour + 12) % 12;
  draw_resource(ctx, HOURS[n]);
  
  // -----------------
  // Left Triangle
  // -----------------
  graphics_draw_bitmap_in_rect(ctx, cursor_bitmap, GRect(center.x - 72, center.y - (10 / 2), 8, 10));
  
  // -----------------
  //  Minute Tens digits
  // -----------------
  n = now%6;
  //n = (local->tm_min)/10;
  //n = 5;
  draw_resource(ctx, TENS[n]);
  
  // -----------------
  //  Minutes ONES digits
  // -----------------
  n = now%10;
  //n = 8;
  //n = (local->tm_min%10);
  draw_resource(ctx, ONES[n]);
  
  // -----------------
  //  2-Letter Weekday
  // -----------------
  int wd = ((local->tm_wday) % 7); // %7 just in case
  graphics_draw_bitmap_in_rect(ctx, weekday_bitmap[wd], GRect(x + 25, y + 6, 16, 7));
  
  // -----------------
  //  2-Digit Date
  // -----------------
  int mday = ((local->tm_mday) % 99); // %99 just in case
  graphics_draw_bitmap_in_rect(ctx, datefont_bitmap[mday/10], GRect(x + 19, y - 10, 13, 13));
  graphics_draw_bitmap_in_rect(ctx, datefont_bitmap[mday%10], GRect(x + 34, y - 10, 13, 13));

  // -----------------
  //  Battery Graph
  // -----------------
  uint8_t batt = 5;
  if (watch_battery.charge_percent >= 90) batt = 0;
  else if (watch_battery.charge_percent >= 70) batt = 1;
  else if (watch_battery.charge_percent >= 50) batt = 2;
  else if (watch_battery.charge_percent >= 30) batt = 3;
  else if (watch_battery.charge_percent >= 20) batt = 4;
  else batt = 5;
    
    GRect bb = gbitmap_get_bounds(battsegs_bitmap[batt]);
    graphics_draw_bitmap_in_rect(ctx, battsegs_bitmap[batt], GRect(x + 36, y - 23 + (46 - bb.size.h), bb.size.w, bb.size.h));
}

// static void root_layer_update(Layer *layer, GContext *ctx) {
//   int x = center.x; int y = center.y;
  
//   time_t now = time(NULL);  // now = number of seconds since 00:00:00 UTC, Jan 1, 1970
//   struct tm *local = localtime(&now);  // Current Watch Time
//   //int seconds = local->tm_sec;
//   //seconds = now;
//   int n = now;
  
//   graphics_context_set_antialiased(ctx, true);
//   graphics_context_set_stroke_width(ctx, 1);
  
//   graphics_context_set_stroke_color(ctx, GColorWhite);
//   graphics_context_set_fill_color(ctx, GColorWhite);

  
//   fill_screen(ctx, GColorDukeBlue);
  
//   graphics_context_set_compositing_mode(ctx, GCompOpSet);

//   // -----------------
//   // Background
//   // -----------------
//   graphics_draw_bitmap_in_rect(ctx, background_bitmap, GRect(center.x - (144 / 2), center.y - (168 / 2), 144, 168));

  
    
//   // -----------------
//   //  Hour digits
//   // -----------------
//   n = ((now)%12)+1;
//   //n = (local->tm_hour + 12) % 12;

//   switch(n){
//     case 1: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE1); break;
//     case 2: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE2); break;
//     case 3: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE3); break;
//     case 4: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE4); break;
//     case 5: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE5); break;
//     case 6: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE6); break;
//     case 7: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE7); break;
//     case 8: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE8); break;
//     case 9: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE9); break;
//     case 10: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE10); break;
//     case 11: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE11); break;
//     case 12: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE12); break;
//   }
// //   printf("Hour Format: %s", get_gbitmapformat_text(gbitmap_get_format(large_bitmap)));
  
//   palette = gbitmap_get_palette(large_bitmap);
//   palette[0] = (GColor){.argb=0b00111111};
//   #if defined(PBL_COLOR)
//   palette[1] = (GColor){.argb=0b01111111};
//   palette[2] = (GColor){.argb=0b10111111};
//   palette[3] = (GColor){.argb=0b11111111};
//   #endif

//   //graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(6, -15, 198, 198));  // HOURS Perfect on Basalt
//   //graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(24, -9, 198, 198));  // Perfect on Chalk
  
//   //graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(center.x - 66, center.y - (198 / 2), 198, 198));
//   graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(center.x + 33 - (198 / 2), center.y - (198 / 2), 198, 198));

  
//   gbitmap_destroy(large_bitmap);
  
//   // -----------------
//   // Left Triangle
//   // -----------------
//   graphics_draw_bitmap_in_rect(ctx, cursor_bitmap, GRect(center.x - 72, center.y - (10 / 2), 8, 10));
  
// //   graphics_context_set_antialiased(ctx, false);
// //   graphics_context_set_stroke_width(ctx, 1);
// //   graphics_context_set_stroke_color(ctx, GColorWhite);
// //   graphics_context_set_fill_color(ctx, GColorWhite);
// //   //gpath_rotate_to(gp_triangle, TRIG_MAX_ANGLE * t->tm_min / 60);
// //   gpath_draw_filled(ctx, gp_triangle);
// //   gpath_draw_outline(ctx, gp_triangle);

  
//   // -----------------
//   //  Minute Tens digits
//   // -----------------
//   n = now%6;
//   //n = (local->tm_min)/10;
//   //n = 5;

//   switch(n){
//     case 0: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TENS0); break;
//     case 1: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TENS1); break;
//     case 2: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TENS2); break;
//     case 3: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TENS3); break;
//     case 4: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TENS4); break;
//     case 5: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TENS5); break;
//   }
// //   printf("Tens Format: %s", get_gbitmapformat_text(gbitmap_get_format(large_bitmap)));
  
//   palette = gbitmap_get_palette(large_bitmap);
//   palette[0] = (GColor){.argb=0b00111111};
//   #if defined(PBL_COLOR)
//   palette[1] = (GColor){.argb=0b01111111};
//   palette[2] = (GColor){.argb=0b10111111};
//   palette[3] = (GColor){.argb=0b11111111};
//   #endif

// //   graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(50, 29, 110, 110));  // basalt tens
//   //graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(24, -9, 198, 198));  // HOURS Perfect on Chalk
  
//   graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(center.x + 33 - (110 / 2), center.y - (110 / 2), 110, 110));
  
//   gbitmap_destroy(large_bitmap);
  
  
  
  
//    // -----------------
//   //  Minutes ONES digits
//   // -----------------
//   n = now%10;
//   //n = 8;
//   //n = (local->tm_min%10);
// //   n = 6;

//   switch(n){
//     case 0: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES0); break;
//     case 1: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES1); break;
//     case 2: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES2); break;
//     case 3: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES3); break;
//     case 4: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES4); break;
//     case 5: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES5); break;
//     case 6: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES6); break;
//     case 7: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES7); break;
//     case 8: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES8); break;
//     case 9: large_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ONES9); break;
//   }
// //   printf("Ones Format: %s", get_gbitmapformat_text(gbitmap_get_format(large_bitmap)));
  
//   palette = gbitmap_get_palette(large_bitmap);
//   palette[0] = (GColor){.argb=0b00111111};
//   #if defined(PBL_COLOR)
//   palette[1] = (GColor){.argb=0b01111111};
//   palette[2] = (GColor){.argb=0b10111111};
//   palette[3] = (GColor){.argb=0b11111111};
//   #endif

//   //graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(60, 39, 90, 90));  // One BASALT
//   //graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(24, -9, 198, 198));  // HOURS Perfect on Chalk
  
//   graphics_draw_bitmap_in_rect(ctx, large_bitmap, GRect(center.x + 33 - (90 / 2), center.y - (90 / 2), 90, 90));
  
//   gbitmap_destroy(large_bitmap);
  
  
    
    
  
  
//   int wd = ((local->tm_wday) % 7); // %7 just in case
//   graphics_draw_bitmap_in_rect(ctx, weekday_bitmap[wd], GRect(x + 25, y + 6, 16, 7));
  
//   int mday = ((local->tm_mday) % 99); // %99 just in case
//   graphics_draw_bitmap_in_rect(ctx, datefont_bitmap[mday/10], GRect(x + 19, y - 10, 13, 13));
//   graphics_draw_bitmap_in_rect(ctx, datefont_bitmap[mday%10], GRect(x + 34, y - 10, 13, 13));

//   graphics_context_set_compositing_mode(ctx, GCompOpOr);
//   uint8_t batt = 5;
//   if (watch_battery.charge_percent >= 90) batt = 0;
//   else if (watch_battery.charge_percent >= 70) batt = 1;
//   else if (watch_battery.charge_percent >= 50) batt = 2;
//   else if (watch_battery.charge_percent >= 30) batt = 3;
//   else if (watch_battery.charge_percent >= 20) batt = 4;
//   else batt = 5;
    
//     GRect bb = gbitmap_get_bounds(battsegs_bitmap[batt]);
//     graphics_draw_bitmap_in_rect(ctx, battsegs_bitmap[batt], GRect(x + 36, y - 23 + (46 - bb.size.h), bb.size.w, bb.size.h));
// }



void battery_handler(BatteryChargeState charge_state) {
  if (  // In case battery state didn't change
    watch_battery.charge_percent == charge_state.charge_percent &&
    watch_battery.is_charging    == charge_state.is_charging    &&
    watch_battery.is_plugged     == charge_state.is_plugged
  )
    return;
  
  // Save battery state
  watch_battery.charge_percent = charge_state.charge_percent;
  watch_battery.is_charging    = charge_state.is_charging;
  watch_battery.is_plugged     = charge_state.is_plugged;
  
  dirty(REBUILD_BACKGROUND);
  
  //printf("Watch Battery %s: %03d%%", watch_battery.is_charging?"Charging":watch_battery.is_plugged?"Powered":"Discharging", watch_battery.charge_percent);
}

void bluetooth_handler(bool connected) {
  if(connected) {
    phone_connected = true;
  } else {
    if (phone_connected)
      vibes_double_pulse();
    phone_connected = false;
  }
  dirty(STATIC_BACKGROUND);
}


// ------------------------------------------------------------------------ //
//  Main Functions
// ------------------------------------------------------------------------ //
static void main_window_load(Window *window) {
//   Layer *window_layer = window_get_root_layer(window);
//   GRect window_frame = layer_get_frame(window_layer);

  root_layer = window_get_root_layer(window);
  
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_background);
     battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_battery);
        date_bitmap = gbitmap_create_with_resource(RESOURCE_ID_date);
         day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_day);
  
  bitmap_set_color(background_bitmap, GColorWhite);
  bitmap_set_color(battery_bitmap, GColorWhite);
  bitmap_set_color(date_bitmap, GColorWhite);
  bitmap_set_color(day_bitmap, GColorWhite);
  
  for (int i = 0; i < 7; ++i)
    weekday_bitmap[i] = gbitmap_create_as_sub_bitmap(day_bitmap, GRect(i*16, 0, 16, 7));
  
  for (int i = 0; i < 10; ++i)
    datefont_bitmap[i] = gbitmap_create_as_sub_bitmap(date_bitmap, GRect(i*13, 0, 13, 13));
  
  //23 x 46
  battsegs_bitmap[0] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1,  0, 23, 46));
  battsegs_bitmap[1] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1,  6, 23, 40));
  battsegs_bitmap[2] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1, 17, 23, 29));
  battsegs_bitmap[3] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1, 30, 23, 16));
  battsegs_bitmap[4] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1, 39, 12,  7));
  battsegs_bitmap[5] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1,  0,  0,  0));
  
  
  cursor_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CURSOR);
  bitmap_set_color(cursor_bitmap, GColorRed);
//   palette = gbitmap_get_palette(cursor_bitmap);
//   palette[0] = (GColor){.argb=0b00111111};
   
  
  
  // IDEA: GRAY CHECKERBOARD BACKGROUND FOR APLTIE
//   layer_set_update_proc(root_layer, root_layer_update);
  graphics_layer = layer_create(layer_get_frame(root_layer));
  layer_set_update_proc(graphics_layer, root_layer_update);
  layer_add_child(root_layer, graphics_layer);
  window_set_background_color(main_window, GColorLightGray);
  window_set_background_color(main_window, GColorDukeBlue);
  
  
  bounds = layer_get_bounds(root_layer);
  center = grect_center_point(&bounds);

}

static void main_window_unload(Window *window) {
//   layer_destroy(graphics_layer);
}


static void init(void) {
//   init_digits();
  // Set up and push main window
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_background_color(main_window, GColorBlack);
  
//   tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

//   bool emulator = watch_info_get_model()==WATCH_INFO_MODEL_UNKNOWN;
//   if(emulator) {
//     printf("Emulator Detected: Turning Backlight On");
//     light_enable(true);
//   }
//   Had a user complain their backlight stayed on!
  
  //-----------------------------------------------------------------
  // Subscribe to Battery and Bluetooth and check current status
  //-----------------------------------------------------------------
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  //bluetooth_connection_service_subscribe(bluetooth_handler);
  connection_service_subscribe((ConnectionHandlers){bluetooth_handler, bluetooth_handler});
  bluetooth_handler(connection_service_peek_pebble_app_connection());
  
  //Begin
  window_stack_push(main_window, true /* Animated */); // Display window.  Callback will be called once layer is dirtied then written
}
  
static void deinit(void) {
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}