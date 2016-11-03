// Copyright 2016 Conor O'Neill <conor@conoroneill.com>
// Apache License Version 2.0. See LICENSE file

// Portions via http://github.com/hybridgroup/gobot

package main

import (
	"fmt"
	"math/rand"
	"net/http"

	"bytes"
	"os"
	"os/exec"

	"github.com/hybridgroup/gobot"
	"github.com/hybridgroup/gobot/platforms/chip"
	"github.com/hybridgroup/gobot/platforms/gpio"
)

func main() {

	const numSounds = 5

	sounds := [numSounds]string{"/home/chip/halloween_sounds/castle_thunder.mp3", "/home/chip/halloween_sounds/clown_laugh_clipped.mp3", "/home/chip/halloween_sounds/home_alone.mp3", "/home/chip/halloween_sounds/howl.mp3", "/home/chip/halloween_sounds/man_screaming_clipped.mp3"}

	playing := 0

	gbot := gobot.NewGobot()

	chipAdaptor := chip.NewChipAdaptor("chip")
	button := gpio.NewButtonDriver(chipAdaptor, "button", "XIO-P0")
	led := gpio.NewLedDriver(chipAdaptor, "led", "XIO-P6")

	work := func() {
		gobot.On(button.Event("push"), func(data interface{}) {
			fmt.Println("button pressed")
			led.Toggle()
		})

		gobot.On(button.Event("release"), func(data interface{}) {
			fmt.Println("button released")

			if playing == 0 {
				playing = 1
				// mDNS was acting up on Halloween night so had to use hard-coded IP
				//resp, err := http.Get("http://skeletony.local/on")
				resp, err := http.Get("http://192.168.0.116/on")
				fmt.Println(resp)

				if err != nil {
					fmt.Fprintf(os.Stderr, "Skeletony error: %v\n", err)
				}
				led.Toggle()
				randSound := rand.Intn(numSounds)
				fmt.Println(randSound)
				fmt.Println(sounds[randSound])
				args1 := []string{sounds[randSound]}

				cmd := exec.Command("mpg123", args1...)
				var out bytes.Buffer
				cmd.Stdout = &out
				err = cmd.Run()
				if err != nil {
					fmt.Fprintf(os.Stderr, "error: %v\n", err)
					os.Exit(1)
				}
				playing = 0
			}
		})
	}

	robot := gobot.NewRobot("buttonBot",
		[]gobot.Connection{chipAdaptor},
		[]gobot.Device{led},
		[]gobot.Device{button},
		work,
	)
	gbot.AddRobot(robot)
	gbot.Start()
}
