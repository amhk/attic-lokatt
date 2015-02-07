from common.liblokatt import Logcat
from cli.cli import Cli


def main():
    logcat = Logcat()
    # TODO: Let parsed arguments decide if we create Cli
    # or Gui as the View instance.
    view = Cli(logcat)
    view.show()

if __name__ == '__main__':
    main()
