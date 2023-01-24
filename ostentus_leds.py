from machine import Pin
from utime import sleep_ms

class o_leds:
    def __init__(self, power=15, battery=14, internet=13, golioth=12, user=11):
        self.power_icon = Pin(power, Pin.OUT, value=1)
        self.battery_icon = Pin(battery, Pin.OUT, value=1)
        self.internet_icon = Pin(internet, Pin.OUT, value=1)
        self.golioth_icon = Pin(golioth, Pin.OUT, value=1)
        self.user_icon = Pin(user, Pin.OUT, value=1)

    def power(self, state):
        if state == 0:
            self.power_icon.value(1)
        else:
            self.power_icon.value(0)

    def battery(self, state):
        if state == 0:
            self.battery_icon.value(1)
        else:
            self.battery_icon.value(0)

    def internet(self, state):
        if state == 0:
            self.internet_icon.value(1)
        else:
            self.internet_icon.value(0)

    def golioth(self, state):
        if state == 0:
            self.golioth_icon.value(1)
        else:
            self.golioth_icon.value(0)

    def user(self, state):
        if state == 0:
            self.user_icon.value(0)
        else:
            self.user_icon.value(1)

    def process_bitmask(self, bitmask):
        if (bitmask & 1):
            self.user_icon.value(0)
        else:
            self.user_icon.value
            (1)
        if (bitmask & 2):
            self.golioth_icon.value(0)
        else:
            self.golioth_icon.value(1)
        if (bitmask & 4):
            self.internet_icon.value(0)
        else:
            self.internet_icon.value(1)
        if (bitmask & 8):
            self.battery_icon.value(0)
        else:
            self.battery_icon.value(1)
        if (bitmask & 16):
            self.power_icon.value(0)
        else:
            self.power_icon.value(1)

    def set_all(self, state):
        if state == 0:
            new_value = 1
        else:
            new_value = 0
        self.power_icon.value(new_value)
        self.battery_icon.value(new_value)
        self.internet_icon.value(new_value)
        self.golioth_icon.value(new_value)
        self.user_icon.value(new_value)

    def boot_animation(self, delay=200):
        self.set_all(0)
        state = 0
        while state < 2:
            self.internet_icon.value(state)
            sleep_ms(delay)
            self.battery_icon.value(state)
            self.golioth_icon.value(state)
            sleep_ms(delay)
            self.power_icon.value(state)
            self.user_icon.value(state)
            sleep_ms(delay)
            state += 1
