#include <umodbus.h>
#include <umodbus_tcp.h>
#include <Ethernet2.h>

#define LED_PIN 13

uint8_t mac[] = { 0xd2, 0x78, 0x54, 0x69, 0x16, 0x75 };

uint16_t enabled    = 0;
uint16_t led_state  = 0;
uint16_t time       = 1000;
uint32_t counter    = 0;
float    factor     = 0.24;
uint32_t curr_time  = 0;

umodbus::register_t registers[] = {
    // { <address>,     <type>,     <ptr to variable>  }
    // COIL REGISTER: points to a uint16_t value. if value is 0 then coil is OFF, otherwise value = 0xFF00.
    {  0, UMODBUS_TYPE_COIL,                UMODBUS_U16_PTROF(enabled) },
    // COIL REGISTER: points to a uint16_t value. if value is 0 then input is OFF, otherwise value = 0xFF00.
    {  0, UMODBUS_TYPE_DISCRETE_INPUT,      UMODBUS_U16_PTROF(led_state) },
    // HOLDING REGISTER: points to a uint16_t value. Can read/write.
    {  1, UMODBUS_TYPE_HOLDING_REGISTER,    UMODBUS_U16_PTROF(time) },
    // INPUT REGISTER: points to a uint16_t value. Can only read.
    {  2, UMODBUS_TYPE_INPUT_REGISTER,      UMODBUS_U16_PTROF(counter) },
    // HOLDING REGISTER as float: As data in modbus should be encoded in big-endian format, 
    // arduino must present the MSB first.
    {  3, UMODBUS_TYPE_HOLDING_REGISTER,    UMODBUS_U16_NPTROF(factor, 1) },
    {  4, UMODBUS_TYPE_HOLDING_REGISTER,    UMODBUS_U16_NPTROF(factor, 0) },
};
EthernetServer server(502);
umodbus::uModbusTcp temp_modbus_modbus(33, registers, 5);

void ethernet_loop();

void setup() {
    uint8_t attempt = 0;
    Serial.begin(115200);
    Ethernet.init();

    while (true)
    {
        if (Ethernet.begin(mac) == 0)
        {
            Serial.print("DHCP Fail. Attempt ");
            Serial.print(++attempt);
            Serial.println(" of 5");
            delay(2000);
        }
        else
        {
            break;
        }
    }
    
    curr_time = millis();
}


void loop() {
    ethernet_loop();

    uint32_t now = millis(); 
    uint32_t dt = now - curr_time;

    if(dt >= time && enabled != UMODBUS_COIL_OFF) {
        Serial.println("Enabled.");
        Serial.print("Led state: ");
        Serial.print(led_state);
        Serial.print("Factor: ");
        Serial.println(factor);

        led_state = (led_state == UMODBUS_COIL_ON) ? UMODBUS_COIL_OFF : UMODBUS_COIL_ON;
        digitalWrite(LED_PIN, (led_state == UMODBUS_COIL_ON) ? HIGH : LOW);
        curr_time = now;
    }
}

void ethernet_loop() {
    EthernetClient client = server.available();
    bool client_connected = false;
    if(client) {
        client_connected = true;
        Serial.println("Connected. Once the connection is closed, loop will resume.");
        while (client.connected()) {
            temp_modbus_modbus.accept(&client);
            temp_modbus_modbus.poll();
        }
    } 
    delay(5);
    client.stop();
    if(client_connected) {
        Serial.println("Disconnected.");
    }
}