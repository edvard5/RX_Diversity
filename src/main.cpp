#include <Arduino.h>

#define PACKET_SIZE 10 // Corrected packet size (without CRC)
#define START_BYTE 0xC8
#define TOTAL_SIZE (PACKET_SIZE + 2) // Start byte + packet + checksum

uint8_t LQ1_value;
uint8_t LQ2_value;

unsigned long previousMillis = 0;  
const long interval = 5000; // Time between resets


bool process_crsf_link_statistics_packet(uint8_t *packet, uint8_t &link_quality) {
    struct crsf_link_statistics {
        uint8_t uplink_rssi_1;
        uint8_t uplink_rssi_2;
        uint8_t uplink_link_quality;
        uint8_t uplink_snr;
        uint8_t active_antenna;
        uint8_t rf_mode;
        uint8_t uplink_tx_power;
        uint8_t downlink_rssi;
        uint8_t downlink_link_quality;
        uint8_t downlink_snr;
    } __attribute__((packed)); 
    
    // Validate checksum
    uint8_t received_checksum = packet[PACKET_SIZE];
    uint8_t calculated_checksum = 0;
    for (int i = 0; i < PACKET_SIZE; i++) {
        calculated_checksum += packet[i];
    }
    calculated_checksum &= 0xFF;

    if (calculated_checksum != received_checksum) {
        Serial.println("Checksum mismatch"); // disable while in use !!! only for debugging
        return false;
    }

    // Cast the packet to the crsf_link_statistics structure
    crsf_link_statistics *stats = (crsf_link_statistics *)packet;
/*
    // Debugging: Print all values
    Serial.print("uplink_rssi_1: "); Serial.println(stats->uplink_rssi_1);
    Serial.print("uplink_rssi_2: "); Serial.println(stats->uplink_rssi_2);
    Serial.print("uplink_link_quality: "); Serial.println(stats->uplink_link_quality);
    Serial.print("uplink_snr: "); Serial.println(stats->uplink_snr);
    Serial.print("active_antenna: "); Serial.println(stats->active_antenna);
    Serial.print("rf_mode: "); Serial.println(stats->rf_mode);
    Serial.print("uplink_tx_power: "); Serial.println(stats->uplink_tx_power);
    Serial.print("downlink_rssi: "); Serial.println(stats->downlink_rssi);
    Serial.print("downlink_link_quality: "); Serial.println(stats->downlink_link_quality);
    Serial.print("downlink_snr: "); Serial.println(stats->downlink_snr);
*/
    // Extract the uplink link quality value
    link_quality = stats->uplink_link_quality;
    return true;
}

void setup() {
    Serial.begin(115200);    // Initialize UART Port for debug/FC
    Serial.println("Diversity started"); // disable while in use !!! only for debugging
    Serial1.begin(115200);   // Initialize UART Port 1 for Receiver #1
    Serial2.begin(115200);   // Initialize UART Port 2 for Receiver #2
}

void loop() {

    unsigned long currentMillis = millis();
    // Read Receiver #1
    if (Serial1.available() >= TOTAL_SIZE) {
        uint8_t start_byte = Serial1.read();
        if (start_byte == START_BYTE) {
            uint8_t buffer[TOTAL_SIZE - 1];  
            Serial1.readBytes(buffer, TOTAL_SIZE - 1);
            if (process_crsf_link_statistics_packet(buffer, LQ1_value)) {
                Serial.print("LQ1_value: "); // disable while in use !!! only for debugging
                Serial.println(LQ1_value); // disable while in use !!! only for debugging
                Serial.print("Last Known LQ2:"); // disable while in use !!! only for debugging
                Serial.println(LQ2_value); // disable while in use !!! only for debugging
            }
        }
    }

    // Read Receiver #2
    if (Serial2.available() >= TOTAL_SIZE) {
        uint8_t start_byte = Serial2.read();
        if (start_byte == START_BYTE) {
            uint8_t buffer[TOTAL_SIZE - 1];  
            Serial2.readBytes(buffer, TOTAL_SIZE - 1);
            if (process_crsf_link_statistics_packet(buffer, LQ2_value)) {
                Serial.print("LQ2_value: "); // disable while in use !!! only for debugging
                Serial.println(LQ2_value); // disable while in use !!! only for debugging
                Serial.print("Last Known LQ1:"); // disable while in use !!! only for debugging
                Serial.println(LQ1_value); // disable while in use !!! only for debugging
            }
        }
    }

    // Determine which UART to use based on link quality
    // RX #1
    if(LQ1_value > LQ2_value){
        Serial1.write(Serial.read());
        if (Serial1.available()) {
            //Serial.write(Serial1.read()); // passthrough to FC
            Serial.println("Using RX1"); // disable while in use !!! only for debugging
        }
    }

    // RX #2
    if(LQ1_value < LQ2_value) {
        Serial2.write(Serial.read());
        if (Serial2.available()) {
            //Serial.write(Serial2.read()); // passthrough to FC
            Serial.println("Using RX2"); // disable while in use !!! only for debugging
        }
    }



  // LQ value reset after 5 seconds
    if (currentMillis - previousMillis >= interval) {
        Serial.println("LQ RESET"); // disable while in use !!! only for debugging
        previousMillis = currentMillis;
        LQ1_value = 0;
        LQ2_value = 0;
    }
}

