#include "spod_library.h"
#include "esp_sleep.h"
#include "soc/rtc.h"            // for wakeup trigger defines
#include "soc/rtc_periph.h"     // for read rtc registers directly (cause)
#include "soc/soc.h"            // for direct register read macros

static const char *TAG = "SLEEP";


char* sleep_wakeup_reason()
{
	static char unknown[64];
	esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
	switch (reason) {
	case ESP_SLEEP_WAKEUP_EXT0:     return (char*)"EXT0";
	case ESP_SLEEP_WAKEUP_EXT1:     return (char*)"EXT1";
	case ESP_SLEEP_WAKEUP_TIMER:    return (char*)"TIMER";
	case ESP_SLEEP_WAKEUP_TOUCHPAD: return (char*)"TOUCHPAD";
	case ESP_SLEEP_WAKEUP_ULP:      return (char*)"ULP";
	default:
		snprintf(unknown, sizeof(unknown), "SLEEP_WAKEUP %d", reason);
		return unknown;
	}
}


void enter_deep_sleep(int msecs)
{
	esp_sleep_enable_timer_wakeup(msecs * 1000);
	esp_deep_sleep_start();
	LOGW(TAG, "%s(%d): wakeup reason '%s'\r\n", __FUNCTION__, msecs, sleep_wakeup_reason());
}


void enter_light_sleep(int msecs)
{
	esp_sleep_enable_timer_wakeup(msecs * 1000);
	esp_light_sleep_start();
	LOGW(TAG, "%s(%d): wakeup reason '%s'\r\n", __FUNCTION__, msecs, sleep_wakeup_reason());
}


void sleep_testing(const char* mode, void (*sleep_func)(int msecs))
{
	LOGW(TAG, "Entering %s SLEEP for 5 seconds!\r\n", mode);
	sleep_func(5000);
	LOGW(TAG, "Back from %s SLEEP!\r\n", mode);
}


void deep_sleep_testing(void)
{
	sleep_testing("DEEP", enter_deep_sleep);
}


void light_sleep_testing(void)
{
	sleep_testing("LIGHT", enter_light_sleep);
}
