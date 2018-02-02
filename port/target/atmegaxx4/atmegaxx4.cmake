# Valid pin numbers.
set(atmega_pinnum_values 0 1 2 3 4 5 6 7 CACHE INTERNAL "" FORCE)

# Valid PIN registers.
set(atmega_pin_values PINA PINB PINC PIND CACHE INTERNAL "" FORCE)

# Valid PORT registers.
set(atmega_port_values PORTA PORTB PORTC PORTD CACHE INTERNAL "" FORCE)

# Valid DDR registers.
set(atmega_ddr_values DDRA DDRB DDRC DDRD CACHE INTERNAL "" FORCE)

# Valid external interrupt sources.
set(atmega_int_values INT0 INT1 INT2 CACHE INTERNAL "" FORCE)

# Save register map function.
set(atmega_regmap ${CMAKE_CURRENT_SOURCE_DIR}/atmegaxx4_regmap.cmake CACHE INTERNAL "" FORCE)