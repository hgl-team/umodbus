#ifndef _TESTUTILS_H_
#define _TESTUTILS_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include "umodbus.h"

template<typename T>
struct array_t{
	T * ptr;
	size_t size;
};

typedef struct __attribute__ ((__packed__)) {
    uint16_t address;
    uint16_t inputCount;
} read_coil_packet_t;

typedef struct __attribute__ ((__packed__)) {
    uint16_t address;
    uint16_t inputCount;
} read_register_packet_t;

typedef struct __attribute__ ((__packed__)) {
    uint16_t address;
    uint16_t value;
} write_coil_packet_t;

typedef struct __attribute__ ((__packed__)){
    uint16_t address;
    uint16_t outputCount;
    uint8_t byteCount;
} write_multiple_coil_packet_t;

typedef struct __attribute__ ((__packed__)){
    uint16_t address;
    uint16_t outputCount;
    uint8_t byteCount;
} write_multiple_register_packet_t;

class ArrayStream {
private:
    array_t<uint8_t> buff;
    size_t read_cursor;
    size_t write_cursor;
    uint8_t endianess;
public:
    ArrayStream(array_t<uint8_t> buff) {
        this->buff = buff;
        this->read_cursor = 0;
        this->write_cursor = 0;
        this->endianess = UMODBUS_BIG_ENDIAN;
    }
    virtual ~ArrayStream() { }

    size_t read(void * buf, const size_t & len) {
        size_t realLen = ((this->buff.size - this->read_cursor) >= len) ? len : (this->buff.size - this->read_cursor);
		
		memcpy(buf, this->buff.ptr + this->read_cursor, realLen);

        this->read_cursor += realLen;

		return realLen;
    }

    size_t write(const void * buf, const size_t & len) {
        size_t realLen = ((this->buff.size - this->write_cursor) >= len) ? len : (this->buff.size - this->write_cursor);

		memcpy(this->buff.ptr + this->write_cursor, buf, realLen);

        this->write_cursor += realLen;

		return realLen;
    }

    uint8_t read() {
        return this->buff.ptr[this->read_cursor++];
    } 

    void read(uint16_t & value) {
        this->read(&value, sizeof(uint16_t));

        if(umodbus::umodbus_get_endianness() != this->endianess) {
            this->byte_flip(value);  
        }
    }

    void write(const uint8_t & value) {
        this->buff.ptr[this->write_cursor++] = value;
    }

    void write(const uint16_t & value) {
        uint16_t copy = value;
        
        if(umodbus::umodbus_get_endianness() != this->endianess) {
            this->byte_flip(copy);
        }

        this->write(&copy, sizeof(uint16_t));
    }

    void rseek(const size_t & value) {
        this->read_cursor = value;
    }

    void roffset(const size_t & offset) {
        this->read_cursor += offset;
    }

    void wseek(const size_t & value) {
        this->write_cursor = value;
    }

    void woffset(const size_t & offset) {
        this->write_cursor += offset;
    }

    template<typename T>
    void byte_flip(T & val) {
        uint8_t * ptr = (uint8_t*)&val;
        size_t size = sizeof(T);

        for(int i = 0; i < size / 2; i++) {
            uint8_t temp = ptr[i];
            ptr[i] = ptr[size - i - 1];
            ptr[size - i - 1] = temp;
        }
    }
};

inline void write_packet(ArrayStream * stream, const read_coil_packet_t & packet) {
    stream->write(packet.address);
    stream->write(packet.inputCount);
}

inline void write_packet(ArrayStream * stream, const read_register_packet_t & packet) {
    stream->write(packet.address);
    stream->write(packet.inputCount);
}

inline void write_packet(ArrayStream * stream, const write_coil_packet_t & packet) {
    stream->write(packet.address);
    stream->write(packet.value);
}

inline void write_packet(ArrayStream * stream, const write_multiple_coil_packet_t & packet) {
    stream->write(packet.address);
    stream->write(packet.outputCount);
    stream->write(packet.byteCount);
}

inline void write_packet(ArrayStream * stream, const write_multiple_register_packet_t & packet) {
    stream->write(packet.address);
    stream->write(packet.outputCount);
    stream->write(packet.byteCount);
}

#endif
