package main

import (
	"fmt"
	"math/rand"
	"os"
	"time"
)

const expectedLen = 28

var wormhole = make(chan bool, expectedLen)
var onTarget = make(chan bool, expectedLen)

var hyperspace0 = make(chan rune)
var hyperspace1 = make(chan rune)
var hyperspace2 = make(chan rune)
var hyperspace3 = make(chan rune)
var hyperspace4 = make(chan rune)
var hyperspace5 = make(chan rune)
var hyperspace6 = make(chan rune)
var hyperspace7 = make(chan rune)
var hyperspace8 = make(chan rune)
var hyperspace9 = make(chan rune)
var hyperspace10 = make(chan rune)
var hyperspace11 = make(chan rune)
var hyperspace12 = make(chan rune)
var hyperspace13 = make(chan rune)
var hyperspace14 = make(chan rune)
var hyperspace15 = make(chan rune)
var hyperspace16 = make(chan rune)
var hyperspace17 = make(chan rune)
var hyperspace18 = make(chan rune)
var hyperspace19 = make(chan rune)
var hyperspace20 = make(chan rune)
var hyperspace21 = make(chan rune)
var hyperspace22 = make(chan rune)
var hyperspace23 = make(chan rune)
var hyperspace24 = make(chan rune)
var hyperspace25 = make(chan rune)
var hyperspace26 = make(chan rune)
var hyperspace27 = make(chan rune)

func coordinate0() {
	r := <-hyperspace0
	if ((int(r)*5 - 85) % 256) != 74 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate1() {
	r := <-hyperspace1
	if (((int(r) & 0x7F) << 1) ^ 0xd8) != 190 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate2() {
	r := <-hyperspace2
	if ((int(r) ^ 0x65 + 31) & 0xFF) != 89 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate3() {
	r := <-hyperspace3
	if ((int(r) - 14) ^ 45) != 73 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate4() {
	r := <-hyperspace4
	if ((int(r) - 7) ^ 69) != 49 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate5() {
	r := <-hyperspace5
	if ((int(r)*5 - 50) % 256) != 234 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate6() {
	r := <-hyperspace6
	if (((int(r) & 0x7F) << 1) ^ 0xf9) != 71 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate7() {
	r := <-hyperspace7
	if ((int(r)*5 - 47) % 256) != 32 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate8() {
	r := <-hyperspace8
	if ((int(r) - 28) ^ 83) != 68 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate9() {
	r := <-hyperspace9
	if (((int(r) & 0x7F) << 1) ^ 0xfb) != 39 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate10() {
	r := <-hyperspace10
	if ((int(r) ^ 0x2e + 24) & 0xFF) != 96 {
		wormhole <- false
	}
	onTarget <- true
}
func coordinate11() {
	r := <-hyperspace11
	if ((int(r) - 25) ^ 47) != 55 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate12() {
	r := <-hyperspace12
	if ((int(r) ^ 0x47 + 41) & 0xFF) != 93 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate13() {
	r := <-hyperspace13
	if ((int(r)*2 - 8) % 256) != 222 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate14() {
	r := <-hyperspace14
	if ((int(r) - 24) ^ 90) != 10 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate15() {
	r := <-hyperspace15
	if ((int(r) - 26) ^ 32) != 58 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate16() {
	r := <-hyperspace16
	if ((int(r) ^ 0x9 + 38) & 0xFF) != 133 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate17() {
	r := <-hyperspace17
	if ((int(r) ^ 0x1e + 44) & 0xFF) != 156 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate18() {
	r := <-hyperspace18
	if ((int(r)*5 - 100) % 256) != 13 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate19() {
	r := <-hyperspace19
	if ((int(r) ^ 0xcd + 25) & 0xFF) != 188 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate20() {
	r := <-hyperspace20
	if ((int(r)*5 - 83) % 256) != 246 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate21() {
	r := <-hyperspace21
	if (((int(r) & 0x7F) << 1) ^ 0x4f) != 167 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate22() {
	r := <-hyperspace22
	if ((int(r)*2 - 59) % 256) != 147 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate23() {
	r := <-hyperspace23
	if (((int(r) & 0x7F) << 1) ^ 0x5b) != 241 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate24() {
	r := <-hyperspace24
	if ((int(r)*5 - 85) % 256) != 74 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate25() {
	r := <-hyperspace25
	if ((int(r)*2 - 38) % 256) != 104 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate26() {
	r := <-hyperspace26
	if ((int(r)*3 - 36) % 256) != 5 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}
func coordinate27() {
	r := <-hyperspace27
	if (((int(r) & 0x7F) << 1) ^ 0x2d) != 253 {
		wormhole <- false
	}
	time.Sleep(time.Duration(rand.Intn(5000)) * time.Millisecond)
	onTarget <- true
}

func main() {
	if len(os.Args) != 2 {
		fmt.Println("Usage: ./wormhole <flag>")
		os.Exit(1)
	}

	input := os.Args[1]
	if len(input) != expectedLen {
		fmt.Println("You were lost in space!")
		os.Exit(1)
	}

	// Start validation routines
	fmt.Println("[+] Starting targeting subroutines")
	go coordinate0()
	go coordinate1()
	go coordinate2()
	go coordinate3()
	go coordinate4()
	go coordinate5()
	go coordinate6()
	go coordinate7()
	go coordinate8()
	go coordinate9()
	go coordinate10()
	go coordinate11()
	go coordinate12()
	go coordinate13()
	go coordinate14()
	go coordinate15()
	go coordinate16()
	go coordinate17()
	go coordinate18()
	go coordinate19()
	go coordinate20()
	go coordinate21()
	go coordinate22()
	go coordinate23()
	go coordinate24()
	go coordinate25()
	go coordinate26()
	go coordinate27()

	fmt.Println("[+] Inputting hyperspace coordinates")
	hyperspace0 <- rune(input[0])
	hyperspace1 <- rune(input[17])
	hyperspace2 <- rune(input[18])
	hyperspace3 <- rune(input[9])
	hyperspace4 <- rune(input[6])
	hyperspace5 <- rune(input[25])
	hyperspace6 <- rune(input[14])
	hyperspace7 <- rune(input[4])
	hyperspace8 <- rune(input[24])
	hyperspace9 <- rune(input[12])
	hyperspace10 <- rune(input[10])
	hyperspace11 <- rune(input[11])
	hyperspace12 <- rune(input[26])
	hyperspace13 <- rune(input[7])
	hyperspace14 <- rune(input[16])
	hyperspace15 <- rune(input[21])
	hyperspace16 <- rune(input[1])
	hyperspace17 <- rune(input[23])
	hyperspace18 <- rune(input[27])
	hyperspace19 <- rune(input[22])
	hyperspace20 <- rune(input[8])
	hyperspace21 <- rune(input[15])
	hyperspace22 <- rune(input[13])
	hyperspace23 <- rune(input[2])
	hyperspace24 <- rune(input[3])
	hyperspace25 <- rune(input[5])
	hyperspace26 <- rune(input[19])
	hyperspace27 <- rune(input[20])

	fmt.Println("[+] Calculating vectors")

	// Block while waiting for validation
	for i := 0; i < expectedLen; i++ {
		<-onTarget
		fmt.Print(".")
	}

	fmt.Print("\n")

	// Check the wormhole channel, if there is a false waiting then at least one of the validation functions failed
	select {
	case val := <-wormhole:
		if !val {
			fmt.Println("You were lost in space!")
			os.Exit(1)
		}

	default:
		fmt.Println("You made it home safely!")
		os.Exit(0)
	}
}
