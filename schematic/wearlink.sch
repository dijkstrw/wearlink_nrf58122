EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:WT51822-0S4AT
LIBS:fibretronickeypad
LIBS:wearlink-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Wearlink"
Date "2018-05-05"
Rev "2"
Comp "Willem Dijkstra <wpd@xs4all.nl>"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L WT51822-0S4AT U1
U 1 1 5AED8A82
P 4650 2850
F 0 "U1" H 4600 3000 60  0000 C CNN
F 1 "WT51822-0S4AT" H 4650 2850 60  0000 C CNN
F 2 "wearlink:WT51822-0S4AT" H 3750 2350 60  0001 C CNN
F 3 "" H 3750 2350 60  0001 C CNN
	1    4650 2850
	1    0    0    -1  
$EndComp
$Comp
L Battery_Cell BT1
U 1 1 5AED9A99
P 2650 2950
F 0 "BT1" H 2400 3050 50  0000 L CNN
F 1 "Battery 2032" H 2150 2900 50  0000 L CNN
F 2 "Battery_Holders:Keystone_3034_1x20mm-CoinCell" V 2650 3010 50  0001 C CNN
F 3 "" V 2650 3010 50  0001 C CNN
	1    2650 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 2750 2650 1850
Wire Wire Line
	2650 1850 7750 1850
Wire Wire Line
	4650 1850 4650 2050
Wire Wire Line
	2650 3050 2650 4050
Wire Wire Line
	2650 4050 7050 4050
$Comp
L +3.3V #PWR01
U 1 1 5AEDB4DB
P 4650 1850
F 0 "#PWR01" H 4650 1700 50  0001 C CNN
F 1 "+3.3V" H 4650 1990 50  0000 C CNN
F 2 "" H 4650 1850 50  0001 C CNN
F 3 "" H 4650 1850 50  0001 C CNN
	1    4650 1850
	1    0    0    -1  
$EndComp
$Comp
L FibretronicKeypad S1
U 1 1 5AEDB75E
P 8650 2350
F 0 "S1" H 9340 2730 60  0000 C CNN
F 1 "FibretronicKeypad" H 8340 2730 60  0000 C CNN
F 2 "Connectors_Molex:Molex_KK-6410-02_02x2.54mm_Straight" H 8650 2350 60  0001 C CNN
F 3 "" H 8650 2350 60  0001 C CNN
	1    8650 2350
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5AEDBA85
P 6250 3550
F 0 "R1" V 6330 3550 50  0000 C CNN
F 1 "20k" V 6250 3550 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 6180 3550 50  0001 C CNN
F 3 "" H 6250 3550 50  0001 C CNN
	1    6250 3550
	1    0    0    -1  
$EndComp
$Comp
L MMBT3904 Q1
U 1 1 5AEDBB1B
P 6950 3200
F 0 "Q1" H 7150 3275 50  0000 L CNN
F 1 "MMBT3904" H 7150 3200 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 7150 3125 50  0001 L CIN
F 3 "" H 6950 3200 50  0001 L CNN
	1    6950 3200
	1    0    0    -1  
$EndComp
$Comp
L R R3
U 1 1 5AEDBDFB
P 7050 2150
F 0 "R3" V 7130 2150 50  0000 C CNN
F 1 "1M" V 7050 2150 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 6980 2150 50  0001 C CNN
F 3 "" H 7050 2150 50  0001 C CNN
	1    7050 2150
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5AEDBE6E
P 6500 3200
F 0 "R2" V 6580 3200 50  0000 C CNN
F 1 "1M" V 6500 3200 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 6430 3200 50  0001 C CNN
F 3 "" H 6500 3200 50  0001 C CNN
	1    6500 3200
	0    1    1    0   
$EndComp
Wire Wire Line
	6250 3700 6250 4050
Connection ~ 6250 4050
Wire Wire Line
	7050 4050 7050 3400
Wire Wire Line
	6650 3200 6750 3200
Wire Wire Line
	7050 2300 7050 3000
Wire Wire Line
	7050 1850 7050 2000
Wire Wire Line
	6250 2450 6250 3400
Connection ~ 7050 2750
Connection ~ 6250 3200
Wire Wire Line
	5450 2750 7050 2750
Wire Wire Line
	5450 2450 7750 2450
Wire Wire Line
	6350 3200 6250 3200
Text GLabel 7050 2750 2    50   Input ~ 0
SENSE
Text GLabel 6250 2450 1    50   Input ~ 0
ADC
Text GLabel 7050 1850 1    50   Input ~ 0
POWER
$Comp
L Conn_01x06 J1
U 1 1 5AEDCC20
P 3200 3050
F 0 "J1" H 3200 3350 50  0000 C CNN
F 1 "JTAG" H 3200 2650 50  0000 C CNN
F 2 "wearlink:TC2030-IDC-NL" H 3200 3050 50  0001 C CNN
F 3 "" H 3200 3050 50  0001 C CNN
	1    3200 3050
	-1   0    0    -1  
$EndComp
Connection ~ 6250 2450
Wire Wire Line
	7750 1850 7750 2250
Connection ~ 7050 1850
Connection ~ 4650 1850
Wire Wire Line
	3700 3150 3850 3150
Wire Wire Line
	3400 2950 3600 2950
Wire Wire Line
	3600 2950 3600 4050
Wire Wire Line
	3600 3150 3400 3150
Connection ~ 3600 4050
Connection ~ 3600 3150
Wire Wire Line
	3400 3350 3500 3350
Wire Wire Line
	3500 3350 3500 1850
Connection ~ 3500 1850
$Comp
L GND #PWR02
U 1 1 5AEE0D87
P 4650 4050
F 0 "#PWR02" H 4650 3800 50  0001 C CNN
F 1 "GND" H 4650 3900 50  0000 C CNN
F 2 "" H 4650 4050 50  0001 C CNN
F 3 "" H 4650 4050 50  0001 C CNN
	1    4650 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 3250 3700 3250
Wire Wire Line
	3700 3250 3700 3150
Wire Wire Line
	3850 3250 3800 3250
Wire Wire Line
	3800 3250 3800 3050
Wire Wire Line
	3800 3050 3400 3050
Wire Wire Line
	4550 3650 4550 3800
Wire Wire Line
	4550 3800 4750 3800
Wire Wire Line
	4650 3650 4650 4050
Wire Wire Line
	4750 3800 4750 3650
Connection ~ 4650 3800
Connection ~ 4650 4050
$EndSCHEMATC
