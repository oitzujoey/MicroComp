/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.	 By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include <stdint.h>
#define NO_BOARD_LIB

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <stddef.h>
#include "core.h"
#include "duckLisp.h"
#include "duckVM.h"

#define BYTECODE_BUFFER_SIZE (8*1024)

LPC_USART_T *usart_host = LPC_UART3;

void usart_init(LPC_USART_T *usart) {
	/* 8-N-1
	   9600 */
	(void) Chip_UART_Init(usart);
	Chip_UART_SetBaud(usart, 9600);
	(void) Chip_UART_ConfigData(LPC_UART3,
	                            UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_EN | UART_LCR_PARITY_EVEN);
	(void) Chip_UART_TXEnable(usart);
}

void delay_init(LPC_USART_T *usart) {
	(void) Chip_TIMER_Init(LPC_TIMER0);
	// This is probably the wrong prescaler value.
	Chip_TIMER_PrescaleSet(LPC_TIMER0, 25000-1);
}


void hang(void) {
	while (true) {
		// "Dummy" NOP to allow source level single
		// stepping of tight while() loop
		__asm volatile ("nop");
	}
}


void delayMilliseconds(uint32_t duration) {
	(void) Chip_TIMER_Reset(LPC_TIMER0);
	(void) Chip_TIMER_Enable(LPC_TIMER0);
	while (Chip_TIMER_ReadCount(LPC_TIMER0) < duration);
	(void) Chip_TIMER_Disable(LPC_TIMER0);
}


void log_info(const char *string, const int length) {
	// Ignore return value because all bytes are always sent. It hangs instead of failing.
	(void) Chip_UART_SendBlocking(usart_host, string, length);
}

void log_warning(const char *string, const int length) {
	(void) log_info(DL_STR("Warning: "));
	(void) log_info(string, length);
}

void log_error(const char *string, const int length) {
	(void) log_info(DL_STR("Error: "));
	(void) log_info(string, length);
}


void send_ptrdiff(const dl_ptrdiff_t ptrdiff) {
	const uint8_t ptrdiff_size_2 = 2*sizeof(dl_ptrdiff_t);
	uint8_t outBuffer[2*sizeof(dl_ptrdiff_t)];
	DL_DOTIMES(i, ptrdiff_size_2) {
		uint8_t nybble = ((ptrdiff >> 4*(ptrdiff_size_2 - i - 1)) & 0xF);
		nybble += (nybble < 10) ? '0' : 'A' - 10;
		outBuffer[i] = nybble;
	}
	// Ignore return value because all bytes are always sent. It hangs instead of failing.
	(void) Chip_UART_SendBlocking(usart_host, outBuffer, ptrdiff_size_2);
}

void send_string(const dl_uint8_t *string, const int length) {
	(void) log_info((char *) string, length);
}

void send_newline(void) {
	uint8_t newline = '\n';
	(void) Chip_UART_SendBlocking(usart_host, &newline, 1);
}


void gpio_init(void) {
	// Ignore return value because all bytes are always sent. It hangs instead of failing.
	(void) Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 2, 9);  // MicroComp clock
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 20);  // Microcode write
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 19);  // Microcode read
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 8);  // Microcode buffer read
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);  // Microcode state read
}



dl_error_t callback_print(duckVM_t *duckVM);


dl_error_t callback_printCons(duckVM_t *duckVM, duckLisp_object_t *cons) {
	dl_error_t e = dl_error_ok;

	if (cons == dl_null) {
		send_string(DL_STR("()"));
	}
	else {
		if (cons->value.cons.car == dl_null) {
			send_string(DL_STR("()"));
		}
		else if (cons->value.cons.car->type == duckLisp_object_type_cons) {
			send_string(DL_STR("("));
			e = callback_printCons(duckVM, cons->value.cons.car);
			if (e) goto cleanup;
			send_string(DL_STR(")"));
		}
		else {
			e = duckVM_push(duckVM, cons->value.cons.car);
			if (e) goto cleanup;
			e = callback_print(duckVM);
			if (e) goto cleanup;
			e = duckVM_pop(duckVM, dl_null);
			if (e) goto cleanup;
		}


		if ((cons->value.cons.cdr == dl_null)
		    || (cons->value.cons.cdr->type == duckLisp_object_type_cons)) {
			if (cons->value.cons.cdr != dl_null) {
				send_string(DL_STR(" "));
				e = callback_printCons(duckVM, cons->value.cons.cdr);
				if (e) goto cleanup;
			}
		}
		else {
			send_string(DL_STR(" . "));
			e = duckVM_push(duckVM, cons->value.cons.cdr);
			if (e) goto cleanup;
			e = callback_print(duckVM);
			if (e) goto cleanup;
			e = duckVM_pop(duckVM, dl_null);
			if (e) goto cleanup;
		}
	}

 cleanup:

	return e;
}

dl_error_t callback_print(duckVM_t *duckVM) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t object;

	// e = duckVM_getArg(duckVM, &string, 0);
	e = duckVM_pop(duckVM, &object);
	if (e) goto cleanup;

	switch (object.type) {
	case duckLisp_object_type_symbol:
		send_string(object.value.symbol.internalString->value.internalString.value,
		            object.value.symbol.internalString->value.internalString.value_length);
		send_string(DL_STR("→"));
		send_ptrdiff(object.value.symbol.id);
		break;
	case duckLisp_object_type_string:
		send_string(&object.value.string.internalString->value.internalString.value[object.value.string.offset],
		            object.value.string.length);
		break;
	case duckLisp_object_type_integer:
		send_ptrdiff(object.value.integer);
		break;
	case duckLisp_object_type_bool:
		if (object.value.boolean) {
			send_string(DL_STR("true"));
		}
		else {
			send_string(DL_STR("false"));
		}
		break;
	case duckLisp_object_type_list:
		if (object.value.list == dl_null) {
			send_string(DL_STR("()"));
		}
		else {
			/* printf("(%i: ", object.value.list->type); */
			send_string(DL_STR("("));

			if (object.value.list->value.cons.car == dl_null) {
				send_string(DL_STR("(())"));
			}
			else if (object.value.list->value.cons.car->type == duckLisp_object_type_cons) {
				send_string(DL_STR("("));
				e = callback_printCons(duckVM, object.value.list->value.cons.car);
				if (e) goto cleanup;
				send_string(DL_STR(")"));
			}
			else {
				e = duckVM_push(duckVM, object.value.list->value.cons.car);
				if (e) goto cleanup;
				e = callback_print(duckVM);
				if (e) goto cleanup;
				e = duckVM_pop(duckVM, dl_null);
				if (e) goto cleanup;
			}

			if (object.value.list->value.cons.cdr == dl_null) {
			}
			else if (object.value.list->value.cons.cdr->type == duckLisp_object_type_cons) {
				if (object.value.list->value.cons.cdr != dl_null) {
					send_string(DL_STR(" "));
					e = callback_printCons(duckVM, object.value.list->value.cons.cdr);
					if (e) goto cleanup;
				}
			}
			else {
				send_string(DL_STR(" . "));
				e = duckVM_push(duckVM, object.value.list->value.cons.cdr);
				if (e) goto cleanup;
				e = callback_print(duckVM);
				if (e) goto cleanup;
				e = duckVM_pop(duckVM, dl_null);
				if (e) goto cleanup;
			}

			send_string(DL_STR(")"));
		}
		break;
	case duckLisp_object_type_closure:
		send_string(DL_STR("(closure "));
		send_ptrdiff(object.value.closure.name);
		DL_DOTIMES(k, object.value.closure.upvalue_array->value.upvalue_array.length) {
			duckLisp_object_t *uv = object.value.closure.upvalue_array->value.upvalue_array.upvalues[k];
			send_string(DL_STR(" "));
			if (uv == dl_null) {
				send_string(DL_STR("$"));
				continue;
			}
			if (uv->value.upvalue.type == duckVM_upvalue_type_stack_index) {
				duckLisp_object_t object = DL_ARRAY_GETADDRESS(duckVM->stack,
				                                               duckLisp_object_t,
				                                               uv->value.upvalue.value.stack_index);
				e = duckVM_push(duckVM, &object);
				if (e) goto cleanup;
				e = callback_print(duckVM);
				if (e) goto cleanup;
				e = duckVM_pop(duckVM, dl_null);
				if (e) goto cleanup;
			}
			else if (uv->value.upvalue.type == duckVM_upvalue_type_heap_object) {
				e = duckVM_push(duckVM, uv->value.upvalue.value.heap_object);
				if (e) goto cleanup;
				e = callback_print(duckVM);
				if (e) goto cleanup;
				e = duckVM_pop(duckVM, dl_null);
				if (e) goto cleanup;
			}
			else {
				while (uv->value.upvalue.type == duckVM_upvalue_type_heap_upvalue) {
					uv = uv->value.upvalue.value.heap_upvalue;
				}
				if (uv->value.upvalue.type == duckVM_upvalue_type_stack_index) {
					e = duckVM_push(duckVM,
					                &DL_ARRAY_GETADDRESS(duckVM->stack,
					                                     duckLisp_object_t,
					                                     uv->value.upvalue.value.stack_index));
					if (e) goto cleanup;
					e = callback_print(duckVM);
					if (e) goto cleanup;
					e = duckVM_pop(duckVM, dl_null);
					if (e) goto cleanup;
				}
				else if (uv->value.upvalue.type == duckVM_upvalue_type_heap_object) {
					e = duckVM_push(duckVM, uv->value.upvalue.value.heap_object);
					if (e) goto cleanup;
					e = callback_print(duckVM);
					if (e) goto cleanup;
					e = duckVM_pop(duckVM, dl_null);
					if (e) goto cleanup;
				}
			}
		}
		send_string(DL_STR(")"));
		break;
	case duckLisp_object_type_vector:
		send_string(DL_STR("["));
		if (object.value.vector.internal_vector != dl_null) {
			for (dl_ptrdiff_t k = object.value.vector.offset;
			     (dl_size_t) k < object.value.vector.internal_vector->value.internal_vector.length;
			     k++) {
				duckLisp_object_t *value = object.value.vector.internal_vector->value.internal_vector.values[k];
				if (k != object.value.vector.offset) send_string(DL_STR(" "));
				e = duckVM_push(duckVM, value);
				if (e) goto cleanup;
				e = callback_print(duckVM);
				if (e) goto cleanup;
				e = duckVM_pop(duckVM, dl_null);
				if (e) goto cleanup;
			}
		}
		send_string(DL_STR("]"));
		break;
	case duckLisp_object_type_type:
		send_string(DL_STR("<"));
		send_ptrdiff(object.value.type);
		send_string(DL_STR(">"));
		break;
	case duckLisp_object_type_composite:
		send_string(DL_STR("(make-instance <"));
		send_ptrdiff(object.value.composite->value.internalComposite.type);
		send_string(DL_STR("> "));
		e = duckVM_push(duckVM, object.value.composite->value.internalComposite.value);
		if (e) goto cleanup;
		e = callback_print(duckVM);
		if (e) goto cleanup;
		e = duckVM_pop(duckVM, dl_null);
		if (e) goto cleanup;
		send_string(DL_STR(" "));
		e = duckVM_push(duckVM, object.value.composite->value.internalComposite.function);
		if (e) goto cleanup;
		e = callback_print(duckVM);
		if (e) goto cleanup;
		e = duckVM_pop(duckVM, dl_null);
		if (e) goto cleanup;
		send_string(DL_STR(")"));
		break;
	default:
		send_string(DL_STR("print: Unsupported type. ["));
		send_ptrdiff(object.type);
		send_string(DL_STR("]\n"));
	}

	e = duckVM_push(duckVM, &object);
	if (e) {
		goto cleanup;
	}

 cleanup:

	return e;
}

/* dl_error_t callback_print(duckVM_t *vm) { */
/* 	dl_error_t e = dl_error_ok; */

/* 	duckLisp_object_t object; */
/* 	e = duckVM_pop(vm, &object); */
/* 	if (e) goto cleanup; */

/* 	switch (object.type) { */
/* 	case duckLisp_object_type_string: */
/* 		(void) send_string(object.value.string.internalString->value.internalString.value, */
/* 		                   object.value.string.internalString->value.internalString.value_length); */
/* 		break; */
/* 	case duckLisp_object_type_integer: */
/* 		(void) send_ptrdiff(object.value.integer); */
/* 		break; */
/* 	default: */
/* 		(void) log_warning(DL_STR("print: Type of value not recognized")); */
/* 	} */

/* 	e = duckVM_push(vm, &object); */
/* 	if (e) goto cleanup; */

/*  cleanup: */
/* 	return e; */
/* } */

dl_error_t callback_readRegister(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t addressObject;
	e = duckVM_pop(vm, &addressObject);
	if (e) goto cleanup;

	uint32_t *address = (uint32_t *) addressObject.value.integer;

	duckLisp_object_t dataObject;
	dataObject.type = duckLisp_object_type_integer;
	dataObject.value.integer = *address;
	e = duckVM_push(vm, &dataObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeRegister(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t dataObject;
	e = duckVM_pop(vm, &dataObject);
	if (e) goto cleanup;

	duckLisp_object_t addressObject;
	e = duckVM_pop(vm, &addressObject);
	if (e) goto cleanup;

	uint32_t *address = (uint32_t *) addressObject.value.integer;
	*address = dataObject.value.integer;

	e = duckVM_push(vm, &dataObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}


struct {uint32_t port; uint8_t pin} microcodeAddressMapTable[] = {
	{0, 7},
	{0, 6},
	{0, 18},
	{0, 17},
	{0, 15},
	{0, 16},
	{4, 28},
	{0, 24},
	{0, 25},
	{3, 26},
};

struct {uint32_t port; uint8_t pin} microcodeDataMapTable[] = {
	{2, 3},
	{2, 2},
	{2, 1},
	{2, 0},
	{0, 11},
	{0, 10},
	{2, 13},
	{4, 29},

	{2, 12},
	{2, 11},
	{2, 10},
	{2, 8},
	{2, 7},
	{2, 6},
	{2, 5},
	{2, 4},

	{0, 28},
	{0, 27},
	{0, 22},
	{0, 21},
	{0, 3},
	{0, 2},
	{1, 31},
	{1, 30},
};

uint32_t microcodeAddressPortMap(unsigned int addressBit) {
	return microcodeAddressMapTable[addressBit].port;
}

uint8_t microcodeAddressPinMap(unsigned int addressBit) {
	return microcodeAddressMapTable[addressBit].pin;
}

uint32_t microcodeDataPortMap(unsigned int dataBit) {
	return microcodeDataMapTable[dataBit].port;
}

uint32_t microcodeDataPinMap(unsigned int dataBit) {
	return microcodeDataMapTable[dataBit].pin;
}

const unsigned int microcodeAddressBitWidth = 10;
const unsigned int microcodeDataBitWidth = 24;

dl_error_t callback_setMicrocodeAddressDirection(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t directionObject;
	e = duckVM_pop(vm, &directionObject);
	if (e) goto cleanup;

	DL_DOTIMES(bit, microcodeAddressBitWidth) {
		(void) (directionObject.value.boolean
		        ? Chip_GPIO_SetPinDIROutput
		        : Chip_GPIO_SetPinDIRInput)(LPC_GPIO,
		                                    microcodeAddressPortMap(bit),
		                                    microcodeAddressPinMap(bit));
	}

	e = duckVM_push(vm, &directionObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeMicrocodeAddress(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t addressObject;
	e = duckVM_pop(vm, &addressObject);
	if (e) goto cleanup;

	// Not using a port mask because it's not that useful with the current configuration.

	uint32_t address = addressObject.value.integer;

	DL_DOTIMES(bit, microcodeAddressBitWidth) {
		(void) Chip_GPIO_SetPinState(LPC_GPIO,
		                             microcodeAddressPortMap(bit),
		                             microcodeAddressPinMap(bit),
		                             (address >> bit) & 0x1);
	}

	e = duckVM_push(vm, &addressObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_readMicrocodeAddress(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t addressObject;

	// Not using a port mask because it's not that useful with the current configuration.

	uint32_t address = 0;
	DL_DOTIMES(bit, microcodeAddressBitWidth) {
		address |= (Chip_GPIO_GetPinState(LPC_GPIO,
		                                  microcodeAddressPortMap(bit),
		                                  microcodeAddressPinMap(bit))
		            << bit);
	}

	addressObject.type = duckLisp_object_type_integer;
	addressObject.value.integer = address;

	e = duckVM_push(vm, &addressObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_setMicrocodeDataDirection(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t directionObject;
	e = duckVM_pop(vm, &directionObject);
	if (e) goto cleanup;

	DL_DOTIMES(bit, microcodeDataBitWidth) {
		(void) (directionObject.value.boolean
		        ? Chip_GPIO_SetPinDIROutput
		        : Chip_GPIO_SetPinDIRInput)(LPC_GPIO,
		                                    microcodeDataPortMap(bit),
		                                    microcodeDataPinMap(bit));
	}

	e = duckVM_push(vm, &directionObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeMicrocodeData(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t dataObject;
	e = duckVM_pop(vm, &dataObject);
	if (e) goto cleanup;

	// Not using a port mask because it's not that useful with the current configuration.

	uint32_t data = dataObject.value.integer;

	DL_DOTIMES(bit, microcodeDataBitWidth) {
		(void) Chip_GPIO_SetPinState(LPC_GPIO,
		                             microcodeDataPortMap(bit),
		                             microcodeDataPinMap(bit),
		                             (data >> bit) & 0x1);
	}

	e = duckVM_push(vm, &dataObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_readMicrocodeData(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t dataObject;

	// Not using a port mask because it's not that useful with the current configuration.

	uint32_t data = 0;
	DL_DOTIMES(bit, microcodeDataBitWidth) {
		data |= (Chip_GPIO_GetPinState(LPC_GPIO,
		                               microcodeDataPortMap(bit),
		                               microcodeDataPinMap(bit))
		         << bit);
	}

	dataObject.type = duckLisp_object_type_integer;
	dataObject.value.integer = data;

	e = duckVM_push(vm, &dataObject);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeMicrocodeBufferOe(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t object;
	e = duckVM_pop(vm, &object);
	if (e) goto cleanup;

	bool data;
	if (object.type == duckLisp_object_type_bool) {
		data = object.value.boolean;
	}
	else if (object.type == duckLisp_object_type_integer) {
		data = object.value.integer;
	}
	else {
		e = dl_error_invalidValue;
		goto cleanup;
	}

	(void) Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, data);

	e = duckVM_push(vm, &object);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeMicrocodeStateOe(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t object;
	e = duckVM_pop(vm, &object);
	if (e) goto cleanup;

	bool data;
	if (object.type == duckLisp_object_type_bool) {
		data = object.value.boolean;
	}
	else if (object.type == duckLisp_object_type_integer) {
		data = object.value.integer;
	}
	else {
		e = dl_error_invalidValue;
		goto cleanup;
	}

	(void) Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, data);

	e = duckVM_push(vm, &object);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeMicrocodeWe(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t object;
	e = duckVM_pop(vm, &object);
	if (e) goto cleanup;

	bool data;
	if (object.type == duckLisp_object_type_bool) {
		data = object.value.boolean;
	}
	else if (object.type == duckLisp_object_type_integer) {
		data = object.value.integer;
	}
	else {
		e = dl_error_invalidValue;
		goto cleanup;
	}

	(void) Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, data);

	e = duckVM_push(vm, &object);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeMicrocodeOe(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t object;
	e = duckVM_pop(vm, &object);
	if (e) goto cleanup;

	bool data;
	if (object.type == duckLisp_object_type_bool) {
		data = object.value.boolean;
	}
	else if (object.type == duckLisp_object_type_integer) {
		data = object.value.integer;
	}
	else {
		e = dl_error_invalidValue;
		goto cleanup;
	}

	(void) Chip_GPIO_SetPinState(LPC_GPIO, 0, 19, data);

	e = duckVM_push(vm, &object);
	if (e) goto cleanup;

 cleanup:
	return e;
}

dl_error_t callback_writeClock(duckVM_t *vm) {
	dl_error_t e = dl_error_ok;

	duckLisp_object_t object;
	e = duckVM_pop(vm, &object);
	if (e) goto cleanup;

	bool data;
	if (object.type == duckLisp_object_type_bool) {
		data = object.value.boolean;
	}
	else if (object.type == duckLisp_object_type_integer) {
		data = object.value.integer;
	}
	else {
		e = dl_error_invalidValue;
		goto cleanup;
	}

	(void) Chip_GPIO_SetPinState(LPC_GPIO, 2, 9, data);

	e = duckVM_push(vm, &object);
	if (e) goto cleanup;

 cleanup:
	return e;
}


int main(void) {
#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#endif

	Board_Debug_Init();

	(void) gpio_init();
	(void) usart_init(usart_host);

	dl_error_t e = dl_error_ok;

	// All user-defined callbacks go here.
	struct {
		const dl_ptrdiff_t key;
		dl_error_t (*callback)(duckVM_t *);
	} callbacks[] = {
		{1,  callback_print},
		{10, callback_readRegister},
		{11, callback_writeRegister},
		{12, callback_setMicrocodeAddressDirection},
		{13, callback_readMicrocodeAddress},
		{14, callback_writeMicrocodeAddress},
		{15, callback_setMicrocodeDataDirection},
		{16, callback_readMicrocodeData},
		{17, callback_writeMicrocodeData},
		{18, callback_writeMicrocodeBufferOe},
		{19, callback_writeMicrocodeStateOe},
		{20, callback_writeMicrocodeOe},
		{21, callback_writeMicrocodeWe},
		{22, callback_writeClock},
		{0, dl_null}
	};

	duckVM_t vm;
	e = duckVM_init(&vm, 100);
	if (e) {
		(void) log_error(DL_STR("Failed to initialize VM\n"));
		(void) hang();
	}

	for (dl_ptrdiff_t i = 0; callbacks[i].callback != dl_null; i++) {
		e = duckVM_linkCFunction(&vm, callbacks[i].key, callbacks[i].callback);
		if (e) {
			(void) log_error(DL_STR("Could not link callback into VM.\n"));
			(void) hang();
		}
	}


	dl_uint8_t bytecode[BYTECODE_BUFFER_SIZE];
	while (true) {
		dl_size_t bytecode_length = 0;
		{
			uint8_t lengthBuffer[4];
			Chip_UART_ReadBlocking(usart_host, lengthBuffer, 4);
			DL_DOTIMES(j, 4) {
				bytecode_length += lengthBuffer[j]<<8*j;
			}
		}
		if (bytecode_length < BYTECODE_BUFFER_SIZE) {
			Chip_UART_ReadBlocking(usart_host, bytecode, bytecode_length);
		}
		else {
			(void) log_warning(DL_STR("Bytecode too long\n"));
			continue;
		}

		e = duckVM_execute(&vm, dl_null, bytecode, sizeof(bytecode)/sizeof(*bytecode));
		if (e) {
			(void) log_warning(DL_STR("Bytecode execution failed\n"));
		}

		e = duckVM_softReset(&vm);
		if (e) {
			(void) log_warning(DL_STR("VM soft reset failed\n"));
		}
		e = duckVM_garbageCollect(&vm);
		if (e) {
			(void) log_warning(DL_STR("Garbage collection failed\n"));
		}
	}

	(void) hang();
	// Doesn't return.
	return 0 ;
}
