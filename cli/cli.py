import datetime

from common.liblokatt import Message
from common.view import View
from colorama import Fore

# This matches the order from android_LogPriority
# in platform/system/core/include/android/log.h
COLORS = [Fore.RESET,   # Unknown
          Fore.RESET,   # Default
          Fore.RESET,   # Verbose
          Fore.BLUE,    # Debug
          Fore.GREEN,   # Info
          Fore.YELLOW,  # Warning
          Fore.RED,     # Error
          Fore.RED,     # Fatal
          Fore.RESET]   # Silent

def print_message(m:Message):
    time = datetime.datetime.utcfromtimestamp(m.sec)
    # Fix the with of pname to 24.
    # Add trailing spaces or remove abundant chars from the left
    pname = str(m.pname, "utf-8").ljust(24, ' ')[-24:]
    tag = str(m.tag, 'utf-8')
    text = str(m.text, 'utf-8')

    # Colors in a separate block to be able to add logic to disable colors
    pname = Fore.YELLOW + pname + Fore.RESET
    tag = COLORS[m.level] + tag
    text = text + Fore.RESET
    print("{:%m-%d %H:%M:%S} {:5} {:5} {} {}: {}".format(time, m.pid, m.tid, pname, tag, text))

class Cli(View):

    '''
    Command Line Interface implementation of View
    '''

    def __init__(self, logcat):
        View.__init__(self, logcat)

    def show(self):
        # We don't need to create any ui components just start the logcat and
        # wait for the messages to arrive to on_message
        self.logcat.start()
        self.logcat.stop()

    def on_message(self, m: Message):
        print_message(m)
