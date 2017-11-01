
import sys
import os
import time


class FatalError(RuntimeError):
    """
    Wrapper class for runtime errors that aren't caused by internal bugs, but by
    ESP8266 responses or input content.
    """
    def __init__(self, message):
        RuntimeError.__init__(self, message)

    @staticmethod
    def WithResult(message, result):
        """
        Return a fatal error object that appends the hex values of
        'result' as a string formatted argument.
        """
        message += " (result was %s)" % hexify(result)
        return FatalError(message)


def main():
    """main comments"""

    try:
        file_length = os.path.getsize(src_file)
    except :
        print("%s open failed !!" %src_file)
        sys.exit(1)

    if md_size <= file_length:
        print("target size is less than source size")
        sys.exit(1)

    buffer = bytearray([0x0]*md_size)

    with open(src_file, 'rb') as f:
        read_buff = bytearray(f.read())
        f.close()

    for i in range(0, len(read_buff)):
        buffer[i] = read_buff[i]

    try:
        with open(target_file,'wb') as f:
            f.write(buffer)
            f.close()
    except:
        print("File access error (%s) " %target_file)



def _main():
    try:
        main()
    except FatalError as e:
        print('\nA fatal error occurred: %s' % e)
        sys.exit(2)


if __name__ == '__main__':
    if len(sys.argv) < 4:
        print("Usage: %s <source> <target> <size>" % sys.argv[0])
        sys.exit(1)

    src_file = sys.argv[1]
    target_file = sys.argv[2]
    md_size  = int(sys.argv[3],16)

    _main()
