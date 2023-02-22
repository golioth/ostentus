import machine
import badger2040
import ostentus_icons
from micropython import const
import cap_touch as touch

class SlideshowSettings:
    def __init__(self, slideshow_delay=None, label_y=None, value_y=None):
        # Set defaults as necessary
        if slideshow_delay is None:
            slideshow_delay = 30000
        self.slideshow_delay_ms = slideshow_delay
        if label_y is None:
            label_y = 50
        self.label_y = label_y
        if value_y is None:
            value_y = 130
        self.value_y = value_y

        self.pages = list()
        self.page_tracker = 0
        self.slideshow_tim = machine.Timer()
        self.partial_update_tim = machine.Timer()

        self.d = badger2040.Badger2040()

sset = SlideshowSettings()

class Icons:
    global sprite, sset
    SPRITE_WIDTH = 192
    ICON_WIDTH = 64
    NAMES = {
        "humidity":0,
        "temperature":1,
        "pressure":2
        }

    sprite = bytearray(ostentus_icons.data())

    def get_id_from_name(self, name):
        if name.lower() in self.NAMES:
            return self.NAMES[name.lower()]
        return None

    def write(self, id):
        if id >= len(self.NAMES):
            return
        sset.d.icon(sprite, id, self.SPRITE_WIDTH, self.ICON_WIDTH, 68, 10)

icons=Icons()

_button_left = const(0x01)
_button_right = const(0x04)
_button_both = const(_button_left | _button_right)

class slide_page:
    def __init__(self, s_id, label):
        self.s_id = s_id
        self.label = label
        self.value = "  ---  "

    def get_label(self):
        return self.label

    def get_value(self):
        return self.value

    def set(self, value):
        self.value = value

    def update_label(self, label):
        self.label = label

def add(s_id, label):
    global sset
    for p in sset.pages:
        if p.s_id == s_id:
            sset.pages.remove(p)
    sset.pages.append(slide_page(s_id, label))
    sset.page_tracker = 0

def set_value_by_id(s_id, value):
    global sset
    for p in sset.pages:
        if p.s_id == s_id:
            p.value = value

def timer_start():
    global sset
    sset.slideshow_tim.init( \
            mode=machine.Timer.PERIODIC, \
            period=sset.slideshow_delay_ms, \
            callback=inc_and_show \
            )

def timer_stop():
    global sset
    sset.slideshow_tim.deinit()

def start(delay_ms):
    global sset
    if len(sset.pages) < 1:
        return
    if (delay_ms >= 6000):
        sset.slideshow_delay_ms = delay_ms
    timer_start()
    touch.register_callback_left(inc_and_show)
    touch.register_callback_right(dec_and_show)
    touch.start()
    show_page()

def stop():
    touch.stop()
    timer_stop()

def show_label(label):
    global sset, icons

    sset.d.pen(15)

    id = icons.get_id_from_name(label)
    print("Label:", label, "ID:", id)
    if id != None:
        icons.write(id)
    else:
        sset.d.font("sans")
        scale = 1
        sset.d.thickness(3)
        sset.d.text(label, 0, sset.label_y, scale)


def page_tracker_inc():
    global sset
    sset.page_tracker += 1
    if sset.page_tracker >= len(sset.pages):
        sset.page_tracker = 0

def page_tracker_dec():
    global sset
    sset.page_tracker -= 1
    if sset.page_tracker < 0:
        sset.page_tracker = len(sset.pages)-1

def dec_and_show(t=None):
    if not t:
        timer_stop()
        timer_start()
    page_tracker_inc()
    show_page()

def inc_and_show(t=None):
    if not t:
        timer_stop()
        timer_start()
    page_tracker_inc()
    show_page()

def show_page():
    global sset
    if len(sset.pages) < 1:
        return
    p = sset.pages[sset.page_tracker]
    sset.d.pen(0)
    sset.d.clear()
    show_label(p.label)
    fit_text(p.value)
    sset.d.update()

def fit_text(text):
    global sset
    sset.d.font("sans")
    scale = 3
    pixels = 0
    while True:
        pixels = sset.d.measure_text(text, scale)
        if pixels > 200:
            scale -= 0.1
        else:
            break
    sset.d.pen(15)
    sset.d.thickness(int(3*scale))
    sset.d.text(text, int((200-pixels)/2), sset.value_y, scale)

