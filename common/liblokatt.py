import ctypes
import inspect
import os

from ctypes import c_char, c_char_p, c_int32, c_uint8


class Message(ctypes.Structure):
    #     struct lokatt_message {
    #     int32_t pid;
    #     int32_t tid;
    #     int32_t sec;
    #     int32_t nsec;
    #     uint8_t level;
    #     const char *tag;
    #     const char *text;
    #     char pname[128];
    #     char msg[MSG_MAX_PAYLOAD_SIZE];
    # };
    _fields_ = [('pid', c_int32),
                ('tid', c_int32),
                ('sec', c_int32),
                ('nsec', c_int32),
                ('level', c_uint8),
                ('tag', c_char_p),
                ('text', c_char_p),
                ('pname', c_char * 128),
                ('msg', c_char * (4 * 1024))]

    def __str__(self):
        return "pname={0.pname} pid={0.pid} tid={0.tid} sec={0.sec} nsec={0.nsec} level={0.level} tag={0.tag} text={0.text}".format(self)


def get_lib_path():
    # path to logcat.py
    path = __file__
    # path to common
    path = os.path.abspath(os.path.split(path)[0])
    # realpath() will prevent symlink problems
    return os.path.realpath(os.path.join(path, 'lib/liblokatt.so'))

liblokatt = ctypes.CDLL(get_lib_path())


def _empty_message_handler(m: Message):
    '''
    Do nothin message handler for Logcat
    '''


class Logcat():

    def __init__(self):
        self.buffer_size = 1024 * 1024
        self.session_handle = liblokatt.create_lokatt_session(self.buffer_size)
        self.message_handler = _empty_message_handler

    def start(self):
        m = Message()
        liblokatt.start_lokatt_session(self.session_handle)
        self.channel_handle = liblokatt.create_lokatt_channel(
            self.session_handle)
        while liblokatt.read_lokatt_channel(self.channel_handle, ctypes.addressof(m)) == 0:
            self.message_handler(m)

    def stop(self):
        liblokatt.destroy_lokatt_channel(self.channel_handle)
        liblokatt.destroy_lokatt_session(self.session_handle)
