#if defined(ESP32_2)

#   define DEBUG    2
//    #define BUZZER  2

#elif defined(ESP32_10)

#   define DEBUG     0
#   define BUZZER    2

#else
this may not happen
#endif

// sense keys pulled down by 100k (no key pressed)
#define KEY_SNS_0 25
#define KEY_SNS_1 26
#define KEY_SNS_2 27
#define KEY_SNS_3 14

// feeder keys pulled up by 51k (no key pressed)
// sense keys connect to feeders on cross point 
// when the corresponding key is depressed
// causing a 'high' level being detected to wakeup.
// after this the matrix is scanned.
#define KEY_HOT_UL 33
#define KEY_HOT_UR 32
#define KEY_HOT_LL 4 
#define KEY_HOT_LR 12

#define KEY_NONE 0xff

#ifdef MCFG_LOCAL
#include "mcfg_local.h"
#include "mcfg_locals.h"
#else
#include "mcfg.h"
#endif
#include "mlcf.h"
#include "mota.h"
#include <esp_wifi.h>

#define KEY_SNS_MASK (1 << KEY_SNS_0 | \
                      1 << KEY_SNS_1 | \
                      1 << KEY_SNS_2 | \
                      1 << KEY_SNS_3)

_u8 key_sns[] = {   // rows
    KEY_SNS_0,
    KEY_SNS_1,
    KEY_SNS_2,
    KEY_SNS_3,
};

_u8 key_hot[] = {   // cols
    KEY_HOT_UL,
    KEY_HOT_UR,
    KEY_HOT_LL,
    KEY_HOT_LR,
};

/*
00 10
01 11
02 12
03 13
20 30
21 31
22 32
23 33
*/
#define KEY_11 0x00
#define KEY_12 0x01
#define KEY_13 0x02
#define KEY_14 0x03
#define KEY_15 0x20
#define KEY_16 0x21
#define KEY_17 0x22
#define KEY_18 0x23
#define KEY_01 0x10
#define KEY_02 0x11
#define KEY_03 0x12
#define KEY_04 0x13
#define KEY_05 0x30
#define KEY_06 0x31
#define KEY_07 0x32
#define KEY_08 0x33

_i8 started_wifi;
_u8 key;

void
exec_cmd(_u8 key)
{
    _u8p cmd = 0;

    if (key == KEY_11) {
        cmd = "zp ^";
    } else if (key == KEY_01) {
        cmd = "zm ^";
    } else if (key == KEY_12) {
//        cmd = "zh ^";
        cmd = "@vol=5%- ^roja ^";
    } else if (key == KEY_02) {
//        cmd = "zk ^";
        cmd = "@vol=5%+ ^roja ^";
    } else if (key == KEY_13) {
        cmd = "zb ^";
    } else if (key == KEY_03) {
        cmd = "zn ^";
    } else if (key == KEY_14) {
        cmd = "zc ^";
    } else if (key == KEY_04) {
        cmd = "zx ^";
    } else if (key == KEY_15) {
        cmd = "zt ^";
    } else if (key == KEY_05) {
        cmd = "zr ^";
    } else if (key == KEY_16) {
        cmd = "cq ^";
    } else if (key == KEY_06) {
        cmd = "zv ^";
    } else if (key == KEY_17) {
        cmd = "cu ^";
    } else if (key == KEY_07) {
//        cmd = "zj ^";
        cmd = "@vol=60% ^roja ^";
    } else if (key == KEY_18) {
        cmd = "cy ^";
    } else if (key == KEY_08) {
        cmd = "xc ^";
    } else {
        Serial.printf("illeg key: %02x\n", key);
    }
    if (cmd) {
        mysend(cmd, TARGET_HOST, TARGET_PORT, 0);
    }
}

#if defined(ESP32_2)

#define KEY_FAKE_0 4
#define KEY_FAKE_1 2

void
init_matrix()
{
    pinMode(KEY_FAKE_0, INPUT_PULLUP);      // ext0_wakeup(), has no external pull-Rs
    pinMode(KEY_FAKE_1, INPUT);             // ext1_wakeup() [ not used here ], with external 470k PULLDOWN
}

_u8
scan_matrix()
{
    if (digitalRead(KEY_FAKE_0)) {      // low if pressed (internal pull-up)
        return KEY_NONE;                
    } else {
        if (digitalRead(KEY_FAKE_1)) {  // high if pressed
            return KEY_07;  // simulate mis volume
        } else {
            return KEY_11;  // simulate player pause
        }
    }
}

#else

void
init_matrix()
{
    _u8 h, s;

    for (h = 0; h < _NE(key_hot); ++h) {
        pinMode(key_hot[h], OUTPUT);
    }
    for (s = 0; s < _NE(key_sns); ++s) {
#if defined(ESP32_2)
        pinMode(key_sns[s], INPUT_PULLDOWN);     // no external circuitry avail
#else
        pinMode(key_sns[s], INPUT);
#endif
    }
}

_u8
scan_matrix()
{
    _u8 h, s, i, key;

    key = KEY_NONE;
    for (h = 0; h < _NE(key_hot); ++h) {
        for (i = 0; i < _NE(key_hot); ++i) {
            digitalWrite(key_hot[i], h == i);
        }
        for (s = 0; s < _NE(key_sns); ++s) {
            if (digitalRead(key_sns[s])) {
                key = h << 4 | s;
                goto end;
            }
        }
    }
end:
    return key;
}

#endif

void 
setup()
{
    Serial.begin(115200);
    init_matrix();
    key = scan_matrix();

if (DEBUG > 5) Serial.printf("A key: 0x%x\n", key);
    if (key != KEY_NONE && !bootCount) {
        if (!mysetup_intro(OTA_SSID, __FILE__, 0)) {
            myota(OTA_PERIOD);
        }
        esp_restart();  // as a side effect resets bootCount to 1 for the next boot
                        // what happens if OTA were run successfully
                        // wouldn't reach this point in that case anyway
                        // we must do this to allow SSID and PASS being reprogrammed
                        // to standard values in case OTA did not take place and/or failed
    } else if (key == KEY_NONE) {
        started_wifi = !mysetup_intro(NA_SSID, __FILE__, 0);  // do not activate WiFi -> started_wifi == 0
    } else {
        started_wifi = !mysetup_intro(ROTA2G_SSID, __FILE__, 0);
    }
}

void 
loop()
{
    if (started_wifi) {
        exec_cmd(key);
if (DEBUG) Serial.printf("<TP03: %u>\n", millis());
        while ((key = scan_matrix()) != KEY_NONE) {
if (DEBUG > 5) Serial.printf("C key: 0x%x\n", key);
            delay(100);
        }
Serial.printf("key RELEASED\n");
    }
    esp_wifi_stop();
#if defined(ESP32_2)
    esp_sleep_enable_ext0_wakeup(KEY_FAKE_0, 0);              // 1 = High, 0 = Low, goes to low if pressed
#else
    esp_sleep_enable_ext1_wakeup(KEY_SNS_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
    beep_sync();  // flush all tones collected so far
if (DEBUG > 5) Serial.printf("<TP14: %u>\n", millis());
    Serial.flush();
    esp_deep_sleep_start();
}

