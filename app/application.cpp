#include <SmingCore.h>
#include <NtpClientDemo.h>
#include <JsonObjectStream.h>
#include <Default/configuration.h> // application configuration
///////////////////////////////////////////////////////////////////
// wget "http://192.168.1.220/api/output?status=1&led=2" -o pippo
///////////////////////////////////////////////////////////////////
#include "special_chars.h"
#include "webserver.h"
#include <SolarCalculator.h>

// COLLEGAMENTO 1 indica passaggagio da 1 a 0, cioè pulsante collegato a massa
#define COLLEGAMENTO 0
#if COLLEGAMENTO == 1
    #define SALITA RISING
    #define DISCESA FALLING
    #define INIZIO INPUT_PULLUP
#else
    #define SALITA FALLING
    #define DISCESA RISING
    #define INIZIO INPUT
#endif

#define TYPEOFBOARD 13
#define MAX_READ 101
#define MAX_DEB 2

Timer Timer0;
Timer flashled;
Timer CronTimer;
Timer TempTimer;
Timer procTimer;

void connectOk(const String& SSID, MacAddress bssid, uint8_t channel);
void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason);
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway);
bool state = true;

int button0, button1, meter_clock0, meter_clock1, meter_clock2, meter_clock3;
int debounce0, debounce1, debounce2, debounce3, readvalue;

SoilConfig SoilCfg;
SoilMessage SoilMsg;
time_t last_clock0, last_clock1, last_clock2, last_clock3;

FtpServer ftp;

// Callback example using defined class ntpClientDemo
NtpClientDemo* demo;
void onNtpReceive(NtpClient& client, time_t timestamp);

// Forward declarations
void onMessageReceived(String topic, String message);
void sendData();

void blink()
{
        digitalWrite(LED_PIN, state);
        state = !state;
}

void JsonOnComplete(TcpClient& client, bool successful)
{
    // debug msg
    debugf("JsonOnComplete");
    debugf("successful: %d", successful);
    client.close();
}

void JsonOnReadyToSend1(TcpClient& client, TcpConnectionEvent sourceEvent)
{
    // debug msg
    debugf("JsonOnReadyToSend");
    debugf("sourceEvent: %d", sourceEvent);

    // The connection is made at the time of shipment
    if(sourceEvent == eTCE_Connected)
    {
//        DynamicJsonDocument doc(1024);
       DynamicJsonBuffer jsonBuffer;
        JsonObject& MsgToSend = jsonBuffer.createObject();
//        auto MsgToSend = doc.createNestedObject("MsgToSend");meter_clock
        MsgToSend["basetta"] = WifiStation.getIP().toString();
        MsgToSend["time"] = SoilMsg.unixtime;
        String MsgToSendAjax="basetta:"+WifiStation.getIP().toString();
        MsgToSendAjax=MsgToSendAjax="basetta:"+WifiStation.getIP().toString();
        
        client.sendString("POST /json HTTP/1.1\r\n");
        client.sendString("Accept: */*\r\n");
        client.sendString("Content-Type: application/json;charset=utf-8\r\n");
        client.sendString("Content-Length: {"+String(MsgToSendAjax.length()+3)+"}\r\n");
        client.sendString("\r\n");
//        char buf[MsgToSendAjax.length()];
//        MsgToSendAjax.printTo(buf, sizeof(buf)+1);
        char *buf = &MsgToSendAjax[0];
        client.sendString(MsgToSendAjax);
//        MsgToSend.printTo(Serial);
        client.sendString("\r\n");
    }
}

void JsonOnReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent)
{
    // debug msg
    debugf("JsonOnReadyToSend");
    debugf("sourceEvent: %d", sourceEvent);
    // The connection is made at the time of shipment
    if(sourceEvent == eTCE_Connected)
    {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& MsgToSend = jsonBuffer.createObject();
    MsgToSend["ip"]        = WifiStation.getIP().toString();
    MsgToSend["evento"]    = SoilMsg.evento;
    MsgToSend["valore"]     = SoilMsg.stato;
    client.sendString("POST /json HTTP/1.1\r\n");
    client.sendString("Accept: */*\r\n");
    client.sendString("Content-Type: application/json;charset=utf-8\r\n");
    client.sendString("Content-Length: "+String(MsgToSend.measureLength()+1)+"\r\n");
    client.sendString("\r\n");
    char buf[MsgToSend.measureLength()];
    MsgToSend.printTo(buf, sizeof(buf)+1);
    client.sendString(buf);
    MsgToSend.printTo(Serial);
    client.sendString("\r\n");
    Serial.printf("\r\nFree Heap: %d\r\n", system_get_free_heap_size());
    }
}

bool JsonOnReceive(TcpClient& client, char *buf, int size)
{
    // debug msg
    debugf("JsonOnReceive");
    debugf("%s", buf);
    client.close();
    return true;
}

// Create an object of class JsonMon TcpClient
TcpClient JsonMon(JsonOnComplete, JsonOnReadyToSend, JsonOnReceive);

// This function will be called by timer
void sendData()
{
    // We read the sensor data
    // connect to the server narodmon
    JsonMon.connect(JSON_HOST, JSON_PORT);
}


// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
    Serial.print(topic);
    Serial.print(":\r\n\t"); // Pretify alignment for printing
    Serial.println(message);
}


////// WEB Clock //////
//Timer clockRefresher;
//HttpClient clockWebClient;
//uint32_t lastClockUpdate = 0;
//DateTime clockValue;
//const int clockUpdateIntervalMs = 10 * 60 * 1000; // Update web clock every 10 minutes


void startFTP()
{
    if (!fileExist("index.html"))
            fileSetContent("index.html", "<h3>Please connect to FTP and upload files from folder 'web/build' (details in code)</h3>");

    // Start FTP server
    ftp.listen(21);
    ftp.addUser("me", "123"); // FTP account
}

    
void SendPresence()
{
    SoilMsg.evento=SEND_PRESENCE;
    SoilMsg.stato=SoilMoisture;
    sendData();
    Serial.print("SendPresence\n");
}

// Callback example using defined class ntpClientDemo
//ntpClientDemo *demo;

void process()
{
    int percentage;
    readvalue = analogRead(A0);
    percentage=(int)((float)(readvalue-SoilCfg.flood)/(float)(SoilCfg.dry-SoilCfg.flood)*100);
    WDT.alive();
    Serial.print("percentage: ");
    Serial.print(percentage);
    Serial.print("  SoilCfg.flood: ");
    Serial.print(SoilCfg.flood);
    Serial.print("  SoilCfg.dry: ");
    Serial.print(SoilCfg.dry);
    Serial.print("  readvalue: ");
    Serial.println(readvalue);
    SoilMsg.stato=percentage;
    
}


void onPrintSystemTime() {
    Serial.print("Local Time    : ");
    Serial.println(SystemClock.getSystemTimeString());
    Serial.print("UTC Time: ");
    Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));
}


// Called when time has been received by NtpClient (option 1 or 2)
// Either after manual requestTime() or when
// and automatic request has been made.
void onNtpReceive(NtpClient& client, time_t timestamp) {
    SystemClock.setTime(timestamp,eTZ_UTC);

    debugf("Time synchronized: ");
    Serial.println(SystemClock.getSystemTimeString());
}

// Will be called when WiFi station was connected to AP
void connectOk(const String& SSID, MacAddress bssid, uint8_t channel)
{
    debugf("I'm CONNECTED");
    WifiAccessPoint.enable(false);
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{

    Serial.println(WifiStation.getIP().toString());
//    Timer0.initializeMs(50, check_button0).start();

    debugf("connected");

    startWebServer();
    startFTP();
    SendPresence();
    demo = new NtpClientDemo();


}

// Will be called when WiFi station timeout was reached
void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
    debugf("I'm NOT CONNECTED!");
    WifiAccessPoint.config("SoilConfig", "", AUTH_OPEN);
    WifiAccessPoint.enable(true);

    startWebServer();

    WifiStation.disconnect();
    WifiStation.connect();
}


void init()
{
    procTimer.initializeMs(500, blink).start();
    WDT.enable(false);
    Serial.systemDebugOutput(true); // Enable debug output
    debugf("Init!");

    spiffs_mount(); // Mount file system, in order to work with files
    Serial.begin(SERIAL_BAUD_RATE); // 115200 or 9600 by default
    delay(3000);
    
    loadConfig();
    procTimer.initializeMs(5000, process).start();    
    // set timezone hourly difference to UTC
    SystemClock.setTimeZone(0);   

    WifiStation.enable(true);
    WifiAccessPoint.enable(false);
    WifiStation.config(SoilCfg.NetworkSSID, SoilCfg.NetworkPassword);
    
    WifiEvents.onStationConnect(connectOk);
    WifiEvents.onStationDisconnect(connectFail);
    WifiEvents.onStationGotIP(gotIP);
    
}
