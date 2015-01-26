from logcat import Logcat, Message

def message_handler(m: Message):
    print(m)

def main():
    logcat = Logcat()
    logcat.message_handler = message_handler
    logcat.start()
    logcat.stop()

if __name__ == '__main__':
    main()
