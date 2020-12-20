// This table is correct for US keyboards, but needs to be modified or overlayed for UK, DE, IT, etc...
#include "keyboard.h"

#include <sys/time.h>
#include "arduino.h"


extern HardwareSerial *usbkb;

namespace fabgl {

static const VirtualKey usb_to_VK[128] = { 
  VirtualKey::VK_NONE, VirtualKey::VK_NONE, VirtualKey::VK_NONE, VirtualKey::VK_NONE,
  VirtualKey::VK_a, VirtualKey::VK_b, VirtualKey::VK_c, VirtualKey::VK_d, VirtualKey::VK_e,
  VirtualKey::VK_f, VirtualKey::VK_g, VirtualKey::VK_h, VirtualKey::VK_i, VirtualKey::VK_j,
  VirtualKey::VK_k, VirtualKey::VK_l, VirtualKey::VK_m, VirtualKey::VK_n, VirtualKey::VK_o,
  VirtualKey::VK_p, VirtualKey::VK_q, VirtualKey::VK_r, VirtualKey::VK_s, VirtualKey::VK_t,
  VirtualKey::VK_u, VirtualKey::VK_v, VirtualKey::VK_w, VirtualKey::VK_x, VirtualKey::VK_y,
  VirtualKey::VK_z, 
  VirtualKey::VK_1, VirtualKey::VK_2, VirtualKey::VK_3, VirtualKey::VK_4, VirtualKey::VK_5,
  VirtualKey::VK_6, VirtualKey::VK_7, VirtualKey::VK_8, VirtualKey::VK_9, VirtualKey::VK_0,
  VirtualKey::VK_RETURN, VirtualKey::VK_ESCAPE, VirtualKey::VK_BACKSPACE, VirtualKey::VK_TAB, VirtualKey::VK_SPACE,
  VirtualKey::VK_MINUS, VirtualKey::VK_EQUALS, VirtualKey::VK_LEFTBRACKET, VirtualKey::VK_RIGHTBRACKET, VirtualKey::VK_BACKSLASH,
  VirtualKey::VK_BACKSLASH, VirtualKey::VK_SEMICOLON, VirtualKey::VK_QUOTE, VirtualKey::VK_GRAVEACCENT,
  VirtualKey::VK_COMMA, VirtualKey::VK_PERIOD, VirtualKey::VK_SLASH, VirtualKey::VK_CAPSLOCK,
  VirtualKey::VK_F1, VirtualKey::VK_F2, VirtualKey::VK_F3, VirtualKey::VK_F4, VirtualKey::VK_F5,
  VirtualKey::VK_F6, VirtualKey::VK_F7, VirtualKey::VK_F8, VirtualKey::VK_F9, VirtualKey::VK_F10,
  VirtualKey::VK_F11, VirtualKey::VK_F12,
  VirtualKey::VK_PRINTSCREEN1, VirtualKey::VK_SCROLLLOCK, VirtualKey::VK_PAUSE,
  /* THESE ARE SPECIAL CASES 
  VirtualKey::VK_PRINTSCREEN1 --> VirtualKey::VK_PRINTSCREEN1, VirtualKey::VK_PRINTSCREEN2,
  VK_LSHIFT | VK_RSHIFT + VirtualKey::VK_PRINTSCREEN1 --> VirtualKey::VK_PRINTSCREEN2,
  VK_LCTRL  | VK_RCTRL  + VirtualKey::VK_PRINTSCREEN1 --> VirtualKey::VK_PRINTSCREEN2,
  VK_LALT   | VK_LALT   + VirtualKey::VK_PRINTSCREEN1 --> VirtualKey::VK_SYSREQ,
  VK_LCTRL  | VK_RCTRL  + VirtualKey::VK_PAUSE        --> VirtualKey::VK_BREAK,
  */
  VirtualKey::VK_INSERT, VirtualKey::VK_HOME, VirtualKey::VK_PAGEUP,
  VirtualKey::VK_DELETE, VirtualKey::VK_END, VirtualKey::VK_PAGEDOWN,
  VirtualKey::VK_RIGHT, VirtualKey::VK_LEFT, VirtualKey::VK_DOWN, VirtualKey::VK_UP,
  VirtualKey::VK_NUMLOCK, VirtualKey::VK_KP_DIVIDE, VirtualKey::VK_KP_MULTIPLY,
  VirtualKey::VK_KP_MINUS, VirtualKey::VK_KP_PLUS, VirtualKey::VK_KP_ENTER,
  VirtualKey::VK_KP_END, VirtualKey::VK_KP_DOWN, VirtualKey::VK_KP_PAGEDOWN,
  VirtualKey::VK_KP_LEFT, VirtualKey::VK_KP_CENTER, VirtualKey::VK_KP_RIGHT,
  VirtualKey::VK_KP_HOME, VirtualKey::VK_KP_UP, VirtualKey::VK_KP_PAGEUP,
  VirtualKey::VK_KP_INSERT, VirtualKey::VK_KP_DELETE,
  VirtualKey::VK_INTL_BACKSLASH, VirtualKey::VK_APPLICATION,
};


extern "C" { int time_diff_msec(struct timeval *t1, struct timeval *t2); }

struct ev {
  uint8_t vk;
  bool keyDown;
} evnt[15];

int evt = 0;
int evt_out = 0;

const VirtualKey meta[8] = { VK_LCTRL, VK_LSHIFT, VK_LALT,  VK_LGUI, VK_RCTRL, VK_RSHIFT, VK_RALT, VK_RGUI };

int Keyboard::identifyUSBKBhost(void)
{
  usbkb->write('R' + 0x80);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  usbkb->write('\n');
  vTaskDelay(100 / portTICK_PERIOD_MS);
  usbkb->write('\n');
  vTaskDelay(10 / portTICK_PERIOD_MS);
  usbkb->write('I' + 0x80);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  usbkb->write('\n');
  vTaskDelay(10 / portTICK_PERIOD_MS);

  char c = 0;
  int i = -1;

  while (usbkb->available() && c != '\n') {
    c = usbkb->read();
    if (c == 'U') i = 0;
    if (c == 'K') i++;
  }

  if (i != -1) {
    usbkb->write('\n');
    vTaskDelay(10 / portTICK_PERIOD_MS);
    usbkb->write('L' + 0x80);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    usbkb->write('G');
    vTaskDelay(10 / portTICK_PERIOD_MS);
    usbkb->write('\n');
    vTaskDelay(500 / portTICK_PERIOD_MS);
    usbkb->write('L' + 0x80);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    usbkb->write('@');
    vTaskDelay(10 / portTICK_PERIOD_MS);
    usbkb->write('\n');
  }

  return i;
}

void Keyboard::updateUSB_LEDS(void)
{
if (u_usb) {
    unsigned char a = '@' + (m_numLockLED?1:0) + (m_capsLockLED?2:0) + (m_scrollLockLED?4:0);
    // ESP_LOGW(__func__, "LEDS: %c", a);
    usbkb->write('\n');
    vTaskDelay(20 / portTICK_PERIOD_MS);
    usbkb->write('L' + 0x80);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    usbkb->write(a);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    usbkb->write('\n');
  }
}

VirtualKey Keyboard::remapUSBtoVK(VirtualKey in_vk, KeyboardLayout const * layout)
{
  VirtualKey vk = VK_NONE;
  uint8_t scancode = 0; 

  if (layout == nullptr)
    layout = m_layout;

  if (layout == &USLayout) return in_vk;

  // Find VK in the USLayout and identify SCANCODE
  for (VirtualKeyDef const * def = (&USLayout)->scancodeToVK; def->virtualKey != VK_NONE; ++def) {
    if (def->virtualKey == in_vk) {
      scancode = def->scancode;
      break;
    }
  }

  // Search down through the heirarchy for a match on SCANCODE for a replacement VK
  // stop when the first match is found 
  while (layout->inherited != nullptr && vk == VK_NONE) {

    if (scancode != 0) {
      for (VirtualKeyDef const * def = layout->scancodeToVK; def->virtualKey != VK_NONE; ++def) {
        if (def->scancode == scancode) {
          vk = def->virtualKey;
          break;
        }
      }
    }

    layout = layout->inherited;
  }

  return vk == VK_NONE ? in_vk : vk;
}

VirtualKey Keyboard::processUSB(bool * keyDown)
{

    static struct timeval t1, t2;
    int tdiff;
    static int tdelay = 500;

    VirtualKey vk = VirtualKey::VK_NONE;
    static VirtualKey last = VirtualKey::VK_NONE;
    bool keydown = false;

    if (!evt) {
      int usbKb = usbkb->available();
      if (usbKb) {
        uint8_t c = 0;
        int head = 0;
        // int len = 0;
        // int msg = 0;
        // int dev = 0;
        while (usbKb && u_cc < 10) {
          c = usbkb->read();
          u_cmd[u_cc++] = c;
          usbKb--;
        }
        if (u_cc >= 10) {
          head = u_cmd[0];
          // len = u_cmd[1] + (u_cmd[2] << 8);
          // msg = u_cmd[3];
          // dev = u_cmd[4]; 
        }
        if (c == '\n' && head == 0xFE /*&& len == 8 && msg == 4 && dev == 6*/) { // checking that we have an HID keyboard poll message

          // ESP_LOGW(__func__, "%d", u_cc);
          if (!u_usb) {
            u_usb = true;
            updateLEDs();
          }
          gettimeofday(&t1, NULL);
          tdelay = 500;

          int a = u_meta ^ u_cmd[1];
          if (a) {
            for (int i = 0; i < 8; i++) {
              if (a & (1<<i)) { 
                vk = meta[i]; 
                keydown = u_cmd[1] & (1<<i); 
                // ESP_LOGW(__func__, "VK %s %s", fabgl::Keyboard::virtualKeyToString(vk), keydown?"DOWN":"UP");
                evnt[evt].vk = vk;
                evnt[evt].keyDown = keydown;
                evt++;
              }
            // if (a & 1) { vk = VirtualKey::VK_LCTRL; keydown = u_cmd[1] & 1; }
            // if (a & 2) { vk = VirtualKey::VK_LSHIFT; keydown = u_cmd[1] & 2; }
            // if (a & 4) { vk = VirtualKey::VK_LALT; keydown = u_cmd[1] & 4; }
            // if (a & 8) { vk = VirtualKey::VK_LGUI; keydown = u_cmd[1] & 8; }
            // if (a & 0x10) { vk = VirtualKey::VK_RCTRL; keydown = u_cmd[1] & 0x10; }
            // if (a & 0x20) { vk = VirtualKey::VK_RSHIFT; keydown = u_cmd[1] & 0x20; }
            // if (a & 0x40) { vk = VirtualKey::VK_RALT; keydown = u_cmd[1] & 0x40; }
            }
            u_meta = u_cmd[1];
          }

          // ESP_LOGW(__func__, "%02x %02x %02x %02x %02x %02x", u_cmd[13], u_cmd[14], u_cmd[15], u_cmd[16], u_cmd[17], u_cmd[18]);
          // handle 6 key roll-over: ie. when another key is pressed and the first is released...
        for (int i = 0; i < 6; i++) {
          
          a = u_prev[i] ^ (u_cmd[i+3]);
          if (a) {
            if (u_prev[i] != 0) {
                // Generate an UP of this key
                // key-UP needs to work for the special keys below (VK_PRINTSCREEN1 & VK_PAUSE)
                vk = usb_to_VK[u_prev[i]];

                switch (vk) {
                  case VirtualKey::VK_PRINTSCREEN1:
                    if (u_meta & 4 || u_meta & 0x40) { vk = VirtualKey::VK_SYSREQ; break; }
                    if (u_meta & 1 || u_meta & 0x10) { vk = VirtualKey::VK_PRINTSCREEN2; break; }
                    if (u_meta & 2 || u_meta & 0x20) { vk = VirtualKey::VK_PRINTSCREEN2; break; }

                    evnt[evt].vk = VirtualKey::VK_PRINTSCREEN1;
                    evnt[evt].keyDown = false;
                    evt++;

                    vk = VirtualKey::VK_PRINTSCREEN2;
                    break;
                  case VirtualKey::VK_PAUSE:
                    if (u_meta & 1 || u_meta & 0x10) vk = VirtualKey::VK_BREAK;
                    break;
                  default: 
                    break;
                }

                evnt[evt].vk = vk;
                evnt[evt].keyDown = false;
                //VK_PAUSE does not generate an UP event
                if (evnt[evt].vk != VK_PAUSE) evt++;
            }

            bool f = false;
            for (int j = 0; j < 6; j++) {
              if (u_cmd[i+3] == u_prev[j]) { f = true; u_prev[j] = 0; break; }
            }
            if (!f) {
              //Generate a DOWN for this key
                // ESP_LOGW(__func__, "USB HID KEYCODE: 0x%02X, Meta: 0x%02X", u_cmd[i+3], u_cmd[1]);
                
                vk = usb_to_VK[u_cmd[i+3]];

                switch (vk) {
                  case VirtualKey::VK_PRINTSCREEN1:
                    if (u_meta & 4 || u_meta & 0x40) { vk = VirtualKey::VK_SYSREQ; break; }
                    if (u_meta & 1 || u_meta & 0x10) { vk = VirtualKey::VK_PRINTSCREEN2; break; }
                    if (u_meta & 2 || u_meta & 0x20) { vk = VirtualKey::VK_PRINTSCREEN2; break; }

                    evnt[evt].vk = VirtualKey::VK_PRINTSCREEN1;
                    evnt[evt].keyDown = true;
                    evt++;

                    vk = VirtualKey::VK_PRINTSCREEN2;
                    break;
                  case VirtualKey::VK_PAUSE:
                    if (u_meta & 1 || u_meta & 0x10) vk = VirtualKey::VK_BREAK;
                    break;
                  default: 
                    break;
                }

                evnt[evt].vk = vk;
                evnt[evt].keyDown = true;
                last = vk;
                evt++;
            }
          }
        }
        for (int i = 0; i < 6; i++) u_prev[i] = u_cmd[i+3];


          u_cc = 0;
        } else if (u_cc >= 10) {
          u_cc = 0;
        }
      }
      if (u_prev[0]) {
        gettimeofday(&t2, NULL);
        tdiff = time_diff_msec(&t1, &t2);
        if (tdiff > tdelay) {
          // ESP_LOGW(__func__, "KEY REPEAT %s", fabgl::Keyboard::virtualKeyToString(last));
          evnt[evt].vk = last;
          evnt[evt].keyDown = true;
          evt++;

          t1 = t2;
          tdelay = 92; // implement autorepeat based on a time interval of 91 ms
        }
      } 
    }

    if (evt) {

      vk = (VirtualKey) evnt[evt_out].vk;
      keydown = evnt[evt_out].keyDown;
      evt_out++;
      evt--;
      if (!evt) evt_out = 0;

      // vk = postProcessVK(vk, keydown, keyDown);
      vk = postProcessVK(remapUSBtoVK(vk), keydown, keyDown);
    }

    return vk;
}

} //namespace