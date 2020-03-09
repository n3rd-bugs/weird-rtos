EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "4-bit Dimmer"
Date "2020-01-26"
Rev "0.1"
Comp "n3rd-bugs"
Comment1 "Usama Masood"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Logic_Programmable:GAL16V8 U1
U 1 1 5E2E726D
P 6850 2250
F 0 "U1" H 6850 3131 50  0000 C CNN
F 1 "GAL16V8" H 6850 3040 50  0000 C CNN
F 2 "Package_DIP:DIP-20_W7.62mm_Socket_LongPads" H 6850 2250 50  0001 C CNN
F 3 "" H 6850 2250 50  0001 C CNN
	1    6850 2250
	1    0    0    -1  
$EndComp
$Comp
L Relay_SolidState:MOC3052M U4
U 1 1 5E2E872B
P 2750 5750
F 0 "U4" H 2750 6075 50  0000 C CNN
F 1 "MOC3052M" H 2750 5984 50  0000 C CNN
F 2 "Package_DIP:DIP-6_W7.62mm_Socket_LongPads" H 2550 5550 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/MO/MOC3052M.pdf" H 2750 5750 50  0001 L CNN
	1    2750 5750
	1    0    0    -1  
$EndComp
$Comp
L Timer:LM555 U3
U 1 1 5E2E9CF7
P 3300 2000
F 0 "U3" H 3300 2581 50  0000 C CNN
F 1 "LM555" H 3300 2490 50  0000 C CNN
F 2 "Package_DIP:DIP-8_W7.62mm_Socket_LongPads" H 3300 2000 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm555.pdf" H 3300 2000 50  0001 C CNN
	1    3300 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 5E2EAA27
P 2250 2250
F 0 "C1" H 2368 2296 50  0000 L CNN
F 1 ".01u" H 2368 2205 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 2288 2100 50  0001 C CNN
F 3 "~" H 2250 2250 50  0001 C CNN
	1    2250 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 2550 6350 2550
Text GLabel 6100 1850 0    50   Input ~ 0
IN-0
Text GLabel 6100 1950 0    50   Input ~ 0
IN-1
Text GLabel 6100 2050 0    50   Input ~ 0
IN-2
Text GLabel 6100 2150 0    50   Input ~ 0
IN-3
Wire Wire Line
	6100 2150 6350 2150
Wire Wire Line
	6100 2050 6350 2050
Wire Wire Line
	6100 1950 6350 1950
Wire Wire Line
	6100 1850 6350 1850
Text GLabel 7550 2450 2    50   Input ~ 0
TRIG
Wire Wire Line
	7550 2450 7350 2450
$Comp
L power:+5V #PWR011
U 1 1 5E2F565A
P 6850 1250
F 0 "#PWR011" H 6850 1100 50  0001 C CNN
F 1 "+5V" H 6865 1423 50  0000 C CNN
F 2 "" H 6850 1250 50  0001 C CNN
F 3 "" H 6850 1250 50  0001 C CNN
	1    6850 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6850 1250 6850 1550
$Comp
L power:GNDD #PWR012
U 1 1 5E2F61E4
P 6850 3100
F 0 "#PWR012" H 6850 2850 50  0001 C CNN
F 1 "GNDD" H 6854 2945 50  0000 C CNN
F 2 "" H 6850 3100 50  0001 C CNN
F 3 "" H 6850 3100 50  0001 C CNN
	1    6850 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6850 3100 6850 2950
Wire Notes Line
	5400 1000 5400 3700
Wire Notes Line
	5400 3700 8300 3700
Wire Notes Line
	8300 3700 8300 1000
Text Notes 7500 3600 2    50   ~ 0
16v8 - Implements delayed trigger
$Comp
L Device:R R4
U 1 1 5E300454
P 3250 5650
F 0 "R4" V 3043 5650 50  0000 C CNN
F 1 "100R" V 3134 5650 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 3180 5650 50  0001 C CNN
F 3 "~" H 3250 5650 50  0001 C CNN
	1    3250 5650
	0    1    1    0   
$EndComp
Wire Wire Line
	3200 5850 3050 5850
Wire Wire Line
	3100 5650 3050 5650
Wire Wire Line
	3500 5700 3500 5650
Wire Wire Line
	3500 5650 3400 5650
$Comp
L Device:R R3
U 1 1 5E3029CC
P 3200 6150
F 0 "R3" H 3270 6196 50  0000 L CNN
F 1 "100R" H 3270 6105 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 3130 6150 50  0001 C CNN
F 3 "~" H 3200 6150 50  0001 C CNN
	1    3200 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 5850 3200 5950
Wire Wire Line
	3350 5950 3200 5950
Wire Wire Line
	3500 6000 3500 6350
$Comp
L Connector:Screw_Terminal_01x04 J2
U 1 1 5E2ECAFA
P 4300 5500
F 0 "J2" H 4380 5492 50  0000 L CNN
F 1 "AC/Load" H 4380 5401 50  0000 L CNN
F 2 "TerminalBlock_4Ucon:TerminalBlock_4Ucon_1x04_P3.50mm_Vertical" H 4300 5500 50  0001 C CNN
F 3 "~" H 4300 5500 50  0001 C CNN
	1    4300 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 5500 4100 5600
Wire Wire Line
	4100 5400 3950 5400
Wire Wire Line
	3500 5400 3500 5650
Connection ~ 3500 5650
Wire Wire Line
	3200 6000 3200 5950
Connection ~ 3200 5950
Wire Wire Line
	3200 6300 3200 6350
Wire Wire Line
	3200 6350 3500 6350
Connection ~ 3500 6350
Wire Wire Line
	3500 6350 3950 6350
Wire Wire Line
	3950 6350 3950 5700
Wire Wire Line
	3950 5700 4100 5700
$Comp
L Device:R R5
U 1 1 5E3078C4
P 3300 5000
F 0 "R5" V 3507 5000 50  0000 C CNN
F 1 "100k" V 3416 5000 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 3230 5000 50  0001 C CNN
F 3 "~" H 3300 5000 50  0001 C CNN
	1    3300 5000
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R6
U 1 1 5E308575
P 3300 5200
F 0 "R6" V 3093 5200 50  0000 C CNN
F 1 "100k" V 3184 5200 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 3230 5200 50  0001 C CNN
F 3 "~" H 3300 5200 50  0001 C CNN
	1    3300 5200
	0    1    1    0   
$EndComp
Wire Wire Line
	3150 5200 3050 5200
Wire Wire Line
	3050 5000 3150 5000
Wire Wire Line
	3450 5000 3950 5000
Wire Wire Line
	3950 5000 3950 5400
Connection ~ 3950 5400
Wire Wire Line
	3950 5400 3500 5400
Wire Wire Line
	3800 5200 3450 5200
Wire Wire Line
	4100 5600 3800 5600
Connection ~ 4100 5600
Wire Wire Line
	3800 5200 3800 5600
$Comp
L Device:R R2
U 1 1 5E30BA09
P 2350 6100
F 0 "R2" H 2420 6146 50  0000 L CNN
F 1 "10k" H 2420 6055 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 2280 6100 50  0001 C CNN
F 3 "~" H 2350 6100 50  0001 C CNN
	1    2350 6100
	1    0    0    -1  
$EndComp
$Comp
L Isolator:4N35 U2
U 1 1 5E2E7D49
P 2750 5100
F 0 "U2" H 2750 5425 50  0000 C CNN
F 1 "4N35" H 2750 5334 50  0000 C CNN
F 2 "Package_DIP:DIP-6_W7.62mm_Socket_LongPads" H 2550 4900 50  0001 L CIN
F 3 "https://www.vishay.com/docs/81181/4n35.pdf" H 2750 5100 50  0001 L CNN
	1    2750 5100
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2450 5200 2350 5200
Wire Wire Line
	2350 5200 2350 5950
$Comp
L power:+5V #PWR09
U 1 1 5E30F5C2
P 2350 4750
F 0 "#PWR09" H 2350 4600 50  0001 C CNN
F 1 "+5V" H 2365 4923 50  0000 C CNN
F 2 "" H 2350 4750 50  0001 C CNN
F 3 "" H 2350 4750 50  0001 C CNN
	1    2350 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2350 4750 2350 5100
Wire Wire Line
	2350 5100 2450 5100
$Comp
L power:GNDD #PWR010
U 1 1 5E3108A1
P 2350 6350
F 0 "#PWR010" H 2350 6100 50  0001 C CNN
F 1 "GNDD" H 2354 6195 50  0000 C CNN
F 2 "" H 2350 6350 50  0001 C CNN
F 3 "" H 2350 6350 50  0001 C CNN
	1    2350 6350
	1    0    0    -1  
$EndComp
Text GLabel 6100 2550 0    50   Input ~ 0
Line
Wire Wire Line
	2100 5200 2350 5200
Text GLabel 2100 5200 0    50   Input ~ 0
Line
Connection ~ 2350 5200
Text GLabel 2250 5850 0    50   Input ~ 0
TRIG
$Comp
L Device:R R1
U 1 1 5E314E2A
P 2100 5650
F 0 "R1" V 1893 5650 50  0000 C CNN
F 1 "1k" V 1984 5650 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 2030 5650 50  0001 C CNN
F 3 "~" H 2100 5650 50  0001 C CNN
	1    2100 5650
	0    1    1    0   
$EndComp
Wire Notes Line
	1450 4450 4800 4450
Wire Notes Line
	4800 4450 4800 6850
Wire Notes Line
	4800 6850 1450 6850
Wire Notes Line
	1450 6850 1450 4450
Text Notes 3500 6700 2    50   ~ 0
Control and ZC detection
$Comp
L Device:CP C2
U 1 1 5E31B886
P 1850 2250
F 0 "C2" H 1968 2296 50  0000 L CNN
F 1 "10u" H 1968 2205 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 1888 2100 50  0001 C CNN
F 3 "~" H 1850 2250 50  0001 C CNN
	1    1850 2250
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C3
U 1 1 5E31C47B
P 2700 2700
F 0 "C3" H 2818 2746 50  0000 L CNN
F 1 "0.01u" H 2818 2655 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 2738 2550 50  0001 C CNN
F 3 "~" H 2700 2700 50  0001 C CNN
	1    2700 2700
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C4
U 1 1 5E31C485
P 4200 2800
F 0 "C4" H 4318 2846 50  0000 L CNN
F 1 ",1u" H 4318 2755 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 4238 2650 50  0001 C CNN
F 3 "~" H 4200 2800 50  0001 C CNN
	1    4200 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT_TRIM RV1
U 1 1 5E31EE6B
P 4200 1800
F 0 "RV1" H 4130 1846 50  0000 R CNN
F 1 "10k" H 4130 1755 50  0000 R CNN
F 2 "Potentiometer_THT:Potentiometer_Bourns_3386P_Vertical" H 4200 1800 50  0001 C CNN
F 3 "~" H 4200 1800 50  0001 C CNN
	1    4200 1800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT_TRIM RV2
U 1 1 5E31F77A
P 4200 2250
F 0 "RV2" H 4130 2296 50  0000 R CNN
F 1 "10k" H 4130 2205 50  0000 R CNN
F 2 "Potentiometer_THT:Potentiometer_Bourns_3386P_Vertical" H 4200 2250 50  0001 C CNN
F 3 "~" H 4200 2250 50  0001 C CNN
	1    4200 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 1650 4350 1650
Wire Wire Line
	4350 1650 4350 1800
Wire Wire Line
	4200 2100 4350 2100
Wire Wire Line
	4350 2100 4350 2250
Wire Wire Line
	4200 1950 4200 2000
Connection ~ 4200 2100
Wire Wire Line
	3800 2000 4200 2000
Connection ~ 4200 2000
Wire Wire Line
	4200 2000 4200 2100
Wire Wire Line
	3800 2200 3900 2200
Wire Wire Line
	3900 2200 3900 1500
Wire Wire Line
	3900 1500 2700 1500
Wire Wire Line
	2700 1500 2700 1800
Wire Wire Line
	2700 1800 2800 1800
Wire Wire Line
	3900 2200 3900 2550
Wire Wire Line
	3900 2550 4200 2550
Connection ~ 3900 2200
Wire Wire Line
	4200 2400 4200 2550
Connection ~ 4200 2550
Wire Wire Line
	4200 2550 4200 2650
Wire Wire Line
	4200 1650 4200 1350
Wire Wire Line
	4200 1350 3300 1350
Wire Wire Line
	3300 1350 3300 1600
Connection ~ 4200 1650
Wire Wire Line
	3300 1350 2600 1350
Wire Wire Line
	2600 1350 2600 2200
Wire Wire Line
	2600 2200 2800 2200
Connection ~ 3300 1350
$Comp
L power:+5V #PWR07
U 1 1 5E32E933
P 3300 1200
F 0 "#PWR07" H 3300 1050 50  0001 C CNN
F 1 "+5V" H 3315 1373 50  0000 C CNN
F 2 "" H 3300 1200 50  0001 C CNN
F 3 "" H 3300 1200 50  0001 C CNN
	1    3300 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 1200 3300 1350
$Comp
L power:GNDD #PWR08
U 1 1 5E330C48
P 3300 3100
F 0 "#PWR08" H 3300 2850 50  0001 C CNN
F 1 "GNDD" H 3304 2945 50  0000 C CNN
F 2 "" H 3300 3100 50  0001 C CNN
F 3 "" H 3300 3100 50  0001 C CNN
	1    3300 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 2950 3300 2950
Wire Wire Line
	3300 2400 3300 2950
Wire Wire Line
	3300 3100 3300 2950
Connection ~ 3300 2950
Wire Wire Line
	2800 2000 2700 2000
Wire Wire Line
	2700 2000 2700 2550
Wire Wire Line
	2700 2850 2700 2950
Wire Wire Line
	2700 2950 3300 2950
Wire Wire Line
	1850 2100 2050 2100
$Comp
L power:+5V #PWR02
U 1 1 5E3445A5
P 2050 1950
F 0 "#PWR02" H 2050 1800 50  0001 C CNN
F 1 "+5V" H 2065 2123 50  0000 C CNN
F 2 "" H 2050 1950 50  0001 C CNN
F 3 "" H 2050 1950 50  0001 C CNN
	1    2050 1950
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR03
U 1 1 5E3450B2
P 2050 2550
F 0 "#PWR03" H 2050 2300 50  0001 C CNN
F 1 "GNDD" H 2054 2395 50  0000 C CNN
F 2 "" H 2050 2550 50  0001 C CNN
F 3 "" H 2050 2550 50  0001 C CNN
	1    2050 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 2550 2050 2400
Wire Wire Line
	2050 2400 1850 2400
Wire Wire Line
	2050 2100 2050 1950
Wire Wire Line
	2250 2100 2050 2100
Connection ~ 2050 2100
Wire Wire Line
	2050 2400 2250 2400
Connection ~ 2050 2400
Text GLabel 4050 1200 2    50   Input ~ 0
CLK555
Wire Wire Line
	4050 1200 3850 1200
Wire Wire Line
	3850 1200 3850 1800
Wire Wire Line
	3850 1800 3800 1800
Wire Notes Line
	1650 850  4550 850 
Wire Notes Line
	4550 850  4550 3750
Wire Notes Line
	4550 3750 1650 3750
Wire Notes Line
	1650 3750 1650 850 
Text Notes 2700 3600 0    50   ~ 0
555 Clock Generator
$Comp
L Connector_Generic:Conn_02x04_Odd_Even J1
U 1 1 5E35F04B
P 9750 3550
F 0 "J1" H 9800 3867 50  0000 C CNN
F 1 "MCU" H 9800 3776 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical" H 9750 3550 50  0001 C CNN
F 3 "~" H 9750 3550 50  0001 C CNN
	1    9750 3550
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR04
U 1 1 5E35FDF2
P 9400 3350
F 0 "#PWR04" H 9400 3200 50  0001 C CNN
F 1 "+5V" H 9415 3523 50  0000 C CNN
F 2 "" H 9400 3350 50  0001 C CNN
F 3 "" H 9400 3350 50  0001 C CNN
	1    9400 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	9400 3350 9400 3450
Wire Wire Line
	9400 3450 9550 3450
$Comp
L power:GNDD #PWR05
U 1 1 5E363295
P 9400 3900
F 0 "#PWR05" H 9400 3650 50  0001 C CNN
F 1 "GNDD" H 9404 3745 50  0000 C CNN
F 2 "" H 9400 3900 50  0001 C CNN
F 3 "" H 9400 3900 50  0001 C CNN
	1    9400 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9550 3750 9400 3750
Wire Wire Line
	9400 3750 9400 3900
Text GLabel 9300 3550 0    50   Input ~ 0
LINEINV
Wire Wire Line
	9300 3550 9550 3550
Text GLabel 9300 3650 0    50   Input ~ 0
CLKMCU
Wire Wire Line
	9300 3650 9550 3650
Text GLabel 10300 3750 2    50   Input ~ 0
IN-0
Text GLabel 10300 3650 2    50   Input ~ 0
IN-1
Text GLabel 10300 3550 2    50   Input ~ 0
IN-2
Text GLabel 10300 3450 2    50   Input ~ 0
IN-3
Wire Wire Line
	10300 3450 10050 3450
Wire Wire Line
	10300 3550 10050 3550
Wire Wire Line
	10300 3650 10050 3650
Wire Wire Line
	10300 3750 10050 3750
Wire Notes Line
	8750 3000 8750 4500
Wire Notes Line
	8750 4500 11000 4500
Wire Notes Line
	11000 4500 11000 3000
Wire Notes Line
	11000 3000 8750 3000
Text Notes 9600 4350 0    50   ~ 0
MCU Interface
Text GLabel 9550 1450 0    50   Input ~ 0
CLKMCU
Wire Wire Line
	9550 1450 9700 1450
$Comp
L Jumper:Jumper_3_Open JP1
U 1 1 5E3913DE
P 9950 1450
F 0 "JP1" H 9950 1674 50  0000 C CNN
F 1 "Jumper_3_Open" H 9950 1583 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 9950 1450 50  0001 C CNN
F 3 "~" H 9950 1450 50  0001 C CNN
	1    9950 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 1750 6350 1750
Text GLabel 6100 1750 0    50   Input ~ 0
CLK
Wire Notes Line
	8300 1000 5400 1000
Wire Wire Line
	9950 1850 9950 1600
Text GLabel 9950 1850 3    50   Input ~ 0
CLK
Text GLabel 10400 1450 2    50   Input ~ 0
CLK555
Wire Wire Line
	10400 1450 10200 1450
Wire Notes Line
	8900 1050 8900 2550
Wire Notes Line
	8900 2550 11000 2550
Wire Notes Line
	11000 2550 11000 1050
Wire Notes Line
	11000 1050 8900 1050
Text Notes 9700 2400 0    50   ~ 0
Clock Selection
Text GLabel 6050 5350 0    50   Input ~ 0
IN-0
Text GLabel 6050 5450 0    50   Input ~ 0
IN-1
Text GLabel 6050 5550 0    50   Input ~ 0
IN-2
Text GLabel 6050 5650 0    50   Input ~ 0
IN-3
Wire Wire Line
	6050 5550 6450 5550
Wire Wire Line
	6050 5450 6350 5450
Wire Wire Line
	6050 5350 6250 5350
$Comp
L Switch:SW_DIP_x04 SW1
U 1 1 5E3BF820
P 6900 5550
F 0 "SW1" H 6900 6017 50  0000 C CNN
F 1 "PWM" H 6900 5926 50  0000 C CNN
F 2 "Button_Switch_THT:SW_DIP_SPSTx04_Slide_9.78x12.34mm_W7.62mm_P2.54mm" H 6900 5550 50  0001 C CNN
F 3 "~" H 6900 5550 50  0001 C CNN
	1    6900 5550
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Network04 RN1
U 1 1 5E3C1CAF
P 6450 5000
F 0 "RN1" H 6638 5046 50  0000 L CNN
F 1 "10K" H 6638 4955 50  0000 L CNN
F 2 "Resistor_THT:R_Array_SIP5" V 6725 5000 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 6450 5000 50  0001 C CNN
	1    6450 5000
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Network04 RN2
U 1 1 5E3C3140
P 7550 5000
F 0 "RN2" H 7738 5046 50  0000 L CNN
F 1 "100R" H 7738 4955 50  0000 L CNN
F 2 "Resistor_THT:R_Array_SIP5" V 7825 5000 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 7550 5000 50  0001 C CNN
	1    7550 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 5200 6250 5350
Connection ~ 6250 5350
Wire Wire Line
	6250 5350 6600 5350
Wire Wire Line
	6350 5200 6350 5450
Connection ~ 6350 5450
Wire Wire Line
	6350 5450 6600 5450
Wire Wire Line
	6450 5200 6450 5550
Connection ~ 6450 5550
Wire Wire Line
	6450 5550 6600 5550
Wire Wire Line
	6550 5200 6550 5650
Wire Wire Line
	6050 5650 6550 5650
Connection ~ 6550 5650
Wire Wire Line
	6550 5650 6600 5650
Wire Wire Line
	7350 5200 7350 5350
Wire Wire Line
	7350 5350 7200 5350
Wire Wire Line
	7200 5450 7450 5450
Wire Wire Line
	7450 5450 7450 5200
Wire Wire Line
	7550 5200 7550 5550
Wire Wire Line
	7550 5550 7200 5550
Wire Wire Line
	7650 5200 7650 5650
Wire Wire Line
	7650 5650 7200 5650
$Comp
L power:GNDD #PWR01
U 1 1 5E3EB1C6
P 5950 4900
F 0 "#PWR01" H 5950 4650 50  0001 C CNN
F 1 "GNDD" H 5954 4745 50  0000 C CNN
F 2 "" H 5950 4900 50  0001 C CNN
F 3 "" H 5950 4900 50  0001 C CNN
	1    5950 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5950 4900 5950 4800
Wire Wire Line
	5950 4800 6250 4800
$Comp
L power:+5V #PWR06
U 1 1 5E3F079A
P 7350 4750
F 0 "#PWR06" H 7350 4600 50  0001 C CNN
F 1 "+5V" H 7365 4923 50  0000 C CNN
F 2 "" H 7350 4750 50  0001 C CNN
F 3 "" H 7350 4750 50  0001 C CNN
	1    7350 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	7350 4800 7350 4750
Wire Notes Line
	5700 4500 5700 6050
Wire Notes Line
	5700 6050 7950 6050
Wire Notes Line
	7950 6050 7950 4500
Wire Notes Line
	7950 4500 5700 4500
Text Notes 6550 5950 0    50   ~ 0
Input Override
Text GLabel 7550 2150 2    50   Input ~ 0
LINEINV
Wire Wire Line
	7550 2150 7350 2150
$Comp
L Connector:Conn_01x01_Female J3
U 1 1 5E2EE2B9
P 4250 6600
F 0 "J3" H 4278 6626 50  0000 L CNN
F 1 "H2" H 4278 6535 50  0000 L CNN
F 2 "Connector_PinHeader_1.00mm:PinHeader_1x01_P1.00mm_Vertical" H 4250 6600 50  0001 C CNN
F 3 "~" H 4250 6600 50  0001 C CNN
	1    4250 6600
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Female J4
U 1 1 5E2F4322
P 4250 6400
F 0 "J4" H 4278 6426 50  0000 L CNN
F 1 "H1" H 4278 6335 50  0000 L CNN
F 2 "Connector_PinHeader_1.00mm:PinHeader_1x01_P1.00mm_Vertical" H 4250 6400 50  0001 C CNN
F 3 "~" H 4250 6400 50  0001 C CNN
	1    4250 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 6400 4050 6600
$Comp
L power:GNDD #PWR?
U 1 1 5E66B6DA
P 6350 2800
F 0 "#PWR?" H 6350 2550 50  0001 C CNN
F 1 "GNDD" H 6354 2645 50  0000 C CNN
F 2 "" H 6350 2800 50  0001 C CNN
F 3 "" H 6350 2800 50  0001 C CNN
	1    6350 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 2800 6350 2650
Wire Wire Line
	2350 6250 2350 6350
Wire Wire Line
	2250 5850 2450 5850
Wire Wire Line
	2450 5650 2250 5650
$Comp
L power:+5V #PWR?
U 1 1 5E67DA7A
P 1850 5550
F 0 "#PWR?" H 1850 5400 50  0001 C CNN
F 1 "+5V" H 1865 5723 50  0000 C CNN
F 2 "" H 1850 5550 50  0001 C CNN
F 3 "" H 1850 5550 50  0001 C CNN
	1    1850 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 5650 1950 5650
Wire Wire Line
	1850 5550 1850 5650
$Comp
L Device:Q_TRIAC_A1A2G D1
U 1 1 5E68C8D1
P 3500 5850
F 0 "D1" H 3628 5896 50  0000 L CNN
F 1 "BTA20" H 3628 5805 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-220-3_Horizontal_TabDown" V 3575 5875 50  0001 C CNN
F 3 "~" V 3500 5850 50  0001 C CNN
	1    3500 5850
	1    0    0    -1  
$EndComp
$EndSCHEMATC
