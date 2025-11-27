// #include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "Galaxy M54 5G 44AD";
const char* password = "0545456527"; 

WebServer server(80);

void theDefaultRoute() {
    WiFiClient client = server.client();
    Serial.printf("Request from %s to %s\n",
                  client.remoteIP().toString().c_str(),
                  server.uri().c_str());
    server.send(200, "text/plain", "hello yair");
}

// 404 handler
void handleNotFound() {
    WiFiClient client = server.client();

    Serial.printf("Unknown request from %s to %s\n",
                  client.remoteIP().toString().c_str(),
                  server.uri().c_str());

    server.send(404, "text/plain", "Not found");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("Connecting to WiFi...");
    
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", theDefaultRoute);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}
