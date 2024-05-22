#pragma once
#include <avr/io.h>

#define F_CPU 16000000UL //CPUI CLOCK 16MHZ

/* UART */
#ifndef BAUD_RATE
#define BAUD_RATE 9600 // Default value
#endif
#define UART_MEM 64  // Buffer max size

/* PINS */
#define PIN8  PB0 // (pin 8) //ddrb
#define PIN9  PB1 // (pin 9)
#define PIN10 PB2 // (pin 10) //ddrd
#define PIN11 PB3  // (pin 11)

/* led */
#define LED0 PIN8
#define LED1 PIN10

/* Commands */

const char *commands[] = { // uart input commands
    "set-led\r",
    "echo\r"
};
const char *messages[] = { //uart responses
    "OK\r\n",
    "ERROR\r\n",
    "led-off\r",
    "data: "
};

#define NUM_COMMANDS 2


// Function prototypes for LED control
void delay_ms(unsigned int ms);
int init_led();  // Returns 1 if error (LED1 and LED2 are the same pin)
void execute_led_command(int led_id, unsigned long time);

// Function prototypes for UART communication
void uart_init(uint32_t baud);
bool uart_ready();
void uart_send_byte(uint8_t data);
uint8_t uart_send_array(uint8_t *data, int len);
uint8_t uart_send_string(uint8_t *text);
int uart_read_string(char *buffer);
int process_command(char *buffer);
