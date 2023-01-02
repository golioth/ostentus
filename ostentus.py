import i2cperipheral
import badger2040

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
        self.x_loc = 0
        self.y_loc = 0

    def clear_all_memory(self, clear_display=False):
        self.str_data = [bytearray(b'\x00'*32)]*6
        self.x_loc = 0
        self.y_loc = 0

        self.display.pen(15)
        self.display.clear()

        if clear_display:
            self.display.update_speed(0x00)
            self.display.update()

    def clear_str_memory(self, line_idx):
        self.str_data[line_idx] = bytearray(b'\x00'*32)

    def show_splash(self):
        splash = bytearray(int(296 * 128 / 8))
        open("splashscreen_rd.bin", 'r').readinto(splash)
        self.display.image(splash)
        self.display.update()

    def write_string(self, line_idx):
        self.display.pen(0)
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

            # Addr 0x01: refresh display
            if regAddress == 0x01:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(refresh_byte, timeout=1000)

                        if 0x00 <= refresh_byte[0] <= 0x03:
                            self.display.update_speed(refresh_byte[0])
                            self.display.update()

                    except OSError:
                        print("Error: timout receiving display refresh command")
                        pass

            # Addr 0x02..0x03: change x_loc or y_loc offsets
            if regAddress in [0x02, 0x03]:
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

            # Addr 0x04: show Golioth splashscreen
            if regAddress == 0x04:
                if i2c.have_recv_req():
                    try:
                        i2c.recv(misc_byte, timeout=1000)
                        self.show_splash()

                    except OSError:
                        print("Error: could not display the splashscreen")
                        pass

            # Addr 0x020..0x26: store string in memory
            elif regAddress in [0x20, 0x21, 0x22, 0x23, 0x24, 0x25]:
                # Handle the controller read/write request.
                if i2c.have_recv_req():
                    try:
                        data_addr = regAddress & 0xF
                        self.clear_str_memory(data_addr)
                        i2c.recv(self.str_data[data_addr], timeout=1000)

                    except OSError:
                        print("Timout receiving string (assuming this is the end of the string)")
                        pass

                    self.write_string(data_addr)

            else:
                continue

def main():
    o = ostentus()
    o.listen()

if __name__ == "__main__":
    main()
