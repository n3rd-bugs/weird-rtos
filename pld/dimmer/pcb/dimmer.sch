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
P 7850 1900
F 0 "U1" H 7850 2781 50  0000 C CNN
F 1 "GAL16V8" H 7850 2690 50  0000 C CNN
F 2 "" H 7850 1900 50  0001 C CNN
F 3 "" H 7850 1900 50  0001 C CNN
	1    7850 1900
	1    0    0    -1  
$EndComp
$Comp
L Relay_SolidState:MOC3052M U4
U 1 1 5E2E872B
P 7700 4950
F 0 "U4" H 7700 5275 50  0000 C CNN
F 1 "MOC3052M" H 7700 5184 50  0000 C CNN
F 2 "" H 7500 4750 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/MO/MOC3052M.pdf" H 7700 4950 50  0001 L CNN
	1    7700 4950
	1    0    0    -1  
$EndComp
$Comp
L Timer:LM555 U3
U 1 1 5E2E9CF7
P 3000 1850
F 0 "U3" H 3000 2431 50  0000 C CNN
F 1 "LM555" H 3000 2340 50  0000 C CNN
F 2 "" H 3000 1850 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm555.pdf" H 3000 1850 50  0001 C CNN
	1    3000 1850
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 5E2EAA27
P 1950 2100
F 0 "C1" H 2068 2146 50  0000 L CNN
F 1 ".1u" H 2068 2055 50  0000 L CNN
F 2 "" H 1988 1950 50  0001 C CNN
F 3 "~" H 1950 2100 50  0001 C CNN
	1    1950 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 2200 7350 2200
Text GLabel 7100 1500 0    50   Input ~ 0
IN-0
Text GLabel 7100 1600 0    50   Input ~ 0
IN-1
Text GLabel 7100 1700 0    50   Input ~ 0
IN-2
Text GLabel 7100 1800 0    50   Input ~ 0
IN-3
Wire Wire Line
	7100 1800 7350 1800
Wire Wire Line
	7100 1700 7350 1700
Wire Wire Line
	7100 1600 7350 1600
Wire Wire Line
	7100 1500 7350 1500
Text GLabel 8550 2100 2    50   Input ~ 0
TRIG
Wire Wire Line
	8550 2100 8350 2100
$Comp
L power:+5V #PWR011
U 1 1 5E2F565A
P 7850 900
F 0 "#PWR011" H 7850 750 50  0001 C CNN
F 1 "+5V" H 7865 1073 50  0000 C CNN
F 2 "" H 7850 900 50  0001 C CNN
F 3 "" H 7850 900 50  0001 C CNN
	1    7850 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	7850 900  7850 1200
$Comp
L power:GNDD #PWR012
U 1 1 5E2F61E4
P 7850 2750
F 0 "#PWR012" H 7850 2500 50  0001 C CNN
F 1 "GNDD" H 7854 2595 50  0000 C CNN
F 2 "" H 7850 2750 50  0001 C CNN
F 3 "" H 7850 2750 50  0001 C CNN
	1    7850 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	7850 2750 7850 2600
Wire Notes Line
	6400 650  6400 3350
Wire Notes Line
	6400 3350 9300 3350
Wire Notes Line
	9300 3350 9300 650 
Text Notes 8500 3250 2    50   ~ 0
16v8 - Implements delayed trigger
$Comp
L Triac_Thyristor:BT169G Q1
U 1 1 5E2FE16C
P 8450 5050
F 0 "Q1" H 8538 5096 50  0000 L CNN
F 1 "BTA20" H 8538 5005 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 8550 4975 50  0001 L CIN
F 3 "https://media.digikey.com/pdf/Data%20Sheets/NXP%20PDFs/BT169_Series.pdf" H 8450 5050 50  0001 L CNN
	1    8450 5050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5E300454
P 8200 4850
F 0 "R4" V 7993 4850 50  0000 C CNN
F 1 "100R" V 8084 4850 50  0000 C CNN
F 2 "" V 8130 4850 50  0001 C CNN
F 3 "~" H 8200 4850 50  0001 C CNN
	1    8200 4850
	0    1    1    0   
$EndComp
Wire Wire Line
	8150 5050 8000 5050
Wire Wire Line
	8050 4850 8000 4850
Wire Wire Line
	8450 4900 8450 4850
Wire Wire Line
	8450 4850 8350 4850
$Comp
L Device:R R3
U 1 1 5E3029CC
P 8150 5350
F 0 "R3" H 8220 5396 50  0000 L CNN
F 1 "100R" H 8220 5305 50  0000 L CNN
F 2 "" V 8080 5350 50  0001 C CNN
F 3 "~" H 8150 5350 50  0001 C CNN
	1    8150 5350
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 5050 8150 5150
Wire Wire Line
	8300 5150 8150 5150
Wire Wire Line
	8450 5200 8450 5550
$Comp
L Connector:Screw_Terminal_01x04 J2
U 1 1 5E2ECAFA
P 9250 4700
F 0 "J2" H 9330 4692 50  0000 L CNN
F 1 "AC/Load" H 9330 4601 50  0000 L CNN
F 2 "" H 9250 4700 50  0001 C CNN
F 3 "~" H 9250 4700 50  0001 C CNN
	1    9250 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4700 9050 4800
Wire Wire Line
	9050 4600 8900 4600
Wire Wire Line
	8450 4600 8450 4850
Connection ~ 8450 4850
Wire Wire Line
	8150 5200 8150 5150
Connection ~ 8150 5150
Wire Wire Line
	8150 5500 8150 5550
Wire Wire Line
	8150 5550 8450 5550
Connection ~ 8450 5550
Wire Wire Line
	8450 5550 8900 5550
Wire Wire Line
	8900 5550 8900 4900
Wire Wire Line
	8900 4900 9050 4900
$Comp
L Device:R R5
U 1 1 5E3078C4
P 8250 4200
F 0 "R5" V 8457 4200 50  0000 C CNN
F 1 "20k" V 8366 4200 50  0000 C CNN
F 2 "" V 8180 4200 50  0001 C CNN
F 3 "~" H 8250 4200 50  0001 C CNN
	1    8250 4200
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R6
U 1 1 5E308575
P 8250 4400
F 0 "R6" V 8043 4400 50  0000 C CNN
F 1 "20k" V 8134 4400 50  0000 C CNN
F 2 "" V 8180 4400 50  0001 C CNN
F 3 "~" H 8250 4400 50  0001 C CNN
	1    8250 4400
	0    1    1    0   
$EndComp
Wire Wire Line
	8100 4400 8000 4400
Wire Wire Line
	8000 4200 8100 4200
Wire Wire Line
	8400 4200 8900 4200
Wire Wire Line
	8900 4200 8900 4600
Connection ~ 8900 4600
Wire Wire Line
	8900 4600 8450 4600
Wire Wire Line
	8750 4400 8400 4400
Wire Wire Line
	9050 4800 8750 4800
Connection ~ 9050 4800
Wire Wire Line
	8750 4400 8750 4800
$Comp
L Device:R R2
U 1 1 5E30BA09
P 7300 5300
F 0 "R2" H 7370 5346 50  0000 L CNN
F 1 "10k" H 7370 5255 50  0000 L CNN
F 2 "" V 7230 5300 50  0001 C CNN
F 3 "~" H 7300 5300 50  0001 C CNN
	1    7300 5300
	1    0    0    -1  
$EndComp
$Comp
L Isolator:4N35 U2
U 1 1 5E2E7D49
P 7700 4300
F 0 "U2" H 7700 4625 50  0000 C CNN
F 1 "4N35" H 7700 4534 50  0000 C CNN
F 2 "Package_DIP:DIP-6_W7.62mm" H 7500 4100 50  0001 L CIN
F 3 "https://www.vishay.com/docs/81181/4n35.pdf" H 7700 4300 50  0001 L CNN
	1    7700 4300
	-1   0    0    -1  
$EndComp
Wire Wire Line
	7400 4400 7300 4400
Wire Wire Line
	7300 4400 7300 5150
$Comp
L power:+5V #PWR09
U 1 1 5E30F5C2
P 7300 3950
F 0 "#PWR09" H 7300 3800 50  0001 C CNN
F 1 "+5V" H 7315 4123 50  0000 C CNN
F 2 "" H 7300 3950 50  0001 C CNN
F 3 "" H 7300 3950 50  0001 C CNN
	1    7300 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7300 3950 7300 4300
Wire Wire Line
	7300 4300 7400 4300
$Comp
L power:GNDD #PWR010
U 1 1 5E3108A1
P 7300 5550
F 0 "#PWR010" H 7300 5300 50  0001 C CNN
F 1 "GNDD" H 7304 5395 50  0000 C CNN
F 2 "" H 7300 5550 50  0001 C CNN
F 3 "" H 7300 5550 50  0001 C CNN
	1    7300 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7300 5550 7300 5500
Text GLabel 7100 2200 0    50   Input ~ 0
Line
Wire Wire Line
	7050 4400 7300 4400
Text GLabel 7050 4400 0    50   Input ~ 0
Line
Connection ~ 7300 4400
Text GLabel 7050 4850 0    50   Input ~ 0
TRIG
Wire Wire Line
	7050 4850 7400 4850
$Comp
L Device:R R1
U 1 1 5E314E2A
P 7000 5050
F 0 "R1" V 6793 5050 50  0000 C CNN
F 1 "1k" V 6884 5050 50  0000 C CNN
F 2 "" V 6930 5050 50  0001 C CNN
F 3 "~" H 7000 5050 50  0001 C CNN
	1    7000 5050
	0    1    1    0   
$EndComp
Wire Wire Line
	7150 5050 7400 5050
Wire Wire Line
	6850 5050 6850 5500
Wire Wire Line
	6850 5500 7300 5500
Connection ~ 7300 5500
Wire Wire Line
	7300 5500 7300 5450
Wire Notes Line
	6400 3650 9750 3650
Wire Notes Line
	9750 3650 9750 6050
Wire Notes Line
	9750 6050 6400 6050
Wire Notes Line
	6400 6050 6400 3650
Text Notes 8450 5900 2    50   ~ 0
Control and ZC detection
$Comp
L Device:CP C2
U 1 1 5E31B886
P 1550 2100
F 0 "C2" H 1668 2146 50  0000 L CNN
F 1 "CP" H 1668 2055 50  0000 L CNN
F 2 "" H 1588 1950 50  0001 C CNN
F 3 "~" H 1550 2100 50  0001 C CNN
	1    1550 2100
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C3
U 1 1 5E31C47B
P 2400 2550
F 0 "C3" H 2518 2596 50  0000 L CNN
F 1 "0.1u" H 2518 2505 50  0000 L CNN
F 2 "" H 2438 2400 50  0001 C CNN
F 3 "~" H 2400 2550 50  0001 C CNN
	1    2400 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C4
U 1 1 5E31C485
P 3900 2650
F 0 "C4" H 4018 2696 50  0000 L CNN
F 1 "CP" H 4018 2605 50  0000 L CNN
F 2 "" H 3938 2500 50  0001 C CNN
F 3 "~" H 3900 2650 50  0001 C CNN
	1    3900 2650
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT_TRIM RV1
U 1 1 5E31EE6B
P 3900 1650
F 0 "RV1" H 3830 1696 50  0000 R CNN
F 1 "10k" H 3830 1605 50  0000 R CNN
F 2 "" H 3900 1650 50  0001 C CNN
F 3 "~" H 3900 1650 50  0001 C CNN
	1    3900 1650
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT_TRIM RV2
U 1 1 5E31F77A
P 3900 2100
F 0 "RV2" H 3830 2146 50  0000 R CNN
F 1 "10k" H 3830 2055 50  0000 R CNN
F 2 "" H 3900 2100 50  0001 C CNN
F 3 "~" H 3900 2100 50  0001 C CNN
	1    3900 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 1500 4050 1500
Wire Wire Line
	4050 1500 4050 1650
Wire Wire Line
	3900 1950 4050 1950
Wire Wire Line
	4050 1950 4050 2100
Wire Wire Line
	3900 1800 3900 1850
Connection ~ 3900 1950
Wire Wire Line
	3500 1850 3900 1850
Connection ~ 3900 1850
Wire Wire Line
	3900 1850 3900 1950
Wire Wire Line
	3500 2050 3600 2050
Wire Wire Line
	3600 2050 3600 1350
Wire Wire Line
	3600 1350 2400 1350
Wire Wire Line
	2400 1350 2400 1650
Wire Wire Line
	2400 1650 2500 1650
Wire Wire Line
	3600 2050 3600 2400
Wire Wire Line
	3600 2400 3900 2400
Connection ~ 3600 2050
Wire Wire Line
	3900 2250 3900 2400
Connection ~ 3900 2400
Wire Wire Line
	3900 2400 3900 2500
Wire Wire Line
	3900 1500 3900 1200
Wire Wire Line
	3900 1200 3000 1200
Wire Wire Line
	3000 1200 3000 1450
Connection ~ 3900 1500
Wire Wire Line
	3000 1200 2300 1200
Wire Wire Line
	2300 1200 2300 2050
Wire Wire Line
	2300 2050 2500 2050
Connection ~ 3000 1200
$Comp
L power:+5V #PWR07
U 1 1 5E32E933
P 3000 1050
F 0 "#PWR07" H 3000 900 50  0001 C CNN
F 1 "+5V" H 3015 1223 50  0000 C CNN
F 2 "" H 3000 1050 50  0001 C CNN
F 3 "" H 3000 1050 50  0001 C CNN
	1    3000 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 1050 3000 1200
$Comp
L power:GNDD #PWR08
U 1 1 5E330C48
P 3000 2950
F 0 "#PWR08" H 3000 2700 50  0001 C CNN
F 1 "GNDD" H 3004 2795 50  0000 C CNN
F 2 "" H 3000 2950 50  0001 C CNN
F 3 "" H 3000 2950 50  0001 C CNN
	1    3000 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 2800 3000 2800
Wire Wire Line
	3000 2250 3000 2800
Wire Wire Line
	3000 2950 3000 2800
Connection ~ 3000 2800
Wire Wire Line
	2500 1850 2400 1850
Wire Wire Line
	2400 1850 2400 2400
Wire Wire Line
	2400 2700 2400 2800
Wire Wire Line
	2400 2800 3000 2800
Wire Wire Line
	1550 1950 1750 1950
$Comp
L power:+5V #PWR02
U 1 1 5E3445A5
P 1750 1800
F 0 "#PWR02" H 1750 1650 50  0001 C CNN
F 1 "+5V" H 1765 1973 50  0000 C CNN
F 2 "" H 1750 1800 50  0001 C CNN
F 3 "" H 1750 1800 50  0001 C CNN
	1    1750 1800
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR03
U 1 1 5E3450B2
P 1750 2400
F 0 "#PWR03" H 1750 2150 50  0001 C CNN
F 1 "GNDD" H 1754 2245 50  0000 C CNN
F 2 "" H 1750 2400 50  0001 C CNN
F 3 "" H 1750 2400 50  0001 C CNN
	1    1750 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 2400 1750 2250
Wire Wire Line
	1750 2250 1550 2250
Wire Wire Line
	1750 1950 1750 1800
Wire Wire Line
	1950 1950 1750 1950
Connection ~ 1750 1950
Wire Wire Line
	1750 2250 1950 2250
Connection ~ 1750 2250
Text GLabel 3750 1050 2    50   Input ~ 0
CLK555
Wire Wire Line
	3750 1050 3550 1050
Wire Wire Line
	3550 1050 3550 1650
Wire Wire Line
	3550 1650 3500 1650
Wire Notes Line
	1350 700  4250 700 
Wire Notes Line
	4250 700  4250 3600
Wire Notes Line
	4250 3600 1350 3600
Wire Notes Line
	1350 3600 1350 700 
Text Notes 2400 3450 0    50   ~ 0
555 Clock Generator
$Comp
L Connector_Generic:Conn_02x04_Odd_Even J1
U 1 1 5E35F04B
P 2150 4750
F 0 "J1" H 2200 5067 50  0000 C CNN
F 1 "MCU" H 2200 4976 50  0000 C CNN
F 2 "" H 2150 4750 50  0001 C CNN
F 3 "~" H 2150 4750 50  0001 C CNN
	1    2150 4750
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR04
U 1 1 5E35FDF2
P 1800 4550
F 0 "#PWR04" H 1800 4400 50  0001 C CNN
F 1 "+5V" H 1815 4723 50  0000 C CNN
F 2 "" H 1800 4550 50  0001 C CNN
F 3 "" H 1800 4550 50  0001 C CNN
	1    1800 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 4550 1800 4650
Wire Wire Line
	1800 4650 1950 4650
$Comp
L power:GNDD #PWR05
U 1 1 5E363295
P 1800 5100
F 0 "#PWR05" H 1800 4850 50  0001 C CNN
F 1 "GNDD" H 1804 4945 50  0000 C CNN
F 2 "" H 1800 5100 50  0001 C CNN
F 3 "" H 1800 5100 50  0001 C CNN
	1    1800 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 4950 1800 4950
Wire Wire Line
	1800 4950 1800 5100
Text GLabel 1700 4750 0    50   Input ~ 0
LINE_INV
Wire Wire Line
	1700 4750 1950 4750
Text GLabel 1700 4850 0    50   Input ~ 0
CLK_MCU
Wire Wire Line
	1700 4850 1950 4850
Text GLabel 2700 4950 2    50   Input ~ 0
IN-0
Text GLabel 2700 4850 2    50   Input ~ 0
IN-1
Text GLabel 2700 4750 2    50   Input ~ 0
IN-2
Text GLabel 2700 4650 2    50   Input ~ 0
IN-3
Wire Wire Line
	2700 4650 2450 4650
Wire Wire Line
	2700 4750 2450 4750
Wire Wire Line
	2700 4850 2450 4850
Wire Wire Line
	2700 4950 2450 4950
Wire Notes Line
	1150 4200 1150 5700
Wire Notes Line
	1150 5700 3400 5700
Wire Notes Line
	3400 5700 3400 4200
Wire Notes Line
	3400 4200 1150 4200
Text Notes 2000 5550 0    50   ~ 0
MCU Interface
Text GLabel 4350 4600 0    50   Input ~ 0
CLK_MCU
Wire Wire Line
	4350 4600 4500 4600
$Comp
L Jumper:Jumper_3_Open JP1
U 1 1 5E3913DE
P 4750 4600
F 0 "JP1" H 4750 4824 50  0000 C CNN
F 1 "Jumper_3_Open" H 4750 4733 50  0000 C CNN
F 2 "" H 4750 4600 50  0001 C CNN
F 3 "~" H 4750 4600 50  0001 C CNN
	1    4750 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 1400 7350 1400
Text GLabel 7100 1400 0    50   Input ~ 0
CLK
Wire Notes Line
	9300 650  6400 650 
Wire Wire Line
	4750 5000 4750 4750
Text GLabel 4750 5000 3    50   Input ~ 0
CLK
Text GLabel 5200 4600 2    50   Input ~ 0
CLK555
Wire Wire Line
	5200 4600 5000 4600
Wire Notes Line
	3700 4200 3700 5700
Wire Notes Line
	3700 5700 5800 5700
Wire Notes Line
	5800 5700 5800 4200
Wire Notes Line
	5800 4200 3700 4200
Text Notes 4500 5550 0    50   ~ 0
Clock Selection
Text GLabel 1500 6700 0    50   Input ~ 0
IN-0
Text GLabel 1500 6800 0    50   Input ~ 0
IN-1
Text GLabel 1500 6900 0    50   Input ~ 0
IN-2
Text GLabel 1500 7000 0    50   Input ~ 0
IN-3
Wire Wire Line
	1500 6900 1900 6900
Wire Wire Line
	1500 6800 1800 6800
Wire Wire Line
	1500 6700 1700 6700
$Comp
L Switch:SW_DIP_x04 SW1
U 1 1 5E3BF820
P 2350 6900
F 0 "SW1" H 2350 7367 50  0000 C CNN
F 1 "PWM" H 2350 7276 50  0000 C CNN
F 2 "" H 2350 6900 50  0001 C CNN
F 3 "~" H 2350 6900 50  0001 C CNN
	1    2350 6900
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Network04 RN1
U 1 1 5E3C1CAF
P 1900 6350
F 0 "RN1" H 2088 6396 50  0000 L CNN
F 1 "10K" H 2088 6305 50  0000 L CNN
F 2 "Resistor_THT:R_Array_SIP5" V 2175 6350 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 1900 6350 50  0001 C CNN
	1    1900 6350
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Network04 RN2
U 1 1 5E3C3140
P 3000 6350
F 0 "RN2" H 3188 6396 50  0000 L CNN
F 1 "100R" H 3188 6305 50  0000 L CNN
F 2 "Resistor_THT:R_Array_SIP5" V 3275 6350 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 3000 6350 50  0001 C CNN
	1    3000 6350
	1    0    0    -1  
$EndComp
Wire Wire Line
	1700 6550 1700 6700
Connection ~ 1700 6700
Wire Wire Line
	1700 6700 2050 6700
Wire Wire Line
	1800 6550 1800 6800
Connection ~ 1800 6800
Wire Wire Line
	1800 6800 2050 6800
Wire Wire Line
	1900 6550 1900 6900
Connection ~ 1900 6900
Wire Wire Line
	1900 6900 2050 6900
Wire Wire Line
	2000 6550 2000 7000
Wire Wire Line
	1500 7000 2000 7000
Connection ~ 2000 7000
Wire Wire Line
	2000 7000 2050 7000
Wire Wire Line
	2800 6550 2800 6700
Wire Wire Line
	2800 6700 2650 6700
Wire Wire Line
	2650 6800 2900 6800
Wire Wire Line
	2900 6800 2900 6550
Wire Wire Line
	3000 6550 3000 6900
Wire Wire Line
	3000 6900 2650 6900
Wire Wire Line
	3100 6550 3100 7000
Wire Wire Line
	3100 7000 2650 7000
$Comp
L power:GNDD #PWR01
U 1 1 5E3EB1C6
P 1400 6250
F 0 "#PWR01" H 1400 6000 50  0001 C CNN
F 1 "GNDD" H 1404 6095 50  0000 C CNN
F 2 "" H 1400 6250 50  0001 C CNN
F 3 "" H 1400 6250 50  0001 C CNN
	1    1400 6250
	1    0    0    -1  
$EndComp
Wire Wire Line
	1400 6250 1400 6150
Wire Wire Line
	1400 6150 1700 6150
$Comp
L power:+5V #PWR06
U 1 1 5E3F079A
P 2800 6100
F 0 "#PWR06" H 2800 5950 50  0001 C CNN
F 1 "+5V" H 2815 6273 50  0000 C CNN
F 2 "" H 2800 6100 50  0001 C CNN
F 3 "" H 2800 6100 50  0001 C CNN
	1    2800 6100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 6150 2800 6100
Wire Notes Line
	1150 5850 1150 7400
Wire Notes Line
	1150 7400 3400 7400
Wire Notes Line
	3400 7400 3400 5850
Wire Notes Line
	3400 5850 1150 5850
Text Notes 2000 7300 0    50   ~ 0
Input Override
$EndSCHEMATC
