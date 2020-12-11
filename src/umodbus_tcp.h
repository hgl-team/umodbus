#ifndef _UMODBUS_TCP_H_
#define _UMODBUS_TCP_H_

#include <Client.h>
#include "umodbus.h"

#ifndef UMODBUS_TCP_BUFFER_SIZE
#define UMODBUS_TCP_BUFFER_SIZE     50
#endif

namespace umodbus {

typedef struct __attribute__ ((__packed__)) {
    uint16_t transaction_identifier;
    uint16_t protocol_id;
    uint16_t length;
    uint8_t unit_id;
} mbap_header_t;

class uModbusTcp: public uModbus
{
private:
    Client * client;
    uint8_t output_buffer[UMODBUS_TCP_BUFFER_SIZE];
    size_t wr_cursor = 0;
public:
    uModbusTcp(const uint8_t& unit_id, register_t * buff, const size_t & len);
    ~uModbusTcp();

    void accept(Client * client);
    void disconnect();
protected:
    virtual uint8_t read();
    virtual size_t  read(uint8_t * buff, const size_t & len);
    virtual void    write(const uint8_t & val);
    virtual size_t  write(const uint8_t * buff, const size_t & len);
    virtual bool    prepare_response();
    virtual bool    data_available();
    virtual void    send();
};

};


#endif
