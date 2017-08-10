# Valid pin numbers.
set(atmega_pinnum_values 0 1 2 3 4 5 6 7 CACHE STRING "" FORCE)

# Valid PIN registers.
set(atmega_pin_values PINA PINB PINC PIND CACHE STRING "" FORCE)

# Valid PORT registers.
set(atmega_port_values PORTA PORTB PORTC PORTD CACHE STRING "" FORCE)

# Valid DDR registers.
set(atmega_ddr_values DDRA DDRB DDRC DDRD CACHE STRING "" FORCE)

# Save register map function.
set(atmega_regmap ${CMAKE_CURRENT_SOURCE_DIR}/atmega644_regmap.cmake CACHE STRING "" FORCE)