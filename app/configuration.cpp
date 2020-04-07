#include <Default/configuration.h>
#include <JsonObjectStream.h>
#include <SmingCore.h>

StandardConfig ActiveConfig;

#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(8))

extern	String NetworkSSID;
extern	String NetworkPassword;
extern  SoilConfig SoilCfg;

void loadConfig()
{
    DynamicJsonBuffer jsonBuffer;
    if (fileExist(SOIL_CONFIG_FILE))
    {
        int size = fileGetSize(SOIL_CONFIG_FILE);
        char* jsonString = new char[size + 1];
        fileGetContent(SOIL_CONFIG_FILE, jsonString, size + 1);
        JsonObject& root = jsonBuffer.parseObject(jsonString);

        SoilCfg.NetworkSSID = String((const char*)root["ssid"]);
        SoilCfg.NetworkPassword = String((const char*)root["password"]);
        SoilCfg.dry = root["dry"];
        SoilCfg.flood = root["flood"];
    }
    else
    {
        debugf("Il file non esiste");
        SoilCfg.NetworkSSID = WIFI_SSID;
        SoilCfg.NetworkPassword = WIFI_PWD;
    }
}


void saveConfig()
{
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["ssid"] = SoilCfg.NetworkSSID.c_str();
    root["password"] = SoilCfg.NetworkPassword.c_str();
    root["dry"] = SoilCfg.dry;
    root["flood"] = SoilCfg.flood;

    char buf[3048];
    root.prettyPrintTo(buf, sizeof(buf));
    fileSetContent(SOIL_CONFIG_FILE, buf);
}

