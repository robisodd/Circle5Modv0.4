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
static GBitmap *battery_bitmap, *date_bitmap, *day_bitmap, *cursor_bitmap;
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

GColor background_color;// = GColorBlack;
GColor dots_color;// = GColorYellow;
GColor battery_color;// = GColorGreen;
GColor date_color;// = GColorOrange;
GColor day_color;// = GColorPurple;
GColor cursor_color;// = GColorRed;
GColor hours_color;// = GColorLightGray;
GColor minutes_color;// = GColorCyan;

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
// void fill_screen(GContext *ctx, GColor color) {
//   uint32_t *framebuffer = (uint32_t*)*(uintptr_t*)ctx;
//   #if defined(PBL_PLATFORM_APLITE)
//     memset(framebuffer, color.argb, 20 * 168);
//     graphics_release_frame_buffer(ctx, graphics_capture_frame_buffer(ctx));  // Needed on Aplite to force screen to draw
  
//   // TODO: Test on Aplite with colors, might not work
  
//   #elif defined(PBL_PLATFORM_BASALT)
//     memset(framebuffer, color.argb, 144 * 168);
//   #elif defined(PBL_PLATFORM_CHALK)
//     memset(framebuffer, color.argb, 76 + 25792); // First pixel on PTR doesn't start til framebuffer + 76
//   #endif
// }

// static void bitmap_set_color(GBitmap *bitmap, GColor color) {
// //   gbitmap_get_palette(bitmap)[0] = (GColor){.argb=0b00111111};
//   GColor *pal;
//   switch (gbitmap_get_format(bitmap)) {
//  		case GBitmapFormat1Bit:         break;
//  		case GBitmapFormat8Bit:         break;
//     case GBitmapFormat8BitCircular: break;
    
//  		case GBitmapFormat4BitPalette: 
//       pal = gbitmap_get_palette(bitmap);
//       pal[0] = (GColor){.argb =  0b00111111};
//     break;
    
//     case GBitmapFormat1BitPalette: 
//       pal = gbitmap_get_palette(bitmap);
//       pal[0] = (GColor){.argb =  0b00111111};
//       pal[1] = (GColor){.argb = (0b11000000 | (color.argb&63))};
//     break;
    
// 		case GBitmapFormat2BitPalette:
//       pal = gbitmap_get_palette(bitmap);
//       pal[0] = (GColor){.argb =  0b00111111};
//       pal[1] = (GColor){.argb = (0b01111111 | (color.argb&63))};
//       pal[2] = (GColor){.argb = (0b10111111 | (color.argb&63))};
//       pal[3] = (GColor){.argb = (0b11111111 | (color.argb&63))};
//     break;
    
//     default:
//     break;
//   }
// }

static void bitmap_set_argb(GBitmap *bitmap, GColor color) {
//   gbitmap_get_palette(bitmap)[0] = (GColor){.argb=0b00111111};
  GColor *pal;
  switch (gbitmap_get_format(bitmap)) {
 		case GBitmapFormat1Bit:         break;
 		case GBitmapFormat8Bit:         break;
    case GBitmapFormat8BitCircular: break;
    
 		case GBitmapFormat4BitPalette: 
      pal = gbitmap_get_palette(bitmap);
      pal[0] = GColorClear;
    break;
    
    case GBitmapFormat1BitPalette: 
      pal = gbitmap_get_palette(bitmap);
      pal[0] = GColorClear;
      pal[1] = color;
    break;
    
		case GBitmapFormat2BitPalette:
      pal = gbitmap_get_palette(bitmap);
      pal[3] = color;
      if(color.argb >63) color.argb -= 0b01000000;
      pal[2] = color;
      if(color.argb >63) color.argb -= 0b01000000;
      pal[1] = color;
      pal[0] = GColorClear;
    break;
    
    default:
    break;
  }
}


void draw_bitmap(GContext *ctx, GBitmap *bmp, GColor color, int16_t shadow_distance, int16_t x_offset, int16_t y_offset) {
  GSize   size = gbitmap_get_bounds(bmp).size;

  // Shadow
  if(shadow_distance>0 && color.argb > 127) {
    bitmap_set_argb(bmp, (GColor){.argb = color.argb - 128});
    graphics_draw_bitmap_in_rect(ctx, bmp, GRect(center.x + x_offset + shadow_distance + 33 - (size.w / 2), center.y + y_offset + shadow_distance - (size.h / 2), size.w, size.h));
  }

  // Actual Image
  //bitmap_set_color(bmp, color);
  bitmap_set_argb(bmp, color);
  graphics_draw_bitmap_in_rect(ctx, bmp, GRect(center.x + x_offset + 33 - (size.w / 2), center.y + y_offset - (size.h / 2), size.w, size.h));
}


void draw_resource(GContext *ctx, uint32_t resource, GColor color, int16_t shadow_distance) {
  // Create
  GBitmap *bmp = gbitmap_create_with_resource(resource);
  
  // Draw
  draw_bitmap(ctx, bmp, color, shadow_distance, 0, 0);
  
  // Destroy
  gbitmap_destroy(bmp);
}

static void root_layer_update(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);  // now = number of seconds since 00:00:00 UTC, Jan 1, 1970
  struct tm *local = localtime(&now);  // Current Watch Time
  //int seconds = local->tm_sec;
  //seconds = now;
  int n = now;
  
//   graphics_context_set_antialiased(ctx, true);
//   graphics_context_set_stroke_width(ctx, 1);
  
//   graphics_context_set_stroke_color(ctx, GColorWhite);
//   graphics_context_set_fill_color(ctx, GColorWhite);

  
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
  draw_resource(ctx, HOURS[n], hours_color, 2);
  
  // -----------------
  // Left Triangle
  // -----------------
  if (phone_connected)
    graphics_draw_bitmap_in_rect(ctx, cursor_bitmap, GRect(center.x - 72, center.y - (10 / 2), 8, 10));
  
  // -----------------
  //  Minute Tens digits
  // -----------------
  n = now%6;
  //n = (local->tm_min)/10;
  //n = 5;
  draw_resource(ctx, TENS[n], minutes_color, 1);
  
  // -----------------
  //  Minutes ONES digits
  // -----------------
  n = now%10;
  //n = 8;
  //n = (local->tm_min%10);
  draw_resource(ctx, ONES[n], minutes_color, 1);
  
  // -----------------
  //  2-Letter Weekday
  // -----------------
  int wd = ((local->tm_wday) % 7); // %7 just in case
  //graphics_draw_bitmap_in_rect(ctx, weekday_bitmap[wd], GRect(center.x + 25, center.y + 6, 16, 7));
  draw_bitmap(ctx, weekday_bitmap[wd], day_color, 1, 0, 9);
  
    
  // -----------------
  //  2-Digit Date
  // -----------------
  int mday = ((local->tm_mday) % 99); // %99 just in case
//   graphics_draw_bitmap_in_rect(ctx, datefont_bitmap[mday/10], GRect(center.x + 19, center.y - 10, 13, 13));
//   graphics_draw_bitmap_in_rect(ctx, datefont_bitmap[mday%10], GRect(center.x + 34, center.y - 10, 13, 13));
  draw_bitmap(ctx, datefont_bitmap[mday/10], date_color, 1, -8, -4);
  draw_bitmap(ctx, datefont_bitmap[mday%10], date_color, 1, 7, -4);

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
    graphics_draw_bitmap_in_rect(ctx, battsegs_bitmap[batt], GRect(center.x + 36, center.y - 23 + (46 - bb.size.h), bb.size.w, bb.size.h));
}


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
  root_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(root_layer);
  center = grect_center_point(&bounds);
  #if defined(PBL_PLATFORM_CHALK)
  center.x += 7;
  #endif

  // IDEA: GRAY CHECKERBOARD BACKGROUND FOR APLTIE
//   layer_set_update_proc(root_layer, root_layer_update);
  graphics_layer = layer_create(layer_get_frame(root_layer));
  layer_set_update_proc(graphics_layer, root_layer_update);
  layer_add_child(root_layer, graphics_layer);
  window_set_background_color(main_window, background_color);

  
  // -----------------
  //  Load Images
  // -----------------
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_background);
     battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_battery);
        date_bitmap = gbitmap_create_with_resource(RESOURCE_ID_date);
         day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_day);
  
  bitmap_set_argb(background_bitmap, dots_color);
  bitmap_set_argb(battery_bitmap, battery_color);
  bitmap_set_argb(date_bitmap, date_color);
  bitmap_set_argb(day_bitmap, day_color);
  
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
  bitmap_set_argb(cursor_bitmap, cursor_color);
}

static void main_window_unload(Window *window) {
   layer_destroy(root_layer);
}

static void init_colors() {
//  background_color = GColorBlack;
//  dots_color = GColorYellow;
//  battery_color = GColorGreen;
//  date_color = GColorOrange;
//  day_color = GColorPurple;
//  cursor_color = GColorRed;
//  hours_color = GColorBlue;
//  minutes_color = GColorCyan;
  background_color = GColorWhite;
 dots_color = GColorBlack;
 battery_color = GColorBlack;
 date_color = GColorBlack;
 day_color = GColorBlack;
 cursor_color = GColorBlack;
 hours_color = GColorBlack;
 minutes_color = GColorBlack;
}

static void init(void) {
  init_colors();
  
  // Set up and push main window
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_background_color(main_window, GColorBlack);
  
//   tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

  //-----------------------------------------------------------------
  // Subscribe to Battery and Bluetooth and check current status
  //-----------------------------------------------------------------
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

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