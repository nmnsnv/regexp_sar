
from regexp_sar import RegexpSar

sar = RegexpSar()


match_str = "hello world abc"

sar.add_regexp('\w+', lambda from_pos,
               to_pos: print("Match Word: " + match_str[from_pos:to_pos]))
sar.add_regexp('world', lambda from_pos, to_pos: print(
    "Found world from: " + str(from_pos) + " to: " + str(to_pos)))

'''
Output:
    Match Word: hello
    Match Word: ello
    Match Word: llo
    Match Word: lo
    Match Word: o
    Match Word: world
    Found world from: 6 to: 11
    Match Word: orld
    Match Word: rld
    Match Word: ld
    Match Word: d
    Match Word: abc
    Match Word: bc
    Match Word: c
'''
sar.match(match_str)
