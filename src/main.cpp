#include <CrsfSerial.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 5

#define TXcrsft Serial

uint8_t RX1uplinkLinkQuality = 0;
uint8_t RX2uplinkLinkQuality = 0;

CrsfSerial RXcrsf1(Serial1, CRSF_BAUDRATE);
CrsfSerial RXcrsf2(Serial2, CRSF_BAUDRATE);

static void RX1packetLinkStatistics(crsfLinkStatistics_t *link)
{
  RX1uplinkLinkQuality = link->uplink_Link_quality;
  //TXcrsft.print("RX1: ");
  //TXcrsft.print(link->uplink_Link_quality, DEC);
  //TXcrsft.println(" %");
}


static void RX2packetLinkStatistics(crsfLinkStatistics_t *link)
{
  RX2uplinkLinkQuality = link->uplink_Link_quality;
  //TXcrsft.print("RX2: ");
  //TXcrsft.print(link->uplink_Link_quality, DEC);
  //TXcrsft.println(" %");
}

static void setupCrsf1()
{
    RXcrsf1.onPacketLinkStatistics = &RX1packetLinkStatistics;
    RXcrsf1.begin();
}

static void setupCrsf2()
{   
    RXcrsf2.onPacketLinkStatistics = &RX2packetLinkStatistics;
    RXcrsf2.begin();
}

void setup()
{
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    TXcrsft.begin(420000);
    setupCrsf1();
    setupCrsf2();
    RXcrsf1.begin();
    RXcrsf2.begin();
    Serial1.begin(420000);
    Serial2.begin(420000);
}

void loop()
{
    if(RX1uplinkLinkQuality > RX2uplinkLinkQuality)
    {
        if(Serial1.available())
        {
            TXcrsft.write(Serial1.read());
            esp_task_wdt_reset();
        }
    }
    else if(RX1uplinkLinkQuality > RX2uplinkLinkQuality)
    {
        if(Serial2.available())
        {
            TXcrsft.write(Serial2.read());
            esp_task_wdt_reset();
        }
    }
    else
    {
        // FAILSAFE?
    }
    RXcrsf1.loop();
    RXcrsf2.loop();
}