from common.view import View
from common.liblokatt import Message


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

    def on_message(self, m:Message):
        print(m)
