#include "main.h"

/*
------------------------------------------------------------------------------------------------
 Version History
------------------------------------------------------------------------------------------------
20161014 v0.4:
  Getting rid of redrawing entire screen every minute
  Now will save background bitmap for the hour
  Will add bluetooth disconenct triangle
  Will add options for white background
  Adding battery flashing code from v0.2
  Added shadows!
  Fixed shadows for Aplite

20161013 v0.3:
  Abandoned v0.2, reverted back from v0.1
  Replaced GPath's with whole ring bitmaps
  Cleaned up a lot of code, renders properly on Aplite-Basalt-Chalk-Diorite

2016xxxx v0.2:
  Battery animates while charing
  Attempted to make rotated bitmaps for each digit and each rotation.


20160920 v0.1:
  Initial release to appstore
  Rotating Digits rendered through GPaths

------------------------------------------------------------------------------------------------
 TODO
------------------------------------------------------------------------------------------------
Display ERROR if ever unable to load bitmap
White on black
Bluetooth disconnect triangle
Build static background

------------------------------------------------------------------------------------------------
 Clay Settings
------------------------------------------------------------------------------------------------
[Display Section]
Default Colors Dropdown:
  (Options shown on all Pebbles: Aplite, Basalt, Chalk, Diorite, [Emery])
  White on Black
  Black on White
  Original (Whtie on Black - no Shadows)
  (Options shown on Color Pebbles: Basalt, Chalk, [Emery])
  Colorful
  
  
 background_color
 dots_color
 cursor_color
 battery_color
 date_color
   Date Shadow Distance [0 - 4]
 day_color
   Day Shadow Distance [0 - 4]
 hours_color
   Hours Shadow Distance [0 - 4]
 minutes_color
   Minutes Shadow Distance [0 - 4]


[Display Options]
  Shadows [Enable / Disable]

[Vibration Options]:
  Vibrate on BT loss [no, short, double, long, double long, triple long]
  Vibrate on BT connected [no, short, double, long]
  Vibrate on Exiting Watchface [no, short, double, long] <- for accidental button presses
  Vibrate on Hourly [no, short, double-quick, double-, triple, long]
    Hourly: exclude quiet hours
  Vibrate 
  Vibrate on fully charged: [no, short, double, long, double-long]

[SUBMIT]
[CANCEL]
Version Text:
v0.4 2016/10/16
Rob Spiess

------------------------------------------------
Clay Ideas:
  Floating Submit button on bottom-right corner
  Floating TEST button on bottom-left corner
    Test button sends settings but doesn't save them in peristant storage.
    On Android: bring settings page back up immediately  (Test if possible on Apple)
    On Android: Cancel or back reverts tested settings
    Exiting and reentering watchface loads settings from watch's persistant storage.
  Cancel button on the very bottom in the middle so both floating buttons surround it
    


*/
// ========================================================================================================================= //
//  Globals and Defines
// ========================================================================================================================= //
static Layer  *graphics_layer = NULL;
static GPoint center;

static GBitmap *background_bitmap, *battery_bitmap, *date_bitmap, *day_bitmap, *cursor_bitmap;
static GBitmap *weekday_bitmap[7];
static GBitmap *datefont_bitmap[10];
static GBitmap *battsegs_bitmap[6];


GColor background_color;
GColor       dots_color;
GColor    battery_color;
GColor       date_color;
GColor        day_color;
GColor     cursor_color;
GColor      hours_color;
GColor    minutes_color;

int8_t date_shadow_distance = 1,
        day_shadow_distance = 1,
    minutes_shadow_distance = 1,
      hours_shadow_distance = 2;


AppTimer *battery_flashing_timer = null;
int phone_connected = 3;
BatteryChargeState watch_battery;
uint8_t watch_battery_icon = 0;
//bool js_connected=false;
// uint8_t phone_battery_level=255;
// bool phone_charging=false;


uint32_t HOURS[13] = {
  RESOURCE_ID_LARGE12,  // hour%12=0, meaning "12" on 12 hour and "0" on 24 hour
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





// ========================================================================================================================= //
//  Drawing Functions
// ========================================================================================================================= //

static void dirty() {
  if (graphics_layer)
    layer_mark_dirty(graphics_layer);
}


// -----------------------------------------------------------------


static void bitmap_set_color(GBitmap *bitmap, GColor color) {
//   gbitmap_get_palette(bitmap)[0] = GColorClear; return;
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
      if (color.argb > 63) color.argb -= 0b01000000;
      pal[2] = color;
      if (color.argb > 63) color.argb -= 0b01000000;
      pal[1] = color;
      pal[0] = GColorClear;
    break;
    
    default:
    break;
  }
}


// -----------------------------------------------------------------


static void draw_bitmap(GContext *ctx, GBitmap *bmp, GColor color, int16_t shadow_distance, int16_t x_offset, int16_t y_offset) {
  GSize size = gbitmap_get_bounds(bmp).size;
  
  // Shadow
  if (shadow_distance > 0 && color.argb > 127) {
    bitmap_set_color(bmp, PBL_IF_COLOR_ELSE((GColor){.argb = color.argb - 128}, GColorLightGray));
    graphics_draw_bitmap_in_rect(ctx, bmp, GRect(center.x + x_offset + shadow_distance + 33 - (size.w / 2), center.y + y_offset + shadow_distance - (size.h / 2), size.w, size.h));
  }

  // Actual Image
  bitmap_set_color(bmp, color);
  graphics_draw_bitmap_in_rect(ctx, bmp, GRect(center.x + x_offset + 33 - (size.w / 2), center.y + y_offset - (size.h / 2), size.w, size.h));
}


// -----------------------------------------------------------------


static void draw_resource(GContext *ctx, uint32_t resource, GColor color, int16_t shadow_distance) {
  // Create
  GBitmap *bmp = gbitmap_create_with_resource(resource);
  // Draw
  draw_bitmap(ctx, bmp, color, shadow_distance, 0, 0);
  // Destroy
  gbitmap_destroy(bmp);
}


// -----------------------------------------------------------------


static void graphics_layer_update(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);  // now = number of seconds since 00:00:00 UTC, Jan 1, 1970
  struct tm *local = localtime(&now);  // Current Watch Time
  int n = now;
  
  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  // -----------------
  // Background
  // -----------------
  graphics_draw_bitmap_in_rect(ctx, background_bitmap, GRect(center.x - (144 / 2), center.y - (168 / 2), 144, 168));
  
    
  // -----------------
  //  Left Triangle
  // -----------------
  if (phone_connected)
    graphics_draw_bitmap_in_rect(ctx, cursor_bitmap, GRect(center.x - 72, center.y - (10 / 2), 8, 10));
  
  
  // -----------------
  //  Hour digits
  // -----------------
  n = ((now)%12)+1;
  n = (local->tm_hour + 12) % 12;
  draw_resource(ctx, HOURS[n], hours_color, hours_shadow_distance);
  
  
  // -----------------
  //  Minute: TENS digits
  // -----------------
  n = now%6;
  n = (local->tm_min)/10;
  //n = 5;
  draw_resource(ctx, TENS[n], minutes_color, minutes_shadow_distance);
  
  
  // -----------------
  //  Minute: ONES digits
  // -----------------
  n = now%10;
  //n = 8;
  n = (local->tm_min%10);
  draw_resource(ctx, ONES[n], minutes_color, minutes_shadow_distance);
  
  
  // -----------------
  //  2-Letter Weekday
  // -----------------
  int wd = ((local->tm_wday) % 7); // %7 just in case
  draw_bitmap(ctx, weekday_bitmap[wd], day_color, day_shadow_distance, 0, 9);
  
    
  // -----------------
  //  2-Digit Date
  // -----------------
  int mday = ((local->tm_mday) % 99); // %99 just in case
  draw_bitmap(ctx, datefont_bitmap[mday/10], date_color, date_shadow_distance, -8, -4);
  draw_bitmap(ctx, datefont_bitmap[mday%10], date_color, date_shadow_distance, 7, -4);
  
  
  // -----------------
  //  Battery Graph
  // -----------------
  GRect bb = gbitmap_get_bounds(battsegs_bitmap[watch_battery_icon]);
  graphics_draw_bitmap_in_rect(ctx, battsegs_bitmap[watch_battery_icon], GRect(center.x + 36, center.y - 23 + (46 - bb.size.h), bb.size.w, bb.size.h));
}






// ========================================================================================================================= //
//  Callback Functions: Battery and Bluetooth and Tick and Dirty
// ========================================================================================================================= //
#define UPDATE_MS 500 // Refresh rate in milliseconds


static void update_battery_icon(void *data) {
  if (watch_battery.is_charging || watch_battery.is_plugged) {
    if (watch_battery.is_charging) {
      // Charging: Animate Battery Graph
      watch_battery_icon = (watch_battery_icon + 5) % 6;
    } else {
      // Plugged in (but not charging): Flash 100%
      watch_battery_icon = watch_battery_icon == 0 ? 5 : 0;
    }
    // Schedule a callback
    battery_flashing_timer = app_timer_register(UPDATE_MS, update_battery_icon, null);
    // Redraw screen (only if on external power)
    dirty();
  } else {
    // Not Plugged in or charging: Display current battery level
    battery_flashing_timer = null;
    if      (watch_battery.charge_percent >= 90) watch_battery_icon = 0;
    else if (watch_battery.charge_percent >= 70) watch_battery_icon = 1;
    else if (watch_battery.charge_percent >= 50) watch_battery_icon = 2;
    else if (watch_battery.charge_percent >= 30) watch_battery_icon = 3;
    else if (watch_battery.charge_percent >= 20) watch_battery_icon = 4;
    else                                         watch_battery_icon = 5;
  }

  // Don't dirty if not on external power,
  //   battery will update next minute, which is fast enough.
  //dirty();
}


// -----------------------------------------------------------------


void battery_handler(BatteryChargeState charge_state) {
  // -------------------------------------
  //  Check if battery state didn't change
  // -------------------------------------
  if (watch_battery.charge_percent == charge_state.charge_percent &&
      watch_battery.is_charging    == charge_state.is_charging    &&
      watch_battery.is_plugged     == charge_state.is_plugged
     )
    return;
  
  // -------------------------------------
  //  Save battery state
  // -------------------------------------
  watch_battery.charge_percent = charge_state.charge_percent;
  watch_battery.is_charging    = charge_state.is_charging;
  watch_battery.is_plugged     = charge_state.is_plugged;
  
  //printf("Watch Battery %s: %03d%%", watch_battery.is_charging?"Charging":watch_battery.is_plugged?"Powered":"Discharging", watch_battery.charge_percent);
  
  // -------------------------------------
  //  Update Battery Icon
  //  (if it's not already scheduled to update)
  // -------------------------------------
  if (!battery_flashing_timer)
    update_battery_icon(null);
}


// -----------------------------------------------------------------


void bluetooth_handler(bool connected) {
  if(connected) {
    if (phone_connected == 0)
      if (false)
        vibes_double_pulse();
    phone_connected = 1;
  } else {
    if (phone_connected == 1)
      if (true)
        vibes_double_pulse();
    phone_connected = 0;
  }
  
  dirty();
}


// -----------------------------------------------------------------


static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  //TODO: If HOUR changed, redo background
    // Delete background, set to NULL
    // Test for null in drawing routine
    // Drawing routine will rebuild background
  
  dirty();
  //layer_mark_dirty(graphics_layer);  // Schedule redraw of screen
}

// ========================================================================================================================= //
//  Main Init/Deinit Functions
// ========================================================================================================================= //

static void init_colors() {
//  background_color = GColorBlack;
//  dots_color = GColorYellow;
//  battery_color = GColorGreen;
//  date_color = GColorOrange;
//  day_color = GColorPurple;
//  cursor_color = GColorRed;
//  hours_color = GColorGreen;
//  minutes_color = GColorCyan;
  
//  background_color = GColorWhite;
//  dots_color = GColorBlack;
//  battery_color = GColorBlack;
//  date_color = GColorBlack;
//  day_color = GColorBlack;
//  cursor_color = GColorBlack;
//  hours_color = GColorBlack;
//  minutes_color = GColorBlack;

 background_color = GColorBlack;
 dots_color = GColorWhite;
 battery_color = GColorWhite;
 date_color = GColorWhite;
 day_color = GColorWhite;
 cursor_color = GColorWhite;
 hours_color = GColorWhite;
 minutes_color = GColorWhite;
  
     date_shadow_distance = PBL_IF_BW_ELSE(0, 1);
      day_shadow_distance = PBL_IF_BW_ELSE(0, 1);
  minutes_shadow_distance = PBL_IF_BW_ELSE(0, 1);
    hours_shadow_distance = PBL_IF_BW_ELSE(0, 2);
}


// -----------------------------------------------------------------


static void window_load(Window *window) {
  // -------------------------------------
  // Subscribe to Battery, Bluetooth and Tick Services
  // -------------------------------------
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  //tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  connection_service_subscribe((ConnectionHandlers){bluetooth_handler, bluetooth_handler});
  bluetooth_handler(connection_service_peek_pebble_app_connection());
  
  
  // -------------------------------------
  //  Load Images
  // -------------------------------------
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_background);
     battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_battery);
        date_bitmap = gbitmap_create_with_resource(RESOURCE_ID_date);
         day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_day);
      cursor_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CURSOR);
  
  
  // -------------------------------------
  //  Assign Colors to Images
  // -------------------------------------
  //TODO: Some of these are unnecessary as they are redone during layer update
  init_colors();
  bitmap_set_color(background_bitmap, dots_color);
  bitmap_set_color(battery_bitmap, battery_color);
  bitmap_set_color(date_bitmap, date_color);
  bitmap_set_color(day_bitmap, day_color);
  bitmap_set_color(cursor_bitmap, cursor_color);
  
  
  // -------------------------------------
  // Isolate 2-letter weekday bitmaps
  // -------------------------------------
  for (int i = 0; i < 7; ++i)
    weekday_bitmap[i] = gbitmap_create_as_sub_bitmap(day_bitmap, GRect(i * 16, 0, 16, 7));
  
  
  // -------------------------------------
  // Isolate Date Digits
  // -------------------------------------
  for (int i = 0; i < 10; ++i)
    datefont_bitmap[i] = gbitmap_create_as_sub_bitmap(date_bitmap, GRect(i * 13, 0, 13, 13));
  
  
  // -------------------------------------
  // Isolate Battery Segment Bitmaps
  // -------------------------------------
  battsegs_bitmap[0] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1,  0, 23, 46)); // 5 Segments (100%)
  battsegs_bitmap[1] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1,  6, 23, 40)); // 4 Segments
  battsegs_bitmap[2] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1, 17, 23, 29)); // 3 Segments
  battsegs_bitmap[3] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1, 30, 23, 16)); // 2 Segments
  battsegs_bitmap[4] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1, 39, 12,  7)); // 1 Semgnet
  battsegs_bitmap[5] = gbitmap_create_as_sub_bitmap(battery_bitmap, GRect(1,  0,  0,  0)); // Blank
  
  
  // -------------------------------------
  // Create and Setup Drawing Layer
  // Note: Leave root layer for option of "gray checkerboard" background on B&W
  // -------------------------------------
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);
  center = grect_center_point(&bounds);

  // Move a little to the right on ROUND
  #if defined(PBL_PLATFORM_CHALK)
  center.x += 7;
  #endif
  
  graphics_layer = layer_create(bounds);
  layer_set_update_proc(graphics_layer, graphics_layer_update);
  layer_add_child(root_layer, graphics_layer);
  
  window_set_background_color(window, background_color);
}


// -----------------------------------------------------------------


static void window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  layer_destroy(graphics_layer);
}


// -----------------------------------------------------------------


int main(void) {
  // -------------------------------------
  // Init: Set up and push main window
  // -------------------------------------
  Window *window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);

  // -------------------------------------
  // Begin Program
  // -------------------------------------
  app_event_loop();
  
  // -------------------------------------
  // Deinit: Destroy main window
  // -------------------------------------
  window_destroy(window);
}