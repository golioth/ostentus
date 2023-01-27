import i2cperipheral
import badger2040
import splashscreen_rd
from ostentus_leds import o_leds

class ostentus:
    def __init__(self, bus=0, sclPin=5, sdaPin=4, address=0x12):
        self.bus = bus
        self.sclPin = sclPin
        self.sdaPin = sdaPin
        self.address = address

        self.display=badger2040.Badger2040()
        self.display.font("serif")
        self.display.thickness(2)

        self.str_data = [bytearray(b'\x00'*32)]*6
        self.text_buffer = ""
        self.x_loc = 0
        self.y_loc = 0

        self.leds = o_leds()
        self.leds.boot_animation()

    def clear_all_memory(self, clear_display=False):
        self.str_data = [bytearray(b'\x00'*32)]*6
        self.text_buffer = ""
        self.x_loc = 0
        self.y_loc = 0

        self.display.pen(0)
        self.display.clear()
        self.display.pen(15)

        if clear_display:
            self.display.update_speed(0x00)
            self.display.update()

    def clear_str_memory(self, line_idx):
        self.str_data[line_idx] = bytearray(b'\x00'*32)

    def show_splash(self):
        splash = bytearray(splashscreen_rd.data())
        self.display.image(splash)
        self.display.update()

    def write_string(self, line_idx):
        self.display.pen(15)
        outstring=""
        for c in self.str_data[line_idx]:
            if c == 0x00:
                # assume null terminator is a the end of the string
                break
            elif c < 0x20 or c > 0x7E:
                outstring += ' '
            else:
                outstring += chr(c)
        print("outstring:",outstring)
        self.display.text(outstring, self.x_loc, 10+(22*line_idx), 0.7)

    def listen(self):

        i2c = i2cperipheral.I2CPeripheral(bus=self.bus, sclPin=self.sclPin,
                sdaPin=self.sdaPin, address=self.address)
        misc_byte = bytearray(1)
        clear_byte = bytearray(1)
        refresh_byte = bytearray(1)
        coordinates = bytearray(2)

        self.clear_all_memory()
        print("Listening...")
        while True:
            regAddressBuff = bytearray(1)

            # First thing controller should send is register address.
            # Poll to see if it has been received yet.
            if not i2c.have_recv_req():
                continue
            i2c.recv(regAddressBuff, timeout=0)

            # Wait for controller to send either the read or write.
            while (not i2c.have_recv_req()) and (not i2c.have_send_req()):
                pass

            # Addr 0x00: clear memory
            regAddress = regAddressBuff[0]
            if regAddress == 0x00:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(clear_byte, timeout=1000)
                    except OSError:
                        print("Error: timout receiving memory clear command")
                        continue

                self.clear_all_memory(clear_byte[0])
                continue

            # Addr 0x01: refresh display
            elif regAddress == 0x01:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(refresh_byte, timeout=1000)

                        if 0x00 <= refresh_byte[0] <= 0x03:
                            self.display.update_speed(refresh_byte[0])
                            self.display.update()

                    except OSError:
                        print("Error: timout receiving display refresh command")
                continue

            # Addr 0x02..0x03: change x_loc or y_loc offsets
            elif regAddress in [0x02, 0x03]:
                if i2c.have_recv_req():
                    try:
                        coordinates = bytearray(b'\x00'*2)
                        i2c.recv(coordinates, timeout=1000)

                    except OSError:
                        print("Error: timout receiving new x/y location")
                        continue

                    loc_value = int.from_bytes(coordinates, "big")
                    if regAddress == 0x02:
                        if 0x00 <= loc_value < badger2040.WIDTH:
                            self.x_loc = loc_value
                        else:
                            print("Received x index out of bounds:", loc_value)
                    if regAddress == 0x03:
                        if 0x00 <= loc_value < badger2040.HEIGHT:
                            self.y_loc = loc_value
                        else:
                            print("Received y index out of bounds:", loc_value)
                continue

            # Addr 0x04: show Golioth splashscreen
            elif regAddress == 0x04:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(misc_byte, timeout=1000)
                        self.show_splash()

                    except OSError:
                        print("Error: could not display the splashscreen")
                        continue

                    print("Showing splashscreen")
                continue

            # Addr 0x05: change the pen thickness
            elif regAddress == 0x05:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(misc_byte, timeout=1000)
                        self.display.thickness(misc_byte[0])

                    except OSError:
                        print("Error: could not display the splashscreen")
                        continue

                    print("Changing pen thickness to: ", misc_byte[0])
                continue

            # Addr 0x06: change the font
            elif regAddress == 0x06:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(misc_byte, timeout=1000)
                        fonts = ["sans", "gothic" "cursive" "serif" "serif_italic"]
                        self.display.font(fonts[misc_byte[0]])

                    except OSError:
                        print("Error: could not change the font")
                        continue

                    print("Updating font to: ", fonts[misc_byte[0]])
                continue

            # Addr 0x07: write the stored text to display memory
            elif regAddress == 0x07:
                if i2c.have_recv_req():
                    text_params = bytearray(3)
                    try:
                        i2c.recv(text_params, timeout=1000)

                    except OSError:
                        print("Error: could not display stored text")
                        continue

                    print("Writing stored text to screen. x={} y={} scale={}".format(text_params[0], text_params[1], text_params[2]/10))
                    self.display.text(self.text_buffer, text_params[0], text_params[1], text_params[2]/10)
                continue

            # Addr 0x08: clear the text buffer
            elif regAddress == 0x08:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(misc_byte, timeout=100)
                        self.text_buffer = ""

                    except OSError:
                        print("Error: could not clear the text buffer")
                        continue

                    print("Clearing text buffer")
                continue

            # Addr 0x10..0x14: set/clear LEDs
            # Addr 0x18 set/clear LEDs from bitmask
            elif regAddress in [0x10, 0x11, 0x12, 0x13, 0x14, 0x18]:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(misc_byte, timeout=1000)
                    except OSError:
                        print("Error: timout receiving LED bitmask")
                        continue
                led_f = { 0x10:self.leds.user, 0x11:self.leds.golioth,
                        0x12:self.leds.internet, 0x13:self.leds.battery,
                        0x14:self.leds.power, 0x18:self.leds.process_bitmask  }

                print("Updating LED: ", regAddress, misc_byte[0])
                led_f[regAddress](misc_byte[0])
                continue

            # Addr 0x20..0x25: store string in memory
            elif regAddress in [0x20, 0x21, 0x22, 0x23, 0x24, 0x25]:
                # Handle the controller read/write request.
                if i2c.have_recv_req():
                    try:
                        data_addr = regAddress & 0xF
                        self.clear_str_memory(data_addr)
                        i2c.recv(self.str_data[data_addr], timeout=1000)

                    except OSError:
                        print("Timeout receiving string (assuming this is the end of the string)")
                        pass

                    self.display.font("serif")
                    self.display.thickness(2)
                    self.write_string(data_addr)
                continue

            # Addr 0x26: store string in memory
            elif regAddressBuff[0] == 0x26:
                # Handle the controller read/write request.
                if i2c.have_recv_req():
                    try:
                        immediate = bytearray(8)
                        i2c.recv(immediate)

                    except OSError:
                        print("Timeout receiving string (assuming this is the end of the string)")

                    print(immediate)

                    for c in immediate:
                        if c == 0x00:
                            # assume null terminator is a the end of the string
                            break
                        elif c < 0x20 or c > 0x7E:
                            self.text_buffer += ' '
                        else:
                            self.text_buffer += chr(c)
                    print("Added to text_buffer:")
                    print("\t", self.text_buffer)

                continue

            else:
                # Clear receive bytes so they don't get reprocessed as a regAddress
                print("Ignoring command on regAddress: ", regAddress)
                if i2c.have_recv_req():
                    try:
                        null_buffer = bytearray(128)
                        i2c.recv(null_buffer, timeout=1000)
                        del null_buffer
                        continue

                    except OSError:
                        continue

def main():
    o = ostentus()
    o.listen()

if __name__ == "__main__":
    main()

