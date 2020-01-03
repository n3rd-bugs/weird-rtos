# Valid GPIO pin numbers.
set(stm32f030_pinnum_values 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 CACHE INTERNAL "" FORCE)

# Valid GPIO port registers.
set(stm32f030_port_values GPIOA GPIOB GPIOC GPIOD GPIOE GPIOF CACHE INTERNAL "" FORCE)

# Save register map function.
set(stm32f030_regmap ${CMAKE_CURRENT_SOURCE_DIR}/stm32f030_regmap.cmake CACHE INTERNAL "" FORCE)