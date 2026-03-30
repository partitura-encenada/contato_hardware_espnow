#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_log.h"
 
const int CANAL_ESPECIFICO = 5;
 
typedef struct struct_message {
    int id; 
    int gyro;
    int accel;
    int touch;
} struct_message;
 
static struct_message MIDImessage;
static struct_message bufferMessage;
volatile bool newData = false;
 
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    uint8_t macTransmissor[] = {0x14, 0x33, 0x5C, 0x52, 0x4D, 0xE0};
 
    if (memcmp(mac_addr, macTransmissor, 6) != 0) return;
    if (len != sizeof(struct_message)) return; // descarta pacote com tamanho errado
 
    memcpy(&MIDImessage, incomingData, sizeof(MIDImessage));
    newData = true;
}
 
void setup() {
    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_NONE); // silencia todos os logs internos do ESP32
 
    WiFi.mode(WIFI_STA);
    esp_wifi_set_max_tx_power(82);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(CANAL_ESPECIFICO, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
 
    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao inicializar ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
    if (newData) {
        // Copia para buffer local antes de processar
        // Evita que um novo callback sobrescreva MIDImessage durante a impressão
        memcpy(&bufferMessage, &MIDImessage, sizeof(MIDImessage));
        newData = false;
 
        // char fixo na stack — sem alocação dinâmica (String() causava fragmentação)
        char buf[64];
        snprintf(buf, sizeof(buf), "D/%d/%d/%d/%d",
                 bufferMessage.id,
                 bufferMessage.gyro,
                 bufferMessage.accel,
                 bufferMessage.touch);
        Serial.println(buf);
    }
}