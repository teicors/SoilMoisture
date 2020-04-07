
#include <SmingCore.h>
#include <JsonObjectStream.h>
#include <Default/configuration.h>

HttpServer server;

bool serverStarted = false;
extern void sendData();
extern SoilConfig SoilCfg;
extern SoilMessage SoilMsg;
extern int readvalue;

void onIndex(HttpRequest &request, HttpResponse &response)
{
    TemplateFileStream *tmpl = new TemplateFileStream("index.html");
    auto &vars = tmpl->variables();
    vars["now_clock"] = SystemClock.now(eTZ_UTC);
    vars["perc"] = 100-SoilMsg.stato;
    debugf("Sent Home Page");
    response.sendNamedStream(tmpl);
}


void onConfiguration(HttpRequest &request, HttpResponse &response)
{
    loadConfig();
    if (request.method == HTTP_POST)
    {
        debugf("Update config");
        if (request.getPostParameter("SSID").length() > 0) // Network
        {
            SoilCfg.NetworkSSID = request.getPostParameter("SSID");
            SoilCfg.NetworkPassword = request.getPostParameter("Password");
        }
        if (request.getPostParameter("dry").length() > 0) // Network
        {
            SoilCfg.dry = request.getPostParameter("dry").toInt();
            SoilCfg.flood = request.getPostParameter("flood").toInt();
        }
        saveConfig();
        response.headers[HTTP_HEADER_LOCATION] = "/";
    }

    debugf("Send template");
    TemplateFileStream *tmpl = new TemplateFileStream("config.html");
    auto &vars = tmpl->variables();
    vars["SSID"] = SoilCfg.NetworkSSID;
    vars["Password"]=SoilCfg.NetworkPassword;
    vars["dry"] = SoilCfg.dry;
    vars["flood"]=SoilCfg.flood;
    vars["readvalue"]=readvalue;
    response.sendNamedStream(tmpl);
}

void onFile(HttpRequest& request, HttpResponse& response)
{
    String file = request.uri.getRelativePath();
    if(file[0] == '.')
            response.code = HTTP_STATUS_FORBIDDEN;
    else {
            response.setCache(86400, true); // It's important to use cache for better performance.
            response.sendFile(file);
    }
}

/// API ///

void onApiTime(HttpRequest &request, HttpResponse &response)
{
    uint32_t time = request.getPostParameter("time").toInt();
    debugf("valore time %d ", time);

    if (time > 0)
    {
        SystemClock.setTime(time, eTZ_UTC);
        debugf("Update config onSet");     

        Serial.print("ntpClientDemo Callback Time_t = ");
        Serial.print(time);
        Serial.print(" Time = ");
        Serial.println(SystemClock.getSystemTimeString());     
    }
}

void onApiDoc(HttpRequest &request, HttpResponse &response)
{
    TemplateFileStream *tmpl = new TemplateFileStream("api.html");
    auto &vars = tmpl->variables();
    vars["IP"] = (WifiStation.isConnected() ? WifiStation.getIP() : WifiAccessPoint.getIP()).toString();
    response.sendNamedStream(tmpl);
}

void onApiStatus(HttpRequest &request, HttpResponse &response)
{
    JsonObjectStream* stream = new JsonObjectStream();
    JsonObject& json = stream->getRoot();
    json["status"] = (bool)true;
    JsonObject& sensors = json.createNestedObject("sensors");
    response.sendDataStream(stream, MIME_JSON);
}

void OnApiShowInfo(HttpRequest &request, HttpResponse &response) {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: %x\r\n", spi_flash_get_id());
    Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));  
}

void OnAjaxDate(HttpRequest& request, HttpResponse& response) {
    JsonObjectStream* stream = new JsonObjectStream();
    JsonObject& json = stream->getRoot();
    json["status"] = (bool)true;
    json["perc"] = 100-SoilMsg.stato;
    json["now_clock"] = SystemClock.now(eTZ_UTC);
    json["readvalue"]=readvalue;
    response.sendDataStream(stream, MIME_JSON);
}

void onReboot(HttpRequest &request, HttpResponse &response)
{
    System.restart();
}


void onSetTime(HttpRequest &request, HttpResponse &response)
{
//    uint32_t time = request.getPostParameter("time","-1").toInt();
    uint32_t time = request.getPostParameter("time").toInt();
    debugf("valore time %d ", time);

    if (time > 0)
    {
        SystemClock.setTime(time, eTZ_UTC);
        debugf("Update config onSet");     

        Serial.print("ntpClientDemo Callback Time_t = ");
        Serial.print(time);
        Serial.print(" Time = ");
        Serial.println(SystemClock.getSystemTimeString());

        
    }
}

void startWebServer()
{
    if (serverStarted) return;

    server.listen(80);
    server.paths.set("/", onIndex);
    server.paths.set("/api", onApiDoc);
    server.paths.set("/api/status", onApiStatus);    
    server.paths.set("/api/time", onApiTime);
    server.paths.set("/api/info", OnApiShowInfo);    
    server.paths.set("/api/date", OnAjaxDate);    
    server.paths.set("/config", onConfiguration);
    server.paths.set("/api/SetTime", onSetTime);    
    server.paths.set("/reboot", onReboot);
    server.paths.setDefault(onFile);
    serverStarted = true;

    if (WifiStation.isEnabled())
            debugf("STA: %s", WifiStation.getIP().toString().c_str());
    if (WifiAccessPoint.isEnabled())
        debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}

/// FileSystem Initialization ///

HttpClient downloadClient;
int dowfid = 0;
void downloadContentFiles()
{
	debugf("DownloadContentFiles");

	downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoControl.html", "index.html");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoConfig.html", "config.html");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/MeteoControl/MeteoAPI.html", "api.html");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/bootstrap.css.gz");
	downloadClient.downloadFile("http://simple.anakod.ru/templates/jquery.js.gz",
								(RequestCompletedDelegate)([](HttpConnection& connection, bool success) -> int {
									if(success) {
										startWebServer();
									}
									return 0;
								}));
}
