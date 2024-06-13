// BINOME 17 MICHON Colin Boudif

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x9e, 0xb7, 0x36, 0x56, 0x3d, 0x09, 0x6a, 0xa2, 0xc3, 0x69, 0x83, 0xff, 0x32, 0x0f, 0x51, 0xcb, 0xf4, 0x5a, 0x10, 0xa8};

ESP8266WiFiMulti WiFiMulti;

const int oneWireBus = 4;
// Mise en place d’une instance oneWire pour communiquer avec des capteurs
//OneWire sinon le programme ne compile pas
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
float temp; // variable Temperature

String MAC_Address = " ";
String data = " ";
String requestString = " ";
String tempString = " ";
int request = 0;

void setup() {
  sensors.begin();

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("binome_17", "tpRT9025");
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);

    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0); // Temperature in Celsius degrees
    Serial.printf("Message: %.2f \n", temp);

    HTTPClient https;
    MAC_Address = WiFi.macAddress();
    request = request + 1;
    requestString = String(request);
    tempString = String(temp);

    // data contient toute les données pour constituer la requete final faite au serveur
    data = "https://192.168.1.129/echo2.php?string=mac:" + MAC_Address + "%20temperature:" + tempString + "%20request%20n°:" + requestString;
    Serial.println("ESP Board MAC Address: " + MAC_Address);

    Serial.println("[HTTPS] begin...");
    Serial.println(data);
    if (https.begin(*client, data)) {  // HTTPS (data cotient la requête)

      Serial.println("[HTTPS] GET...");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(10000);
}
