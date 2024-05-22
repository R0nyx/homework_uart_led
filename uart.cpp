// Author: R0nyx
// Date: 22.05.2024
// MCU: Atmega328p (target)

// This code is designed for Atmega328p microcontroller.
// Diemzel uz rokam neatradas neviens MCU ko varetu izmantot prieks testa


#include <avr/io.h>
#include "uart.h"

int main() {

  init_led();

  char buffer[UART_BUFFER_SIZE];
  while(1){
      uart_read_string(buffer);
      process_command(buffer);
  }
  return 0;
}


void delay_ms(unsigned int ms) {
  unsigned int cycles_us = F_CPU / 1000;
  unsigned int cycles_needed = cycles_us * ms;
  for (unsigned int i = 0; i < cycles_needed; i++) {
    // delay
  }
}

void init_led(){
  // ini only if def.
  if(LED0 == LED1){
    // abi uz viena pin
    // handle error case
    DDRB |= (1 << LED0);
    return 1;
  }
  //set ddr pins to output
  #ifdef LED0
    DDRB |= (1 << LED0);
  #endif

  #ifdef LED1
    DDRB |= (1 << LED1);  
  #endif
}

void execute_led_command(int led_id, unsigned long time){
  switch(led_id){ //switch for performance
    case 1:
      PORTB |= (1 << LED0);
      delay_ms(time);
      PORTB &= ~(1 << LED0);
      uart_send_string(*messages[2]);
      break;
    case 2:
      #ifdef LED1
        PORTB |= (1 << LED1);
        delay_ms(time);
        PORTB &= ~(1 << LED1);
        uart_send_string(*messages[2]);
      #endif
      break;
    default:
      uart_send_string(*messages[1]);
      break;
  }
}

void uart_init(uint32_t baud) {

  //calc baud rate value
  uint32_t UBRR = (F_CPU / (16 * baud)) - 1;

  //set baudrate reg.
  UBRR0H = UBRR >> 8;
  UBRR0L = UBRR & 0xff;

  //activate receive, transmit
  UCSR0B = (1<<RXEN0)|(1<<TXEN0); //enable rec, transm
  UCSR0C = (0<<UPM01) | (1<<UPM00); //no parity
  UCSR0C = (0<<USBS0); //1 STOP bit
  UCSR0C = (0<<UCSZ02) | (1<<UCSZ01) | (1<<UCSZ00); //8 bit comm.
}

int uart_ready() {
  //cehcks if uart is ready to communicate
  return (UCSR0A & (1 << UDRE0)) != 0;
}

void uart_send_byte(uint8_t data) { //sends byte via uart
  while (uart_ready()!=1);
    // Send the byte by writing to the UART data register
  UDR0 = data;
  return 1;
}

void uart_send_array(uint8_t *data, int len){ //sends byte by byte based on lenght of the data
  for(int i = 0; i < len; i++){
      while(uart_ready()!=1)
      uart_send_byte(data[i]);
  }
}

void uart_send_string(uint8_t *text){ 
    int i = 0;
    while (text[i] != '\r' || text[i] != '\0' ) {
      uart_send_byte(text[i]);
      i++;
      text[i] = message[i]; 
    }
    uart_send_byte(text[i]); 
}

char uart_read_byte() { // reads uart byte
  while ( !(UCSR0A & (1 << RXC0)) );
  return UDR0;
}

int uart_read_string(char *buffer) { //
    uint8_t read_bytes = 0;
    char byte;

    while (read_bytes != '\r' || read_bytes != '\n' ) {
        byte = uart_read_byte(); 

        if (byte == '\r') {
            break;
        }

        buffer[read_bytes] = byte;  
        read_bytes++;
    }

    buffer[read_bytes] = '\0'; 
    return read_bytes;
}

int process_command(char *buffer) {
  uint8_t mismatch; //flag for invalid command

  // itirate trough every command in LUT messages[];
  for (int command_index = 0; command_index < NUM_COMMANDS; command_index++) {
    char *command = commands[command_index];
    int j = 0;
    mismatch = 0;

    // Compare characters until mismatch or terminated
    for (int k = 0; buffer[k] != '\n' && buffer[k] != '\r'; k++) {
      if (buffer[j] != command[k]) {
        mismatch = 1; // Mismatch found
        break;
      }
      j++;
    }

    if (mismatch == 0) {
      // Check for complete command match (excluding newline)
      if (command[j] == ' ' && buffer[j] == '\r') {
        // Command found
        switch (command_index) {
          case 0: // "set-led" command
            if (buffer[j + 1] < 0 || buffer[j + 1] > 1) {
              uart_send_string(*messages[1]); // ERROR - LED-id incorrect, use only 0, 1 for now
              return -1;
            }

            // Extract led_id:
            uint8_t led_id = buffer[j + 1]; // extract led_id

            // Extract time:
            uint32_t time = 0;
            uint8_t digit = 0;
            uint8_t time_index = buffer[j + 3];

            while (buffer[time_index] != '\r') {
              if (buffer[time_index] >= '0' && buffer[time_index] <= '9') {
                time = time * 10 + (buffer[time_index] - '0'); // convert value to int from ascii and accumulate
              } else {
                uart_send_string(*messages[1]); // wrong value
                return -1;
              }
              time_index++;
            }
            //execute led command based on extracted commands. aftewrards send "OK"
            execute_led_command(led_id, time);
            uart_send_string(messages[0]);
            break;

          case 1: // "echo" command
            uint8_t data_len = 0;

            // Check if data exceeds range
            if (buffer[j + 1] < 0 || buffer[j + 1] > UART_BUFFER_SIZE) {
                uart_send_string(*messages[1]); // ERROR - LED-id incorrect, use only 0, 1 for now
                return -1;
            }

            uint8_t data_len_index = buffer[j + 1]; // start after space

            //extract data size
            while (buffer[data_len_index] != ',') {
                if (buffer[data_len_index] >= '0' && buffer[data_len_index] <= '9') {
                  data_len = data_len * 10 + (buffer[data_len_index] - '0'); // convert value to int from ascii and accumulate
                } else {
                  uart_send_string(*messages[1]); // wrong value
                  return -1;
                }
                data_len_index++;
            }

            //extract data:
            char data_buffer [data_len];
            uint8_t data_start_index =  buffer[data_len_index + 2]; 
            uint8_t data_index = 0;

            while (buffer[data_start_index] != '\r') { // get data
                data_buffer[data_index] = buffer[data_start_index];
                data_start_index++;
                data_index++;
            }

            //construct message:
            //adding prefix
            char echo_message [data_len+8];
            for (int i = 0; i < 7; i++) { //len of echo message prefix "data :"
              echo_message[i] = messages[3][i];
            }
            //add data:
            for (int i = 0; i < data_len; i++) {
              echo_message[i + 7] = data_buffer[i]; 
            }
            //send constructed data
            uart_send_array(echo_message, (data_len+8));
            uart_send_string(messages[0]);

        }
      }
    }
    else {
      uart_send_array(*messages[1]); // Send "ERROR\r\n" for unrecognized command
      return -1; // Example error code (replace with your specific value)
    }
  }
}
