import machine
from machine import Pin, I2C
from micropython import const

cap1203_sda = 2
cap1203_scl = 3
cap1203_bus = 1
cap1203_int_pin = 7
i2c_addr = 0x28

# Workaround for PCB errors
# (These pins are jumpered to 2/3)
p8 = Pin(8, Pin.IN, pull=None)
p9 = Pin(9, Pin.IN, pull=None)

i2c = I2C(id=cap1203_bus, sda=Pin(cap1203_sda), scl=Pin(cap1203_scl))
touch_int = Pin(cap1203_int_pin, Pin.IN, Pin.PULL_UP)

DEBOUNCE_MS = const(300)
debounce_tim = machine.Timer()

touch_callback_left = None
touch_callback_right = None
touch_callback_up = None

def i2c_get_state():
    try:
        return i2c.readfrom_mem(0x28, 0x03, 0x01)
    except Exception as e:
        print("Cap touch i2c Error:", e)
        return 0x00

def i2c_clear_int(t=None):
    try:
        i2c.writeto_mem(i2c_addr, 0x00, b'\x00')
    except Exception as e:
        print("Cap touch i2c Error:", e)

def register_callback_left(method_to_run):
    global touch_callback_left
    if is_valid_callback(method_to_run):
        touch_callback_left = method_to_run

def register_callback_right(method_to_run):
    global touch_callback_right
    if is_valid_callback(method_to_run):
        touch_callback_right = method_to_run

def register_callback_up(method_to_run):
    global touch_callback_up
    if is_valid_callback(method_to_run):
        touch_callback_up = method_to_run

def is_valid_callback(method_to_run):
    if not callable(method_to_run):
        Print("cap_touch: Error registering callback (not callable)")
        return False
    return True

def irq_handler(pin):
    global touch_callback_left, touch_callback_right, touch_callback_up
    state = i2c_get_state()
    debounce_tim.init(mode=machine.Timer.ONE_SHOT, period=DEBOUNCE_MS, callback=i2c_clear_int)

    # FIXME: these are blocking calls (from an irq)
    if state[0] == 0x01 and touch_callback_left:
        touch_callback_left()
    if state[0] == 0x02 and touch_callback_up:
        touch_callback_up()
    if state[0] == 0x04 and touch_callback_right:
        touch_callback_right()

def start():
    global touch_callback
    if not touch_callback_left and not touch_callback_up and not touch_callback_right:
        print("cap_touch: Error starting, please register a callback first")
        return
    i2c_clear_int()
    touch_int.irq(handler=irq_handler, trigger=Pin.IRQ_FALLING)

def stop():
    touch_int.irq(handler=None, trigger=0)
    i2c_clear_int()

