import machine
import badger2040

_label_y = 50
_value_y = 130

d=badger2040.Badger2040()

_pages = list()
_page_tracker = 0
_slideshow_tim = machine.Timer()

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

def start(delay_ms):
    global _pages
    if len(_pages) < 1:
        return
    if (delay_ms < 6000):
        delay_ms = 6000
    _slideshow_tim.init(mode=machine.Timer.PERIODIC, period=delay_ms, callback=show_page)
    show_page(None)

def stop():
    _slideshow_tim.deinit()

def show_label(label):
    global _label_y
    d.font("sans")
    scale = 1
    d.thickness(3)
    d.pen(15)
    d.text(label, 0, _label_y, scale)

def show_page(t):
    global _pages
    global _page_tracker
    if len(_pages) < 1:
        return
    p = _pages[_page_tracker]
    d.pen(0)
    d.clear()
    show_label(p.label)
    fit_text(p.value)
    _page_tracker += 1
    if _page_tracker >= len(_pages):
        _page_tracker = 0
    d.update()

def fit_text(text):
    global _value_y
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
