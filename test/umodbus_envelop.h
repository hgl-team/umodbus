#ifndef _UMODBUS_H_
#define _UMODBUS_H_

#include <stddef.h>
#include <string.h>
#include "umodbus.h"
#include "testutils.h"

class uModbusEnvelop : umodbus::uModbus {
	array_t<uint8_t> read_buf;
	size_t read_cursor;
	array_t<uint8_t> write_buf;
	size_t write_cursor;
public:
	uModbusEnvelop() : uModbus() { 
		this->read_cursor = 0;
		this->write_cursor = 0;
	}

	uModbusEnvelop(const uint8_t & unit_id, umodbus::register_t * buff, const size_t &len) : uModbus(unit_id, buff, len) { 
		this->read_cursor = 0;
		this->write_cursor = 0;
	}

	virtual ~uModbusEnvelop() { }

	array_t<uint8_t> * get_read_buf() {
		return &(this->read_buf);
	}

	array_t<uint8_t> * get_write_buf() {
		return &(this->write_buf);
	}

	size_t get_read_cursor() {
		return this->read_cursor;
	}

	size_t get_write_cursor() {
		return this->write_cursor;
	}

    size_t enveloped_binary_search(const uint8_t & address) {
        return this->binary_search(address);
    }

    void enveloped_read_as_byte(const uint8_t & fnc) {
        this->read_as_byte(fnc);
    }

    void enveloped_read_as_register(const uint8_t & fnc) {
        this->read_as_register(fnc);
    }

    void enveloped_write_single_as_byte(const uint8_t & fnc) {
        this->write_single_as_byte(fnc);
    }

	void enveloped_write_single_as_register(const uint8_t & fnc) {
        this->write_single_as_register(fnc);
    }

	void enveloped_write_multiple_as_byte(const uint8_t & fnc) {
		this->write_multiple_as_byte(fnc);
	}

	void enveloped_write_multiple_registers(const uint8_t & fnc) {
		this->write_multiple_as_register(fnc);
	}

protected:
	virtual uint8_t read() {
		if(this->read_cursor < this->read_buf.size) {
			return *(this->read_buf.ptr + this->read_cursor++);
		} else {
			return 0;
		}
	}

    virtual size_t  read(uint8_t * buff, const size_t & len) {
		size_t realLen = ((this->read_buf.size - this->read_cursor) >= len) ? len : (this->read_buf.size - this->read_cursor);
		
		memcpy(buff, this->read_buf.ptr + this->read_cursor, realLen);

        this->read_cursor += realLen;

		return realLen;
	}

    virtual void    write(const uint8_t & val) {
		if(this->write_cursor < this->write_buf.size) {
			*(this->write_buf.ptr + this->write_cursor++) = val;
		}
	}

    virtual size_t  write(const uint8_t * buff, const size_t & len) {
		size_t realLen = ((this->write_buf.size - this->write_cursor) >= len) ? len : (this->write_buf.size - this->write_cursor);

		memcpy(this->write_buf.ptr + this->write_cursor, buff, realLen);

        this->write_cursor += realLen;

		return realLen;
	}

    virtual bool    prepare_response() {
		return true;
	}

	virtual void    send() { 
		return;
	}

	virtual void    read_mei_type(const uint8_t & fnc) {
		return;
	}

	virtual void    perform_diagnostics(const uint8_t & fnc) {
		return;
	}

};

#endif