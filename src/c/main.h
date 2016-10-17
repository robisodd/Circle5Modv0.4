#pragma once
#include <pebble.h>
#define null NULL

/*
index.js
var Clay = require('pebble-clay');

main.h
#define SETTINGS_VERSION_KEY 1
#define SETTINGS_KEY 2

#define VIBRATION_NONE

// A structure containing our settings
typedef struct ClaySettings {
  GColor BackgroundColor;
  GColor ForegroundColor;
  bool SecondTick;
  int StepType;
  int connect_vibration;
  int disconnect_vibration
    
} __attribute__((__packed__)) ClaySettings;





main.c

// ========================================================================================================================= //
//  Watchface Settings
// ========================================================================================================================= //
ClaySettings settings;

// Read settings from persistent storage
static void load_settings() {
//   // Load the default settings
//   settings.BackgroundColor = (GColor){.argb = 0b11000110};//GColorDukeBlue;
//   settings.ForegroundColor = GColorWhite;
//   settings.SecondTick = false;
//   settings.StepType = 0;

//   // Read settings from persistent storage, if they exist
//   persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void save_settings() {
//   persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Handle the response from AppMessage
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
//   Tuple *option_seconds, *option_steptype;
  
// // Background Color
// //   if (bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor))
// //     settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);

//   // Foreground Color
// //   if (fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor))
// //     settings.ForegroundColor = GColorFromHEX(fg_color_t->value->int32);

//   // Second Tick
//   if ((option_seconds = dict_find(iter, MESSAGE_KEY_SecondTick))) {
//     settings.SecondTick = option_seconds->value->int32 == 1;
    
//     // Apply new timer settings
//     tick_timer_service_unsubscribe();
//     if (settings.SecondTick)
//       tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
//     else
//       tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
//   }

//   // Step Type
//   if ((option_steptype = dict_find(iter, MESSAGE_KEY_StepType)))
//     settings.StepType = option_steptype->value->int32;

//   // Save the new settings to persistent storage
//   save_settings();
  
//   // Update the display with the new settings
//   layer_mark_dirty(root_layer);
}
*/
