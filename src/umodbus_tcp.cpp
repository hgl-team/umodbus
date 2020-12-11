#include "umodbus_tcp.h"
#include <Arduino.h>

namespace umodbus {


uModbusTcp::uModbusTcp(const uint8_t& unit_id, register_t * buff, const size_t & len) : uModbus(unit_id, buff, len) {
    this->wr_cursor = 0;
}

uModbusTcp::~uModbusTcp() { }

uint8_t uModbusTcp::read() {
    int val;
    while((val = this->client->read()) < 0);
    return (uint8_t)val;
}

size_t  uModbusTcp::read(uint8_t * buff, const size_t & len) {
    if(this->client->connected()) {
        size_t size = (size_t)(this->client->read(buff, len));
        return size;
    } else return 0;
}

void    uModbusTcp::write(const uint8_t & val) {
    if(this->wr_cursor < UMODBUS_TCP_BUFFER_SIZE) {
        this->output_buffer[this->wr_cursor] = val;
        this->wr_cursor += 1;
    }
}

size_t  uModbusTcp::write(const uint8_t * buff, const size_t & len) {
    if((this->wr_cursor + len) < UMODBUS_TCP_BUFFER_SIZE) {
        memcpy(this->output_buffer + this->wr_cursor, buff, len);
        this->wr_cursor += len;
    }
}

bool    uModbusTcp::prepare_response() {    
    mbap_header_t header;
    delay(10);
    this->wr_cursor = 0;

    if(this->data_available()) {

        this->read_data(header.transaction_identifier);     // read mbap header
        this->read_data(header.protocol_id);                // read mbap header
        this->read_data(header.length);                     // read mbap header
        header.unit_id = this->read();                      // read mbap header

        // copy the header into response buffer as-is. will be updated later.
        this->write_data(header.transaction_identifier);   
        this->write_data(header.protocol_id);
        this->write_data(header.length);   
        this->write(header.unit_id);        // Should validate unit_id. Not implemented yet.

        return true;
    } else {
        return false;
    }
}

void    uModbusTcp::send() {
    uint16_t size = this->wr_cursor - 6;
    uint8_t * len = this->output_buffer + 4;
    *(len)      = (uint8_t)((size & 0xFF00) >> 8);
    *(len + 1)  = (uint8_t)((size & 0x00FF));
    
    if(this->client != 0 && this->client->connected()) {
        this->client->write(this->output_buffer, this->wr_cursor);
    }
}

void    uModbusTcp::accept(Client * client) {
    this->client = client;
}

void    uModbusTcp::disconnect() {
    this->client = 0;
}

bool    uModbusTcp::data_available() {
    return this->client->available() > 0;
}

};