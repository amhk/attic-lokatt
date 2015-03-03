from common.view import View
from common.liblokatt import Message


class Gui(View):

    '''
    Graphical User Interface implementation of View
    Not implemented yet!
    '''

    def __init__(self, logcat):
        View.__init__(self, logcat)

    def show(self):
        # Create Window and what ever we need to show a window
        pass

    def on_message(self, m: Message):
        pass
