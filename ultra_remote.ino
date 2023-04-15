#if defined(ESP32_2)
//#   define TESTER_MODE                      // continuous beep for signal strength analysis
#   define DEBUG 2
#else
this may not happen
#endif

#include "cfg.h"
#include "mlcf.h"
#include "mota.h"
#include <esp_wifi.h>                   // required by esp_wifi_stop()

_i8 started_wifi;
_u8 key;

void
init_key()
{
    pinMode(BUTTON, INPUT_PULLUP);    // ext0_wakeup()
}

_u8
scan_key()
{
    return !digitalRead(BUTTON);
}

void 
setup()
{
    Serial.begin(115200);
if (DEBUG > 5) TP05
    init_key();
    key = scan_key();

#ifdef TESTER_MODE
    key = 1; bootCount = 1;
#endif

if (DEBUG > 5) Serial.printf("A key: 0x%x\n", key);
    if (key && !bootCount) {
        if (!mysetup_intro(OTA_SSID, __FILE__, 0)) {
            myota(OTA_PERIOD);
        }
        esp_restart();  // as a side effect resets bootCount to 1 for the next boot
                        // what happens if OTA were run successfully
                        // wouldn't reach this point in that case anyway
                        // we must do this to allow SSID and PASS being reprogrammed
                        // to standard values in case OTA did not take place and/or failed
    } else if (!key) {
        started_wifi = !mysetup_intro(NA_SSID, __FILE__, 0);  // do not activate WiFi -> started_wifi == 0
    } else {
        started_wifi = !mysetup_intro(ROTA2G_SSID, __FILE__, 0);
    }
}

/*
 * typical timing profiles
 *
 * [ /tmp/mkESP/ultra_door_1b_esp32/ultra_door_1b.ino.cpp on esp32_2 ] ready to operate, WiFi stat: WL_CONNECTED
 * <TP02: 186>
 * <cmd: ca ^
 * .stat: #[XX]#[0]#[0]#[xxx]#[0]#[0]
 * >
 * <TP03: 239>
 * key RELEASED
 * <cmd: ci ^
 * .stat: #[XX]#[0]#[0]#[xxx]#[0]#[0]
 * >
 * <TP04: 280>
 * 
 * EV: 284 Disconnected from WiFi access point
 * EV: 306 WiFi clients stopped
 * <TP14: 315>
 */
void 
loop()
{
    _i32 err = 0;

if (DEBUG > 5) TP05
#ifdef TESTER_MODE
    delay(500);
#endif
    if (started_wifi) {
        _i8 _buf[64];

        if (!err && mysend("@beep= f:1000 c:1 t:.05 p:.25 g:-20 ^roja ^", TARGET_HOST, TARGET_PORT, 0)) {
            Serial.printf("could not issue intro cmd\n");
            ++err;
        }
if (DEBUG) Serial.printf("<TP03: %u>\n", millis());         // <= ESSENTIAL TP03: wakeup to status
if (DEBUG > 1) Serial.printf("RSSI: %d\n", WiFi.RSSI());
        while (key = scan_key()) {
if (DEBUG > 5) Serial.printf("C key: 0x%x\n", key);
            delay(100);
        }
#ifdef TESTER_MODE
        delay(500);
#endif
if (DEBUG > 1) Serial.printf("key RELEASED\n");
        sprintf(_buf, "@beep= f:%d c:1 t:.05 p:.25 g:-20 ^roja ^", 1000 + 4 * (-WiFi.RSSI() - 60));
        if (!err && mysend(_buf, TARGET_HOST, TARGET_PORT, 0)) {
            Serial.printf("could not issue extro cmd\n");
            ++err;
        }
    }
#ifndef TESTER_MODE
    esp_wifi_stop();
    esp_sleep_enable_ext0_wakeup(BUTTON, 0);              // 1 = High, 0 = Low     
#endif
    beep_sync();  // flush all tones collected so far
if (DEBUG > 5) Serial.printf("<TP14: %u>\n", millis());
    Serial.flush();
#ifndef TESTER_MODE
    esp_deep_sleep_start();
#endif
}

