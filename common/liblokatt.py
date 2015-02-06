from ctypes import c_char, c_char_p, c_int32, c_uint8, CFUNCTYPE
import ctypes
import os


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

    '''
    Class that represents a logcat session
    A logcat session can have several open channels that will receive callbacks
    for new messages.
    '''

    def __init__(self):
        self.buffer_size = 1024 * 1024
        self.session = liblokatt.create_lokatt_session(self.buffer_size)
        self._is_open = False

    def start(self):
        if (self._is_open):
            raise ValueError('Logcat session is already started')
        liblokatt.start_lokatt_session(self.session)
        self.open = True

    def stop(self):
        if (not self._is_open):
            raise ValueError('Logcat session is not started')
        liblokatt.destroy_lokatt_session(self.session)
        self._is_open = False

    def create_channel(self, message_callback=_empty_message_handler):
        if (not self.is_open):
            raise ValueError('Logcat session is not started')
        # TODO: add filter arguments when it is supported in liblokatt.so
        return LogcatChannel(self.session, message_callback)


class LogcatChannel():

    '''
    A channel for receiving logcat messages. The channel is connected to a
    logcat session.
    '''

    def __init__(self, session, message_callback=_empty_message_handler):
        self.session = session
        self.callback = self._get_callback_function(message_callback)

    def _get_callback_function(self, message_callback):
        # Wrap the message_callback in a ctypes function
        # The caller need to keep a reference to the returned type to keep it
        # from being garbage collected.
        def callback(m):
            # This closure function is needed to get a function without
            # a self argument
            message_callback(m[0])

        CBFUNC = CFUNCTYPE(None, ctypes.POINTER(Message))
        return CBFUNC(callback);

    def open(self):
        '''
        Open this channel. After open is called the message_handler will start
        to receive messages on a dedicated thread.
        '''
        self.handle = liblokatt.open_new_channel(self.session, self.callback)

    def close(self):
        '''
        Close the channel. This will cause the message_handler to stop receiving
        messages.
        '''
        liblokatt.close_async_channel(self.handle)
