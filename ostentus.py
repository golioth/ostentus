import ostentus_i2c
import badger2040
import splashscreen_rd
from ostentus_leds import o_leds
import ostentus_slideshow as slideshow


class ostentus:
    def __init__(self, bus=0, sclPin=5, sdaPin=4, address=0x12):
        #FIXME: Add pins and address to initialization
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

        self.init()
        self.leds = o_leds()

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

    def print_param_count_err(self, cmd, expected_cnt, actual_cnt):
        print("Error: cmd {} expects data length {} but got {}".format(cmd, expected_cnt, actual_cnt))

    def init(self):
        ostentus_i2c.init()

    def listen(self):
        self.clear_all_memory()
        print("Listening...")
        while True:
            if ostentus_i2c.has_data():

                data = ostentus_i2c.pop()
                if len(data) < 2:
                    continue
                regAddress = data[0]
                dataLen = data[1]
                dataStart = 2
                if len(data) != dataLen+2:
                    print("Error, unexpected packet length: ", len(data))
                    print(data)
                    continue

                # Addr 0x00: clear memory
                if regAddress == 0x00:
                    self.clear_all_memory(0)
                    continue

                # Addr 0x01: refresh display
                elif regAddress == 0x01:
                    slideshow.stop()
                    self.display.update_speed(0)
                    self.display.update()
                    continue

                # Addr 0x02..0x03: change x_loc or y_loc offsets
                elif regAddress in [0x02, 0x03]:
                    if dataLen != 1:
                        self.print_param_count_err(regAddress, 1, dataLen)
                        continue
                    if regAddress == 0x02:
                        if 0x00 <= data[dataStart] < badger2040.WIDTH:
                            self.x_loc = data[dataStart]
                        else:
                            print("Received x index out of bounds:", loc_value)
                    if regAddress == 0x03:
                        if 0x00 <= data[dataStart] < badger2040.HEIGHT:
                            self.y_loc = data[dataStart]
                        else:
                            print("Received y index out of bounds:", loc_value)
                    continue

                # Addr 0x04: show Golioth splashscreen
                elif regAddress == 0x04:
                    slideshow.stop()
                    self.show_splash()
                    print("Showing splashscreen")
                    continue

                # Addr 0x05: change the pen thickness
                elif regAddress == 0x05:
                    if dataLen != 1:
                        self.print_param_count_err(regAddress, 1, dataLen)
                        continue
                    slideshow.stop()
                    self.display.thickness(data[dataStart])
                    print("Changing pen thickness to: ", data[dataStart])
                    continue

                # Addr 0x06: change the font
                elif regAddress == 0x06:
                    if dataLen != 1:
                        self.print_param_count_err(regAddress, 1, dataLen)
                        continue
                    slideshow.stop()
                    fonts = ["sans", "gothic" "cursive" "serif" "serif_italic"]
                    self.display.font(fonts[data[dataStart]])
                    print("Updating font to: ", data[dataStart])
                    continue

                # Addr 0x07: write the stored text to display memory
                elif regAddress == 0x07:
                    if dataLen != 3:
                        self.print_param_count_err(regAddress, 3, dataLen)
                        continue
                    print("Writing stored text to screen. x={} y={} scale={}".format(data[dataStart], data[dataStart+1], data[dataStart+2]/10))
                    slideshow.stop()
                    self.display.text(self.text_buffer, data[dataStart], data[dataStart+1], data[dataStart+2]/10)
                    continue

                # Addr 0x08: clear the text buffer
                elif regAddress == 0x08:
                    self.text_buffer = ""
                    print("Clearing text buffer")
                    continue

                # Addr 0x09: clear a rectangle bounded by x, y, w, h
                elif regAddress == 0x09:
                    if dataLen != 4:
                        self.print_param_count_err(regAddress, 4, dataLen)
                        continue
                    print("Clearing rectangle")
                    slideshow.stop()
                    self.display.pen(0)
                    self.display.rectangle(data[dataStart], data[dataStart+1], data[dataStart+2], data[dataStart+3])
                    self.display.pen(15)
                    continue

                # Addr 0x0A: Register a slideshow entry
                elif regAddress == 0x0A:
                    if dataLen < 2:
                        self.print_param_count_err(regAddress, 2, dataLen)
                        continue
                    slideshow_id = data[dataStart]
                    slideshow_label = data[dataStart+1:dataStart+dataLen]
                    print("Adding new slideshow page:", slideshow_id, slideshow_label)
                    slideshow.add(slideshow_id, slideshow_label)
                    continue

                # Addr 0x0B: Update the values of slideshow entries by id
                elif regAddress == 0x0B:
                    if dataLen < 2:
                        self.print_param_count_err(regAddress, 2, dataLen)
                        continue
                    slideshow_id = data[dataStart]
                    slideshow_value = data[dataStart+1:dataStart+dataLen]
                    print("Set value using id:", slideshow_id, slideshow_value)
                    slideshow.set_value_by_id(slideshow_id, slideshow_value)
                    continue

                # Addr 0x0C: Start/stop slideshow
                elif regAddress == 0x0C:
                    if dataLen != 1:
                        self.print_param_count_err(regAddress, 1, dataLen)
                        continue
                    if data[dataStart]:
                        print("Starting slideshow")
                        slideshow.start(6000)
                    else:
                        print("Ending slideshow")
                        slideshow.stop()
                    continue

                # Addr 0x10..0x14: set/clear LEDs
                # Addr 0x18 set/clear LEDs from bitmask
                elif regAddress in [0x10, 0x11, 0x12, 0x13, 0x14, 0x18]:
                    if dataLen != 1:
                        self.print_param_count_err(regAddress, 1, dataLen)
                        continue
                    led_f = { 0x10:self.leds.user, 0x11:self.leds.golioth,
                            0x12:self.leds.internet, 0x13:self.leds.battery,
                            0x14:self.leds.power, 0x18:self.leds.process_bitmask  }

                    print("Updating LED: ", regAddress, data[dataStart])
                    led_f[regAddress](data[dataStart])
                    continue

                # Addr 0x26: store string in memory
                elif regAddress == 0x26:
                    if dataLen == 0:
                        self.print_param_count_err(regAddress, ">0", dataLen)
                        continue

                    for c in data[dataStart:dataStart+dataLen]:
                        if c == 0x00:
                            # assume null terminator is at the end of the string
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

'''
def main():
    o = ostentus()
    o.listen()

if __name__ == "__main__":
    main()
'''
