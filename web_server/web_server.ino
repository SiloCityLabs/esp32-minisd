#include <WiFi.h>
#include <SD.h>
#include <WebServer.h>

// Uncomment or define the specific board in the IDE or here
// #define BOARD_ESP32C3
#define BOARD_ESP32C6

// Board-specific configurations
#ifdef BOARD_ESP32C3
  #define SD_CS 4
  #define SD_MOSI 5
  #define SD_SCLK 6
  #define SD_MISO 7
  #define SD_EN 21 // Input - Is SD card inserted or not
#elif defined(BOARD_ESP32C6)
  #define SD_CS 2
  #define SD_MOSI 21
  #define SD_SCLK 22
  #define SD_MISO 23
  #define SD_EN 16 // Input - Is SD card inserted or not
#else
  #error "Please define a supported board."
#endif

WebServer server(80);

char wifi_mode[8] = "CLIENT"; // CLIENT, AP
char ssid[32] = "";
char password[32] = "";
char web_user[32] = ""; // Optional web authentication username
char web_pass[32] = ""; // Optional web authentication password

void setup() {
  Serial.begin(115200);

  // Setup GPIOs
  pinMode(SD_EN, INPUT);

  // Check if the SD card is inserted
  if (digitalRead(SD_EN) == LOW) {
    Serial.println("SD Card Not inserted!!!");
  } else {
    Serial.println("SD Card Found!");
  }

  // Initialize SD card
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("SD card initialization failed!");
    while (true);
  }
  Serial.println("SD card initialized.");

  // Read configuration
  readConfig();
  if (strcmp(wifi_mode, "") == 0 || strcmp(ssid, "") == 0 || strcmp(password, "") == 0) {
    Serial.println("Please fill in the blanks in the config file.");
    while (true);
  }

  // Start Wi-Fi
  if (strcmp(wifi_mode, "CLIENT") == 0) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Wi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.println("Wi-Fi AP started.");

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/file", handleFile);

  // Start the web server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  // Check for authentication if credentials are set
  if (strlen(web_user) > 0 && strlen(web_pass) > 0 && !server.authenticate(web_user, web_pass)) {
    return server.requestAuthentication();
  }

  String html = "<html><body>";
  html += "<h1>ESP32 MiniSD Web Server</h1>";
  html += "<p>Welcome to the ESP32 MiniSD Web Server.</p>";
  html += "<p>Access files on the SD card by visiting <a href='/file'>/file</a>.</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleFile() {
  // Check for authentication if credentials are set
  if (strlen(web_user) > 0 && strlen(web_pass) > 0 && !server.authenticate(web_user, web_pass)) {
    return server.requestAuthentication();
  }

  String path = server.arg("path");
  if (path.length() == 0) {
    server.send(400, "text/plain", "Bad Request: Missing 'path' parameter");
    return;
  }

  File file = SD.open(path);
  if (!file) {
    server.send(404, "text/plain", "File Not Found");
    return;
  }

  if (file.isDirectory()) {
    String html = "<html><body><h1>Directory Listing: " + path + "</h1><ul>";
    File entry = file.openNextFile();
    while (entry) {
      html += "<li><a href='/file?path=" + String(entry.name()) + "'>" + String(entry.name()) + "</a></li>";
      entry = file.openNextFile();
    }
    html += "</ul></body></html>";
    server.send(200, "text/html", html);
  } else {
    server.streamFile(file, getContentType(path));
  }
  file.close();
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".txt")) return "text/plain";
  else if (filename.endsWith(".pdf")) return "application/pdf";
  else return "application/octet-stream";
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

      if (line.startsWith("MODE=")) {
        strncpy(wifi_mode, line.substring(5).c_str(), sizeof(wifi_mode) - 1);
        wifi_mode[sizeof(wifi_mode) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("HOMEWIFI=")) {
        strncpy(ssid, line.substring(9).c_str(), sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("HOMEPASS=")) {
        strncpy(password, line.substring(9).c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("WEBUSER=")) {
        strncpy(web_user, line.substring(8).c_str(), sizeof(web_user) - 1);
        web_user[sizeof(web_user) - 1] = '\0'; // Null-terminate
      } else if (line.startsWith("WEBPASS=")) {
        strncpy(web_pass, line.substring(8).c_str(), sizeof(web_pass) - 1);
        web_pass[sizeof(web_pass) - 1] = '\0'; // Null-terminate
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
  Serial.print("Web User: ");
  Serial.println(strlen(web_user) > 0 ? web_user : "None");
  Serial.print("Web Password: ");
  Serial.println(strlen(web_pass) > 0 ? "******" : "None");
  Serial.println("=========================");
}

void writeExampleConfig() {
  File file = SD.open("/config.txt", FILE_WRITE);
  if (file) {
    file.println("MODE=CLIENT");
    file.println("HOMEWIFI=WIFINAME");
    file.println("HOMEPASS=PASSWORD");
    file.println("WEBUSER="); // Optional
    file.println("WEBPASS="); // Optional
    file.close();
    Serial.println("Example config file written.");
  } else {
    Serial.println("Error opening file.");
  }
}
