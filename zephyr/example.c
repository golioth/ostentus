import "ostentus.h"

/* Add ostentus.c to the CMakeLists.txt */

/*
 * This demonstrates using the ostentus.c helper functions to write to the
 * Ostentus board.
 */
int main(void) {
	show_splash();
	clear_memory();

	/* Write "Temperature" to framebuffer */
	update_thickness(3); // Set pen thickness
	update_font(0); // Choose sans font
	char msg[] = "Temperature";
	store_text(msg, strlen(msg)); // Write message to data buffer
	write_text(3, 60, 10); // Write data buffer text at x=3, y=60 scale=1.0

	update_thickness(8); // Set pen thickness
	uint8_t mask = 1;
	uint8_t whole = 26;
	uint8_t decimal = 0;
	while(1) {
		led_bitmask(mask); // Update Ostentus LEDS using bitmask
		mask = mask<<1;
		if (mask > 1<<4) {
			mask = 1;
		}

		/* Write number "##.#" to framebuffer */
		clear_rectangle(0, 100, 200, 80); // Clear last number from framebuffer
		snprintk(msg, 5, "%d.%d", whole, decimal);
		store_text(msg, strlen(msg)); // Write message to data buffer
		write_text(12, 140, 26); // Write data buffer text at x=3, y=60 scale=1.0
		update_display(); // Write framebuffer to epaper and refresh the display
		++decimal;
		if (decimal > 9) {
			decimal = 0;
			++whole;
		}
		k_sleep(K_SECONDS(2));
	}
}
