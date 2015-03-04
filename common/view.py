from abc import ABCMeta, abstractmethod
from common.liblokatt import Message


class View(metaclass=ABCMeta):

    '''
    Base class for views. This is the main interface
    to report events to the UI layer
    '''

    @abstractmethod
    def __init__(self, logcat):
        self.logcat = logcat
        logcat.message_handler = self.on_message

    @abstractmethod
    def show(self):
        '''
        Show the UI to the user.
        '''
        pass

    @abstractmethod
    def on_message(self, m: Message):
        '''
        Callback that is called for all new logcat messages
        :param m:Message the new message
        '''
