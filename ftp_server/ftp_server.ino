#include <WiFi.h>
#include <SD.h>
#include <WebServer.h>

#include <SimpleFTPServer.h>


// Uncomment or define the specific board in the IDE or here
// #define BOARD_ESP32C3
#define BOARD_ESP32C6

// Board-specific configurations
#ifdef BOARD_ESP32C3
  #define SD_CS 4
  #define SD_MOSI 5
  #define SD_SCLK 6
  #define SD_MISO 7
  #define SD_EN 21 //Input - Is SD card inserted or not
  #define CHARGE_STATUS_PIN 10 //Input - Whether device is charging or not
  #define BATTERY_METER_PIN 2 //Input - Voltage divider output with 2x 220kohm
  #define BATTERY_METER_EN_PIN 3 //Output - Enable or disable voltage divider
#elif defined(BOARD_ESP32C6)
  #define SD_CS 2
  #define SD_MOSI 21
  #define SD_SCLK 22
  #define SD_MISO 23
  #define SD_EN 16 //Input - Is SD card inserted or not
  #define CHARGE_STATUS_PIN 18 //Input - Whether device is charging or not
  #define BATTERY_METER_PIN 0 //Input - Voltage divider output with 2x 220kohm
  #define BATTERY_METER_EN_PIN 1 //Output - Enable or disable voltage divider
  #define USER_LED 15
#else
    #error "Please define a supported board."
#endif

FtpServer ftpSrv;
WebServer server(80);

char wifi_mode[8] = "CLIENT"; // CLIENT, AP
char ssid[32] = "WIFINAME";
char password[32] = "PASSWORD";
char ftp_user[32] = "user";
char ftp_pass[32] = "password";

float readBatteryVoltage() {
    uint32_t Vbatt = 0;

    // Take 16 readings for averaging
    for (int i = 0; i < 16; i++) {
        Vbatt += analogReadMilliVolts(BATTERY_METER_PIN); // Use corrected ADC readings
    }

    // Calculate the averaged voltage, accounting for attenuation ratio (1/2)
    float Vbattf = 2.0 * (Vbatt / 16.0) / 1000.0; // Convert mV to V

    return Vbattf; // Return the voltage in volts
}


void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace){
	Serial.print(">>>>>>>>>>>>>>> _callback " );
	Serial.print(ftpOperation);
	Serial.print(" ");
	Serial.print(freeSpace);
	Serial.print(" ");
	Serial.println(totalSpace);
	if (ftpOperation == FTP_CONNECT) Serial.println(F("CONNECTED"));
	if (ftpOperation == FTP_DISCONNECT) Serial.println(F("DISCONNECTED"));
};


void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize) {
    Serial.print("Transfer Operation: ");
    Serial.println(ftpOperation);
    Serial.print("File Name: ");
    Serial.println(name ? name : "NULL");
    Serial.print("Transferred Size: ");
    Serial.println(transferredSize);
}

void handleRoot() {
    float voltage = readBatteryVoltage();
    float percentage = (voltage - 3.0) / (4.2 - 3.0) * 100.0;
    percentage = constrain(percentage, 0, 100);

    String html = "<html><body>";
    html += "<h1>Battery Percentage: " + String(percentage) + "%</h1>";
    html += "<p>Battery Voltage: " + String(voltage, 2) + " V</p>";
    html += "<p>SD Card Inserted: " + String(digitalRead(SD_EN)) + "</p>";
    html += "<p>Voltage Divider Active: " + String(digitalRead(BATTERY_METER_EN_PIN)) + "</p>";
    html += "<form method='POST' action='/toggle'><button type='submit'>Toggle Divider</button></form>";
    html += "<form method='POST' action='/led'><button type='submit'>Toggle LED</button></form>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleToggle() {
    digitalWrite(BATTERY_METER_EN_PIN, !digitalRead(BATTERY_METER_EN_PIN));
    server.sendHeader("Location", "/");
    server.send(303); // Redirect to root
}

void handleLed() {
    digitalWrite(USER_LED, !digitalRead(USER_LED));
    server.sendHeader("Location", "/");
    server.send(303); // Redirect to root
}

void setup() {
  Serial.begin(115200);

  // Setup GPIOs
  pinMode(SD_EN, INPUT);
  pinMode(BATTERY_METER_PIN, INPUT);
  pinMode(CHARGE_STATUS_PIN, INPUT);
  pinMode(BATTERY_METER_EN_PIN, OUTPUT);
  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED,1);

  //Is the SD card inserted
  if (0 == digitalRead(SD_EN)){
    Serial.println("SD Card Not inserted!!!");
  }else{
    Serial.println("SD Card Found!");
  }

  // Initialize SD card
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS,SPI)) {
    Serial.println("SD card initialization failed!");
    while (true)
      ;
  }
  Serial.println("SD card initialized.");

  // Read configuration
  readConfig();
  if (strcmp(wifi_mode, "") == 0 || strcmp(ssid, "") == 0 || strcmp(password, "") == 0 || strcmp(ftp_user, "") == 0 || strcmp(ftp_pass, "") == 0) {
    Serial.println("Please fill in the blanks in the config file.");
    while (true)
      ;
  }

  if (strcmp(wifi_mode, "CLIENT") == 0) {
    // Start Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Wi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }else{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.println("Wi-Fi AP started.");

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }

  // Start FTP server
  ftpSrv.setCallback(_callback);
  ftpSrv.setTransferCallback(_transferCallback);
  ftpSrv.begin(ftp_user, ftp_pass, " ESP32 MicroSD NAS by SiloCityLabs");//Change the username and password
  Serial.println("FTP server started.");

  // Start Web server
  server.on("/", handleRoot);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/led", HTTP_POST, handleLed);
  server.begin();
  Serial.println("Web server started.");

  //Enable led
  digitalWrite(USER_LED,0);

}

void loop() {
  // Handle FTP server
  ftpSrv.handleFTP();

  // Handle Web server
  server.handleClient();

}

void readConfig() {
  // Check if the config file exists
  if (!SD.exists("/config.txt")) {
    writeExampleConfig();
    Serial.println("Config file not found. Creating a new one.");
    return;
  }

  // Open the config file for reading
  File file = SD.open("/config.txt");
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim(); // Remove whitespace or newline characters

      Serial.println("Read line: " + line);

      if (line.startsWith("MODE=")) {
        strncpy(wifi_mode, line.substring(5).c_str(), sizeof(wifi_mode) - 1);
        wifi_mode[sizeof(wifi_mode) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("HOMEWIFI=")) {
        strncpy(ssid, line.substring(9).c_str(), sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("HOMEPASS=")) {
        strncpy(password, line.substring(9).c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("FTPUSER=")) {
        strncpy(ftp_user, line.substring(8).c_str(), sizeof(ftp_user) - 1);
        ftp_user[sizeof(ftp_user) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("FTPPASS=")) {
        strncpy(ftp_pass, line.substring(8).c_str(), sizeof(ftp_pass) - 1);
        ftp_pass[sizeof(ftp_pass) - 1] = '\0'; // Null-terminate
      } else {
        Serial.println("Invalid line in config file: " + line);
      }
    }
    file.close();
  } else {
    Serial.println("Error opening config file.");
    return;
  }

  // Log the read values
  Serial.println("===== CONFIGURATION =====");
  Serial.print("Wi-Fi Mode: ");
  Serial.println(wifi_mode);
  Serial.print("Wi-Fi SSID: ");
  Serial.println(ssid);
  Serial.print("Wi-Fi Password: ");
  Serial.println(password);
  Serial.print("FTP User: ");
  Serial.println(ftp_user);
  Serial.print("FTP Password: ");
  Serial.println(ftp_pass);
  Serial.println("=========================");
}


void writeExampleConfig() {
  File file = SD.open("/config.txt", FILE_WRITE);
  if (file) {
    file.println("MODE=CLIENT");
    file.println("HOMEWIFI=WIFINAME");
    file.println("HOMEPASS=PASSWORD");
    file.println("FTPUSER=user");
    file.println("FTPPASS=password");
    file.close();
    Serial.println("Example File written.");
  } else {
    Serial.println("Error opening file.");
  }
}