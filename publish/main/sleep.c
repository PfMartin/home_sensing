#include <stdio.h>
#include <time.h>

#include "lwip/netdb.h"
#include "esp_log.h"

#include "esp_sleep.h"


static RTC_DATA_ATTR struct timeval sleep_enter_time;

void handle_deep_sleep_wakeup(void) {
  struct timeval now;
  gettimeofday(&now, NULL);
  int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

  switch(esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_TIMER: {
      printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
      break;
    }
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      printf("Not a deep sleep reset\n");
  }
}

void set_wakeup_timer(int wakeup_time_sec) {
  printf("Enabling time wakeup, %ds\n", wakeup_time_sec);
  esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
}

void start_deep_sleep() {
  printf("Entering deep sleep\n");
  esp_deep_sleep_start();
}
