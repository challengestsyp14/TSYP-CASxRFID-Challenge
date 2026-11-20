// Main tag firmware v0.19 = v0.18 + the handling profile is sent to the backend, so the dashboard shows it.
//   Every upload now carries "profile": {"fragile": true, "temp_mode": "room", "temp_min_c": 5, "temp_max_c": 30} (the alert limits of the mode).
//   When the profile changes (button or k / t) or after a restart, the tag uploads it at once, even with no new event (then "events" is empty).
//   Events, seq/ack and the detection rules are unchanged.
// Main tag firmware v0.18 = v0.17 + the HANDLING PROFILE ("modes"), chosen on the box with the knob + button and saved in flash (survives a reboot).
//   Events, the upload format, the model and the detection rules are unchanged: the profile only decides what counts as an ALERT on the box and which temperature limits apply.
//   - FRAGILE page: button = YES / NO.  YES: every drop and every impact is an alert.  NO: only a drop of DROP_ALERT_M (0.30 m) or more is an alert, an impact is not.
//   - TEMP MODE page: button = next mode: COLD (goods 2-8 C) / ROOM (5-30 C, the contract's starting values) / MEDICATION (15-25 C). The tag logs temp_breach (and alerts) when the
//     reading leaves the limits of the mode. The limits include a margin for the DHT11 (+-2 C, whole degrees): see TEMP_MODES. A demo-level range check, not a cold-chain certificate.
//   - ALERT: on a violation the screen shows the reason full screen (DROP 0.42m, IMPACT, TEMP 11 C) and the red LED stays on until the button is pressed (OK). The button also works
//     with no screen connected. Other events (tap, tamper, upside_down, a drop or impact that is not a violation) keep the old behaviour: red LED for 3 s, no full-screen alert.
//   - STATUS shows the profile ("Mode FRAGILE ROOM"). There are now 8 pages: 1 STATUS 2 EVENTS 3 FRAGILE 4 TEMP MODE 5 SENSORS 6 LINK 7 LID SETUP 8 IMPACT LIMIT.
//     FRAGILE, TEMP MODE, LID SETUP and IMPACT LIMIT go back to STATUS after 30 s without a touch, so a stray press in the demo changes nothing.
//   - After the temperature mode changes, the temperature is judged again only after PROFILE_SETTLE_MS (4 s): scrolling through COLD in a warm room raises nothing.
//   - New serial commands: k = fragile YES / NO, t = next temperature mode (the same as the button pages), b = press the button from the keyboard (to test an alert before the
//     button is wired). The boot text and the 'idle' status line show the profile.
//   - Default profile after a flash with wiped memory: FRAGILE YES + ROOM, which is how v0.17 behaved (every drop, impact and temp_breach alerts).
// Main tag firmware v0.17 = v0.16 + a small screen, a knob and a push button. The tag logic (events, upload, model, rules) is unchanged.
//   Screen: 1.3" 128x64 I2C OLED on its OWN I2C bus (SDA GPIO16 = pin "RX2", SCL GPIO17 = pin "TX2"), so it cannot disturb the IMU bus. It runs in its own task on core 0
//   and only reads the tag's variables. With no screen connected the tag works exactly as before (the status line says "screen MISSING").
//   Knob (potentiometer, wiper on GPIO34): turn it to choose the page (the 6 squares in the top bar show which one):
//     1 STATUS   holder, lid, temperature, last event, ALERT (same as the red LED), Wi-Fi + events not yet uploaded      2 EVENTS  the last 8 events since boot, button = older
//     3 SENSORS  lid number + calibration, temperature, model, orientation, card reader                              4 LINK    Wi-Fi, tag IP, unsent events, server, last upload
//     5 LID SETUP   button: learn CLOSED, then learn OPEN (the same as 'c' and 'o', no laptop needed)                 6 IMPACT LIMIT  button: +10, wraps from 105 to 25 (the same as '+')
//   Push button (GPIO13 to GND): acts on the page shown (only pages 2, 5 and 6 react). Pages 5 and 6 go back to STATUS after 30 s without a touch, so a stray press during the demo does nothing.
//   Libraries (Library Manager): "Adafruit GFX Library" + "Adafruit SH110X" (1.3" modules have the SH1106 chip). A 0.96" module has the SSD1306 chip:
//   set OLED_IS_SSD1306 to 1 below and install "Adafruit SSD1306" instead. New command 'p' = knob raw value, page, button state, screen state.
//   The lid defaults are now this box's measured values (closed 3400, open in a lit room 1750): after a flash with a wiped memory the lid works without setup.
// Main tag firmware v0.16 = v0.15 + the impact limit can be changed while the tag runs, the status line shows what the tag decided, the lid reading is always shown.
//   - '+' / '-' raise / lower the impact limit by 10 m/s2 (saved in flash). 'i' prints the limit and the last shock. A hit is an impact event only if its peak is at least the limit.
//     Tuning: put the box down firmly; if an impact event appears, press '+' until it does not, then check that a real hit still counts.
//   - The status line no longer says 'impact 100%' for a bump below the limit: it says 'bump, peak 38 < limit 45' (the model's opinion, which the tag ignored). One note per bump is printed too.
//   - The lid reading (ldr number) is always in the status line ('n' hides it). A warning appears if it is stuck at 0 / 4095 (wire on AO P32, module on 3V3).
// Main tag firmware v0.15 = v0.14 + card reader (RC522) check, and a higher impact limit.
//   - At boot the RC522 version register is printed (0x91 / 0x92 = the chip answers; 0x00 or 0xFF = no answer: wiring or 3.3 V). Every second the tag checks that the
//     RC522 still answers and that its antenna is on; if not it restarts the chip (a dip of the 3.3 V rail resets the RC522 registers: antenna off, and then it never sees
//     a card again until it is initialised again). 'r' prints the reader state. The antenna gain is set to maximum (more range through the box wall).
//   - IMPACT_MIN_PEAK 30 -> 45 m/s2 (4.6 g): setting the box down firmly gave impact events. On the 143 recorded hits 133 still count (the 10 lightest are lost), no false events.
//     The impact event line now shows the peak of the hit, so you can see what a firm set-down gives and tune the limit (one number, see IMPACT_MIN_PEAK).
// Main tag firmware v0.14 = v0.13 + upside_down is now a RULE on gravity, not a model output (like the drop).
//   Upside down = the vertical axis (az) averages below -5 m/s2 over the last 1 s, in 2 results in a row (0.5 s apart). That gives one event;
//   the next one only after the box has been upright again (az above +5). Why: the model cannot see which way is up (tested on the dataset:
//   a still box turned over is still 'still' 97% of the time, a really upside-down box turned upright is still 'upside_down' 94%): its upside_down
//   label only recognizes the recordings, so it flickered 'upside_down' on a box standing still and on a box being carried.
//   The rule fired on 20 of 20 upside-down recordings and on none of the other 200. The model's upside_down probability is added to carried
//   (both mean 'the box is being handled', no event). The model still decides impact / still / carried. Library 1.0.5 or 1.0.6 (4 labels).
// Main tag firmware v0.13 MODEL v3 = v0.12 + the new 4-class model (carried / impact / still / upside_down, library 1.0.5,
//   trained on the rebuilt dataset). The model now knows a still box itself. The v0.12 checks stay as a safety net
//   (motion gate, impact needs a real shock, upside_down needs a really inverted box) and can be switched off for a test:
//   'g' = gates ON/OFF. OFF = the model alone decides (still needs 0.70 confidence and 2 results in a row, as always).
//   Use OFF to see what the model really does, ON for the demo. The compile stops with a message if the old 3-label library is installed.
// Main tag firmware v0.12 = v0.11 + two sanity checks on the model (the model proposes, physics confirms):
//   impact only counts if the window has a real shock (acceleration magnitude of at least 30 m/s2 = 3 g, the same limit as the drop landing),
//   upside_down only counts if the box really is inverted (average vertical acceleration below -5 m/s2 over the 2 s).
//   Why: the model took picking the box up for 'upside_down' (its upside_down training windows include lifting; carried never did),
//   and sometimes reported small bumps as 'impact'. Both limits come from the training data (see IMPACT_MIN_PEAK / UPSIDE_MAX_AZ).
// Main tag firmware v0.11 = v0.10 + spike cleaning: the IMU sometimes returns a bad read (one or two garbage samples, e.g. ay 87 m/s2 on a
//   still box) and the model called that 'impact' / 'upside_down'. Now a sample that jumps too much is read again 1.5 ms later and the clean
//   read is used; real motion is still there on the second read and is kept. I2C is 100 kHz (was 400 kHz). Test commands:
//   f = RFID polling off/on, h = DHT11 off/on, w = Wi-Fi off/on (to find what disturbs the IMU; 's' prints the spike count).
// Main tag firmware v0.10 = v0.9 + jump detector: 's' now runs 10 s, prints 1 raw line per second AND every sample that jumps
//   (a one-sample spike a 10 Hz print would miss); 'm' lines also show the window peak and the number of jumps.
// Main tag firmware v0.9 = v0.8 + command 's' (prints the raw IMU numbers for 3 s, to check what the sensor really sees).
// Main tag firmware v0.8 MODEL + MOTION GATE = v0.7 + a motion gate: when the last 2 s are quiet (box still)
//   the model is not asked at all and no impact / upside_down is logged. Fixes the model calling a still box 'impact' or
//   'upside_down' (it was never trained on a still box). Thresholds: GATE_ACCEL_STD / GATE_GYRO_STD below.
// Main tag firmware v0.7 MODEL = v0.6 + the Edge Impulse model (the v2 hybrid, library 1.0.4).
//   Drop + height: free-fall rule (150 ms below 0.5 g, then a landing spike of 30 m/s2), as before.
//   impact / upside_down: the model, every 0.5 s on a 2 s window (200 samples x 6 axes at 100 Hz).
//   carried is not an event. Rule-drop wins: the model's impact is ignored for 2.5 s after a fall.
// BEFORE COMPILING: 1) rename the library zip to ei-parcel-tag-arduino-1.0.6.zip (or 1.0.5; remove the '#') and add it with
//   Sketch > Include Library > Add .ZIP Library (if the IDE says the library 'parcel-tag_inferencing' already exists, delete the old folder first: Documents/Arduino/libraries/parcel-tag_inferencing).  2) Tools > Partition Scheme > "Huge APP (3MB No OTA/1MB SPIFFS)"
//   (the model does not fit in the default one).  The first compile takes several minutes, later ones are faster.
// From v0.6: Wi-Fi upload to the backend, LDR lid sensor with built-in
// light/dark values (open about 800, closed about 1300), stricter drop rule (150 ms fall + landing spike).
// Every event is saved to flash first. A background task then POSTs the events that the server has not
// confirmed yet to http://SERVER_IP:SERVER_PORT/api/upload and deletes them only after the server answers
// with ack_seq. If Wi-Fi or the server is down, events stay in flash and are sent later.
// Fill in WIFI_SSID / WIFI_PASS (2.4 GHz hotspot, same one as the laptop that runs the backend).
// Lid sensor: LDR light sensor, no setup needed: it uses LDR_DEFAULT_OPEN (lit) and LDR_DEFAULT_CLOSED (dark) below.
// If your room is different, teach it: send 'c' with the lid closed and 'o' with the lid open (saved in flash).
// 'a' goes back to the default values. Send 'n' to see the raw light number and check the two values.
// Serial Monitor 115200 commands: c = learn CLOSED value, o = learn OPEN value, d = dump the saved log,
// x = clear the log (seq keeps counting, nothing is sent), j = also show the raw JSON of each event,
// n = show/hide the raw light number next to the lid state in the status line, r = card reader (RC522) state, p = knob / button / screen state, k = fragile box YES / NO (saved), t = next temperature mode (saved), b = press the button from the keyboard (clears an alert), + / - = impact limit up / down by 10 m/s2 (saved), i = show the limit and the last shock,
// s = for 10 s: print the raw sensor numbers once per second + every spike (bad read) and what was done with it, then a summary line,
// g = gates ON/OFF (v0.12 safety checks: motion gate + impact needs a real shock; OFF = model alone, for testing), m = print every model result (carried / impact / still probabilities, carried includes the model's upside_down; how long the model took; the motion numbers and az of the window).
// Wiring (labels on the board): MPU6050 SDA P21 / SCL P22, RC522 SS P5 / SCK P18 / MOSI P23 / MISO P19 /
// RST P27 (3V3 only), DHT11 out P4, green LED P25, red LED P26 (220 ohm each), LDR module AO on P32.
// Screen (OLED, 4 pins): GND, VCC to 3V3, SCL to P17 (pin TX2), SDA to P16 (pin RX2). Knob (potentiometer): outer legs to 3V3 and GND, middle leg (wiper) to P34.
// Push button: one side to P13, the opposite corner to GND.
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MFRC522.h>
#include <DHT.h>
#define OLED_IS_SSD1306 0                // 0 = SH1106 screen chip (1.3" modules), 1 = SSD1306 chip (0.96" modules)
#define SH110X_NO_SPLASH                 // no library logo in the program
#define SSD1306_NO_SPLASH
#if OLED_IS_SSD1306
#include <Adafruit_SSD1306.h>
#else
#include <Adafruit_SH110X.h>
#endif
#include <parcel-tag_inferencing.h>   // Edge Impulse library 1.0.6 or 1.0.5 (4 labels: carried / impact / still / upside_down)
#if EI_CLASSIFIER_LABEL_COUNT != 4
#error "Wrong Edge Impulse library: this firmware needs the 4-label model (carried, impact, still, upside_down), library 1.0.6 or 1.0.5. Replace the old library (Sketch > Include Library > Add .ZIP Library, choose to replace)."
#endif

// ---------- settings ----------
const char* BOX_ID = "B001";
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* SERVER_IP = "your laptop ip here";   // the laptop that runs the backend
const int   SERVER_PORT = 8000;
const int   MAX_BATCH = 20;               // events per upload
const bool LID_SENSOR_ENABLED = true;    // LDR light sensor module, AO on GPIO 32, inside the dark box
const uint32_t LDR_PERIOD_MS = 50;       // read the light sensor 20 times per second
const int LDR_DEFAULT_CLOSED = 3400;     // reading with the lid closed (dark inside): measured on this box. Redo it with 'c' / the LID SETUP page where the demo happens
const int LDR_DEFAULT_OPEN = 1750;       // reading with the lid open in a normally lit room (1700-1800 measured). Daylight and a flash read lower, which also counts as open
const int LDR_MIN_CONTRAST = 300;        // closed and open readings must differ by at least this (0-4095)
const int LDR_AUTO_DELTA = 400;          // no calibration: a change this big (0-4095) means the lid moved
const int LDR_STABLE_BAND = 100;         // the light counts as settled when it stays inside this band ...
const uint32_t LDR_SETTLE_MS = 500;      // ... for this long
const bool  MODEL_ENABLED = true;        // false = rule-only tag (drop, tap, tamper, temp_breach), like v0.6
const float MODEL_MIN_CONF = 0.70f;      // the best label counts only if its probability is at least this (Edge Impulse test used 0.6)
const uint8_t MODEL_CONFIRM = 2;         // this many results in a row (0.5 s apart) must agree before an event is logged
const uint32_t I2C_HZ = 100000;          // 100 kHz: slower than 400 kHz but far less sensitive to long wires / noise (a read still takes < 2 ms)
const uint16_t MODEL_STEP_SAMPLES = 50;  // run the model every 50 samples = every 0.5 s (same step as in Edge Impulse)
const uint32_t IMPACT_QUIET_MS = 2500;   // one hit stays in the 2 s window for 4 results: after an impact event, or after a
                                         // rule-drop, the model's impact is ignored this long
const float GATE_ACCEL_STD = 0.30f;      // motion gate: if the last 2 s have an accelerometer std (worst axis) below this (m/s2) ...
const float GATE_GYRO_STD  = 0.05f;      // ... and a gyro std (worst axis) below this (rad/s), the box is still: no model, no event
const float IMPACT_PEAK_DEFAULT = 45.0f; // m/s2 (45 = 4.6 g): an impact needs a shock this big. Recorded hits: median 137, lightest 33; carrying never above 23. Change it while running with '+' / '-' (saved)
const float UPSIDE_MAX_AZ = -5.0f;       // m/s2: upside_down RULE: the vertical axis averages below this over the last 1 s (upright reads about +10, turned over about -10)
const float UPRIGHT_MIN_AZ = 5.0f;       // m/s2: above this on average the box is upright again, so the next flip can be logged
const uint16_t UPSIDE_SAMPLES = 100;     // the newest 100 samples = 1 s are averaged for the upside_down rule
// >>> PROFILE-DATA BEGIN
// ---------- handling profile (v0.18): chosen with the knob + button, saved in flash ----------
struct TempMode { const char* name; const char* shortName; float goodsLow, goodsHigh, margin; };
// temp_breach (and an alert) below goodsLow - margin and above goodsHigh + margin. The DHT11 is only +-2 C and reads whole degrees, so the margin keeps it from crying wolf.
// The DHT11 cannot read below 0 C, so a limit of 0 or less never triggers. ROOM keeps the contract's starting values (below 5 or above 30, no margin). All of these are to be tuned.
const TempMode TEMP_MODES[3] = {
  {"COLD",       "COLD",  2.0f,  8.0f, 2.0f},      // goods that must stay at 2-8 C: alert below 0 or above 10
  {"ROOM",       "ROOM",  5.0f, 30.0f, 0.0f},      // general goods: alert below 5 or above 30
  {"MEDICATION", "MED",  15.0f, 25.0f, 2.0f},      // medicines kept at 15-25 C: alert below 13 or above 27
};
const uint8_t TEMP_MODE_COUNT = 3;
const bool PROFILE_FRAGILE_DEFAULT = true;       // used after a flash with wiped memory; afterwards the profile comes from flash
const uint8_t PROFILE_TEMP_DEFAULT = 1;          // 1 = ROOM
const float DROP_ALERT_M = 0.30f;                // NOT fragile: only a drop at least this high is an alert (the contract's limit between warning and severe)
const float TEMP_HYST_C = 1.0f;                  // the temperature must come back this far inside the limits to re-arm
const uint32_t PROFILE_SETTLE_MS = 4000;         // after the temperature mode changes, the temperature is judged again only after this pause
uint32_t tempCheckAfter = 0;                     // (time) the temperature is not judged before this moment
// >>> PROFILE-DATA END
const bool OLED_ENABLED = true;          // screen + knob + button (v0.17). false = the tag runs without them
const bool KNOB_ENABLED = true;          // false = ignore the knob (the screen stays on STATUS): use it while the knob is not wired, a free pin reads random values
const uint8_t OLED_ADDR = 0x3C;          // screen I2C address (0x3C on most modules; the tag also tries 0x3D)
const uint32_t OLED_I2C_HZ = 400000;     // the screen has its own bus, so this cannot disturb the IMU. Lower it (100000) if the wires to the screen are long
const uint32_t POT_PERIOD_MS = 20;       // the knob is read 50 times per second
const int POT_LOW = 150, POT_HIGH = 3950;   // knob readings at the two ends of the turn (the ESP32 ADC is not linear at the ends); the 6 pages are spread evenly in between
const float POT_HYST = 0.15f;            // a page changes only when the knob is this fraction of a page past the border: no flicker
const uint32_t UI_IDLE_RETURN_MS = 30000;   // LID SETUP and IMPACT LIMIT go back to STATUS after this long without a touch

#define SS_PIN    5
#define RST_PIN   27
#define DHT_PIN   4
#define GREEN_PIN 25
#define RED_PIN   26
#define LID_PIN   32
#define OLED_SDA_PIN 16                  // pin labelled RX2
#define OLED_SCL_PIN 17                  // pin labelled TX2
#define POT_PIN   34                     // knob wiper (ADC1, input only)
#define BTN_PIN   13                     // push button to GND

const uint32_t PERIOD_US = 10000;        // 100 Hz motion loop
const float G = 9.81f;
const float FF_THRESH = 0.5f * G;        // below 0.5 g = free fall
const int   END_SAMPLES = 2;             // samples above the threshold needed to end the fall
const uint32_t MIN_FF_MS = 150;          // shorter falls (under about 11 cm) are ignored; knocks on the box give 80-140 ms dips
const float LAND_SPIKE = 30.0f;          // a real drop ends with a landing spike of at least this (m/s2) ...
const uint32_t LAND_WINDOW_MS = 150;     // ... within this time after the fall
const uint32_t MAX_FF_MS = 1500;
const uint32_t ALERT_MS  = 3000;         // red LED time after an event
const char* LOG_FILE = "/events.log";

// ---------- globals ----------
Adafruit_MPU6050 mpu;
MFRC522 rfid(SS_PIN, RST_PIN);
DHT dht(DHT_PIN, DHT11);
Preferences prefs;
QueueHandle_t tapQueue;

volatile float gTemp = NAN, gHum = NAN;  // latest DHT11 values (written by the DHT task)
uint32_t seq = 0;
uint32_t nextTick;
uint32_t alertUntil = 0;
bool tempAlarm = false;
uint32_t dropLockoutUntil = 0;
bool showJson = false;
const char* holderName = "nobody yet";
char holderBuf[24] = "";
int lastLdr = 0;
uint32_t lastLdrMs = 0;
int ldrClosed = -1, ldrOpen = -1;        // learned readings, kept in flash
bool ldrCalibrated = false;
bool lidWasClosed = true;
bool showNumbers = true;                 // the light number is always shown in the status line ('n' hides it)
int ldrRef = 0;                          // no-calibration mode: the light level of the current state
bool ldrSettling = false;
int ldrWinMin = 0, ldrWinMax = 0, ldrOverCount = 0;
uint32_t ldrWinStart = 0;
uint8_t ldrVotes = 0;                    // consecutive readings that disagree with the current lid state
uint32_t lastStatusMs = 0;
uint32_t ackSeq = 0;                     // highest seq the server has confirmed
SemaphoreHandle_t fsMutex;               // protects the log file, seq and the saved numbers
// MPU watchdog: a working sensor always has noise, so identical or all-zero samples mean it stopped answering
float prevRaw[6] = {0, 0, 0, 0, 0, 0};
uint8_t deadCount = 0;
uint16_t mpuRestarts = 0;
uint32_t lastRecover = 0;

// ---------- model (Edge Impulse) ----------
const int N_SAMPLES = EI_CLASSIFIER_RAW_SAMPLE_COUNT;          // 200 = 2 s at 100 Hz
const int N_AXES = EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;        // 6: ax ay az gx gy gz
static_assert(N_AXES == 6 && N_SAMPLES == 200, "the model must use 200 samples x 6 axes (see the ML contract)");
struct ModelResult {
  float p[EI_CLASSIFIER_LABEL_COUNT];
  const char* label[EI_CLASSIFIER_LABEL_COUNT];
  uint32_t totalMs, dspMs, nnMs;
  bool quiet;                            // the gate said the box is still, the model was not run
  float stdA, stdG;                      // worst-axis std of the window: accelerometer (m/s2), gyro (rad/s)
  float peakA;                           // largest acceleration magnitude in the window (m/s2)
  float meanAz;                          // average of the vertical axis over the whole 2 s window (m/s2)
  float meanAzRecent;                    // average of the vertical axis over the newest 1 s (m/s2): about +10 upright, about -10 upside down
  int jumps;                             // samples in the window that jumped compared with the one before
};
float ring[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];                // the last 2 s, circular
float snapBuf[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];             // copy handed to the model task, oldest sample first
int ringPos = 0;
uint16_t ringCount = 0;                  // valid samples in the ring (restarts after a sensor restart)
uint16_t sinceInfer = 0;
volatile bool modelBusy = false;
TaskHandle_t modelTaskHandle = NULL;
QueueHandle_t modelQueue;
bool showModel = false;
volatile bool gatesOn = true;            // 'g': the v0.12 safety checks (motion gate, impact peak) on/off. The upside_down rule is always on
uint32_t rawPrintUntil = 0;              // 's' command: print raw samples until this time
uint8_t rawDiv = 0;
float prevGood[6];                       // the last accepted sample
bool havePrevGood = false;
uint32_t spikesSeen = 0, spikesFixed = 0;   // bad-looking samples since boot, and how many a second read replaced
const bool CLEAN_SPIKES = true;
const float JUMP_ACCEL = 8.0f, JUMP_GYRO = 0.2f;   // m/s2 and rad/s change between two samples 10 ms apart: not plausible for a box that is not being hit
volatile bool rfidOn = true, dhtOn = true, wifiOn = true;   // test switches ('f', 'h', 'w')
volatile uint8_t rfidVersion = 0;        // RC522 VersionReg: 0x91 / 0x92 (clones 0x88, 0x12) = the chip answers; 0x00 or 0xFF = no answer on SPI (wires or 3.3 V)
volatile bool rfidAlive = false;         // the chip answers and its antenna is on
volatile uint16_t rfidRestarts = 0;      // times the tag had to re-initialise the RC522
float lastImpactPeak = 0;                // peak (m/s2) of the window that produced the last impact event
float impactMinPeak = IMPACT_PEAK_DEFAULT;   // the impact limit in use (m/s2), from flash, changed with '+' / '-'
float lastBumpPeak = 0;                  // peak of the last shock the model called impact but the tag ignored (below the limit)
bool lastBumpIgnored = false;
bool sActive = false;                    // 's' check running
uint32_t sSpikes0 = 0, sFixed0 = 0;
const char* lastModelLabel = "-";
float lastModelProb = -1;
uint8_t impactStreak = 0, upsideStreak = 0, upsideClear = 0;
bool upsideLatched = false;              // upside_down already logged, waiting until the box is upright again
uint32_t impactQuietUntil = 0;

// ---------- screen, knob, push button (v0.17): shared variables ----------
// >>> UI-GLOBALS BEGIN
const uint8_t PG_STATUS = 0, PG_EVENTS = 1, PG_FRAGILE = 2, PG_TEMP = 3, PG_SENSORS = 4, PG_LINK = 5, PG_LID = 6, PG_LIMIT = 7, PG_COUNT = 8;
struct UiEvent { uint32_t seq; char type[14]; float value; char holder; };
const int UI_EVENTS = 8;                 // the screen remembers the last 8 events since boot (the flash log is separate)
UiEvent uiEvents[UI_EVENTS];
int uiEvCount = 0, uiEvHead = 0;         // head = next slot to write
char holderLetter = '-';                 // A / B / C = known cards, ? = other card, - = nobody has tapped yet
portMUX_TYPE uiMux = portMUX_INITIALIZER_UNLOCKED;   // protects the event list and the holder text (loop() writes, the screen task reads)
volatile uint8_t uiReq = 0;              // request from the screen button to loop(): 1 learn CLOSED, 2 learn OPEN, 3 impact limit +10, 4 fragile YES/NO, 5 next temperature mode
volatile int potRaw = -1;                // knob reading 0-4095, written by loop() (the task that also reads the light sensor)
uint32_t lastPotMs = 0;
volatile bool btnEdge = false;           // set by the button interrupt
volatile uint32_t btnLastMs = 0;
volatile bool oledOk = false;            // the screen answers on I2C
volatile uint8_t oledAddrUsed = 0;
const char* volatile uiBootMsg = "starting";   // shown on the screen until setup() has finished
volatile bool tagReady = false;
volatile uint8_t uiPage = 0;             // page on the screen (0-7)
volatile bool profFragile = PROFILE_FRAGILE_DEFAULT;   // handling profile, saved in flash: fragile box yes / no
volatile bool profileDirty = true;       // the backend has not got the current profile yet (true after a restart)
volatile uint8_t profTemp = PROFILE_TEMP_DEFAULT;      // handling profile, saved in flash: index in TEMP_MODES
volatile bool alertLatched = false;      // a violation of the profile: on the screen (and the red LED) until the button is pressed
char alertWhy[12] = "";                  // its reason, at most 10 characters: "DROP 0.42m", "IMPACT", "TEMP 11 C"
char alertHolder[24] = "";               // who held the box when it happened (the holder can change before OK is pressed)
uint8_t alertCount = 0;                  // violations since the last OK (the screen shows how many)
volatile int knobZone = 0;               // page the knob points at
volatile uint32_t lastUploadMs = 0;      // when the last upload attempt ended
volatile int8_t lastUploadOk = -1;       // -1 none yet, 1 last attempt ok, 0 last attempt failed
bool ldrIsDefault = true;                // the lid values in use are the built-in ones, not learned
bool uiNetUp = false;                    // Wi-Fi info, refreshed once per second by the screen task
int uiNetRssi = 0;
char uiNetIp[20] = "-";
#if OLED_IS_SSD1306
Adafruit_SSD1306 oled(128, 64, &Wire1, -1, OLED_I2C_HZ, 100000);
#define OLED_WHITE SSD1306_WHITE
#define OLED_BLACK SSD1306_BLACK
#else
Adafruit_SH1106G oled(128, 64, &Wire1, -1, OLED_I2C_HZ, 100000);
#define OLED_WHITE SH110X_WHITE
#define OLED_BLACK SH110X_BLACK
#endif
// >>> UI-GLOBALS END

bool inFree = false;
bool dropPending = false;                // a fall was seen, waiting to see the landing spike
float pendingHeight = 0, landPeak = 0, prevMag = 0;
uint32_t pendingUntil = 0;
uint32_t ffStartUs = 0, endUs = 0;
int aboveCount = 0;

// >>> PROFILE-LOGIC BEGIN
float tempLimitLow(uint8_t m)  { return TEMP_MODES[m].goodsLow - TEMP_MODES[m].margin; }
float tempLimitHigh(uint8_t m) { return TEMP_MODES[m].goodsHigh + TEMP_MODES[m].margin; }

// One temperature check. alarm = a breach is already being held. Returns 1 = new breach (log temp_breach), 2 = back inside the limits (re-arm), 0 = nothing.
int tempStep(uint8_t mode, float tc, bool alarm) {
  if (isnan(tc)) return 0;
  float lo = tempLimitLow(mode), hi = tempLimitHigh(mode);
  if (!alarm && (tc > hi || tc < lo)) return 1;
  if (alarm && tc <= hi - TEMP_HYST_C && tc >= lo + TEMP_HYST_C) return 2;
  return 0;
}

// Switch the temperature mode: judge the temperature against the new limits from scratch, but only after a pause (scrolling through the modes raises nothing).
void profileSetTemp(uint8_t mode, uint32_t now) {
  profTemp = mode;
  tempAlarm = false;
  tempCheckAfter = now + PROFILE_SETTLE_MS;
}

// The temperature check of loop(), every pass. Returns true when a temp_breach event must be logged now (one per excursion, re-armed after it comes back).
bool tempCheck(uint32_t now, float tc) {
  if ((int32_t)(now - tempCheckAfter) < 0) return false;       // the temperature mode was just changed: wait
  int st = tempStep(profTemp, tc, tempAlarm);
  if (st == 1) { tempAlarm = true; return true; }
  if (st == 2) tempAlarm = false;
  return false;
}

// Is this event a violation of the profile? If yes, 'why' gets the short reason (at most 10 characters: the screen draws it at double size).
bool profileViolation(const char* type, float heightM, float tempNow, bool fragile, char* why, size_t n) {
  why[0] = 0;
  if (!strcmp(type, "drop")) {
    bool high = !isnan(heightM) && heightM >= DROP_ALERT_M;
    if (!fragile && !high) return false;                       // not fragile: a small drop is only logged
    if (isnan(heightM)) snprintf(why, n, "DROP");
    else if (heightM < 10.0f) snprintf(why, n, "DROP %.2fm", heightM);
    else snprintf(why, n, "DROP %.1fm", heightM > 99.9f ? 99.9f : heightM);   // (a free fall is at most 1.5 s = 11 m: the clamp only keeps the text at 10 characters)
    return true;
  }
  if (!strcmp(type, "impact")) {
    if (!fragile) return false;                                // not fragile: an impact is only logged
    snprintf(why, n, "IMPACT");
    return true;
  }
  if (!strcmp(type, "temp_breach")) {
    if (isnan(tempNow)) snprintf(why, n, "TEMP"); else snprintf(why, n, "TEMP %.0f C", tempNow);
    return true;
  }
  return false;                                                // tap, tamper, upside_down: no full-screen alert
}
// >>> PROFILE-LOGIC END

int readLdr() {                          // average of 8 reads, 12 bit (0-4095)
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(LID_PIN);
  return (int)(sum / 8);
}

int measureLdr(uint32_t ms) {            // average reading over ms milliseconds
  long sum = 0; int n = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < ms) { sum += readLdr(); n++; delay(10); }
  return n ? (int)(sum / n) : 0;
}

void updateLdrCalibration() {
  ldrCalibrated = (ldrClosed >= 0 && ldrOpen >= 0 && abs(ldrOpen - ldrClosed) >= LDR_MIN_CONTRAST);
  ldrIsDefault = !(prefs.isKey("ldrC") && prefs.isKey("ldrO"));          // true until both values were learned (the screen shows DEFAULT)
}

// After a step that blocks loop() for a second (learning the light), the motion history has a hole: start the 2 s window again.
void restartMotionWindow() {
  ringCount = 0; sinceInfer = 0; impactStreak = 0; upsideStreak = 0;
  havePrevGood = false; inFree = false; dropPending = false;
  nextTick = micros();
}

void learnClosed() {                     // 'c' on the serial monitor, or the button on the LID SETUP page
  ldrClosed = measureLdr(1000);
  prefs.putInt("ldrC", ldrClosed);
  updateLdrCalibration();
  lidWasClosed = true; ldrVotes = 0; ldrRef = ldrClosed; ldrSettling = false; ldrOverCount = 0;
  Serial.printf("CLOSED value learned: %d\n", ldrClosed);
  if (ldrOpen >= 0 && !ldrCalibrated) Serial.println("Contrast too small: darken the box inside, then learn 'c' and 'o' again.");
  restartMotionWindow();
}

void learnOpen() {                       // 'o' on the serial monitor, or the button on the LID SETUP page
  ldrOpen = measureLdr(1000);
  prefs.putInt("ldrO", ldrOpen);
  updateLdrCalibration();
  lidWasClosed = false; ldrVotes = 0; ldrRef = ldrOpen; ldrSettling = false; ldrOverCount = 0;   // the lid is open right now, so no tamper event
  Serial.printf("OPEN value learned: %d\n", ldrOpen);
  if (ldrClosed >= 0 && !ldrCalibrated) Serial.println("Contrast too small: darken the box inside, then learn 'c' and 'o' again.");
  restartMotionWindow();
}

void stepImpactLimit() {                 // the button on the IMPACT LIMIT page: +10 m/s2, from above 105 back to 25
  float v = impactMinPeak + 10.0f;
  if (v > 105.0f) v = 25.0f;
  impactMinPeak = v;
  prefs.putFloat("impLim", impactMinPeak);
  Serial.printf("impact limit now %.0f m/s2 (%.1f g), saved. Hits with a smaller peak are ignored.\n", impactMinPeak, impactMinPeak / 9.81f);
}

void toggleFragile() {                   // the button on the FRAGILE page, or 'k'
  profFragile = !profFragile;
  profileDirty = true;                   // after the change, so the upload task never sends the old value as the new one
  prefs.putBool("pFrag", profFragile);
  if (profFragile) Serial.println("Profile: fragile box YES (saved): every drop and every impact is an alert.");
  else Serial.printf("Profile: fragile box NO (saved): only a drop of %.2f m or more is an alert, an impact is not.\n", DROP_ALERT_M);
}

void nextTempMode() {                    // the button on the TEMP MODE page, or 't'
  profileSetTemp((uint8_t)((profTemp + 1) % TEMP_MODE_COUNT), millis());     // judged again only after a short pause: scrolling through the modes raises nothing
  profileDirty = true;                    // tell the backend (after the change)
  prefs.putUChar("pTemp", profTemp);
  Serial.printf("Profile: temperature mode %s (saved): alert below %.0f or above %.0f C. The temperature is judged again in %lu s.\n",
                TEMP_MODES[profTemp].name, tempLimitLow(profTemp), tempLimitHigh(profTemp), (unsigned long)(PROFILE_SETTLE_MS / 1000));
}

bool mpuStart() {
  if (!mpu.begin()) return false;        // begin() also resets and wakes the chip
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  return true;
}

// Free a stuck I2C bus (a device holding SDA low), then restart the bus and the MPU.
bool mpuRecover() {
  Wire.end();
  pinMode(21, INPUT_PULLUP);
  pinMode(22, OUTPUT);
  for (int i = 0; i < 9; i++) {
    digitalWrite(22, LOW);  delayMicroseconds(5);
    digitalWrite(22, HIGH); delayMicroseconds(5);
  }
  pinMode(21, OUTPUT);                   // STOP condition
  digitalWrite(21, LOW);  delayMicroseconds(5);
  digitalWrite(22, HIGH); delayMicroseconds(5);
  digitalWrite(21, HIGH); delayMicroseconds(5);
  Wire.begin(21, 22);
  Wire.setClock(I2C_HZ);
  return mpuStart();
}

const char* cardName(const char* uid) {
  if (!strcmp(uid, "17733800")) return "A (warehouse)";
  if (!strcmp(uid, "4BBEF804")) return "B (driver)";
  if (!strcmp(uid, "53A0B3A7")) return "C (client)";
  return uid;
}

// >>> UI-HELPERS BEGIN
char letterOf(const char* uid) {         // known card: its letter A / B / C, any other card: ?
  const char* n = cardName(uid);
  return strcmp(n, uid) ? n[0] : '?';
}

void uiPushEvent(uint32_t sq, const char* type, float heightM) {   // called by logEvent(): the screen keeps the last 8 events
  portENTER_CRITICAL(&uiMux);
  UiEvent& e = uiEvents[uiEvHead];
  e.seq = sq;
  strncpy(e.type, type, sizeof(e.type) - 1);
  e.type[sizeof(e.type) - 1] = 0;
  e.value = heightM;
  e.holder = holderLetter;
  uiEvHead = (uiEvHead + 1) % UI_EVENTS;
  if (uiEvCount < UI_EVENTS) uiEvCount++;
  portEXIT_CRITICAL(&uiMux);
}

// Called by logEvent() for every event. True = the event is a violation of the profile: the alert stays on the screen until the button is pressed.
bool profileOnEvent(const char* type, float heightM) {
  if (!OLED_ENABLED) return false;       // no screen task and no button: nobody could press OK
  char why[12];
  if (!profileViolation(type, heightM, gTemp, profFragile, why, sizeof(why))) return false;
  portENTER_CRITICAL(&uiMux);
  strcpy(alertWhy, why);
  snprintf(alertHolder, sizeof(alertHolder), "%s", holderBuf);
  if (alertCount < 99) alertCount++;
  alertLatched = true;
  portEXIT_CRITICAL(&uiMux);
  return true;
}
// >>> UI-HELPERS END

// ---------- event log ----------
void logEvent(const char* type, float heightM, const char* badge, float conf = NAN) {
  xSemaphoreTake(fsMutex, portMAX_DELAY);
  seq++;
  prefs.putUInt("seq", seq);

  char hBuf[16], tBuf[16], huBuf[16], bBuf[40], cBuf[16];
  if (isnan(heightM)) strcpy(hBuf, "null"); else snprintf(hBuf, sizeof(hBuf), "%.2f", heightM);
  if (isnan(conf)) strcpy(cBuf, "null"); else snprintf(cBuf, sizeof(cBuf), "%.2f", conf);
  float t = gTemp, h = gHum;
  if (isnan(t)) strcpy(tBuf, "null"); else snprintf(tBuf, sizeof(tBuf), "%.1f", t);
  if (isnan(h)) strcpy(huBuf, "null"); else snprintf(huBuf, sizeof(huBuf), "%.0f", h);
  if (badge) snprintf(bBuf, sizeof(bBuf), "\"%s\"", badge); else strcpy(bBuf, "null");

  char line[256];
  snprintf(line, sizeof(line),
           "{\"box_id\":\"%s\",\"seq\":%lu,\"uptime_ms\":%lu,\"type\":\"%s\",\"conf\":%s,"
           "\"height_m\":%s,\"temp_c\":%s,\"hum_pct\":%s,\"badge_id\":%s}",
           BOX_ID, (unsigned long)seq, (unsigned long)millis(), type, cBuf, hBuf, tBuf, huBuf, bBuf);

  File f = LittleFS.open(LOG_FILE, FILE_APPEND);
  if (f) { f.println(line); f.close(); }
  else Serial.println("WARNING: could not write to flash");
  xSemaphoreGive(fsMutex);
  char label[64];
  if (!strcmp(type, "drop")) snprintf(label, sizeof(label), "DROP, about %s m", hBuf);
  else if (!strcmp(type, "tap")) snprintf(label, sizeof(label), "TAP by card %s", badge ? cardName(badge) : "?");
  else if (!strcmp(type, "tamper")) snprintf(label, sizeof(label), "TAMPER, lid opened (light detected)");
  else if (!strcmp(type, "temp_breach")) snprintf(label, sizeof(label), "TEMPERATURE ALERT, out of range");
  else if (!strcmp(type, "impact")) snprintf(label, sizeof(label), "IMPACT (model %s, peak %.0f m/s2 = %.1f g)", cBuf, lastImpactPeak, lastImpactPeak / 9.81f);
  else if (!strcmp(type, "upside_down")) snprintf(label, sizeof(label), "UPSIDE DOWN (box turned over, gravity rule)");
  else snprintf(label, sizeof(label), "%s", type);
  Serial.printf("\n>>> EVENT #%lu: %s\n", (unsigned long)seq, label);
  Serial.printf("    temp=%s C  humidity=%s %%  time=%.1f s since boot  (saved to flash)\n",
                tBuf, huBuf, millis() / 1000.0);
  if (showJson) Serial.println(line);
  if (!strcmp(type, "tap") && badge) {
    char hb[24];
    snprintf(hb, sizeof(hb), "%s", cardName(badge));
    char lt = letterOf(badge);
    portENTER_CRITICAL(&uiMux);
    strcpy(holderBuf, hb);
    holderLetter = lt;
    portEXIT_CRITICAL(&uiMux);
    holderName = holderBuf;
  }
  uiPushEvent(seq, type, heightM);                   // for the screen
  if (profileOnEvent(type, heightM))                 // a violation of the profile: full-screen alert until the button is pressed
    Serial.printf("    ALERT on the screen: %s  (profile: %s, %s). Press the button to clear it.\n", alertWhy, profFragile ? "fragile" : "not fragile", TEMP_MODES[profTemp].name);
}

// ---------- background tasks on core 0 (so they never delay the 100 Hz loop) ----------
void dhtTask(void*) {
  for (;;) {
    if (dhtOn) {
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (!isnan(h) && !isnan(t)) { gHum = h; gTemp = t; }
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// RC522 health: it answers on SPI and its antenna is on. Only the RFID task (and setup, before the task starts) talks to the chip.
bool rfidHealthy() {
  byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  rfidVersion = v;
  if (v == 0x00 || v == 0xFF) return false;                              // no answer: wires, SS pin or 3.3 V
  return (rfid.PCD_ReadRegister(MFRC522::TxControlReg) & 0x03) == 0x03;  // both antenna drivers on
}

void rfidTask(void*) {
  uint32_t blockUntil = 0, nextCheck = 0, lastWarn = 0;
  for (;;) {
    if (rfidOn && millis() > nextCheck) {                                // once per second: is the reader still alive?
      nextCheck = millis() + 1000;
      bool ok = rfidHealthy();
      if (!ok) {
        rfid.PCD_Init();                                                 // hard reset + antenna on
        rfid.PCD_SetAntennaGain(rfid.RxGain_max);
        rfidRestarts++;
        ok = rfidHealthy();
        if (ok) Serial.printf("RC522 had stopped -> restarted (#%u), version 0x%02X, it reads cards again\n", (unsigned)rfidRestarts, rfidVersion);
        else if (millis() - lastWarn > 10000) {
          lastWarn = millis();
          Serial.printf("RC522 not answering (version 0x%02X): check wires SS P5, SCK P18, MOSI P23, MISO P19, RST P27 and its power: 3V3 pin, never 5 V\n", rfidVersion);
        }
      }
      rfidAlive = ok;
    }
    if (rfidOn && millis() > blockUntil && rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      char uid[24] = {0};
      for (byte i = 0; i < rfid.uid.size && i < 10; i++) sprintf(uid + i * 2, "%02X", rfid.uid.uidByte[i]);
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      xQueueSend(tapQueue, uid, 0);
      blockUntil = millis() + 1000;
    }
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

// ---------- upload to the backend (runs on core 0, so it never delays the 100 Hz loop) ----------
bool uploadBatch() {
  String events;
  int count = 0;
  uint32_t firstSeq = 0, lastSeq = 0;
  xSemaphoreTake(fsMutex, portMAX_DELAY);
  File f = LittleFS.open(LOG_FILE, FILE_READ);
  if (f) {
    while (f.available() && count < MAX_BATCH) {
      String line = f.readStringUntil('\n');
      line.trim();
      int p = line.indexOf("\"seq\":");
      if (line.length() < 10 || p < 0) continue;
      uint32_t sq = strtoul(line.c_str() + p + 6, NULL, 10);
      if (sq <= ackSeq) continue;
      if (count) events += ",";
      events += line;
      if (!count) firstSeq = sq;
      lastSeq = sq;
      count++;
    }
    f.close();
  }
  xSemaphoreGive(fsMutex);
  bool sendingProfile = profileDirty;
  if (!count && !sendingProfile) return true;
  profileDirty = false;                              // set again below if this upload fails, or by a change made while it runs

  uint8_t pm = profTemp;
  String mode = TEMP_MODES[pm].name;
  mode.toLowerCase();
  char prof[128];
  snprintf(prof, sizeof(prof), "{\"fragile\":%s,\"temp_mode\":\"%s\",\"temp_min_c\":%.1f,\"temp_max_c\":%.1f}",
           profFragile ? "true" : "false", mode.c_str(), tempLimitLow(pm), tempLimitHigh(pm));
  String body = String("{\"box_id\":\"") + BOX_ID + "\",\"uploaded_at_uptime_ms\":" +
                String((unsigned long)millis()) + ",\"profile\":" + prof + ",\"events\":[" + events + "]}";
  String url = String("http://") + SERVER_IP + ":" + SERVER_PORT + "/api/upload";
  HTTPClient http;
  http.setConnectTimeout(3000);
  http.setTimeout(4000);
  if (!http.begin(url)) { if (sendingProfile) profileDirty = true; Serial.println("UPLOAD failed: bad URL"); lastUploadOk = 0; lastUploadMs = millis(); return false; }
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);
  String resp = (code > 0) ? http.getString() : String("");
  http.end();
  if (code != 200) {
    if (sendingProfile) profileDirty = true;
    Serial.printf("UPLOAD failed: %s (will retry, events stay saved)\n",
                  code > 0 ? (String("server answered ") + code).c_str() : HTTPClient::errorToString(code).c_str());
    lastUploadOk = 0; lastUploadMs = millis();
    return false;
  }
  int p = resp.indexOf("ack_seq");
  p = (p < 0) ? -1 : resp.indexOf(':', p);
  if (p < 0) { if (sendingProfile) profileDirty = true; Serial.println("UPLOAD failed: no ack_seq in the answer"); lastUploadOk = 0; lastUploadMs = millis(); return false; }
  uint32_t ack = strtoul(resp.c_str() + p + 1, NULL, 10);
  if (!count) {                                      // a profile-only upload: nothing to acknowledge
    Serial.printf("UPLOAD ok: profile sent (%s, %s)\n", profFragile ? "fragile" : "not fragile", TEMP_MODES[pm].name);
    lastUploadOk = 1; lastUploadMs = millis();
    return true;
  }
  if (ack > lastSeq) ack = lastSeq;                  // never mark events we did not send as delivered
  if (ack < firstSeq) { Serial.printf("UPLOAD: server acked %lu, below this batch (%lu-%lu), will retry\n",
                                       (unsigned long)ack, (unsigned long)firstSeq, (unsigned long)lastSeq); lastUploadOk = 0; lastUploadMs = millis(); return false; }
  xSemaphoreTake(fsMutex, portMAX_DELAY);
  if (ack > ackSeq) { ackSeq = ack; prefs.putUInt("ack", ackSeq); }
  if (ackSeq >= seq) LittleFS.remove(LOG_FILE);      // everything confirmed: free the flash
  xSemaphoreGive(fsMutex);
  Serial.printf("UPLOAD ok: sent %d event(s), seq %lu-%lu, server acked %lu\n",
                count, (unsigned long)firstSeq, (unsigned long)lastSeq, (unsigned long)ack);
  lastUploadOk = 1; lastUploadMs = millis();
  return true;
}

void uploadTask(void*) {
  uint32_t lastWifiTry = 0, nextUpload = 0, backoff = 1000;
  bool wasUp = false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (;;) {
    if (!wifiOn) {                                        // test switch 'w'
      if (WiFi.getMode() != WIFI_OFF) { WiFi.disconnect(true); WiFi.mode(WIFI_OFF); wasUp = false; }
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }
    if (WiFi.getMode() == WIFI_OFF) { WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASS); lastWifiTry = millis(); }
    if (WiFi.status() != WL_CONNECTED) {
      if (wasUp) { Serial.println("Wi-Fi lost, events are kept in flash"); wasUp = false; }
      if (millis() - lastWifiTry > 8000) {           // one try every 8 s, never faster
        lastWifiTry = millis();
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASS);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }
    if (!wasUp) { wasUp = true; Serial.printf("Wi-Fi connected, tag IP %s\n", WiFi.localIP().toString().c_str()); nextUpload = 0; }
    if (millis() >= nextUpload && (seq > ackSeq || profileDirty)) {
      if (uploadBatch()) { backoff = 1000; nextUpload = millis() + 300; }
      else { nextUpload = millis() + backoff; backoff = (backoff * 2 > 10000) ? 10000 : backoff * 2; }
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// ---------- screen, knob and push button (v0.17) ----------
// The screen task runs on core 0 and only READS the tag's variables. Everything that changes something (learning the light, the impact limit)
// is done by loop() on request (uiReq), so the 100 Hz loop never waits for the screen, and a missing screen changes nothing.

// >>> UI-LOGIC BEGIN
bool knobSeen = false;
float knobSm = 0;
uint8_t lastKnobZone = 0;
uint8_t wizStep = 0;                     // LID SETUP: 0 = waiting for the lid CLOSED, 1 = waiting for the lid OPEN, 2 = done
bool wizWait = false;                    // a learning step is running in loop()
int evScroll = 0;                        // EVENTS page: how many events down the list the top row is
uint32_t uiLastTouch = 0;
uint32_t lastNewestSeq = 0;
char uiToast[24] = "";
uint32_t uiToastUntil = 0;

void uiToastSet(const char* s, uint32_t now) {
  snprintf(uiToast, sizeof(uiToast), "%s", s);
  uiToastUntil = now + 1500;
}

// raw = knob reading 0-4095 (written by loop()). Smooths it and decides which page the knob points at.
// A page changes only when the knob is POT_HYST of a page past the border, so noise at a border cannot flicker the screen.
void knobUpdate(int raw) {
  if (raw < 0) return;
  if (!knobSeen) knobSm = raw; else knobSm += 0.2f * (raw - knobSm);
  float pos = (knobSm - POT_LOW) / (float)(POT_HIGH - POT_LOW);
  if (pos < 0.0f) pos = 0.0f;
  if (pos > 0.9999f) pos = 0.9999f;
  float zf = pos * PG_COUNT;
  int z = (int)zf;
  if (!knobSeen) { knobSeen = true; knobZone = z; lastKnobZone = z; return; }   // first reading: start on the page the knob points at, no change
  if (z != knobZone && (zf < knobZone - POT_HYST || zf > knobZone + 1 + POT_HYST)) knobZone = z;
}

void alertClear(uint32_t now) {          // the button pressed while an alert is on the screen = OK
  portENTER_CRITICAL(&uiMux);
  alertLatched = false; alertCount = 0; alertWhy[0] = 0; alertHolder[0] = 0;
  portEXIT_CRITICAL(&uiMux);
  uiLastTouch = now;
  uiToastSet("alert cleared", now);
}

void uiShowPage(uint8_t p, uint32_t now) {
  uiPage = p;
  uiLastTouch = now;
  wizStep = 0; wizWait = false; evScroll = 0;
}

void uiOnPress(uint32_t now) {           // the button was pressed: what it does depends on the page
  uiLastTouch = now;
  if (uiPage == PG_EVENTS) {             // show older events, then back to the newest
    int maxOff = uiEvCount > 6 ? uiEvCount - 6 : 0;
    evScroll = (evScroll >= maxOff) ? 0 : evScroll + 1;
  } else if (uiPage == PG_LID) {         // 1: lid closed + press, 2: lid open + press, 3: done, press = again
    if (uiReq != 0 || wizWait) return;
    if (wizStep == 0) { uiReq = 1; wizWait = true; }
    else if (wizStep == 1) { uiReq = 2; wizWait = true; }
    else wizStep = 0;
  } else if (uiPage == PG_LIMIT) {       // impact limit +10 (wraps)
    if (uiReq == 0) { uiReq = 3; uiToastSet("limit saved", now); }
  } else if (uiPage == PG_FRAGILE) {     // fragile box YES / NO
    if (uiReq == 0) { uiReq = 4; uiToastSet("saved", now); }
  } else if (uiPage == PG_TEMP) {        // next temperature mode
    if (uiReq == 0) { uiReq = 5; uiToastSet("saved", now); }
  }
}

// The button was pressed (called by the screen task). An alert on the screen is cleared first: that works even when the screen is missing, so the red LED can always be cleared.
void uiButton(uint32_t now) {
  if (alertLatched) alertClear(now);
  else if (oledOk) uiOnPress(now);
}

void uiTick(uint32_t now) {              // every 10 ms
  if (knobZone != lastKnobZone) { lastKnobZone = knobZone; uiShowPage((uint8_t)knobZone, now); }
  if (wizWait && uiReq == 0) { wizWait = false; wizStep++; uiLastTouch = now; }       // loop() has finished learning
  if ((uiPage == PG_LID || uiPage == PG_LIMIT || uiPage == PG_FRAGILE || uiPage == PG_TEMP) && uiReq == 0 && now - uiLastTouch > UI_IDLE_RETURN_MS) uiShowPage(PG_STATUS, now);
  if (uiToastUntil && (int32_t)(now - uiToastUntil) >= 0) { uiToastUntil = 0; uiToast[0] = 0; }
  if (uiEvCount > 0) {                   // a new event: show the newest ones again
    uint32_t newest = uiEvents[(uiEvHead + UI_EVENTS - 1) % UI_EVENTS].seq;
    if (newest != lastNewestSeq) { lastNewestSeq = newest; evScroll = 0; }
  } else lastNewestSeq = 0;
}
// >>> UI-LOGIC END

// >>> UI-DRAW BEGIN
const char* const PAGE_NAME[PG_COUNT] = {"STATUS", "EVENTS", "FRAGILE", "TEMP MODE", "SENSORS", "LINK", "LID SETUP", "IMPACT LIMIT"};

void modelText(char* out, size_t n, bool shortForm) {      // what the model thinks, for the serial status line and the screen
  if (!MODEL_ENABLED) snprintf(out, n, "off");
  else if (lastModelProb < 0) { if (shortForm) snprintf(out, n, "warming up"); else snprintf(out, n, "warming up %u/%d", (unsigned)ringCount, N_SAMPLES); }
  else if (!strcmp(lastModelLabel, "still")) snprintf(out, n, "still");
  else if (lastBumpIgnored) { if (shortForm) snprintf(out, n, "bump %.0f<%.0f", lastBumpPeak, impactMinPeak); else snprintf(out, n, "bump, peak %.0f < limit %.0f", lastBumpPeak, impactMinPeak); }
  else snprintf(out, n, "%s %.0f%%", lastModelLabel, lastModelProb * 100);
}

void uiEventText(const UiEvent& e, char* out, size_t n) {
  if (!strcmp(e.type, "drop") && !isnan(e.value)) snprintf(out, n, "drop %.2fm", e.value);
  else snprintf(out, n, "%s", e.type);
}

void tempHumText(char* out, size_t n) {
  float tt = gTemp, hh = gHum;
  char ts[12], hs[12];
  if (isnan(tt)) strcpy(ts, "--"); else snprintf(ts, sizeof(ts), "%.0f C", tt);
  if (isnan(hh)) strcpy(hs, "--"); else snprintf(hs, sizeof(hs), "%.0f %%", hh);
  snprintf(out, n, "Temp %s  Hum %s", ts, hs);
}

void uiHeader(const char* title) {       // white bar with the page name, and one square per page (filled = this page)
  oled.fillRect(0, 0, 128, 9, OLED_WHITE);
  oled.setTextSize(1);
  oled.setTextColor(OLED_BLACK);
  oled.setCursor(2, 1);
  oled.print(title);
  for (int i = 0; i < PG_COUNT; i++) {
    int x = 121 - (PG_COUNT - 1 - i) * 6;
    if (i == uiPage) oled.fillRect(x, 2, 5, 5, OLED_BLACK);
    else oled.drawRect(x, 2, 5, 5, OLED_BLACK);
  }
  oled.setTextColor(OLED_WHITE);
}

void uiRow(int r, const char* s, bool inverted) {          // 6 rows of 21 characters under the header (the last row ends exactly at the bottom edge: nothing is cut off)
  int y = 11 + r * 9;
  char buf[22];
  snprintf(buf, sizeof(buf), "%s", s);
  oled.setTextSize(1);
  if (inverted) { oled.fillRect(0, y - 1, 128, 9, OLED_WHITE); oled.setTextColor(OLED_BLACK); oled.setCursor(1, y); }
  else { oled.setTextColor(OLED_WHITE); oled.setCursor(0, y); }
  oled.print(buf);
  oled.setTextColor(OLED_WHITE);
}

void drawBoot() {
  char b[24];
  oled.setTextColor(OLED_WHITE);
  oled.setTextSize(2);
  oled.setCursor(4, 4);
  oled.print("PARCEL TAG");
  oled.setTextSize(1);
  oled.setCursor(4, 26);
  oled.print("custody tag");
  snprintf(b, sizeof(b), "box %s", BOX_ID);
  oled.setCursor(4, 38);
  oled.print(b);
  snprintf(b, sizeof(b), "%s", (const char*)uiBootMsg);
  oled.setCursor(4, 52);
  oled.print(b);
}

void drawStatus(uint32_t now) {
  char t[24], hold[24], b[48], et[20];
  UiEvent last;
  int n;
  snprintf(t, sizeof(t), "%s STATUS", BOX_ID);
  uiHeader(t);
  portENTER_CRITICAL(&uiMux);
  snprintf(hold, sizeof(hold), "%s", holderBuf);
  n = uiEvCount;
  last = uiEvents[(uiEvHead + UI_EVENTS - 1) % UI_EVENTS];
  portEXIT_CRITICAL(&uiMux);
  snprintf(b, sizeof(b), "Holder %s", hold[0] ? hold : "nobody yet");
  uiRow(0, b, false);
  bool red = ((int32_t)(alertUntil - now) > 0) || tempAlarm || alertLatched;     // the same condition as the red LED
  snprintf(b, sizeof(b), "Lid %-6s Box %s", !LID_SENSOR_ENABLED ? "off" : (lidWasClosed ? "CLOSED" : "OPEN"), red ? "ALERT" : "OK");
  uiRow(1, b, red);
  tempHumText(b, sizeof(b));
  uiRow(2, b, false);
  if (n == 0) snprintf(b, sizeof(b), "Last   none yet");
  else { uiEventText(last, et, sizeof(et)); snprintf(b, sizeof(b), "Last   %s", et); }
  uiRow(3, b, false);
  snprintf(b, sizeof(b), "Mode %s %s", profFragile ? "FRAGILE" : "not fragile", TEMP_MODES[profTemp].shortName);
  uiRow(4, b, false);
  if (rfidOn && !rfidAlive) uiRow(5, "! CARD READER FAILED", true);
  else if (LID_SENSOR_ENABLED && (lastLdr >= 4000 || lastLdr <= 30)) uiRow(5, "! LID SENSOR STUCK", true);
  else if (LID_SENSOR_ENABLED && !ldrCalibrated) uiRow(5, "! LID: use LID SETUP", true);
  else { snprintf(b, sizeof(b), "WiFi %s  unsent %lu", uiNetUp ? "ok" : "OFF", (unsigned long)(seq - ackSeq)); uiRow(5, b, false); }
}

void drawFragile() {
  char b[48];
  uiHeader(PAGE_NAME[PG_FRAGILE]);
  snprintf(b, sizeof(b), "Fragile box  %s", profFragile ? "YES" : "NO");
  uiRow(0, b, true);
  if (profFragile) {
    uiRow(2, "every drop and impact", false);
    uiRow(3, "is an ALERT", false);
  } else {
    snprintf(b, sizeof(b), "only a drop of %.2f m", DROP_ALERT_M);
    uiRow(2, b, false);
    uiRow(3, "or more is an ALERT", false);
  }
  uiRow(5, "button = change", false);
}

void drawTempMode() {
  char b[48];
  uiHeader(PAGE_NAME[PG_TEMP]);
  const TempMode& m = TEMP_MODES[profTemp];
  snprintf(b, sizeof(b), "Mode  %s", m.name);
  uiRow(0, b, true);
  snprintf(b, sizeof(b), "Goods %.0f to %.0f C", m.goodsLow, m.goodsHigh);
  uiRow(1, b, false);
  snprintf(b, sizeof(b), "Alert < %.0f or > %.0f C", tempLimitLow(profTemp), tempLimitHigh(profTemp));
  uiRow(2, b, false);
  if (m.margin > 0) snprintf(b, sizeof(b), "(sensor margin %.0f C)", m.margin); else snprintf(b, sizeof(b), "(no margin added)");
  uiRow(3, b, false);
  float t = gTemp;
  if (isnan(t)) snprintf(b, sizeof(b), "Now   --"); else snprintf(b, sizeof(b), "Now   %.0f C", t);
  uiRow(4, b, false);
  uiRow(5, "button = next mode", false);
}

void drawAlert(uint32_t now) {           // a violation of the profile: the whole screen, until the button is pressed
  char why[12], hold[24], b[48];
  int cnt;
  portENTER_CRITICAL(&uiMux);
  snprintf(why, sizeof(why), "%s", alertWhy);
  cnt = alertCount;
  snprintf(hold, sizeof(hold), "%s", alertHolder);                  // the holder at the time of the violation, not whoever holds the box now
  portEXIT_CRITICAL(&uiMux);
  bool on = ((now / 400) & 1) == 0;      // the title bar blinks
  oled.setTextSize(1);
  if (on) { oled.fillRect(0, 0, 128, 9, OLED_WHITE); oled.setTextColor(OLED_BLACK); } else oled.setTextColor(OLED_WHITE);
  snprintf(b, sizeof(b), "! ALERT  %s", BOX_ID);
  oled.setCursor(2, 1);
  oled.print(b);
  oled.setTextColor(OLED_WHITE);
  oled.setTextSize(2);
  oled.setCursor(4, 14);
  oled.print(why);
  oled.setTextSize(1);
  snprintf(b, sizeof(b), "Holder %s", hold[0] ? hold : "nobody yet");
  uiRow(3, b, false);
  snprintf(b, sizeof(b), "%s, %s", profFragile ? "fragile" : "not fragile", TEMP_MODES[profTemp].shortName);
  uiRow(4, b, false);
  if (cnt > 1) snprintf(b, sizeof(b), "%d alerts: press = OK", cnt); else snprintf(b, sizeof(b), "press button = OK");
  uiRow(5, b, true);
}

void drawEvents() {
  UiEvent copy[UI_EVENTS];
  int n, head;
  uiHeader(evScroll > 0 ? "EVENTS older" : "EVENTS");
  portENTER_CRITICAL(&uiMux);
  memcpy(copy, uiEvents, sizeof(copy));
  n = uiEvCount; head = uiEvHead;
  portEXIT_CRITICAL(&uiMux);
  if (n == 0) { uiRow(0, "no event since boot", false); return; }
  for (int r = 0; r < 6; r++) {
    int k = evScroll + r;                                  // 0 = newest
    if (k >= n) break;
    const UiEvent& e = copy[(head - 1 - k + 2 * UI_EVENTS) % UI_EVENTS];
    char et[20], left[32], line[32];
    uiEventText(e, et, sizeof(et));
    snprintf(left, sizeof(left), "#%lu %s", (unsigned long)e.seq, et);
    snprintf(line, sizeof(line), "%-20.20s%c", left, e.holder);     // holder letter at the right edge
    uiRow(r, line, false);
  }
}

void drawSensors() {
  char b[48];
  uiHeader(PAGE_NAME[PG_SENSORS]);
  if (LID_SENSOR_ENABLED) snprintf(b, sizeof(b), "Lid  %s  ldr %d", lidWasClosed ? "CLOSED" : "OPEN", lastLdr);
  else snprintf(b, sizeof(b), "Lid  off");
  uiRow(0, b, false);
  bool weak = ldrIsDefault || !ldrCalibrated;
  snprintf(b, sizeof(b), "Cal %d/%d%s", ldrClosed, ldrOpen, ldrIsDefault ? " DEFAULT" : (ldrCalibrated ? "" : " weak"));
  uiRow(1, b, weak);
  tempHumText(b, sizeof(b));
  uiRow(2, b, false);
  char m[40];
  modelText(m, sizeof(m), true);
  snprintf(b, sizeof(b), "Model %s", m);
  uiRow(3, b, false);
  snprintf(b, sizeof(b), "Box   %s", upsideLatched ? "UPSIDE DOWN" : "upright");
  uiRow(4, b, upsideLatched);
  bool bad = rfidOn && !rfidAlive;
  uiRow(5, bad ? "Card reader FAILED" : "Card reader ok", bad);
}

void drawLink(uint32_t now) {
  char b[48];
  uiHeader(PAGE_NAME[PG_LINK]);
  if (uiNetUp) snprintf(b, sizeof(b), "WiFi online %d dBm", uiNetRssi); else snprintf(b, sizeof(b), "WiFi OFFLINE");
  uiRow(0, b, !uiNetUp);
  snprintf(b, sizeof(b), "IP %s", uiNetIp);
  uiRow(1, b, false);
  snprintf(b, sizeof(b), "Seq %lu  unsent %lu", (unsigned long)seq, (unsigned long)(seq - ackSeq));
  uiRow(2, b, false);
  snprintf(b, sizeof(b), "Server acked #%lu", (unsigned long)ackSeq);
  uiRow(3, b, false);
  if (lastUploadOk < 0) snprintf(b, sizeof(b), "Upload none yet");
  else {
    uint32_t age = (now - lastUploadMs) / 1000;
    char a[16];
    if (age < 120) snprintf(a, sizeof(a), "%lu s", (unsigned long)age); else snprintf(a, sizeof(a), "%lu m", (unsigned long)(age / 60 > 999 ? 999 : age / 60));
    snprintf(b, sizeof(b), "Upload %s %s ago", lastUploadOk ? "ok" : "FAIL", a);
  }
  uiRow(4, b, lastUploadOk == 0);
  snprintf(b, sizeof(b), "Srv %s:%d", SERVER_IP, SERVER_PORT);
  uiRow(5, b, false);
}

void drawLid() {
  char b[48];
  uiHeader(PAGE_NAME[PG_LID]);
  snprintf(b, sizeof(b), "Now %s  ldr %d", lidWasClosed ? "CLOSED" : "OPEN", lastLdr);
  uiRow(0, b, false);
  if (uiReq == 1 || uiReq == 2) {
    uiRow(2, uiReq == 1 ? "LEARNING closed..." : "LEARNING open...", true);
    uiRow(3, "keep the lid still", false);
  } else if (wizStep == 0) {
    uiRow(2, "1/2  CLOSE the lid", false);
    uiRow(3, "then press the button", false);
    snprintf(b, sizeof(b), "saved: %d / %d", ldrClosed, ldrOpen);
    uiRow(5, b, false);
  } else if (wizStep == 1) {
    uiRow(2, "2/2  OPEN the lid", false);
    uiRow(3, "(demo light), then", false);
    uiRow(4, "press the button", false);
  } else {
    snprintf(b, sizeof(b), "closed %d open %d", ldrClosed, ldrOpen);
    uiRow(2, b, false);
    uiRow(3, ldrCalibrated ? "OK, saved" : "TOO CLOSE: darken box", !ldrCalibrated);
    uiRow(5, "press = start again", false);
  }
}

void drawLimit() {
  char b[48];
  uiHeader(PAGE_NAME[PG_LIMIT]);
  snprintf(b, sizeof(b), "Limit %.0f m/s2", impactMinPeak);
  uiRow(0, b, true);
  snprintf(b, sizeof(b), "= %.1f g. Weaker hits", impactMinPeak / 9.81f);
  uiRow(1, b, false);
  uiRow(2, "are ignored.", false);
  if (lastBumpPeak > 0) snprintf(b, sizeof(b), "Last ignored: %.0f", lastBumpPeak); else snprintf(b, sizeof(b), "Last ignored: none");
  uiRow(3, b, false);
  if (lastImpactPeak > 0) snprintf(b, sizeof(b), "Last impact:  %.0f", lastImpactPeak); else snprintf(b, sizeof(b), "Last impact:  none");
  uiRow(4, b, false);
  uiRow(5, "button = +10 (wraps)", false);
}

void uiDraw(uint32_t now) {
  oled.clearDisplay();
  if (!tagReady) drawBoot();
  else if (alertLatched) drawAlert(now);
  else {
    switch (uiPage) {
      case PG_EVENTS: drawEvents(); break;
      case PG_FRAGILE: drawFragile(); break;
      case PG_TEMP: drawTempMode(); break;
      case PG_SENSORS: drawSensors(); break;
      case PG_LINK: drawLink(now); break;
      case PG_LID: drawLid(); break;
      case PG_LIMIT: drawLimit(); break;
      default: drawStatus(now); break;
    }
    if (uiToastUntil) uiRow(5, uiToast, true);
  }
  oled.display();
}
// >>> UI-DRAW END

// >>> UI-TASK BEGIN
void IRAM_ATTR btnIsr() {
  uint32_t t = millis();
  if (t - btnLastMs > 250) { btnLastMs = t; btnEdge = true; }   // 250 ms lockout: contact bounce and double presses count once
}

bool oledStart() {                       // looks for the screen at OLED_ADDR, then at the other usual address
  uint8_t cand[2] = {OLED_ADDR, (uint8_t)(OLED_ADDR == 0x3C ? 0x3D : 0x3C)};
  for (int i = 0; i < 2; i++) {
    Wire1.beginTransmission(cand[i]);
    if (Wire1.endTransmission() != 0) continue;
#if OLED_IS_SSD1306
    bool ok = oled.begin(SSD1306_SWITCHCAPVCC, cand[i], true, false);
#else
    bool ok = oled.begin(cand[i], true);
#endif
    if (ok) {
      oledAddrUsed = cand[i];
      oled.setTextWrap(false);
      oled.clearDisplay();
      return true;
    }
  }
  return false;
}

void uiTask(void*) {
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), btnIsr, FALLING);
  Wire1.begin(OLED_SDA_PIN, OLED_SCL_PIN, OLED_I2C_HZ);
  uint32_t lastProbe = 0, lastDraw = 0, lastNet = 0, lastAlive = 0;
  bool probed = false, hinted = false, wasReady = false;
  for (;;) {
    uint32_t now = millis();
    if (!oledOk && (!probed || now - lastProbe >= 3000)) {          // find the screen (and find it again if it was unplugged)
      probed = true; lastProbe = now;
      if (oledStart()) {
        oledOk = true;
        Serial.printf("Screen: OLED found at 0x%02X (%s chip)\n", oledAddrUsed, OLED_IS_SSD1306 ? "SSD1306" : "SH1106");
      } else if (!hinted && now > 4000) {
        hinted = true;
        Serial.println("Screen: no OLED answers on SDA P16 (pin RX2) / SCL P17 (pin TX2). Check the 4 wires and VCC on 3V3. The tag works without it.");
      }
    }
    if (oledOk && now - lastAlive >= 1000) {                        // still connected?
      lastAlive = now;
      Wire1.beginTransmission(oledAddrUsed);
      if (Wire1.endTransmission() != 0) { oledOk = false; lastProbe = now; Serial.println("Screen: lost (loose wire?), looking for it again every 3 s"); }
    }
    if (KNOB_ENABLED) knobUpdate(potRaw);
    if (btnEdge) { btnEdge = false; if (tagReady) uiButton(now); }
    if (tagReady && !wasReady) { wasReady = true; lastKnobZone = knobZone; uiShowPage(PG_STATUS, now); }   // start on STATUS, wherever the knob is
    if (tagReady) uiTick(now);
    if (tagReady && now - lastNet >= 1000) {
      lastNet = now;
      uiNetUp = (WiFi.status() == WL_CONNECTED);
      if (uiNetUp) { uiNetRssi = WiFi.RSSI(); snprintf(uiNetIp, sizeof(uiNetIp), "%s", WiFi.localIP().toString().c_str()); }
      else strcpy(uiNetIp, "-");
    }
    if (oledOk && now - lastDraw >= 200) { lastDraw = now; uiDraw(now); }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
// >>> UI-TASK END

// ---------- model task (core 0, so the 100 Hz loop on core 1 is never blocked) ----------
void modelTask(void*) {
  static ei_impulse_result_t result;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);               // the loop has put a 2 s window in snapBuf
    float stdA = 0, stdG = 0;                              // motion gate: worst-axis std over the 2 s window
    for (int ax = 0; ax < N_AXES; ax++) {
      float sum = 0, sq = 0;
      for (int i = 0; i < N_SAMPLES; i++) { float v = snapBuf[i * N_AXES + ax]; sum += v; sq += v * v; }
      float mean = sum / N_SAMPLES;
      float sd = sqrtf(fmaxf(sq / N_SAMPLES - mean * mean, 0.0f));
      if (ax < 3) { if (sd > stdA) stdA = sd; } else { if (sd > stdG) stdG = sd; }
    }
    float peakA = 0, sumAz = 0, sumAzRecent = 0; int jumps = 0;
    for (int i = 0; i < N_SAMPLES; i++) {
      bool jump = false;
      const float* sp = &snapBuf[i * N_AXES];
      float mg = sqrtf(sp[0] * sp[0] + sp[1] * sp[1] + sp[2] * sp[2]);
      if (mg > peakA) peakA = mg;
      sumAz += sp[2];
      if (i >= N_SAMPLES - UPSIDE_SAMPLES) sumAzRecent += sp[2];
      for (int k = 0; k < N_AXES; k++) {
        float v = snapBuf[i * N_AXES + k];
        if (i > 0 && fabsf(v - snapBuf[(i - 1) * N_AXES + k]) > (k < 3 ? JUMP_ACCEL : JUMP_GYRO)) jump = true;
      }
      if (jump) jumps++;
    }
    if (gatesOn && stdA < GATE_ACCEL_STD && stdG < GATE_GYRO_STD) {
      ModelResult q;
      memset(&q, 0, sizeof(q));
      q.quiet = true; q.stdA = stdA; q.stdG = stdG; q.peakA = peakA; q.jumps = jumps; q.meanAz = sumAz / N_SAMPLES; q.meanAzRecent = sumAzRecent / UPSIDE_SAMPLES;
      xQueueSend(modelQueue, &q, 0);
      modelBusy = false;
      continue;
    }
    signal_t signal;
    int err = numpy::signal_from_buffer(snapBuf, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err == 0) {
      memset(&result, 0, sizeof(result));
      uint32_t t0 = millis();
      EI_IMPULSE_ERROR e = run_classifier(&signal, &result, false);
      if (e == EI_IMPULSE_OK) {
        ModelResult r;
        for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) { r.p[i] = result.classification[i].value; r.label[i] = result.classification[i].label; }
        r.totalMs = millis() - t0; r.dspMs = result.timing.dsp; r.nnMs = result.timing.classification;
        r.quiet = false; r.stdA = stdA; r.stdG = stdG; r.peakA = peakA; r.jumps = jumps; r.meanAz = sumAz / N_SAMPLES; r.meanAzRecent = sumAzRecent / UPSIDE_SAMPLES;
        xQueueSend(modelQueue, &r, 0);
      } else {
        Serial.printf("MODEL error %d\n", (int)e);
      }
    } else {
      Serial.printf("MODEL signal error %d\n", err);
    }
    modelBusy = false;
  }
}

// upside_down is a rule on gravity, not a model output (see the v0.14 note at the top). Called with every window, still or moving.
void updateOrientation(float azRecent) {
  bool inverted = azRecent < UPSIDE_MAX_AZ;
  bool upright = azRecent > UPRIGHT_MIN_AZ;
  upsideStreak = inverted ? (upsideStreak < 255 ? upsideStreak + 1 : 255) : 0;
  upsideClear = upright ? (upsideClear < 255 ? upsideClear + 1 : 255) : 0;
  if (upsideLatched && upsideClear >= MODEL_CONFIRM) upsideLatched = false;   // upright again: the next flip can be logged
  if (!upsideLatched && upsideStreak >= MODEL_CONFIRM) {
    upsideLatched = true;
    logEvent("upside_down", NAN, nullptr);                                      // a rule, like the drop: no confidence value
    alertUntil = millis() + ALERT_MS;
  }
}

// Called from loop() with each model result. Rules: a label counts only above MODEL_MIN_CONF and only after
// MODEL_CONFIRM results in a row; carried and still are not events; impact is ignored while a rule-drop / earlier hit is still in the window.
// The model's upside_down probability is added to carried: it is not trusted (it cannot see which way is up).
void handleModelResult(const ModelResult& r) {
  updateOrientation(r.meanAzRecent);
  if (r.quiet) {                                           // box is still: nothing can have hit it, no impact
    static uint8_t quietPrints = 0;
    lastModelLabel = "still";
    lastModelProb = 1.0f;
    lastBumpIgnored = false;
    impactStreak = 0;
    if (showModel && (quietPrints++ % 10) == 0)            // 1 line per 5 s while the box is still, so the log stays readable
      Serial.printf("model: still (motion accel %.2f m/s2, gyro %.3f rad/s, peak %.1f, az %.1f, jumps %d) -> no model, no event  [printed every 5 s while still]\n", r.stdA, r.stdG, r.peakA, r.meanAzRecent, r.jumps);
    return;
  }
  float pCarried = 0, pImpact = 0, pStill = 0;
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (!strcmp(r.label[i], "impact")) pImpact = r.p[i];
    else if (!strcmp(r.label[i], "still")) pStill = r.p[i];
    else pCarried += r.p[i];                               // carried + upside_down: the box is being handled
  }
  const char* bestLabel = "carried"; float bestP = pCarried;
  if (pImpact > bestP) { bestLabel = "impact"; bestP = pImpact; }
  if (pStill > bestP) { bestLabel = "still"; bestP = pStill; }
  lastModelLabel = bestLabel;
  lastModelProb = bestP;
  bool sure = bestP >= MODEL_MIN_CONF;
  bool weakBump = sure && gatesOn && !strcmp(bestLabel, "impact") && r.peakA < impactMinPeak;   // the model says impact, but the shock is below the limit
  lastBumpIgnored = weakBump;
  if (weakBump) {
    static uint32_t lastBumpNote = 0;
    lastBumpPeak = r.peakA;
    if (millis() - lastBumpNote > 3000) {
      lastBumpNote = millis();
      Serial.printf("bump ignored: peak %.0f m/s2 (%.1f g) is below the impact limit %.0f (send '-' to count lighter hits)\n", r.peakA, r.peakA / 9.81f, impactMinPeak);
    }
  }
  if (showModel) {
    Serial.printf("model: carried %.2f impact %.2f still %.2f  -> %s  (%lu ms; motion accel %.2f m/s2, gyro %.3f rad/s, peak %.1f, az %.1f, jumps %d)",
                  pCarried, pImpact, pStill, sure ? bestLabel : "uncertain", (unsigned long)r.totalMs, r.stdA, r.stdG, r.peakA, r.meanAzRecent, r.jumps);
    if (gatesOn && sure && !strcmp(bestLabel, "impact") && r.peakA < impactMinPeak) Serial.print("  [impact ignored: no real shock]");
    if (!gatesOn) Serial.print("  [gates OFF]");
    Serial.println();
  }
  uint32_t now = millis();
  bool isImpact = sure && !strcmp(bestLabel, "impact") && (!gatesOn || r.peakA >= impactMinPeak) && (int32_t)(now - impactQuietUntil) >= 0;
  impactStreak = isImpact ? impactStreak + 1 : 0;
  if (impactStreak >= MODEL_CONFIRM) {
    impactStreak = 0;
    impactQuietUntil = now + IMPACT_QUIET_MS;
    lastImpactPeak = r.peakA;
    logEvent("impact", NAN, nullptr, bestP);
    alertUntil = millis() + ALERT_MS;
  }
}

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  if (OLED_ENABLED) xTaskCreatePinnedToCore(uiTask, "ui", 8192, NULL, 1, NULL, 0);   // the screen shows 'starting' while the rest boots
  fsMutex = xSemaphoreCreateMutex();
  setCpuFrequencyMhz(160);               // plenty for 100 Hz, and the board runs cooler than at 240 MHz
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  prefs.begin("tag", false);
  seq = prefs.getUInt("seq", 0);
  ackSeq = prefs.getUInt("ack", 0);
  if (ackSeq > seq) ackSeq = seq;
  impactMinPeak = prefs.getFloat("impLim", IMPACT_PEAK_DEFAULT);
  if (impactMinPeak < 20.0f || impactMinPeak > 200.0f) impactMinPeak = IMPACT_PEAK_DEFAULT;
  profFragile = prefs.getBool("pFrag", PROFILE_FRAGILE_DEFAULT);
  profTemp = prefs.getUChar("pTemp", PROFILE_TEMP_DEFAULT);
  if (profTemp >= TEMP_MODE_COUNT) profTemp = PROFILE_TEMP_DEFAULT;
  if (!LittleFS.begin(true)) Serial.println("WARNING: LittleFS failed to start");

  Wire.begin(21, 22);
  Wire.setClock(I2C_HZ);
  while (!mpuStart()) {
    Serial.println("MPU6050 not found - check wiring");
    uiBootMsg = "MPU6050 NOT FOUND";
    digitalWrite(RED_PIN, !digitalRead(RED_PIN));
    delay(1000);
  }
  uiBootMsg = "starting...";

  pinMode(POT_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  if (LID_SENSOR_ENABLED) {
    pinMode(LID_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(LID_PIN, ADC_11db);
    ldrClosed = prefs.getInt("ldrC", LDR_DEFAULT_CLOSED);
    ldrOpen = prefs.getInt("ldrO", LDR_DEFAULT_OPEN);
    updateLdrCalibration();
    if (ldrCalibrated) {
      int v = readLdr();                 // start in whatever state the box is in now, without raising an event
      lidWasClosed = abs(v - ldrClosed) <= abs(v - ldrOpen);
      Serial.printf("Lid sensor: closed=%d open=%d, lid is now %s\n", ldrClosed, ldrOpen, lidWasClosed ? "CLOSED" : "OPEN");
    } else {
      ldrRef = readLdr();                // the closed and open values are too close: fall back to change detection
      lidWasClosed = true;
      Serial.println("Lid sensor: closed and open values too close, using change detection. Send 'c' (lid closed) and 'o' (lid open) again.");
    }
  }

  SPI.begin(18, 19, 23, SS_PIN);
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);      // 48 dB instead of 33 dB: reads a card through the box wall more reliably
  {
    byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);
    rfidVersion = v;
    rfidAlive = rfidHealthy();
    if (v == 0x00 || v == 0xFF) Serial.printf("RC522 card reader: NOT ANSWERING (version 0x%02X). Check wires SS P5, SCK P18, MOSI P23, MISO P19, RST P27 and its power: 3V3 pin, never 5 V.\n", v);
    else Serial.printf("RC522 card reader: version 0x%02X, antenna %s\n", v, rfidAlive ? "on, OK" : "OFF (the tag will restart it)");
  }
  dht.begin();
  tapQueue = xQueueCreate(4, 24);
  xTaskCreatePinnedToCore(dhtTask, "dht", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(rfidTask, "rfid", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(uploadTask, "upload", 8192, NULL, 1, NULL, 0);
  if (MODEL_ENABLED) {
    modelQueue = xQueueCreate(3, sizeof(ModelResult));
    xTaskCreatePinnedToCore(modelTask, "model", 24576, NULL, 1, &modelTaskHandle, 0);
    Serial.printf("Model ready: %d labels, window %d samples, first result after 2 s of motion data. The model decides impact / still / carried; upside_down is a gravity rule (az below -5 m/s2 for 1 s). Safety gates ON (send g to switch)\n", EI_CLASSIFIER_LABEL_COUNT, N_SAMPLES);
    Serial.printf("Impact limit: %.0f m/s2 (%.1f g). Send + / - to change it, i to show it.\n", impactMinPeak, impactMinPeak / 9.81f);
  }

  Serial.printf("Profile: fragile box %s, temperature mode %s (alert below %.0f or above %.0f C). Change it with the knob + button (pages FRAGILE and TEMP MODE), or send k / t.\n",
                profFragile ? "YES" : "NO", TEMP_MODES[profTemp].name, tempLimitLow(profTemp), tempLimitHigh(profTemp));
  Serial.printf("Tag ready. box_id=%s  last seq=%lu  not yet uploaded=%lu\n", BOX_ID, (unsigned long)seq, (unsigned long)(seq - ackSeq));
  digitalWrite(RED_PIN, LOW);
  nextTick = micros();
  tagReady = true;                       // the screen leaves the start-up page
}

// ---------- reading the IMU, with a check for bad reads ----------
void readSample(float* out) {
  sensors_event_t a = {}, g = {}, t = {};
  mpu.getEvent(&a, &g, &t);
  out[0] = a.acceleration.x; out[1] = a.acceleration.y; out[2] = a.acceleration.z;
  out[3] = g.gyro.x;         out[4] = g.gyro.y;         out[5] = g.gyro.z;
}

bool looksWrong(const float* cur, const float* prev) {
  for (int i = 0; i < 6; i++) if (fabsf(cur[i] - prev[i]) > (i < 3 ? JUMP_ACCEL : JUMP_GYRO)) return true;
  if (cur[3] == cur[4] && cur[4] == cur[5] && fabsf(cur[3]) > 0.001f) return true;   // three gyro axes exactly equal = a bad read
  return false;
}

// ---------- main loop ----------
void loop() {
  while ((int32_t)(micros() - nextTick) < 0) {
    if ((int32_t)(nextTick - micros()) > 2500) delay(1);   // sleep instead of spinning the CPU at 100%
  }
  nextTick += PERIOD_US;

  // 1) motion: drop from free-fall time
  float raw[6];
  readSample(raw);
  if (havePrevGood && looksWrong(raw, prevGood)) {           // jumped more than a box can in 10 ms: bad read, or a real violent hit
    spikesSeen++;
    float bad[6];
    memcpy(bad, raw, sizeof(raw));
    bool fixed = false;
    if (CLEAN_SPIKES) {
      for (int k = 0; k < 2 && !fixed; k++) {                // read again: a bad read is gone, real motion is still there
        delayMicroseconds(1500);
        float again[6];
        readSample(again);
        if (!looksWrong(again, prevGood)) { memcpy(raw, again, sizeof(raw)); fixed = true; spikesFixed++; }
      }
    }
    if (sActive)
      Serial.printf("SPIKE  ax %7.2f  ay %7.2f  az %7.2f  |  gx %7.3f  gy %7.3f  gz %7.3f   (sample before: ax %.2f ay %.2f az %.2f)  -> %s\n",
                    bad[0], bad[1], bad[2], bad[3], bad[4], bad[5], prevGood[0], prevGood[1], prevGood[2],
                    fixed ? "bad read, replaced by a second read" : "still there on re-read, kept (real motion?)");
  }
  memcpy(prevGood, raw, sizeof(raw)); havePrevGood = true;
  float ax = raw[0], ay = raw[1], az = raw[2];
  if (sActive) {
    if (millis() >= rawPrintUntil) {
      sActive = false;
      Serial.printf("10 s done: %lu spikes seen, %lu replaced by a second read | RFID polling %s, DHT11 %s, Wi-Fi %s\n",
                    (unsigned long)(spikesSeen - sSpikes0), (unsigned long)(spikesFixed - sFixed0),
                    rfidOn ? "ON" : "off", dhtOn ? "ON" : "off", wifiOn ? "ON" : "off");
    } else if (++rawDiv >= 100) {                              // one raw sample per second
      rawDiv = 0;
      Serial.printf("raw  ax %7.2f  ay %7.2f  az %7.2f  |  gx %7.3f  gy %7.3f  gz %7.3f\n", raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);
    }
  }
  bool same = true, nearZero = true;
  for (int i = 0; i < 6; i++) {
    if (raw[i] != prevRaw[i]) same = false;
    if (fabsf(raw[i]) > 0.01f) nearZero = false;
    prevRaw[i] = raw[i];
  }
  bool glitch = same || nearZero;        // sensor dropout, not a real free fall
  deadCount = glitch ? (deadCount < 255 ? deadCount + 1 : 255) : 0;
  if (deadCount >= 5 && millis() - lastRecover > 300) {   // frozen for 50 ms: restart it
    lastRecover = millis();
    mpuRestarts++;
    bool ok = mpuRecover();
    Serial.printf("MPU frozen -> restart #%u: %s\n", mpuRestarts, ok ? "ok" : "FAILED");
    inFree = false;
    ringCount = 0; sinceInfer = 0; impactStreak = 0; upsideStreak = 0;   // the model needs 2 s of clean data again
    havePrevGood = false;
    nextTick = micros();
  }
  if (!glitch) {
    float mag = sqrtf(ax * ax + ay * ay + az * az);
    if (dropPending && mag > landPeak) landPeak = mag;
    if (MODEL_ENABLED) {                                   // keep the last 2 s for the model
      memcpy(&ring[ringPos * N_AXES], raw, sizeof(raw));
      ringPos = (ringPos + 1) % N_SAMPLES;
      if (ringCount < N_SAMPLES) ringCount++;
      if (ringCount >= N_SAMPLES && ++sinceInfer >= MODEL_STEP_SAMPLES) {
        sinceInfer = 0;
        if (!modelBusy) {                                  // skip this step if the model is still busy
          for (int i = 0; i < N_SAMPLES; i++) memcpy(&snapBuf[i * N_AXES], &ring[((ringPos + i) % N_SAMPLES) * N_AXES], sizeof(raw));
          modelBusy = true;
          xTaskNotifyGive(modelTaskHandle);
        }
      }
    }
    if (!inFree) {
      if (mag < FF_THRESH) { inFree = true; ffStartUs = micros(); aboveCount = 0; }
    } else if (mag >= FF_THRESH) {
      if (aboveCount == 0) endUs = micros();
      if (++aboveCount >= END_SAMPLES) {
        inFree = false;
        uint32_t ffMs = (endUs - ffStartUs) / 1000;
        if (ffMs >= MIN_FF_MS && ffMs <= MAX_FF_MS && millis() > dropLockoutUntil) {
          float tt = ffMs / 1000.0f;
          pendingHeight = 0.5f * G * tt * tt;
          dropPending = true;
          impactQuietUntil = millis() + IMPACT_QUIET_MS;     // the landing also looks like an impact to the model: ignore it
          impactStreak = 0;
          landPeak = (mag > prevMag) ? mag : prevMag;       // include the first sample after the fall
          pendingUntil = millis() + LAND_WINDOW_MS;
          dropLockoutUntil = millis() + 1000;
        }
      }
    } else {
      aboveCount = 0;
    }
    prevMag = mag;
  }
  if (dropPending && millis() >= pendingUntil) {            // a drop must end with a landing spike, a knock does not
    dropPending = false;
    if (landPeak >= LAND_SPIKE) {
      logEvent("drop", pendingHeight, nullptr);
      alertUntil = millis() + ALERT_MS;
    }
  }

  // 1b) model results from the model task
  if (MODEL_ENABLED) {
    ModelResult mr;
    while (xQueueReceive(modelQueue, &mr, 0) == pdTRUE) handleModelResult(mr);
  }

  // 2) badge taps (read by the RFID task)
  char uid[24];
  while (xQueueReceive(tapQueue, uid, 0) == pdTRUE) logEvent("tap", NAN, uid);

  // 3) temperature breach (one event when it leaves the limits of the temperature mode, re-armed after it comes back)
  if (tempCheck(millis(), gTemp)) {
    logEvent("temp_breach", NAN, nullptr);
    alertUntil = millis() + ALERT_MS;
  }

  // 4) lid (tamper): the LDR sees light when the lid opens. Uses the default closed/open values, or the ones taught with 'c' and 'o'.
  if (LID_SENSOR_ENABLED && millis() - lastLdrMs >= LDR_PERIOD_MS) {
    lastLdrMs = millis();
    lastLdr = readLdr();
    if (ldrCalibrated) {
      float mid = (ldrClosed + ldrOpen) / 2.0f;
      float band = abs(ldrOpen - ldrClosed) / 6.0f;        // hysteresis so it does not flicker
      float dir = (ldrOpen > ldrClosed) ? 1.0f : -1.0f;
      float score = (lastLdr - mid) * dir;                 // > 0 means closer to the OPEN value
      bool currentlyOpen = !lidWasClosed;
      bool wantOpen = currentlyOpen ? (score > -band) : (score > band);
      if (wantOpen != currentlyOpen) {
        if (++ldrVotes >= 3) {                             // 3 readings in a row = 150 ms, not one noisy one
          ldrVotes = 0;
          if (wantOpen) {                                  // closed -> open = tamper
            lidWasClosed = false;
            logEvent("tamper", NAN, nullptr);
            alertUntil = millis() + ALERT_MS;
          } else {
            lidWasClosed = true;
          }
        }
      } else {
        ldrVotes = 0;
      }
    } else {
      // no calibration: a big sudden change in light means the lid moved
      if (ldrSettling) {
        if (lastLdr < ldrWinMin) ldrWinMin = lastLdr;
        if (lastLdr > ldrWinMax) ldrWinMax = lastLdr;
        if (ldrWinMax - ldrWinMin > LDR_STABLE_BAND) {
          ldrWinMin = ldrWinMax = lastLdr;
          ldrWinStart = millis();
        } else if (millis() - ldrWinStart >= LDR_SETTLE_MS) {
          ldrRef = lastLdr;
          ldrSettling = false;
        }
      } else if (abs(lastLdr - ldrRef) > LDR_AUTO_DELTA) {
        if (++ldrOverCount >= 3) {       // 3 readings in a row, not one noisy one
          ldrOverCount = 0;
          if (lidWasClosed) {            // closed -> open = tamper
            lidWasClosed = false;
            logEvent("tamper", NAN, nullptr);
            alertUntil = millis() + ALERT_MS;
          } else {                       // open -> closed again
            lidWasClosed = true;
          }
          ldrSettling = true;
          ldrWinMin = ldrWinMax = lastLdr;
          ldrWinStart = millis();
        }
      } else {
        ldrOverCount = 0;
      }
    }
  }

  // 4b) knob: read here, in the same task as the light sensor, so the two never use the ADC at the same moment
  if (OLED_ENABLED && millis() - lastPotMs >= POT_PERIOD_MS) {
    lastPotMs = millis();
    long ps = 0;
    for (int i = 0; i < 8; i++) ps += analogRead(POT_PIN);
    potRaw = (int)(ps / 8);
  }

  // 4c) requests from the screen button (the screen task never touches the light sensor or the flash itself)
  if (uiReq) {
    if (uiReq == 1) learnClosed();
    else if (uiReq == 2) learnOpen();
    else if (uiReq == 3) stepImpactLimit();
    else if (uiReq == 4) toggleFragile();
    else if (uiReq == 5) nextTempMode();
    uiReq = 0;
  }

  // 5) LEDs: green when fine, red for a few seconds after an event, while the temperature is out of range, and while an alert waits for OK
  bool red = (millis() < alertUntil) || tempAlarm || alertLatched;
  digitalWrite(RED_PIN, red);
  digitalWrite(GREEN_PIN, !red);

  // 5b) status line every 2 s so you can see the tag is alive
  if (millis() - lastStatusMs >= 2000) {
    lastStatusMs = millis();
    float tt = gTemp, hh = gHum;
    const char* lidTxt = !LID_SENSOR_ENABLED ? "off" : (lidWasClosed ? "CLOSED" : "OPEN");
    char ldrTxt[24] = "";
    if (showNumbers) snprintf(ldrTxt, sizeof(ldrTxt), " (ldr %d)", lastLdr);
    char warn[200] = "";
    if (rfidOn && !rfidAlive) strcat(warn, " | CARD READER NOT ANSWERING (send r)");
    if (LID_SENSOR_ENABLED && (lastLdr >= 4000 || lastLdr <= 30)) {
      char w[110]; snprintf(w, sizeof(w), " | LID SENSOR STUCK at %d: check the AO wire on P32 and that the module is on 3V3 (not 5 V)", lastLdr); strcat(warn, w);
    }
    char modelTxt[40];
    modelText(modelTxt, sizeof(modelTxt), false);
    const char* scrTxt = !OLED_ENABLED ? "off" : (oledOk ? "ok" : "MISSING");
    Serial.printf("idle | temp %.0f C, humidity %.0f %% | lid %s%s | model %s | box %s | holder: %s | last seq %lu | wifi %s, not yet uploaded %lu | profile %s, %s | screen %s%s\n",
                  isnan(tt) ? -99.0f : tt, isnan(hh) ? -99.0f : hh, lidTxt, ldrTxt, modelTxt, upsideLatched ? "UPSIDE DOWN" : "upright", holderName, (unsigned long)seq,
                  WiFi.status() == WL_CONNECTED ? "online" : "offline", (unsigned long)(seq - ackSeq),
                  profFragile ? "fragile" : "not fragile", TEMP_MODES[profTemp].name, scrTxt, warn);
  }

  // 6) serial commands
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'd') {
      xSemaphoreTake(fsMutex, portMAX_DELAY);
      File f = LittleFS.open(LOG_FILE, FILE_READ);
      Serial.println("--- saved log (events not yet confirmed by the server) ---");
      if (f) { while (f.available()) Serial.write(f.read()); f.close(); }
      Serial.println("--- end ---");
      xSemaphoreGive(fsMutex);
    } else if (c == 'c') {
      learnClosed();
    } else if (c == 'o') {
      learnOpen();
    } else if (c == 'a') {
      prefs.remove("ldrC"); prefs.remove("ldrO");
      ldrClosed = LDR_DEFAULT_CLOSED; ldrOpen = LDR_DEFAULT_OPEN;
      updateLdrCalibration();
      int v = readLdr();
      lidWasClosed = abs(v - ldrClosed) <= abs(v - ldrOpen);
      ldrVotes = 0; ldrRef = v; ldrSettling = false; ldrOverCount = 0;
      Serial.printf("back to the default values (closed %d, open %d); lid is now %s\n", ldrClosed, ldrOpen, lidWasClosed ? "CLOSED" : "OPEN");
    } else if (c == 'n') {
      showNumbers = !showNumbers;
      Serial.printf("light number in the status line %s\n", showNumbers ? "ON" : "OFF");
    } else if (c == 's') {
      rawPrintUntil = millis() + 10000; rawDiv = 99; sActive = true; sSpikes0 = spikesSeen; sFixed0 = spikesFixed;
      Serial.println("10 s check: one raw line per second, and a SPIKE line for every bad-looking sample. Box still: az about 10, steady, no SPIKE lines.");
    } else if (c == 'f') {
      rfidOn = !rfidOn; Serial.printf("RFID polling %s\n", rfidOn ? "ON" : "OFF (test only, card taps are ignored)");
    } else if (c == 'h') {
      dhtOn = !dhtOn; Serial.printf("DHT11 %s\n", dhtOn ? "ON" : "OFF (test only, the temperature stays at the last value)");
    } else if (c == 'w') {
      wifiOn = !wifiOn; Serial.printf("Wi-Fi %s\n", wifiOn ? "ON" : "OFF (test only, nothing is uploaded)");
    } else if (c == '+' || c == '-') {
      impactMinPeak += (c == '+') ? 10.0f : -10.0f;
      if (impactMinPeak < 20.0f) impactMinPeak = 20.0f;
      if (impactMinPeak > 200.0f) impactMinPeak = 200.0f;
      prefs.putFloat("impLim", impactMinPeak);
      Serial.printf("impact limit now %.0f m/s2 (%.1f g), saved. Hits with a smaller peak are ignored.\n", impactMinPeak, impactMinPeak / 9.81f);
    } else if (c == 'i') {
      Serial.printf("impact limit %.0f m/s2 (%.1f g) | last bump ignored: peak %.0f | last impact event: peak %.0f | '+' raises, '-' lowers the limit by 10\n",
                    impactMinPeak, impactMinPeak / 9.81f, lastBumpPeak, lastImpactPeak);
    } else if (c == 'r') {
      Serial.printf("RC522 card reader: version 0x%02X (%s), %s, restarted %u times, polling %s\n", rfidVersion,
                    (rfidVersion == 0x00 || rfidVersion == 0xFF) ? "NO ANSWER: wires / 3.3 V" : "chip answers",
                    rfidAlive ? "antenna on, working" : "NOT working", (unsigned)rfidRestarts, rfidOn ? "ON" : "OFF ('f')");
    } else if (c == 'p') {
      char scr[28];
      if (oledOk) snprintf(scr, sizeof(scr), "ok (0x%02X)", oledAddrUsed); else snprintf(scr, sizeof(scr), "%s", OLED_ENABLED ? "MISSING" : "off");
      Serial.printf("panel: knob raw %d -> page %d of %d (%s), showing page %d | button %s | screen %s\n", (int)potRaw, (int)knobZone + 1, (int)PG_COUNT, PAGE_NAME[knobZone],
                    (int)uiPage + 1, digitalRead(BTN_PIN) ? "up" : "PRESSED", scr);
    } else if (c == 'k') {
      toggleFragile();
    } else if (c == 't') {
      nextTempMode();
    } else if (c == 'b') {
      btnEdge = true;                                    // the same as pressing the push button: clears an alert on the screen (bring-up without the button wired)
      Serial.println("button pressed (from the keyboard)");
    } else if (c == 'g') {
      gatesOn = !gatesOn;
      Serial.printf("safety gates %s\n", gatesOn ? "ON (motion gate, impact needs a real shock)" : "OFF (test only: the model alone decides impact, every model result is printed with m; upside_down stays a gravity rule)");
    } else if (c == 'm') {
      showModel = !showModel;
      Serial.printf("model results printing %s\n", showModel ? "ON" : "OFF");
    } else if (c == 'j') {
      showJson = !showJson;
      Serial.printf("raw JSON printing %s\n", showJson ? "ON" : "OFF");
    } else if (c == 'x') {
      xSemaphoreTake(fsMutex, portMAX_DELAY);
      LittleFS.remove(LOG_FILE);
      ackSeq = seq;
      prefs.putUInt("ack", ackSeq);
      xSemaphoreGive(fsMutex);
      portENTER_CRITICAL(&uiMux);
      uiEvCount = 0; uiEvHead = 0;                     // the screen's event list starts empty too
      portEXIT_CRITICAL(&uiMux);
      Serial.println("log cleared (seq keeps counting, nothing is sent)");
    }
  }
}
