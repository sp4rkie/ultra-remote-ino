
// runs with either -DESP8266 or -DESP32

#if defined(ESP_0)
#define HOST "esp_0"
#elif defined(ESP_1)
#define HOST "esp_1"
#elif defined(ESP_2)
#define HOST "esp_2"

#elif defined(ESP32_0)
#define HOST "esp32_0"
#elif defined(ESP32_1)
#define HOST "esp32_1"
#elif defined(ESP32_2)
#define HOST "esp32_2"
#elif defined(ESP32_3)
#define HOST "esp32_3"
#elif defined(ESP32_4)
#define HOST "esp32_4"
#elif defined(ESP32_5)
#define HOST "esp32_5"
#elif defined(ESP32_6)
#define HOST "esp32_6"
#elif defined(ESP32_7)
#define HOST "esp32_7"
#elif defined(ESP32_8)
#define HOST "esp32_8"
#elif defined(ESP32_9)
#define HOST "esp32_9"

#elif defined(ESP32_10)
#define HOST "esp32_10"
#elif defined(ESP32_11)
#define HOST "esp32_11"
#elif defined(ESP32_12)
#define HOST "esp32_12"
#elif defined(ESP32_13)
#define HOST "esp32_13"
#elif defined(ESP32_14)
#define HOST "esp32_14"
#elif defined(ESP32_15)
#define HOST "esp32_15"
#elif defined(ESP32_16)
#define HOST "esp32_16"
#elif defined(ESP32_17)
#define HOST "esp32_17"
#elif defined(ESP32_18)
#define HOST "esp32_18"
#elif defined(ESP32_19)
#define HOST "esp32_19"
#elif defined(ESP32_20)
#define HOST "esp32_20"

#else 

#error HOST "undefined"

#endif

#define GENERAL_RETRY_TIMEOUT    6000   // don't try most cmds longer than this/ see comments [ EXPERIMENTAL ONLY! ]
#define WLAN_RECONNECT_TIMEOUT  10000   // try to actively reconnect WLAN after this as built-in crap does not work?!
#define ACCESSPT_CONN_RETRY_DELAY   1   // retry rate to connect to access point

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#include <ArduinoOTA.h>
 
#ifdef USE_STATIC_IP_
IPAddress staticIP(192, 168, 106, 249);
IPAddress gateway( 192, 168, 106, 195);
IPAddress subnet(  255, 255, 255,   0);
IPAddress dns(     192, 168, 104, 195);
#endif

#ifdef TARGET_PORT

#include <regex.h>

/*
 * consider
 *    bin/pq.awklib [ STATUS_MATCH ]
 *    bin/p         [ server_status ]
 *
 * return  "#["     cmd      "]" \
 *         "#[" !!IS_REPEAT  "]" \
 *         "#[" !!IS_STEALTH "]" \
 *         "#["   PLAY_MODE  "]" \
 *         "#["     misc     "]" \
 *         "#["     stat     "]"
 *
 * also if tampering here. typical stat:
 *    #[GS]#[0]#[0]#[fil]#[ 2.31]#[0]
 */
_i8p STATUS_MATCH =
                                        // <== JUST ONE \ COMPARED TO l6.awklib?!?!?!?
                "^#\\[([^\]]+)\\]" \
                 "#\\[([01])\\]" \
                 "#\\[([01])\\]" \
                 "#\\[([a-z0-9]+)\\]" \
                 "#\\[([^()]+)\\]" \
                 "#\\[([01])\\]$";
regex_t _regex;
regmatch_t _pmatch[7];  // nr of parenthesized subexprs in STATUS_MATCH + 1
#define STAT_CMD 1     // parenthesized subexprs indices
#define STAT_REPEAT 2
#define STAT_STEALTH 3
#define STAT_PLAY_MODE 4
#define STAT_MISC 5
#define STAT_STAT 6

_i8 _buf[128];
_i8p _err[] = {
    "-----",   // sample text

    // this module
    "err#1",   // no WiFi conn (so no status)   
    "err#2",   // no target conn (so no status)
    "err#3",   // conn but no status          
    "err#4",   // wrong status format        
    "err#5",   // status nok                     

    // others          
    "err#6",   // temp conversion not complete
    "err#7",   // temp not plausible
};
#endif

_i8p
give_wifi_status(_i32 stat)
{
    _i8p ret;

    switch (stat) {
        case WL_NO_SHIELD: ret = "WL_NO_SHIELD"; break;
        case WL_IDLE_STATUS: ret = "WL_IDLE_STATUS"; break;
        case WL_NO_SSID_AVAIL: ret = "WL_NO_SSID_AVAIL"; break;
        case WL_SCAN_COMPLETED: ret = "WL_SCAN_COMPLETED"; break;
        case WL_CONNECTED: ret = "WL_CONNECTED"; break;
        case WL_CONNECT_FAILED: ret = "WL_CONNECT_FAILED"; break;
        case WL_CONNECTION_LOST: ret = "WL_CONNECTION_LOST"; break;
#ifdef ESP8266
        case WL_WRONG_PASSWORD: ret = "WL_WRONG_PASSWORD"; break;
#endif
        case WL_DISCONNECTED: ret = "WL_DISCONNECTED"; break;
        default: ret = "unknown"; break;
    }
    return ret;
}

#define WIFI_DIAG(a) \
{ \
    _u8 macAddr[6]; \
\
    Serial.printf("\n---vvv--- WIFI diag ---vvv---\n"); \
    Serial.printf("WiFi status: %s\n", give_wifi_status(WiFi.status())); \
    if (a) { \
        Serial.printf("my IP:  %s\n", WiFi.localIP().toString().c_str()); \
        Serial.printf("GW IP:  %s\n", WiFi.gatewayIP().toString().c_str()); \
        Serial.printf("subnet: %s\n", WiFi.subnetMask().toString().c_str()); \
        Serial.printf("DNS0:   %s\n", WiFi.dnsIP(0).toString().c_str()); \
        Serial.printf("DNS1:   %s\n", WiFi.dnsIP(1).toString().c_str()); \
    } \
\
    WiFi.printDiag(Serial); \
    Serial.printf("BSSID: %s\n", WiFi.BSSIDstr().c_str()); \
    Serial.printf("RSSI: %d\n", WiFi.RSSI()); \
    WiFi.macAddress(macAddr); \
    Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], \
                                                          macAddr[1], \
                                                          macAddr[2], \
                                                          macAddr[3], \
                                                          macAddr[4], \
                                                          macAddr[5]); \
    Serial.printf("---^^^--- WIFI diag ---^^^---\n"); \
}

#ifdef ESP32

RTC_DATA_ATTR _u8  ssid_last = 0;
RTC_DATA_ATTR _u32 bootCount = 0;

// 
// uint64_t        esp_sleep_get_gpio_wakeup_status(void);  // only esp32c3 defines this?!?
// uint64_t        esp_sleep_get_ext1_wakeup_status(void);
// touch_pad_t     esp_sleep_get_touchpad_wakeup_status(void);
// esp_sleep_wakeup_cause_t 
//                 esp_sleep_get_wakeup_cause(void);
//
void 
print_wakeup_touchpad()
{
    touch_pad_t touchPin = esp_sleep_get_touchpad_wakeup_status();

    switch (touchPin) {
        case 0  : Serial.printf("Touch detected on GPIO 4\n"); break;
        case 1  : Serial.printf("Touch detected on GPIO 0\n"); break;
        case 2  : Serial.printf("Touch detected on GPIO 2\n"); break;
        case 3  : Serial.printf("Touch detected on GPIO 15\n"); break;
        case 4  : Serial.printf("Touch detected on GPIO 13\n"); break;
        case 5  : Serial.printf("Touch detected on GPIO 12\n"); break;
        case 6  : Serial.printf("Touch detected on GPIO 14\n"); break;
        case 7  : Serial.printf("Touch detected on GPIO 27\n"); break;
        case 8  : Serial.printf("Touch detected on GPIO 33\n"); break;
        case 9  : Serial.printf("Touch detected on GPIO 32\n"); break;
        default : Serial.printf("wakeup not by touchpad\n"); break;
    }
}

void
print_wakeup_gpio_wakeup()
{
#ifdef SOC_GPIO_SUPPORT_DEEPSLEEP_WAKEUP    // only esp32c3 defines this?!?
    _u64 GpioPin = esp_sleep_get_gpio_wakeup_status();

    Serial.printf("wakeup by gpio [ 0x%016llx ], PIN: %u\n", GpioPin, ffs(GpioPin) - 1);
#else

    /*
     * there solely exists exactly the one defined by 
     *     esp_sleep_enable_ext0_wakeup(PIN, 0);
     */
    Serial.printf("wakeup by ext0 PIN X, check esp_sleep_enable_ext0_wakeup()\n");
#endif
}

void 
print_wakeup_ext1_wakeup()
{
    _u64 GpioPin = esp_sleep_get_ext1_wakeup_status();

    Serial.printf("wakeup by ext1 [ 0x%016llx ], PIN: %u\n", GpioPin, ffs(GpioPin) - 1);
}

void 
print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.printf("wakeup caused by external signal using RTC_IO (EXT0)\n"); 
            print_wakeup_gpio_wakeup();
            break;
        case ESP_SLEEP_WAKEUP_EXT1: 
            Serial.printf("wakeup caused by external signal using RTC_CNTL (EXT1)\n"); 
            print_wakeup_ext1_wakeup();
            break;
        case ESP_SLEEP_WAKEUP_TIMER: 
            Serial.printf("wakeup caused by timer\n"); 
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD: 
            Serial.printf("wakeup caused by touchpad\n"); 
            print_wakeup_touchpad();
            break;
        case ESP_SLEEP_WAKEUP_ULP: 
            Serial.printf("wakeup caused by ULP program\n"); 
            break;
        default: 
            Serial.printf("wakeup was not caused by deep sleep (reason == %u)\n", wakeup_reason); 
            break;
    }
}

#ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "esp32/rom/rtc.h"                      # <== OURs!
#elif CONFIG_IDF_TARGET_ESP32S2
//#include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
//#include "esp32c3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S3
//#include "esp32s3/rom/rtc.h"
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else // ESP32 Before IDF 4.0
//#include "rom/rtc.h"
#endif

void 
print_reset_reason(_u32 cpu)
{
    _i32 reason = rtc_get_reset_reason(cpu);

    Serial.printf("CPU%d reset reason: ", cpu);
    switch (reason) {
        case 1: 
            Serial.printf("Vbat power on reset\n");
            break;
        case 3: 
            Serial.printf("Software reset digital core\n");
            break;
        case 4: 
            Serial.printf("Legacy watch dog reset digital core\n");
            break;
        case 5: 
            Serial.printf("Deep Sleep reset digital core\n");
            break;
        case 6: 
            Serial.printf("Reset by SLC module, reset digital core\n");
            break;
        case 7: 
            Serial.printf("Timer Group0 Watch dog reset digital core\n");
            break;
        case 8: 
            Serial.printf("Timer Group1 Watch dog reset digital core\n");
            break;
        case 9: 
            Serial.printf("RTC Watch dog Reset digital core\n");
            break;
        case 10: 
            Serial.printf("Instrusion tested to reset CPU\n");
            break;
        case 11: 
            Serial.printf("Time Group reset CPU\n");
            break;
        case 12: 
            Serial.printf("Software reset CPU\n");
            break;
        case 13: 
            Serial.printf("RTC Watch dog Reset CPU\n");
            break;
        case 14: 
            Serial.printf("for APP CPU, reseted by PRO CPU\n");
            break;
        case 15: 
            Serial.printf("Reset when the vdd voltage is not stable\n");
            break;
        case 16: 
            Serial.printf("RTC Watch dog reset digital core and rtc module\n");
            break;
        default: 
            Serial.printf("NO_MEAN\n");
            break;
    }
}
#endif

#ifdef ESP8266
// Callback functions will be called as long as these handler objects exist.
// => so make them global here
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
#else

/*
0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
2  SYSTEM_EVENT_STA_START                < ESP32 station start
3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
25 SYSTEM_EVENT_MAX
*/

#include <WiFi.h>
 
void WiFiEvent(WiFiEvent_t event) {
//  Serial.printf("[WiFi-event] event: %u\n", event);
  Serial.printf("\nEV: %u ", millis());

  switch (event) {
    case SYSTEM_EVENT_WIFI_READY: 
      Serial.println("WiFi interface ready");
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      Serial.println("Completed scan for access points");
      break;
    case SYSTEM_EVENT_STA_START:
      Serial.println("WiFi client started");
      break;
    case SYSTEM_EVENT_STA_STOP:
      Serial.println("WiFi clients stopped");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("Connected to access point");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Disconnected from WiFi access point");
      WIFI_DIAG(0);
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      Serial.println("Authentication mode of access point has changed");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.print("Obtained IP address");
      WIFI_DIAG(1);
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      Serial.println("Lost IP address and IP address is reset to 0");
      break;
    case SYSTEM_EVENT_STA_BSS_RSSI_LOW:
      Serial.println("WiFi SYSTEM_EVENT_STA_BSS_RSSI_LOW");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP:
      Serial.println("WiFi SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP");
      break;
    case SYSTEM_EVENT_AP_START:
      Serial.println("WiFi access point started");
      break;
    case SYSTEM_EVENT_AP_STOP:
      Serial.println("WiFi access point  stopped");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("Client connected");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("Client disconnected");
      break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
      Serial.println("Assigned IP address to client");
      break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      Serial.println("Received probe request");
      break;
    case SYSTEM_EVENT_ACTION_TX_STATUS:
      Serial.println("WiFi SYSTEM_EVENT_ACTION_TX_STATUS");
      break;
    case SYSTEM_EVENT_ROC_DONE:
      Serial.println("WiFi SYSTEM_EVENT_ROC_DONE");
      break;
    case SYSTEM_EVENT_STA_BEACON_TIMEOUT:
      Serial.println("WiFi SYSTEM_EVENT_STA_BEACON_TIMEOUT");
      break;
    case SYSTEM_EVENT_FTM_REPORT:
      Serial.println("WiFi SYSTEM_EVENT_FTM_REPORT");
      break;
    case SYSTEM_EVENT_GOT_IP6:
      Serial.println("IPv6 is preferred");
      break;
    case SYSTEM_EVENT_ETH_START:
      Serial.println("Ethernet started");
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("Ethernet stopped");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("Ethernet connected");
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet disconnected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.println("Obtained IP address");
      break;
    case SYSTEM_EVENT_ETH_LOST_IP:
      Serial.println("WiFi SYSTEM_EVENT_ETH_LOST_IP");
      break;
    default:
      Serial.printf("============= NO CASE [ %u ] ==============\n", event);
      break;
  }
}

#endif

#include <time.h>

#define TIME_ZONE           3600        // offset in secs
#define DAY_LIGHT_SAVING    3600        // offset in secs

#ifdef ESP8266
_u32
getLocalTime(struct tm *tip)
{
    time_t now = time(nullptr);

    *tip = *localtime(&now);
    return 1;
}
#endif

#define     BEEP_2540   2540
#define     BEEP_1300   1300
#define     BEEP_1000   1000
#define     BEEP_OK     BEEP_2540
#define     BEEP_ERR    BEEP_1000
#define     BEEP_INFO   BEEP_1300

#ifdef BUZZER

#define SINGLE_PULSE_WIDTH 300
#define MULTI_PULSE_WIDTH 70
#define MULTI_PULSE_PAUSE 250

_u32 tone_last_syncpoint;
_u32 tone_duration;

void
beep_sync() 
{
    _i32 tmp = tone_duration - (millis() - tone_last_syncpoint);

    if (tmp > 0) {
if (DEBUG > 5) Serial.printf("syncing tone(s) for %dms\n", tmp);
        delay(tmp);
    }
    tone_last_syncpoint = millis();
    tone_duration = 0;
}

void
beep(_u32 frequ, _u32 cnt)
{
    beep_sync();
    if (cnt > 1) {
        tone_duration += cnt * MULTI_PULSE_WIDTH + (cnt - 1) * MULTI_PULSE_PAUSE;
        --cnt;
        tone(BUZZER, frequ, MULTI_PULSE_WIDTH);
        while (cnt--) {
            tone(BUZZER, 0, MULTI_PULSE_PAUSE); // effective silent
            tone(BUZZER, frequ, MULTI_PULSE_WIDTH);
        }
    } else {
        tone_duration += SINGLE_PULSE_WIDTH;
        tone(BUZZER, frequ, SINGLE_PULSE_WIDTH);
    }
}
#else
void beep_sync() {}
void beep(_u32 frequ, _u32 cnt) {}
#endif

#define NA_SSID 0
#define OTA_SSID 1
#define DOOR_SSID 2
#define SMART_SSID 3
#define ROTA2G_SSID 4
#define ROTA5G_SSID 5

_i8p accpts[] = {
    WIFI0_SSID, WIFI0_PASSWORD,
    WIFI1_SSID, WIFI1_PASSWORD,
    WIFI2_SSID, WIFI2_PASSWORD,
    WIFI3_SSID, WIFI3_PASSWORD,
    WIFI4_SSID, WIFI4_PASSWORD,
    WIFI5_SSID, WIFI5_PASSWORD,
};

_i32
mysetup_intro(_u8 ssid, _i8p prg, _u8 force_dhcp) 
{
    _u32 last, now;
    _i32 err;

    Serial.flush();
    Serial.print("+");
if (DEBUG > 1) Serial.printf("\n<TP01: %u>\n", millis());     // <=== TP01 about 36ms on ESP32
                                                            // <=== TP01 about 71ms on ESP8266
#ifdef TARGET_PORT
    if (err = regcomp(&_regex, STATUS_MATCH, REG_EXTENDED)) {
        regerror(err, &_regex, _buf, _SZ(_buf));
        Serial.printf("%s\n", _buf);
    }
#endif

#ifdef ESP8266
    gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
        WIFI_DIAG(1);
    });
    disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
        WIFI_DIAG(0);
    });
#else
    ++bootCount;

    /*
     * once programmed -> purge it even if given explicitly
     *
     * COMMENT #1:
     * UPDATE as of 2023_01_10:
     *  - always set WiFi.persistent(false);  and
     *  - always set WiFi.begin(ssid, pass);
     * as our ssid is always hardcoded in program (no dynamic change required)
     * so no need to save dynamic things to NVRAM arises
     *
     * as a result we now always sequence through:
     *  WiFi.persistent(false);
     *  WiFi.begin(ssid, pass);
     *
     * the previous version would not reprogram ssid after boot 1
     * which may conflict with some newer use cases (multi AP remotes)
     */
if (DEBUG > 1) Serial.printf("Boot number: %u\n", bootCount);
if (DEBUG > 5) print_reset_reason(0);
if (DEBUG > 5) print_reset_reason(1);
if (DEBUG > 5) print_wakeup_reason();
if (DEBUG > 5) WiFi.onEvent(WiFiEvent);
#endif

    if (!ssid) {
        Serial.printf("no WiFi requested\n");
        return 1;
    }

    /*
     * you must initially issue an valid SSID and PASS.
     * this then will be saved to non volatile RAM.
     * after this feel free to set this to 0 to speed
     * up the setup process and to avoid wear.
     *
     * i.e.
     * avoid WiFi.persistent() setting to true
     */
    if (ssid_last != ssid) {  // see COMMENT #1
if (DEBUG > 1) Serial.printf("SSID: EXPLICIT, SAVING TO NVRAM\n");
        WiFi.persistent(true);  // save params in NVRAM when new connect with new SSID params is given
    } else {
if (DEBUG > 1) Serial.printf("SSID: NOT EXPLICIT, REUSING NVRAM\n");
        WiFi.persistent(false); // avoid wear through saving data to mem // Don't save WiFi configuration in flash
    }
    WiFi.mode(WIFI_STA);

/*
 * no need to flag force DHCP if USE_STATIC_IP is not defined anyway
 */
#ifdef USE_STATIC_IP
    if (force_dhcp) {
if (DEBUG > 1) Serial.printf("DHCP: forced\n");
    } else {
if (DEBUG > 1) Serial.printf("DHCP: not used\n");
        if (!WiFi.config(staticIP, gateway, subnet, dns, dns)) {
            Serial.printf("static IP WiFi configuration failed\n");
            beep(BEEP_ERR, 3);
            return 1;
        }
    }
#endif
    if (ssid_last != ssid) { 
if (DEBUG > 1) Serial.printf("set ssid_last from %x -> %x\n", ssid_last, ssid);
        ssid_last = ssid;
        WiFi.begin(accpts[(ssid << 1) + 0], accpts[(ssid << 1) + 1]); 
    } else {
        WiFi.begin(); 
    }
    ArduinoOTA.onStart([]() {
        _i8p type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }
        Serial.printf("start updating %s\n", type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.printf("\nend\n");
    });
    ArduinoOTA.onProgress([](_i32 progress, _i32 total) {
        Serial.printf("progress: %u%%\r", progress / (total / 100));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.printf("Auth Failed\n");
        else if (error == OTA_BEGIN_ERROR) Serial.printf("Begin Failed\n");
        else if (error == OTA_CONNECT_ERROR) Serial.printf("Connect Failed\n");
        else if (error == OTA_RECEIVE_ERROR) Serial.printf("Receive Failed\n");
        else if (error == OTA_END_ERROR) Serial.printf("End Failed\n");
    });
    ArduinoOTA.begin();

    /*
     * (static) line of this length costs about 10ms
     * call to give_wifi_status() fct has no impact
     * <TP0x: 326>
     * [ /tmp/mkESP/ultra_smartremote_14b_esp32/ultra_smartremote_14b.ino.cpp on esp32_2 ] ready to operate, WiFi stat: WL_CONNECTED
     * <TP0y: 337>
     */
if (DEBUG > 5) Serial.printf("[ %s on %s ] ready to operate, WiFi stat: %s\n", prg, HOST, give_wifi_status(WiFi.status()));
if (DEBUG > 1) Serial.printf("<TP02: %u>\n", millis());
    return 0;
}

#if 0 // NTP STUFF
/*
would even be better:
    * ESP32 NTP Time - Setting Up Timezones and Daylight Saving Time | Random Nerd Tutorials
    https://randomnerdtutorials.com/esp32-ntp-timezones-daylight-saving/
*/

/*
 * we do your best to check for wait4WiFi here
 */
if (wait4WiFi && ntp) {
    struct tm timeinfo;
    _u32 last, now;

    configTime(DAY_LIGHT_SAVING, TIME_ZONE, NTP_SERVER);
    last = now = millis();
    while (GENERAL_RETRY_TIMEOUT > now - last) {
        // first getLocalTime() may need a little time...
        if (getLocalTime(&timeinfo)) {
            Serial.printf("[ %s on %s ] ready to operate at [ ", prg, HOST);
#ifdef ESP8266
            Serial.printf("%02u-%02u-%02u %02u:%02u:%02u ]\n",
                                timeinfo.tm_year - 100,
                                timeinfo.tm_mon + 1,
                                timeinfo.tm_mday,
                                timeinfo.tm_hour,
                                timeinfo.tm_min,
                                timeinfo.tm_sec);
#else
            Serial.println(&timeinfo, "%y-%m-%d %H:%M:%S ]");
#endif
            break;
        }
        Serial.printf("getLocalTime() failed, retrying...\n");
        delay(200);
        now = millis();
    }
    if (!(GENERAL_RETRY_TIMEOUT > now - last)) {
        Serial.printf("getLocalTime() failed, giving up, WiFi stat: %s\n", give_wifi_status(WiFi.status()));
    }
}
#endif

_i32
wait4wifi()
{
    _u32 last, now;
    _i32 stat = 0;

if (DEBUG > 1) Serial.printf("waiting for WiFi: ");
    last = now = millis();                
    do {
        if (WiFi.status() == WL_CONNECTED) {
            break;
        }
        Serial.print("#");                          // <== timing! keep this independent from DEBUG
        delay(ACCESSPT_CONN_RETRY_DELAY);
        now = millis();
    } while (GENERAL_RETRY_TIMEOUT > now - last);
    if (!(GENERAL_RETRY_TIMEOUT > now - last)) {
        Serial.printf("\nno WiFi connection\n");
        stat = 1;
        goto end;
    }
    if (last != now) Serial.print("\n");            // <== timing! keep this independent from DEBUG
if (DEBUG > 1) Serial.printf("is up\n");
end:
    return stat;
}

// check if WLAN still is there. WHY IS THAT REQUIRED?!
_i32
myconn_check() 
{
    _u32 cur = millis();
    static _u32 last;       // inited to 0 -> implies max. TIMEOUT of WLAN_RECONNECT_TIMEOUT after reboot
    static _u8 connected;

    if (WiFi.status() == WL_CONNECTED) {
        if (!connected && last) {
if (DEBUG > 0) Serial.printf("WLAN (re)connected\n");
if (DEBUG > 5) Serial.printf("<TP03: %u>\n", cur);
        }
        connected = 1;
        last = cur;
    } else {
        if (cur - last < WLAN_RECONNECT_TIMEOUT) {
if (DEBUG > 5) {
            Serial.printf(">");
            delay(200);
}
        } else {
if (DEBUG) Serial.printf("\nWLAN lost, trying to reconnect\n");
            WiFi.disconnect();
            WiFi.reconnect();
            last = cur;
        }
        connected = 0;
    }
    return !connected;
}

#define OTA_PERIOD 20000

void
myota(_u32 ota_period)
{
    Serial.printf("OTA issued\n");
    if (!wait4wifi()) {
        _u32 ota_start;

        beep(BEEP_INFO, 5);
        beep_sync();  // flush to avoid interference with OTA
        ota_start = millis();
        Serial.printf("OTA_PERIOD of %us starts\n", ota_period / 1000);
        while (ota_period > millis() - ota_start) {
            ArduinoOTA.handle();
            yield();
        }
        Serial.printf("OTA_PERIOD ends with no update\n");
    } else {
        beep(BEEP_INFO, 10);
        Serial.printf("OTA failed\n");
    }
}

void
myloop_intro(_u32 ota_period)
{
    static _u8 local_bootCount; // OTA the first loop on first boot only!

#ifdef ESP8266
    if (!local_bootCount) { // bootCount does not exist on ESP8266
#else
    if (!local_bootCount && bootCount == 1) {
#endif
        myota(ota_period);
        ++local_bootCount;
    }
}

#ifdef TARGET_PORT

WiFiClient target;

#define TARGET_CONNECT_RETRY_DELAY  1   // retry rate to connect target
#define TARGET_STATUS_RETRY_DELAY   1   // retry rate to retrieve status

/*
 *    condition:                    statmsg:  stat:
 *
 *    status ok                     STAT_STAT 0   
 *    no WiFi conn (so no status)   "err#1"   1      
 *    no target conn (so no status) "err#2"   2     
 *    conn but no status            "err#3"   3     
 *    wrong status format           "err#4"   4     
 *    status nok                    "err#5"   5     
 */
_i32
mysend(_i8p cmd, _i8p host, _u16 port, _i8p *statmsg)
{
    _u32 last, now;
    _i32 stat;
    _i32 err;
    String line = "";

#if 1
// ---vvv--- ancient wait4wifi ---vvv---
    last = now = millis();                
    do {
        if (WiFi.status() == WL_CONNECTED) {
            break;
        }
        Serial.print("#");                          // <== timing! keep this independent from DEBUG
        delay(ACCESSPT_CONN_RETRY_DELAY);
        now = millis();
    } while (GENERAL_RETRY_TIMEOUT > now - last);
    if (!(GENERAL_RETRY_TIMEOUT > now - last)) {
        Serial.printf("\nno WiFi connection\n");
        stat = 1;
        goto end;
    }
    if (last != now) Serial.print("\n");            // <== timing! keep this independent from DEBUG
// ---^^^--- ancient wait4wifi ---^^^---
#else
    if (wait4wifi()) {
        stat = 1;
        goto end;
    }
#endif
    last = now = millis();                
    do {
        /*
         * trick to autoconvert host to a valid address since we don't have it prior to this
         * -> mostly needed to remote-control a tethered smartphone via WiFi
         */
        if (target.connect(host ? host : WiFi.gatewayIP().toString().c_str(), port)) {
            break;
        }
        Serial.print("|");                          // <== timing! keep this independent from DEBUG
        delay(TARGET_CONNECT_RETRY_DELAY);
        now = millis();
    } while (GENERAL_RETRY_TIMEOUT > now - last);
    if (!(GENERAL_RETRY_TIMEOUT > now - last)) {
        Serial.printf("\nno target connection\n");
        stat = 2; 
        goto end;
    }
    if (last != now) Serial.print("\n");            // <== timing! keep this independent from DEBUG
if (DEBUG > 1) Serial.printf("cmd: %s\n", cmd);
    if (target.printf("%s\n", cmd) != strlen(cmd) + 1) {
        Serial.printf("could not send all bytes\n");
    }
    last = now = millis();
    do {
        if (target.available()) {
            line = target.readStringUntil('\n');
            break;
        }
        Serial.print(".");                          // <== timing! keep this independent from DEBUG
        delay(TARGET_STATUS_RETRY_DELAY); 
        now = millis();
    } while (GENERAL_RETRY_TIMEOUT > now - last);
    Serial.print("\n");                             // <== timing! keep this independent from DEBUG
    if (!(GENERAL_RETRY_TIMEOUT > now - last) || !line.length()) {
        Serial.printf("no status received (yet)\n");
        stat = 3;
    } else {
if (DEBUG > 1) Serial.printf("stat: %s\n", line.c_str());
        if (err = regexec(&_regex, line.c_str(), _NE(_pmatch), _pmatch, 0)) {
            // no match
            regerror(err, &_regex, _buf, _SZ(_buf));
if (DEBUG > 1) Serial.printf("%s\n", _buf);
            stat = 4;
        } else if (strncmp("0", line.c_str() + _pmatch[STAT_STAT].rm_so, 1)) {
if (DEBUG > 1) Serial.printf("status failed\n");
            stat = 5;
        } else {
            // valid res
            *_buf = 0;
            strncat(_buf, line.c_str() + _pmatch[STAT_MISC].rm_so, _pmatch[STAT_MISC].rm_eo - _pmatch[STAT_MISC].rm_so);
            if (statmsg) *statmsg = _buf;                                      
            stat = 0;
        }
    }
end:
    if (stat) {
        if (statmsg) *statmsg = _err[stat];
        beep(BEEP_ERR, 3);
    } else {
        beep(BEEP_OK, 1);
    }
    return stat;
}
#endif

