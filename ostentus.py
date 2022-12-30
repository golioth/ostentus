import i2cperipheral
import badger2040

def listen():
    display=badger2040.Badger2040()
    display.font("serif")                                                                                                                                                     
    display.thickness(2)

    i2c = i2cperipheral.I2CPeripheral(bus=0, sclPin=5, sdaPin=4, address=0x12)
    data = list()
    for i in range(7):
        data.append(bytearray(b'\x00'*32))

    print("Listening...")
    while True:
        regAddressBuff = bytearray(1)

        # First thing master should send is register address.
        # Poll to see if it has been received yet.
        if not i2c.have_recv_req():
            continue
        i2c.recv(regAddressBuff, timeout=0)

        # Wait for master to send either the read or write.
        while (not i2c.have_recv_req()) and (not i2c.have_send_req()):
            pass

        # Only support read/write requests for register 0x01.
        regAddress = regAddressBuff[0]
        if regAddress == 0x00:
            display.pen(15)
            display.clear()
        if regAddress == 0x01:
            display.pen(15)
            display.clear()
            display.pen(0)

            for i in range(len(data)):
                outstring=""
                for c in data[i]:
                    if c < 0x20 or c > 0x7E:
                        outstring += ' '
                    else:
                        outstring += chr(c)
                display.text(outstring, 0, 10+(22*i), 0.7)
            display.update()
            #else:
            #i2c.send(data, timeout=1000)
        elif regAddress in [0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26]:
            # Handle the master read/write request.
            if i2c.have_recv_req():
                try:
                    data_addr = regAddress & 0xF
                    data[data_addr] = bytearray(b'\x00'*32)
                    i2c.recv(data[data_addr], timeout=1000)
                except OSError:
                    pass
        else:
            continue

listen()

