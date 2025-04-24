#include <HTTPClient.h>

// For optimal microSD card format on Linux (partitioned, exFAT):
// sudo parted /dev/mmcblk0 mklabel msdos
// sudo mkfs.exfat -n SDCARD -s 128 /dev/mmcblk0p1

#include <SdFat.h>
SdExFat sd;
ExFatFile file;

// Uncomment one to match your board
// #define BOARD_ESP32C3
#define BOARD_ESP32C6

// Board-specific configurations
#ifdef BOARD_ESP32C3
  #define SD_CS    4
  #define SD_MOSI  5
  #define SD_SCLK  6
  #define SD_MISO  7
  #define SD_EN 21 //Input - Is SD card inserted or not
  #define CHARGE_STATUS_PIN 10 //Input - Whether device is charging or not
  #define BATTERY_METER_PIN 2 //Input - Voltage divider output with 2x 220kohm
  #define BATTERY_METER_EN_PIN 3 //Output - Enable or disable voltage divider
#elif defined(BOARD_ESP32C6)
  #define SD_CS    2
  #define SD_MOSI  21
  #define SD_SCLK  22
  #define SD_MISO  23
  #define SD_EN 16 //Input - Is SD card inserted or not
  #define CHARGE_STATUS_PIN 18 //Input - Whether device is charging or not
  #define BATTERY_METER_PIN 0 //Input - Voltage divider output with 2x 220kohm
  #define BATTERY_METER_EN_PIN 1 //Output - Enable or disable voltage divider
  #define USER_LED 15
#else
  #error "Please define BOARD_ESP32C3 or BOARD_ESP32C6"
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Optional: manually start SPI bus
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

  if (!sd.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  Serial.println("exFAT SD card initialized.");

  writeFile("/testfile.txt", "Line 1\n");
  appendLine("/testfile.txt", "Line 2");
  readLines("/testfile.txt");
  copyFile("/testfile.txt", "/testfile_copy.txt");
  moveFile("/testfile_copy.txt", "/testfile_moved.txt");
  deleteFile("/testfile.txt");
}

void loop() {}

void writeFile(const char* path, const char* content) {
  if (!file.open(path, O_WRITE | O_CREAT | O_TRUNC)) {
    Serial.println("Failed to open file for writing.");
    return;
  }
  file.write((const uint8_t*)content, strlen(content));
  file.close();
  Serial.println("File written.");
}

void appendLine(const char* path, const char* line) {
  if (!file.open(path, O_APPEND | O_WRITE)) {
    Serial.println("Failed to open file for appending.");
    return;
  }
  file.write((const uint8_t*)line, strlen(line));
  file.write('\n');
  file.close();
  Serial.println("Line appended.");
}

void readLines(const char* path) {
  if (!file.open(path, O_READ)) {
    Serial.println("Failed to open file for reading.");
    return;
  }
  char ch;
  while (file.available()) {
    file.read(&ch, 1);
    Serial.print(ch);
  }
  file.close();
}

void deleteFile(const char* path) {
  if (sd.remove(path)) {
    Serial.println("File deleted.");
  } else {
    Serial.println("Failed to delete file.");
  }
}

void moveFile(const char* src, const char* dst) {
  if (sd.rename(src, dst)) {
    Serial.println("File moved.");
  } else {
    Serial.println("Move failed.");
  }
}

void copyFile(const char* src, const char* dst) {
  ExFatFile in, out;
  if (!in.open(src, O_READ)) {
    Serial.println("Copy failed: could not open source.");
    return;
  }
  if (!out.open(dst, O_WRITE | O_CREAT | O_TRUNC)) {
    Serial.println("Copy failed: could not open dest.");
    in.close();
    return;
  }

  uint8_t buf[256];
  int n;
  while ((n = in.read(buf, sizeof(buf))) > 0) {
    out.write(buf, n);
  }

  in.close();
  out.close();
  Serial.println("File copied.");
}
