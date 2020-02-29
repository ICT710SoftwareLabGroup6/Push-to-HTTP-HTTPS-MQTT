
#define MQTTCLIENT_QOS2 1
#include <mbed.h>
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "MQTTClientMbedOs.h"

int arrivedcount = 0;


DigitalOut led(LED1);
InterruptIn button(USER_BUTTON);
Thread t;
EventQueue queue(5 * EVENTS_EVENT_SIZE);
Serial pc(USBTX, USBRX);
WiFiInterface *wifi;


const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int scan_wifi() {
    WiFiAccessPoint *ap;

    printf("Scan:\n");
    int count = wifi->scan(NULL,0);
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;
    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\n", count);

    delete[] ap;   

    return count; 
}


void Connectwifi() {
    int count;
    count = scan_wifi();
    if (count == 0) {
        pc.printf("No WIFI APs found - can't continue further.\n");
        return;
    }

    pc.printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        pc.printf("\nConnection error: %d\n", ret);
        return;
    }

    pc.printf("Success\n\n");
    pc.printf("MAC: %s\n", wifi->get_mac_address());
    pc.printf("IP: %s\n", wifi->get_ip_address());
    pc.printf("Netmask: %s\n", wifi->get_netmask());
    pc.printf("Gateway: %s\n", wifi->get_gateway());
    pc.printf("RSSI: %d\n\n", wifi->get_rssi());

    pc.printf("\nDone\n");    
}


 
 
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    pc.printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);

    pc.printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

    ++arrivedcount;
}
// --------- main ---------------------------
int main(int argc, char* argv[]) {

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {

        printf("ERROR: No WiFiInterface found.\n");

        return -1;
    }

    Connectwifi();

    pc.printf("Starting\n");

    // MQTT part

    float version = 0.6;

    char* topic = "@msg/to_me";
 
    pc.printf("HelloMQTT: version is %.2f\r\n", version);
 
    const char* hostname = "mqtt.netpie.io";
    int port = 1883;

    TCPSocket socket;
    MQTTClient client(&socket);
    socket.open(wifi);

    int rc = socket.connect(hostname,port);
    pc.printf("Connecting to %s:%d\r\n", hostname, port);

    // connect to client
 	// packet setup
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    data.MQTTVersion = 3;
    data.clientID.cstring = "56a8d999-cfe5-47f9-b400-b9c0f027feaf";
    data.username.cstring = "xEQEPeDv2NaTHa1e5H9os9NsXMoS9sHS";
    //data.password.cstring = "r$vvvTWA3vB3B-GNCnw*QozO8_ZSN2V2";

  	client.connect(data);
    // while(!client.isConnected()){
    // 	client.connect(data);
    // }

 	
 
    MQTT::Message message;
 
    // QoS 0
    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app version %f\r\n", version);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    // QoS 1
    sprintf(buf, "Hello World!  QoS 1 message from app version %f\r\n", version);
    message.qos = MQTT::QOS1;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    // QoS 2
    sprintf(buf, "Hello World!  QoS 2 message from app version %f\r\n", version);
    message.qos = MQTT::QOS2;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);

    
 
}
