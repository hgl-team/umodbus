/*
Copyright 2020 Jerson Leonardo Huerfano Romero

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <gtest/gtest.h>
#include <endian.h>
#include <iostream>

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#include "umodbus.h"
#include "umodbus_envelop.h"
#include "string.h"

using namespace testing;

uint16_t registers[10];

class uModbusTestBase: public testing::Test {
public:
	uModbusEnvelop envelop;
	
};

class uModbusCoilTest: public uModbusTestBase {
public:
	umodbus::register_t registers[10];
	uint16_t register_value_buf[10];
	uint8_t input[50];
	uint8_t output[50];
	
	uModbusCoilTest() {
		envelop = uModbusEnvelop(0, registers, 10);

		(envelop.get_read_buf())->ptr 	= input;
		(envelop.get_write_buf())->ptr 	= output;

		(envelop.get_read_buf())->size 	= 50;
		(envelop.get_write_buf())->size = 50;
	}

	void configure_registers(const uint8_t & registerSize) {
		for(size_t i = 0; i < 10; i++) {
			registers[i] = {
				(uint16_t)i, registerSize, (register_value_buf + i)
			};
			this->set_register(i, 0);
		}
	}

	void reset_registers() {
		for(size_t i = 0; i < 10; i++) {
			register_value_buf[i] = 0;
		}
	}

	void set_register(const size_t & index, const uint16_t & value) {
		*(register_value_buf + index) = value;
	}

	void set_registers(const uint16_t & value) {
		for(size_t i = 0; i < 10; i++) {
			this->set_register(i, value);
		}
	}
};

TEST_F(uModbusCoilTest, searchExistingAddress) {
	uint16_t address = 4;

	this->configure_registers(UMODBUS_TYPE_COIL);
	size_t index = this->envelop.enveloped_binary_search(address);

	ASSERT_EQ(address, index);
}

TEST_F(uModbusCoilTest, searchNonExistingAddress) {
	uint16_t address = 14;

	this->configure_registers(UMODBUS_TYPE_COIL);
	size_t index = this->envelop.enveloped_binary_search(address);

	ASSERT_EQ(SIZE_MAX, index);
}

TEST_F(uModbusCoilTest, readSingleOffCoil) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });
	read_coil_packet_t packet = { 4, 1 };
	uint8_t fnc;
	uint8_t bytelen;
	uint8_t value;

	this->configure_registers(UMODBUS_TYPE_COIL);
	this->reset_registers();
	write_packet(&is, packet);

	this->envelop.enveloped_read_as_byte(UMODBUS_FNCODE_RD_M_COIL);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_COIL);
	bytelen = os.read();
	ASSERT_EQ(bytelen, 1);
	value = os.read();
	ASSERT_EQ(value, 0);
}

TEST_F(uModbusCoilTest, readSingleOnCoil) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });
	read_coil_packet_t packet = { 4, 1 };
	uint8_t fnc;
	uint8_t bytelen;
	uint8_t value;

	this->configure_registers(UMODBUS_TYPE_COIL);
	this->set_register(0, 1);
	write_packet(&is, packet);

	this->envelop.enveloped_read_as_byte(UMODBUS_FNCODE_RD_M_COIL);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_COIL);
	bytelen = os.read();
	ASSERT_EQ(bytelen, 1);
	value = os.read();
	ASSERT_EQ(value, 0);
}

TEST_F(uModbusCoilTest, readMultipleOffCoil) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });
	read_coil_packet_t packet = { 0, 9 };
	uint8_t fnc;
	uint8_t bytelen;
	uint8_t value;

	this->configure_registers(UMODBUS_TYPE_COIL);
	this->reset_registers();
	write_packet(&is, packet);

	this->envelop.enveloped_read_as_byte(UMODBUS_FNCODE_RD_M_COIL);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_COIL);
	bytelen = os.read();
	ASSERT_EQ(bytelen, 2);
	
	for(uint8_t i = 0; i < bytelen; i++) {
		value = os.read();

		for(uint8_t i = 0; i < (9 - 8 * i); i++) {
			ASSERT_EQ(0, value & (1 << i));
		}
	}
}

TEST_F(uModbusCoilTest, readMultipleOnCoil) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });

	read_coil_packet_t packet = { 0, 9 };
	
	uint8_t fnc;
	uint8_t bytelen;
	uint8_t value;

	this->configure_registers(UMODBUS_TYPE_COIL);
	this->set_registers(UMODBUS_COIL_ON);
	write_packet(&is, packet);

	this->envelop.enveloped_read_as_byte(UMODBUS_FNCODE_RD_M_COIL);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_COIL);
	bytelen = os.read();
	ASSERT_EQ(bytelen, 2);

	for(uint8_t i = 0; i < bytelen; i++) {
		value = os.read();

		for(uint8_t i = 0; i < (9 - 8 * i); i++) {
			ASSERT_EQ(1, value & (1 << i));
		}
	}
}

TEST_F(uModbusCoilTest, readSingleRegister) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });
	read_register_packet_t packet = { 4, 1 };
	uint8_t fnc;
	uint8_t count;
	uint16_t value;

	this->configure_registers(UMODBUS_TYPE_INPUT_REGISTER);
	this->set_register(4, 0x3435);
	write_packet(&is, packet);

	this->envelop.enveloped_read_as_register(UMODBUS_FNCODE_RD_M_INPUT_REG);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_INPUT_REG);
	count = os.read();
	ASSERT_EQ(count, 2);
	os.read(value);
	ASSERT_EQ(0x3435, value);
}

TEST_F(uModbusCoilTest, readMultipleRegister) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });

	read_register_packet_t packet = { 2, 5 };
	uint8_t fnc;
	uint8_t count;
	uint16_t value;

	this->configure_registers(UMODBUS_TYPE_INPUT_REGISTER);
	this->set_register(2, 0x3435);
	this->set_register(3, 0x3436);
	this->set_register(4, 0x3437);
	this->set_register(5, 0x3438);
	this->set_register(6, 0x3439);

	write_packet(&is, packet);

	this->envelop.enveloped_read_as_register(UMODBUS_FNCODE_RD_M_INPUT_REG);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_INPUT_REG);
	count = os.read();
	ASSERT_EQ(count, 10);

	os.read(value);
	ASSERT_EQ(0x3435, value);
	os.read(value);
	ASSERT_EQ(0x3436, value);
	os.read(value);
	ASSERT_EQ(0x3437, value);
	os.read(value);
	ASSERT_EQ(0x3438, value);
	os.read(value);
	ASSERT_EQ(0x3439, value);
}

TEST_F(uModbusCoilTest, writeSingleCoilOff) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });

	write_coil_packet_t packet = { 4, UMODBUS_COIL_OFF };
	uint8_t fnc;
	uint16_t address;
	uint16_t value;

	this->configure_registers(UMODBUS_TYPE_COIL);
	this->set_register(4, UMODBUS_COIL_ON);
	write_packet(&is, packet);

	this->envelop.enveloped_write_single_as_byte(UMODBUS_FNCODE_RD_M_INPUT_REG);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_INPUT_REG);
	os.read(address);
	ASSERT_EQ(4, address);
	os.read(value);
	ASSERT_EQ(UMODBUS_COIL_OFF, value);

	ASSERT_EQ(UMODBUS_COIL_OFF, *(registers[4].ptr));
}

TEST_F(uModbusCoilTest, writeSingleCoilOn) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });
	write_coil_packet_t packet = { 4, UMODBUS_COIL_ON };
	uint8_t fnc;
	uint16_t address;
	uint16_t value;

	this->configure_registers(UMODBUS_TYPE_COIL);
	this->set_register(4, UMODBUS_COIL_OFF);
	write_packet(&is, packet);

	this->envelop.enveloped_write_single_as_byte(UMODBUS_FNCODE_RD_M_INPUT_REG);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_INPUT_REG);
	os.read(address);
	ASSERT_EQ(4, address);
	os.read(value);
	ASSERT_EQ(UMODBUS_COIL_ON, value);

	ASSERT_EQ(UMODBUS_COIL_ON, *(registers[4].ptr));
}


TEST_F(uModbusCoilTest, writeSingleRegister) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });

	write_coil_packet_t packet = { 4, 0x2255 };
	
	uint8_t fnc;
	uint16_t address;
	uint16_t value;

	this->configure_registers(UMODBUS_TYPE_HOLDING_REGISTER);
	this->set_register(4, 0x3215);
	write_packet(&is, packet);

	this->envelop.enveloped_write_single_as_register(UMODBUS_FNCODE_RD_M_INPUT_REG);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_INPUT_REG);
	os.read(address);
	ASSERT_EQ(4, address);
	os.read(value);
	ASSERT_EQ(0x2255, value);

	ASSERT_EQ(0x2255, *(registers[4].ptr));
}

TEST_F(uModbusCoilTest, writeMultipleCoil) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });
	write_multiple_coil_packet_t packet = { 0, 9, 2 };
	uint8_t val[] =  { 0xFF, 0x01 };
	
	uint8_t fnc;
	uint16_t address;
	uint16_t outputCount;

	this->configure_registers(UMODBUS_TYPE_COIL);
	this->reset_registers();

	write_packet(&is, packet);
	is.write(val, 2);

	this->envelop.enveloped_write_multiple_as_byte(UMODBUS_FNCODE_RD_M_INPUT_REG);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_RD_M_INPUT_REG);
	os.read(address);
	ASSERT_EQ(0, address);
	os.read(outputCount);
	ASSERT_EQ(9, outputCount);

	for(size_t i = 0; i < 9; i++) {
		uint8_t b = val[i / 8];

		if((b & (1<< (i % 8))) > 0) {
			ASSERT_EQ(UMODBUS_COIL_ON, *(registers[address + i].ptr));
		} else {
			ASSERT_EQ(UMODBUS_COIL_OFF, *(registers[address + i].ptr));
		}
	}
}


TEST_F(uModbusCoilTest, writeMultipleRegisters) {
	ArrayStream is({ input, 50 });
	ArrayStream os({ output, 50 });
	write_multiple_coil_packet_t packet = { 0, 5, 10 };
	uint16_t val[] =  { 
		0x3541,
		0x3541,
		0x3541,
		0x3541,
		0x3541 };
	
	uint8_t fnc;
	uint16_t address;
	uint16_t outputCount;

	this->configure_registers(UMODBUS_TYPE_HOLDING_REGISTER);
	this->reset_registers();
	this->set_register(0, 0x2221);
	this->set_register(1, 0x2222);
	this->set_register(2, 0x2223);
	this->set_register(3, 0x2224);
	this->set_register(4, 0x2225);

	write_packet(&is, packet);
	for(int i = 0; i < 5; i++) {
		is.write(val[i]);
	}

	this->envelop.enveloped_write_multiple_registers(UMODBUS_FNCODE_WR_M_HOLDING_REGS);	

	fnc = os.read();
	ASSERT_EQ(fnc, UMODBUS_FNCODE_WR_M_HOLDING_REGS);
	os.read(address);
	ASSERT_EQ(0, address);
	os.read(outputCount);
	ASSERT_EQ(5, outputCount);

	for(size_t i = 0; i < 5; i++) {
		uint16_t v = val[i];
		ASSERT_EQ(v, *(registers[address + i].ptr));
	}
}
