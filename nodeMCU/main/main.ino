// WildfootW 2018
// https://github.com/WildfootW

//#include "lwip/tcp_impl.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
extern "C"
{
    #include <cont.h>
    #include "user_interface.h"
    extern cont_t g_cont;
}

#include "CUSTOM_SETTING.hpp"

#define PWM_MOSFET_PIN 5

static int light_mode;
static bool is_online;
static os_timer_t pwm_timer;
static const int pwm_status_set[4][20] = //以陣列儲存亮度的模式 array[幾種模式][模式內容] 並注意要同時更改pwm_status_set_num
{
    {0}, //燈不亮並循環播放 數字由0 ~ 1023
    {0, 128, 256, 384, 512, 640, 768, 896, 1023}, //燈由小到亮並循環播放
    {0, 0, 1023, 1023, 0, 0, 1023, 1023}, //燈閃爍並循環播放
    {1023, 896, 768, 640, 512, 384, 256, 128, 0}
};
static const int pwm_status_set_num[4] = {1, 9, 8, 9}; //模式內容中有幾個數值 對應到pwm_status_set
static int pwm_status_count[4] = {0};
static WiFiClientSecure client;

void update_status_code()
{
    if(!is_online)
    {
        light_mode = 0;
        return;
    }
////////////////////// get request //////////////////////
    String url_path = "/pinecone/status.html";

    Serial.print("requesting URL: ");
    Serial.print(remote_host);
    Serial.println(url_path);
    client.print(String("GET ") + url_path + " HTTP/1.1\r\n" +
                 "Host: " + remote_host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n" +
                 "\r\n");
    Serial.println("request sent");

    // receive header
    while(client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("headers receive end");
            break;
        }
    }
    // receive container
    String line = client.readStringUntil('\n');
    Serial.print("status code: ");
    int ret_code = line[0] - '0';
    Serial.println(ret_code);
    light_mode = ret_code;
    return;
}

void pwm_callback( void* arg )
{
    if(!(pwm_status_count[light_mode] < pwm_status_set_num[light_mode]))
        pwm_status_count[light_mode] = 0;

    analogWrite(PWM_MOSFET_PIN, pwm_status_set[light_mode][pwm_status_count[light_mode]]);
    pwm_status_count[light_mode]++;
    return;
}

void setup()
{
    Serial.begin(115200);
    Serial.flush();

    // stack & heap analysis
    //Serial.println(String("(in setup())Free heap  : ") + ESP.getFreeHeap());
    //Serial.println(String("(in setup())Free stack : ") + cont_get_free_stack(&g_cont));

    // set wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(my_wifi_ssid, my_wifi_password);

    // set timer
    os_timer_setfn(&pwm_timer, pwm_callback, NULL);
    os_timer_arm(&pwm_timer, 1000, true);   //paramenter2 為燈的亮度更新頻率

    // set pwm pin
    pinMode(PWM_MOSFET_PIN, OUTPUT);
}

void loop()
{
    // stack & heap analysis
    //Serial.println(String("(loop top)Free heap  : ") + ESP.getFreeHeap());
    //Serial.println(String("(loop top)Free stack : ") + cont_get_free_stack(&g_cont));

////////////////////// check if WIFI still connected //////////////////////
    ETS_GPIO_INTR_DISABLE();
    for(int count = 0;count < 5, WiFi.status() != WL_CONNECTED;count++)
    {
        is_online = false;
        Serial.print("connecting to ");
        Serial.print(my_wifi_ssid);
        Serial.println(" ...");
        delay(1000);
        if(WiFi.status() == WL_CONNECTED)
        {
            is_online = true;
            Serial.print("WiFi connected, IP address : ");
            Serial.println(WiFi.localIP());
        }
    }
    ETS_GPIO_INTR_ENABLE();

    if(is_online)
    {
        // Use WiFiClientSecure class to create TLS connection
        Serial.print("Connecting to ");
        Serial.println(remote_host);
        if(!client.connect(remote_host, httpsPort))
        {
            Serial.print("Connection to");
            Serial.print(remote_host);
            Serial.println(" failed.");
            return;
        }

        update_status_code();

    }
    else
    {
        delay(5000);
    }
    //if(client.verify(fingerprint, remote_host))
    //    Serial.println("Certificate matches");
    //else
    //    Serial.println("Warning : Certificate doesn't match");

    delay(5000);

    // stack & heap analysis
    //Serial.println(String("(before delete)Free heap  : ") + ESP.getFreeHeap());
    //Serial.println(String("(before delete)Free stack : ") + cont_get_free_stack(&g_cont));
}

