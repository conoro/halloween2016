/*
 * Copyright (c) 2016, Conor O'Neill <conor@conoroneill.com>
 * Copyright (c) 2015, Vladimir Komendantskiy https://github.com/vkomenda/ssd1306
 * Copyright (c) Guillermo A. Amaral B. <g@maral.me> http://elinux.org/RPi_GPIO_Code_Samples
 *
 * MIT License
 *
 * SSD1306 demo of static image 
 * Reset code added by Conor O'Neill from Raspberry Pi to make it work with Adafruit module
 * Connect RESET PIN to XIO-P1
 * Also see example in http://www.eddy2099.com/accessing-chip-gpio-xojo/
 * Need to run this Bash code after every reboot to setup access to the GPIO pin. 
 * Not sure why. It creates /sys/class/gpio/gpio1017 dir and files
 * The C code should do this but it doesn't seem to be working for some reason

sudo sh -c 'echo 1017 > /sys/class/gpio/export'
sudo sh -c 'echo out > /sys/class/gpio/gpio1017/direction'

 */

#include <glib.h>
#include <glib/gprintf.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ssd1306.h"
#include "font5x7.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
 
#define IN  0
#define OUT 1
 
#define LOW  0
#define HIGH 1
 
#define POUT 1017  /* XIO-P1 */
#define BUFFER_MAX 5
#define DIRECTION_MAX 40
#define VALUE_MAX 40



#define FRAME_LENGTH        500000000L  // ns
#define NSEC_PER_SEC       1000000000L

static const unsigned char block8x8_patterns[5][8] = {
	{0, 0x7E, 0x7C, 0x78, 0x70, 0x60, 0x40, 0},    // big triangle
	{0, 0,    0x3C, 0x38, 0x30, 0x20, 0,    0},    // medium-size triangle
	{0, 0,    0,    0x18, 0x18, 0,    0,    0},    // 2x2 square
	{0, 0,    0x3C, 0x24, 0x24, 0x3C, 0,    0},    // 4x4 square outline
	{0, 0x7E, 0x42, 0x42, 0x42, 0x42, 0x7E, 0}     // 6x6 square outline
};

int ssd1306_demo(struct display_info* disp)
{
	struct timespec t;
	int status = 0, cycle = 0, step = 0, offset = 0;
	struct sized_array payload;

	while (1) {
		clock_gettime(CLOCK_MONOTONIC ,&t);

		// set the next timestamp
                t.tv_nsec += FRAME_LENGTH;
                while (t.tv_nsec >= NSEC_PER_SEC) {
                       t.tv_nsec -= NSEC_PER_SEC;
                        t.tv_sec++;
                }

		payload.size = sizeof(display_draw);
		payload.array = display_draw;

		status = ssd1306_send(disp, &payload);
		if (status < 0)
			return -1;

		memcpy(disp->buffer, font, sizeof(disp->buffer));

		status = ssd1306_send_buffer(disp);
		if (status < 0)
			return status;

		cycle++;
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
	}

	return 0;
}

void show_error(void)
{
	const gchar* errmsg;
	errmsg = g_strerror(errno);
	printf("\nERROR: %s\n\n", errmsg);
}

void show_usage(char* progname)
{
	printf("\nUsage:\n%s <I2C bus device node>\n", progname);
}


 
static int GPIOExport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
 
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}
 
static int GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
 
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}
 
static int GPIODirection(int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";
 
	char path[DIRECTION_MAX];
	int fd;
 
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}
 
	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}
 
 
static int GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";
 
	char path[VALUE_MAX];
	int fd;
 
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}
 
	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}
 
int reset_oled()
{

	/*
	 * Enable GPIO pins
	 */
	if (-1 == GPIOExport(POUT))
		return(1);
 
	/*
	 * Set GPIO directions
	 */
	if (-1 == GPIODirection(POUT, OUT))
		return(2);
 
	/*
	 * Write GPIO value
	 */
	if (-1 == GPIOWrite(POUT, 0))
  	return(3);
 
 	usleep(500 * 1000);

	/*
	 * Write GPIO value
	 */
	if (-1 == GPIOWrite(POUT, 1))
  	return(3);
 
	/*
	 * Disable GPIO pins
	 */
	if (-1 == GPIOUnexport(POUT))
		return(4);
 
	return(0);
}

int main(int argc, char** argv)
{
	char filename[32];
	const gchar* errmsg;
	struct display_info disp;
	
	if (argc < 2) {
		show_usage(argv[0]);
		return -1;
	}

	reset_oled();

	
	memset(&disp, 0, sizeof(disp));

	sprintf(filename, argv[1]);
	disp.address = SSD1306_I2C_ADDR;

	if (ssd1306_open(&disp, filename) < 0 ||
	    ssd1306_init(&disp)           < 0 ||
	    ssd1306_demo(&disp)           < 0)
	{
		show_error();
	}

	return 0;
}
