Summary of Atmega328p UART LED Control (Theoretical) - No hardware to test on

Hardware: 
  Atmega328p, 2 LEDs, UART

Software:
  Initializes UART with baud rate, receiver, transmitter, and frame format.
  Continuously checks for received bytes via UART.
  Processes received bytes, building a command string until termination character.
  Executes actions based on the command:
  "set-led <ID> <duration>": Controls LEDs with specified ID and duration.
  "echo <data_length> <data>": Echoes received data back.
  "unknown_command": Sends error message.
  Communication Protocol:
  Text commands followed by carriage return.
  "set-led": Sets LED ID (0/1) and duration (ms).
  "echo": Echoes data with specified length.
  Responses: "OK\r\n" (success), "ERROR\r\n" (failure).
Limitations:
  Excludes error handling and specific implementation details.
  This summary provides a high-level overview of the theoretical design for controlling LEDs on an Atmega328p using UART commands
