import machine
import badger2040
import ostentus_icons
import ostentus_leds
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
        self.full_update_pending = False
        self.last_shown_value = None

        self.summary_flag = False
        self.summary_last_displayed = None
        self.summary_title = "Golioth"
        self.summary_y = (26, 86, 146) #Y coord for each of 3 summary blocks
        self.summary_p_update_count = 0
        self.summary_full_update_after_X_partials = 10
        self.summary_update_tim = machine.Timer()

        self.touch_left_pending = False
        self.touch_right_pending = False
        self.touch_up_pending = False

        self.d = badger2040.Badger2040()
        self.leds = ostentus_leds.o_leds()

    def summary_remember_last_values(self):
        self.summary_last_displayed = [x.value for x in self.pages[:3]]

    def summary_values_are_new(self):
        return self.summary_last_displayed == [x.value for x in self.pages[:3]]

    def get_page_count(self):
        return len(self.pages)

    def get_page_value(self):
        return self.pages[self.page_tracker].value

    def summary_flag_get(self):
        return self.summary_flag

    def summary_flag_set(self, state):
        self.summary_flag = True if state else False
        print("Set summary_flag to:", self.summary_flag_get())

    def clear_flags(self):
        self.full_update_pending = False
        self.touch_right_pending = False
        self.touch_left_pending = False
        self.touch_up_pending = False


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

    def get_page_value(self):
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
            callback=full_update_flag_set \
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
    touch.register_callback_up(touch_up)
    touch.register_callback_left(touch_left)
    touch.register_callback_right(touch_right)
    touch.start()
    show_page()

def touch_left(t=None):
    global sset
    sset.touch_left_pending = True
    sset.leds.user(1)

def touch_right(t=None):
    global sset
    sset.touch_right_pending = True
    sset.leds.user(1)

def touch_up(t=None):
    global sset
    sset.touch_up_pending = True
    sset.leds.user(1)

def stop():
    global sset
    touch.stop()
    timer_stop()
    sset.full_update_pending = False

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

def full_update_flag_set(t=None):
    global sset
    sset.full_update_pending = True

def stop_summary():
    global sset
    sset.summary_flag_set(False)
    sset.summary_update_tim.deinit()

def show_value_partial_update():
    global sset
    sset.d.pen(0)
    sset.d.rectangle(0, sset.value_y-50, 200, 100)
    fit_text(sset.get_page_value())
    sset.d.partial_update(0, sset.value_y-50, 200, 100)

def service_slideshow():
    global sset
    #These statements are in order by priority
    if sset.touch_right_pending:
        stop_summary()
        inc_and_show()
        sset.leds.user(0)
        sset.clear_flags()
    elif sset.touch_left_pending:
        stop_summary()
        dec_and_show()
        sset.leds.user(0)
        sset.clear_flags()
    elif sset.touch_up_pending:
        #Toggle the summary view
        if (sset.summary_flag_get()):
            stop_summary()
            inc_and_show()
            pass
        else:
            sset.summary_flag_set(True)
            summary()
        sset.leds.user(0)
        sset.clear_flags()
    elif sset.summary_flag_get():
        if sset.summary_values_are_new():
            #Use a timer for the update so if we have multiple value updates at
            #there is only one refrash made
            sset.summary_update_tim.deinit()
            sset.summary_update_tim.init( \
                    mode = machine.Timer.ONE_SHOT, \
                    period = 250, \
                    callback = summary_timed_update \
                    )
    elif sset.full_update_pending:
        inc_and_show()
        sset.clear_flags()
    elif sset.last_shown_value is not None:
        if sset.get_page_value() is not sset.last_shown_value:
            sset.last_shown_value = sset.get_page_value()
            show_value_partial_update()

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
    sset.last_shown_value = p.value
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

def summary_title_set(new_title):
    global sset
    sset.summary_title = new_title

def summary(full_update=True):
    global sset
    top_text = "Golioth"
    d_width = 200
    page_cnt = sset.get_page_count()

    if page_cnt < 1:
        return;

    if full_update:
        sset.d.pen(0)
        sset.d.clear()
        sset.d.font("sans")
        sset.d.thickness(2)
        sset.d.pen(15)
        sset.d.rectangle(0,0,d_width,24)
        sset.d.pen(0)
        tt_offset = int((d_width - sset.d.measure_text(sset.summary_title, 0.9)) / 2)
        sset.d.text(sset.summary_title, tt_offset, 12, 0.9)

    if full_update:
        for i in range(3):
            if i >= page_cnt:
                break;

            sset.summary_p_update_count = 0

            summary_block_data( \
                    sset.pages[i].label, \
                    sset.pages[i].value, \
                    0, \
                    sset.summary_y[i])
        sset.d.update()
        sset.summary_remember_last_values()

    else:
        for i in range(3):
            if i >= page_cnt:
                break;

            sset.summary_p_update_count += 1

            summary_block_update_value( \
                    sset.pages[i].value, \
                    0, \
                    sset.summary_y[i])
        sset.d.partial_update_execute()
        sset.summary_remember_last_values()

def summary_partial_update():
    summary(False)

def summary_timed_update(t=None):
    global sset
    summary(sset.summary_p_update_count >= sset.summary_full_update_after_X_partials)

def summary_block_data(label, value, x, y):
    """Write a label and value to ePaper memory

    Write a smaller label text with x,y as the upper left corner, with a larger
    value text below it. This function does not call the screen update command.
    """

    global sset
    sset.d.pen(15)
    sset.d.font("sans")
    sset.d.thickness(2)
    sset.d.text(label, x, y+14, 0.7)
    sset.d.thickness(3)
    sset.d.text(value, x, y+42, 1)

def summary_block_update_value(value, x, y):
    """Write a new value to ePaper memory in preparation for a partial update

    Clearn the value in ePaper memory and write a new value. The value is
    written at the offset established with the summary_block() method. This
    method does not call the display update command and should be used to pepare
    the display for a partial update. If several values are being updated, they
    all must be rewritten to memory using this command between each partial
    update command.
    """

    global sset
    # Design values
    y_offset = 28
    height = 38
    value_offset = 42
    screen_height = 200
    screen_width = 200

    # Calculated values (NO EDITS!)
    y_start = y + y_offset
    if y_start + height > screen_height:
        height = screen_height-(y_start)
    value_start = y + value_offset

    #Partial update y,h must be multiples of 8
    partial_start = y_start - (y_start % 8)
    partial_height = height + (y_start - partial_start)
    partial_height = height + (8 - (height % 8))

    sset.d.pen(0)
    sset.d.rectangle(0, y_start, screen_width, height)
    sset.d.pen(15)
    sset.d.text(value, x, value_start, 1)
    sset.d.partial_update_data(0, partial_start, screen_width, partial_height)
