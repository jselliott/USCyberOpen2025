package main

import (
	"fmt"
	"os"
)

const expectedLen = 28

var resultChan = make(chan bool, expectedLen)

var ch0 = make(chan rune)
var ch1 = make(chan rune)
var ch2 = make(chan rune)
var ch3 = make(chan rune)
var ch4 = make(chan rune)
var ch5 = make(chan rune)
var ch6 = make(chan rune)
var ch7 = make(chan rune)
var ch8 = make(chan rune)
var ch9 = make(chan rune)
var ch10 = make(chan rune)
var ch11 = make(chan rune)
var ch12 = make(chan rune)
var ch13 = make(chan rune)
var ch14 = make(chan rune)
var ch15 = make(chan rune)
var ch16 = make(chan rune)
var ch17 = make(chan rune)
var ch18 = make(chan rune)
var ch19 = make(chan rune)
var ch20 = make(chan rune)
var ch21 = make(chan rune)
var ch22 = make(chan rune)
var ch23 = make(chan rune)
var ch24 = make(chan rune)
var ch25 = make(chan rune)
var ch26 = make(chan rune)
var ch27 = make(chan rune)

func validator0()  { r := <-ch0; resultChan <- ((int(r)*2 - 30) % 256) == 136 }
func validator1()  { r := <-ch1; resultChan <- ((int(r) ^ 0x7e + 44) & 0xFF) == 84 }
func validator2()  { r := <-ch2; resultChan <- ((int(r)*5 - 85) % 256) == 84 }
func validator3()  { r := <-ch3; resultChan <- ((int(r) ^ 0x6a + 32) & 0xFF) == 89 }
func validator4()  { r := <-ch4; resultChan <- ((int(r) ^ 0xde + 16) & 0xFF) == 173 }
func validator5()  { r := <-ch5; resultChan <- ((int(r) - 16) ^ 74) == 125 }
func validator6()  { r := <-ch6; resultChan <- ((int(r)*2 - 87) % 256) == 159 }
func validator7()  { r := <-ch7; resultChan <- ((int(r) ^ 0x5b + 10) & 0xFF) == 50 }
func validator8()  { r := <-ch8; resultChan <- (((int(r) & 0x7F) << 1) ^ 0x41) == 171 }
func validator9()  { r := <-ch9; resultChan <- ((int(r) - 15) ^ 18) == 113 }
func validator10() { r := <-ch10; resultChan <- ((int(r)*2 - 74) % 256) == 130 }
func validator11() { r := <-ch11; resultChan <- ((int(r) - 22) ^ 41) == 50 }
func validator12() { r := <-ch12; resultChan <- ((int(r) - 10) ^ 78) == 42 }
func validator13() { r := <-ch13; resultChan <- ((int(r) ^ 0x67 + 46) & 0xFF) == 46 }
func validator14() { r := <-ch14; resultChan <- (((int(r) & 0x7F) << 1) ^ 0x61) == 223 }
func validator15() { r := <-ch15; resultChan <- ((int(r)*3 - 54) % 256) == 38 }
func validator16() { r := <-ch16; resultChan <- ((int(r) ^ 0xfc + 27) & 0xFF) == 175 }
func validator17() { r := <-ch17; resultChan <- ((int(r) ^ 0xdc + 27) & 0xFF) == 10 }
func validator18() { r := <-ch18; resultChan <- ((int(r)*2 - 32) % 256) == 158 }
func validator19() { r := <-ch19; resultChan <- ((int(r) - 29) ^ 19) == 85 }
func validator20() { r := <-ch20; resultChan <- ((int(r)*3 - 63) % 256) == 249 }
func validator21() { r := <-ch21; resultChan <- ((int(r)*5 - 78) % 256) == 182 }
func validator22() { r := <-ch22; resultChan <- ((int(r) ^ 0xd1 + 18) & 0xFF) == 209 }
func validator23() { r := <-ch23; resultChan <- ((int(r) - 24) ^ 86) == 0 }
func validator24() { r := <-ch24; resultChan <- ((int(r) ^ 0xe + 26) & 0xFF) == 87 }
func validator25() { r := <-ch25; resultChan <- ((int(r)*2 - 92) % 256) == 124 }
func validator26() { r := <-ch26; resultChan <- ((int(r)*2 - 63) % 256) == 167 }
func validator27() { r := <-ch27; resultChan <- ((int(r) ^ 0xa8 + 22) & 0xFF) == 235 }

func main() {
	if len(os.Args) != 2 {
		fmt.Println("Usage: ./wormhole <flag>")
		os.Exit(1)
	}

	input := os.Args[1]
	if len(input) != expectedLen {
		fmt.Println("Incorrect!")
		os.Exit(1)
	}

	go validator0()
	go validator1()
	go validator2()
	go validator3()
	go validator4()
	go validator5()
	go validator6()
	go validator7()
	go validator8()
	go validator9()
	go validator10()
	go validator11()
	go validator12()
	go validator13()
	go validator14()
	go validator15()
	go validator16()
	go validator17()
	go validator18()
	go validator19()
	go validator20()
	go validator21()
	go validator22()
	go validator23()
	go validator24()
	go validator25()
	go validator26()
	go validator27()

	ch0 <- rune(input[0])
	ch1 <- rune(input[1])
	ch2 <- rune(input[2])
	ch3 <- rune(input[3])
	ch4 <- rune(input[4])
	ch5 <- rune(input[5])
	ch6 <- rune(input[6])
	ch7 <- rune(input[7])
	ch8 <- rune(input[8])
	ch9 <- rune(input[9])
	ch10 <- rune(input[10])
	ch11 <- rune(input[11])
	ch12 <- rune(input[12])
	ch13 <- rune(input[13])
	ch14 <- rune(input[14])
	ch15 <- rune(input[15])
	ch16 <- rune(input[16])
	ch17 <- rune(input[17])
	ch18 <- rune(input[18])
	ch19 <- rune(input[19])
	ch20 <- rune(input[20])
	ch21 <- rune(input[21])
	ch22 <- rune(input[22])
	ch23 <- rune(input[23])
	ch24 <- rune(input[24])
	ch25 <- rune(input[25])
	ch26 <- rune(input[26])
	ch27 <- rune(input[27])

	valid := true
	for i := 0; i < expectedLen; i++ {
		if !<-resultChan {
			valid = false
		}
	}

	if valid {
		fmt.Println("Correct!")
	} else {
		fmt.Println("Incorrect!")
	}
}
