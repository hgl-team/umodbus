#ifndef _U_MODBUS_H_
#define _U_MODBUS_H_

#include <stddef.h>
#include <stdint.h>

#define UMODBUS_PTROF(v)                    ((uint8_t *)(&(v))) 

#define UMODBUS_U16_PTROF(v)                ((uint16_t *)(&(v))) 
#define UMODBUS_U16_NPTROF(v,n)             ((uint16_t *)(&(v)) + (n))

#define UMODBUS_TOPDIV(a,b)                 (((a) + (b) - 1) / (b))

#define UMODBUS_SIZE_COIL                   1
#define UMODBUS_SIZE_REGISTER               2

#define UMODBUS_TYPE_COIL                   1
#define UMODBUS_TYPE_DISCRETE_INPUT         5
#define UMODBUS_TYPE_HOLDING_REGISTER       2
#define UMODBUS_TYPE_INPUT_REGISTER         6

#define UMODBUS_VALUEOF(a)                  ((a)->ptr)
#define UMODBUS_GET_SIZE(a)                 (((a)->type) & 3)
#define UMODBUS_IS_READONLY(a)              ((((a)->type) & 4) > 0)

#define UMODBUS_COIL_OFF                    0x0000
#define UMODBUS_COIL_ON                     0xFF00

#define UMODBUS_FNCODE_RD_M_COIL            0x01
#define UMODBUS_FNCODE_RD_M_DISCRETE_INPUT  0x02
#define UMODBUS_FNCODE_RD_M_HOLDING_REG     0x03
#define UMODBUS_FNCODE_RD_M_INPUT_REG       0x04
#define UMODBUS_FNCODE_WR_S_COIL            0x05
#define UMODBUS_FNCODE_WR_S_HOLDING_REG     0x06

#define UMODBUS_FNCODE_RD_EXCEPTION_STATUS  0x07
#define UMODBUS_FNCODE_DIAGNOSTICS          0x08
#define UMODBUS_FNCODE_GET_COMM_EV_CNTR     0x0B
#define UMODBUS_FNCODE_GET_COMM_EV_LOG      0x0C

#define UMODBUS_FNCODE_WR_M_COIL            0x0F
#define UMODBUS_FNCODE_WR_M_HOLDING_REGS    0x10
#define UMODBUS_FNCODE_REPORT_SVR_ID        0x11

#define UMODBUS_FNCODE_RD_FILE_RECORD       0x14
#define UMODBUS_FNCODE_WR_FILE_RECORD       0x15

#define UMODBUS_FNCODE_MSK_WR_REG           0x16
#define UMODBUS_FNCODE_RW_M_REG             0x17
#define UMODBUS_FNCODE_RD_FIFO_QUEUE        0x18
#define UMODBUS_FNCODE_RD_DEV_ID            0x2B

#define UMODBUS_LITTLE_ENDIAN               1
#define UMODBUS_BIG_ENDIAN                  2

namespace umodbus {

uint8_t umodbus_get_endianness();

typedef struct
{
    uint16_t address;
    uint8_t type;
    uint16_t * ptr;
} register_t;

class uModbus {
private:
    uint8_t unit_id;
    register_t * reg;
    size_t reg_size;
public:
    uModbus();
    uModbus(const uint8_t &unit_id, register_t * buff, const size_t & len);

    virtual ~uModbus() { }

    register_t * get_registers();
    void poll(); 

protected:
    virtual uint8_t read() = 0;
    virtual size_t  read(uint8_t * buff, const size_t & len) = 0;
    virtual void    write(const uint8_t & val) = 0;
    virtual size_t  write(const uint8_t * buff, const size_t & len) = 0;
    virtual bool    prepare_response() = 0;
    virtual void    send() = 0;

    void    read_data(uint16_t & val);
    void    write_data(const uint16_t & val);

    void set_registers(register_t * buff, const size_t & len);
    
    void read_as_byte(const uint8_t & fnc);
    void read_as_register(const uint8_t & fnc);

    void write_single_as_byte(const uint8_t & fnc);
    void write_single_as_register(const uint8_t & fnc);

    void write_multiple_as_byte(const uint8_t & fnc);
    void write_multiple_as_register(const uint8_t & fnc);
    
    void read_write_as_register(const uint8_t & fnc);

    virtual void read_mei_type(const uint8_t & fnc);
    virtual void execute_function(const uint8_t & fnc);

    size_t binary_search(const uint16_t & address);
};

};

#endif