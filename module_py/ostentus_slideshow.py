import machine
import badger2040
import ostentus_icons
from micropython import const
import cap_touch as touch

_label_y = 50
_value_y = 130

_slideshow_delay_ms = 30000

class Icons:
    global sprite, d
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
        d.icon(sprite, id, self.SPRITE_WIDTH, self.ICON_WIDTH, 68, 10)

d=badger2040.Badger2040()
icons=Icons()

_pages = list()
_page_tracker = 0
_slideshow_tim = machine.Timer()

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
    global _pages
    global _page_tracker
    for p in _pages:
        if p.s_id == s_id:
            _pages.remove(p)
    _pages.append(slide_page(s_id, label))
    _page_tracker = 0

def set_value_by_id(s_id, value):
    global _pages
    for p in _pages:
        if p.s_id == s_id:
            p.value = value

def timer_start():
    _slideshow_tim.init( \
            mode=machine.Timer.PERIODIC, \
            period=_slideshow_delay_ms, \
            callback=inc_and_show \
            )

def timer_stop():
    _slideshow_tim.deinit()

def start(delay_ms):
    global _pages, _slideshow_delay_ms
    if len(_pages) < 1:
        return
    if (delay_ms >= 6000):
        _slideshow_delay_ms = delay_ms
    timer_start()
    touch.register_callback_left(inc_and_show)
    touch.register_callback_right(dec_and_show)
    touch.start()
    show_page()

def stop():
    touch.stop()
    timer_stop()

def show_label(label):
    global _label_y, d, icons

    d.pen(15)

    id = icons.get_id_from_name(label)
    print("Label:", label, "ID:", id)
    if id != None:
        icons.write(id)
    else:
        d.font("sans")
        scale = 1
        d.thickness(3)
        d.text(label, 0, _label_y, scale)


def page_tracker_inc():
    global _page_tracker, _pages
    _page_tracker += 1
    if _page_tracker >= len(_pages):
        _page_tracker = 0

def page_tracker_dec():
    global _page_tracker, _pages
    _page_tracker -= 1
    if _page_tracker < 0:
        _page_tracker = len(_pages)-1

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
    global _pages, _page_tracker, d
    if len(_pages) < 1:
        return
    p = _pages[_page_tracker]
    d.pen(0)
    d.clear()
    show_label(p.label)
    fit_text(p.value)
    d.update()

def fit_text(text):
    global _value_y, d
    d.font("sans")
    scale = 3
    pixels = 0
    while True:
        pixels = d.measure_text(text, scale)
        if pixels > 200:
            scale -= 0.1
        else:
            break
    d.pen(15)
    d.thickness(int(3*scale))
    d.text(text, int((200-pixels)/2), _value_y, scale)
    d.text(text, int((200-pixels)/2), _value_y, scale)

