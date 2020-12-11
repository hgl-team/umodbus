#include <string.h>
#include "umodbus.h"

namespace umodbus {

uint8_t umodbus_get_endianness() {
    uint16_t i = 0x00FF;
    uint8_t * ptr = (uint8_t*)&i;
    return *ptr == 0x00 ? UMODBUS_BIG_ENDIAN : UMODBUS_LITTLE_ENDIAN;
}

uModbus::uModbus() {
    this->reg = 0;
    this->unit_id = 0;
    this->reg_size = 0;
}

uModbus::uModbus(const uint8_t &unit_id, register_t * buff, const size_t & len) {
    this->reg = buff;
    this->reg_size = len;
    this->unit_id = unit_id;
}

register_t * uModbus::get_registers() {
    return this->reg;
}

void uModbus::set_registers(register_t * buff, const size_t & len) {
    this->reg = buff;
    this->reg_size = len;
}

void uModbus::poll() {
    if(this->prepare_response()) {

        uint8_t fnc = this->read();

        switch (fnc)
        {
        case UMODBUS_FNCODE_RD_M_DISCRETE_INPUT:
        case UMODBUS_FNCODE_RD_M_COIL:
            this->read_as_byte(fnc);
            break;
        case UMODBUS_FNCODE_RD_M_INPUT_REG:
        case UMODBUS_FNCODE_RD_M_HOLDING_REG:
            this->read_as_register(fnc);
            break;

        case UMODBUS_FNCODE_WR_S_COIL:
            this->write_single_as_byte(fnc);
            break;
        case UMODBUS_FNCODE_WR_M_COIL:
            this->write_multiple_as_byte(fnc);
            break;
        case UMODBUS_FNCODE_WR_S_HOLDING_REG:
            this->write_single_as_register(fnc);
            break;
        case UMODBUS_FNCODE_WR_M_HOLDING_REGS:
            this->write_multiple_as_register(fnc);
            break;
        case UMODBUS_FNCODE_RW_M_REG:
            this->read_write_as_register(fnc);
            break;
        case UMODBUS_FNCODE_RD_DEV_ID:
            this->read_mei_type(fnc);
            break;
        case UMODBUS_FNCODE_DIAGNOSTICS:
        case UMODBUS_FNCODE_RD_EXCEPTION_STATUS:
        case UMODBUS_FNCODE_RD_FILE_RECORD:
        case UMODBUS_FNCODE_RD_FIFO_QUEUE:
        case UMODBUS_FNCODE_WR_FILE_RECORD:
        case UMODBUS_FNCODE_GET_COMM_EV_CNTR:
        case UMODBUS_FNCODE_GET_COMM_EV_LOG:
        case UMODBUS_FNCODE_REPORT_SVR_ID:
        case UMODBUS_FNCODE_MSK_WR_REG:
        default: 
            this->execute_function(fnc);
            break;
        }

        this->send();
    }
}

void uModbus::read_as_byte(const uint8_t & fnc) {
    uint16_t startingAddress = 0;
    uint16_t inputCount = 0;
    size_t regIndex = 0;

    this->read_data(startingAddress);
    this->read_data(inputCount);
    
    if(0x0000 <= inputCount && inputCount <= 0x07D0) {
        regIndex = this->binary_search(startingAddress);

        if(regIndex != SIZE_MAX && (regIndex + inputCount) <= reg_size) {
            uint8_t status[UMODBUS_TOPDIV(inputCount, 8)];
            bool success = true;

            for(uint16_t i = 0; i < inputCount; i++) {
                register_t * reg_i = reg + regIndex + i;
                status[i / 8] = ((i % 8) == 0) ? 0 : status[i / 8];

                if(UMODBUS_GET_SIZE(reg_i) == UMODBUS_SIZE_COIL) {
                    status[i / 8] |= (*UMODBUS_VALUEOF(reg_i) == UMODBUS_COIL_OFF)? 0 : (1 << (i % 8));
                } else {
                    success = false;
                    break;
                }
            }

            if(success) {
                this->write(fnc);
                this->write((uint8_t) UMODBUS_TOPDIV(inputCount, 8));
                this->write(status, UMODBUS_TOPDIV(inputCount, 8));
            } else {
                this->write(fnc + 0x80);
                this->write(0x04);
            }
        } else {
            this->write(fnc + 0x80);
            this->write(0x02);
        }
    } else {
        this->write(fnc + 0x80);
        this->write(0x03);
    }
}

void uModbus::read_as_register(const uint8_t & fnc) {
    uint16_t startingAddress;
    uint16_t inputCount;
    size_t regIndex;
    
    this->read_data(startingAddress);
    this->read_data(inputCount);

    if(0x0001 <= inputCount && inputCount <= 0x007D) {
        regIndex = this->binary_search(startingAddress);
        if(regIndex != SIZE_MAX && (regIndex + inputCount) <= reg_size) {
            uint16_t status[inputCount];
            bool success = true;

            for(uint16_t i = 0; i < inputCount; i++) {
                register_t * reg_i = reg + regIndex + i;
                uint8_t type = UMODBUS_GET_SIZE(reg_i);
                if(type == UMODBUS_SIZE_REGISTER) {
                    uint16_t val = *UMODBUS_VALUEOF(reg_i);
                    if(umodbus_get_endianness() == UMODBUS_LITTLE_ENDIAN) {
                        val = (val >> 8) + (val << 8);
                    }
                    memcpy(status + i, UMODBUS_PTROF(val), 2);
                } else {
                    success = false;
                    break;
                }
            }

            if(success) {
                this->write(fnc);
                this->write((uint8_t) inputCount * 2);
                this->write((uint8_t*) status, inputCount * 2);
            } else {
                this->write(fnc + 0x80);
                this->write(0x04);
            }
        } else {
            this->write(fnc + 0x80);
            this->write(0x02);
        }
    } else {
        this->write(fnc + 0x80);
        this->write(0x03);
    }
}

void uModbus::write_single_as_byte(const uint8_t & fnc) {
    uint16_t address;
    uint16_t value;
    size_t regIndex;
    
    this->read_data(address);
    this->read_data(value);

    if(value == UMODBUS_COIL_ON || value == UMODBUS_COIL_OFF) {
        regIndex = this->binary_search(address);

        if(regIndex != SIZE_MAX) {
            register_t * reg_i = this->reg + regIndex;

            if(UMODBUS_GET_SIZE(reg_i) == UMODBUS_SIZE_COIL) {
                *UMODBUS_VALUEOF(reg_i) = value;

                this->write(fnc);
                this->write_data(address);
                this->write_data(value);
            } else {
                this->write(fnc + 0x80);
                this->write(0x04);
            }
        } else {
            this->write(fnc + 0x80);
            this->write(0x02);
        }
    } else {
        this->write(fnc + 0x80);
        this->write(0x03);    
    }
}

void uModbus::write_single_as_register(const uint8_t & fnc) {
    uint16_t address;
    uint16_t value;
    size_t regIndex;
    
    this->read_data(address);
    this->read_data(value);
    
    regIndex = this->binary_search(address);

    if(regIndex != SIZE_MAX && (this->reg + regIndex)->type == UMODBUS_SIZE_REGISTER) {
        register_t * reg_i = this->reg + regIndex;

        if(UMODBUS_GET_SIZE(reg_i) == UMODBUS_SIZE_REGISTER) {
            memcpy(UMODBUS_VALUEOF(reg_i), &value, 2);

            this->write(fnc);
            this->write_data(address);
            this->write_data(value);
        } else {
            this->write(fnc + 0x80);
            this->write(0x04);
        }
    } else {
        this->write(fnc + 0x80);
        this->write(0x02);
    }
}

void uModbus::write_multiple_as_byte(const uint8_t & fnc) {
    uint16_t address;
    uint16_t outputCount;
    uint8_t byteCount;
    size_t regIndex;

    this->read_data(address);
    this->read_data(outputCount);
    byteCount = this->read();

    if(0x0001 <= outputCount && outputCount <= 0x07B0) {
        regIndex = this->binary_search(address);

        if(regIndex != SIZE_MAX && (regIndex + outputCount) <= this->reg_size) {
            uint8_t data[byteCount];
            bool success = true;

            this->read(data, byteCount);

            for(uint16_t i = 0; i < outputCount; i++) {
                uint8_t bk = data[i / 8];
                register_t * reg_i = this->reg + regIndex + i;

                if(UMODBUS_GET_SIZE(reg_i) == UMODBUS_SIZE_COIL) {
                    uint8_t bki = (bk & (1 << (i % 8)));
                    *(reg_i->ptr) = bki > 0 ? UMODBUS_COIL_ON : UMODBUS_COIL_OFF;
                } else {
                    success = false;
                    break;
                }
            }

            if(success) {
                this->write(fnc);
                this->write_data(address);
                this->write_data(outputCount);
            } else {
                this->write(fnc + 0x80);
                this->write(0x04);
            }
        } else {
            this->write(fnc + 0x80);
            this->write(0x02);
        }
    } else {
        this->write(fnc + 0x80);
        this->write(0x03);
    }
}

void uModbus::write_multiple_as_register(const uint8_t & fnc) {
    uint16_t address;
    uint16_t outputCount;
    uint8_t byteCount;
    size_t regIndex;

    this->read_data(address);
    this->read_data(outputCount);
    byteCount = this->read();

    if(0x0001 <= outputCount && outputCount <= 0x007B) {
        regIndex = this->binary_search(address);

        if(regIndex != SIZE_MAX && (regIndex + outputCount) <= this->reg_size) {
            
            bool success = true;

            for(uint16_t i = 0; i < outputCount; i++) {
                uint16_t value;    
                register_t * reg_i = this->reg + regIndex + i;

                this->read_data(value);

                if(success && UMODBUS_GET_SIZE(reg_i) == UMODBUS_SIZE_REGISTER) {
                    *(reg_i->ptr) = value;
                } else {
                    success = false;
                }
            }

            if(success) {
                this->write(fnc);
                this->write_data(address);
                this->write_data(outputCount);
            } else {
                this->write(fnc + 0x80);
                this->write(0x04);
            }
        } else {
            this->write(fnc + 0x80);
            this->write(0x02);
        }
    } else {
        this->write(fnc + 0x80);
        this->write(0x03);
    }
}
    

void uModbus::read_write_as_register(const uint8_t & fnc) {

}

void uModbus::read_mei_type(const uint8_t & fnc) {
    this->write(fnc + 0x80);
    this->write(0x01);
}

void uModbus::execute_function(const uint8_t & fnc) {
    this->write(fnc + 0x80);
    this->write(0x01);
}

size_t uModbus::binary_search(const uint16_t & address) {
    size_t first = 0;
    size_t last = this->reg_size - 1;
    size_t middle = (first + last) / 2;

    while (first <= last) {
        register_t * reg_i = this->reg + middle;
        if(reg_i->address < address) {
            first = middle + 1;
        } else if (reg_i->address > address) {
            last = middle - 1;
        } else { 
            return middle;
        }
        
        middle = (first + last) / 2;
    }

    return SIZE_MAX;
}

void    uModbus::read_data(uint16_t & val) {
    if(umodbus_get_endianness() == UMODBUS_LITTLE_ENDIAN) {
        val = this->read();
        val = (val << 8) + this->read();
    } else {
        val = this->read();
        val = val + (((uint16_t)this->read()) << 8);
    }
}

void    uModbus::write_data(const uint16_t & val) {
    if(umodbus_get_endianness() == UMODBUS_LITTLE_ENDIAN) {
        this->write((uint8_t)(val >> 8));
        this->write((uint8_t)(val & 0x00FF));
    } else {
        this->write((uint8_t)(val & 0x00FF));
        this->write((uint8_t)(val >> 8));
    }    
}

};