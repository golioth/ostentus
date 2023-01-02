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
        x_loc = 0
        y_loc = 0

    def clear_all_memory(self, clear_display=False):
        self.str_data = [bytearray(b'\x00'*32)]*6
        x_loc = 0
        y_loc = 0

        self.display.pen(15)
        self.display.clear()

        if clear_display:
            self.display.update_speed(0x00)
            self.display.update()

    def clear_str_memory(self, line_idx):
        self.str_data[line_idx] = bytearray(b'\x00'*32)

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
        self.display.text(outstring, 0, 10+(22*line_idx), 0.7)

    def listen(self):

        i2c = i2cperipheral.I2CPeripheral(bus=self.bus, sclPin=self.sclPin,
                sdaPin=self.sdaPin, address=self.address)
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
                        pass

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
                        pass

                    self.write_string(data_addr)

            else:
                continue

def main():
    o = ostentus()
    o.listen()

if __name__ == "__main__":
    main()
