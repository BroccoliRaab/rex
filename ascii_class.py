from enum import auto, IntFlag

reserved_chars = "[]\\+*?$.{}|/"

class ascii_class(IntFlag):
    reserved = auto()
    word = auto()
    digit = auto()


for c in range(0, 128):
    i = 0
    c = chr(c)
    i |= ascii_class.digit if c.isdigit() else 0
    i |= ascii_class.word if c.isalnum() or c == '_' else 0
    i |= ascii_class.reserved if c in reserved_chars else 0
    print(hex(i), end=", ")
print()
