
from nose.tools import nottest

from regexp_sar import RegexpSar


class VarPointer:
    def __init__(self):
        super().__init__()
        self.val = 0

    def set(self, val):
        self.val = val

    def append(self, amount):
        self.val += amount


@nottest
def validate(expected_val, calculated_val, msg=None):
    if msg is None:
        msg = f"expected: {expected_val}, received: {calculated_val}"
    assert expected_val == calculated_val, msg


def test1():
    obj = VarPointer()
    sar = RegexpSar()
    sar.add_regexp("abcd", lambda from_pos, to_pos: obj.append(1))
    sar.match("qabcdef")
    validate(1, obj.val)


def test2():
    obj = VarPointer()
    sar = RegexpSar()
    sar.add_regexp("abcd", lambda from_pos, to_pos: obj.append(1))
    sar.match("qabcde abcd f")
    validate(2, obj.val)


def test3():
    obj = VarPointer()
    sar = RegexpSar()
    sar.add_regexp("abcd", lambda from_pos, to_pos: obj.append(1))
    sar.match("qqqqqqqqqqqq")
    validate(0, obj.val)


def test4():
    obj = VarPointer()
    sar = RegexpSar()
    sar.add_regexp("abcd", lambda from_pos, to_pos: obj.append(1))
    sar.add_regexp("nm", lambda from_pos, to_pos: obj.append(2))
    my_str = "qabcdefnmq"
    sar.match_from(my_str, 1)
    validate(3, obj.val)

    obj.set(0)
    sar.match_from(my_str, 2)
    validate(2, obj.val)


def test5():
    obj = VarPointer()
    sar = RegexpSar()
    re = "abcd"
    myStr = "qabcdef"
    sar.add_regexp(re, lambda from_pos, to_pos: obj.append(1))
    sar.match(myStr)
    validate(1, obj.val)


def test6():
    obj = VarPointer()
    sar = RegexpSar()
    myStr = "0123456 789"
    sar.add_regexp("\\d+", lambda from_pos,
                   to_pos: obj.set(myStr[from_pos:to_pos]))
    sar.match_at(myStr, 3)
    validate("3456", obj.val)


def test7():
    obj = VarPointer()
    sar = RegexpSar()
    sar.add_regexp("abcd", lambda from_pos, to_pos: obj.append(1))
    sar.match("qabcdeabcdkabcdf")
    validate(3, obj.val)

    obj2 = VarPointer()
    sar2 = RegexpSar()

    def sar2_callback(from_pos, to_pos):
        obj2.append(1)
        if obj2.val == 2:
            sar2.stop_match()

    sar2.add_regexp("abcd", sar2_callback)
    sar2.match("qabcdeabcdkabcdf")
    validate(2, obj2.val)


def test8():
    objFrom = VarPointer()
    objTo = VarPointer()
    sar = RegexpSar()
    myStr = "qabcdef"
    re = "abcd"

    def sar_callback(from_pos, to_pos):
        objFrom.set(from_pos)
        objTo.set(to_pos)

    sar.add_regexp(re, sar_callback)
    sar.match(myStr)
    validate(1, objFrom.val)
    validate(5, objTo.val)
    matchStr = myStr[objFrom.val:objTo.val]
    validate(re, matchStr)
    objFrom.set(0)
    objTo.set(0)
    sar.match("qqqqabcdttt")
    validate(4, objFrom.val)
    validate(8, objTo.val)


def test9():
    objFrom = VarPointer()
    objTo = VarPointer()
    sar = RegexpSar()
    myStr = "1234"
    re = "\\d+"

    def sar_callback(from_pos, to_pos):
        objFrom.set(from_pos)
        objTo.set(to_pos)
        sar.stop_match()

    sar.add_regexp(re, sar_callback)
    sar.match(myStr)
    validate(myStr, myStr[objFrom.val:objTo.val])


def test10():
    myStr = "abc"
    res = []
    sar = RegexpSar()
    sar.add_regexp("\\w+",
                   lambda from_pos, to_pos: res.append(myStr[from_pos:to_pos]))
    sar.match(myStr)
    validate("abc", res[0])
    validate("bc", res[1])
    validate("c", res[2])
    validate(3, len(res))


def test11():
    myStr = "123abc456"
    res = []
    sar = RegexpSar()

    def sar_callback(from_pos, to_pos):
        res.append(myStr[from_pos:to_pos])
        sar.continue_from(to_pos + 1)

    sar.add_regexp("\\a+", sar_callback)
    sar.match(myStr)
    validate("abc", res[0])
    validate(1, len(res))


def test12():
    myStr = "123abc456"
    res = []
    sar = RegexpSar()

    def call(from_pos, to_pos):
        res.append(myStr[from_pos:to_pos])
        sar.continue_from(to_pos)

    sar.add_regexp("\\a+", call)
    sar.add_regexp("\\d+", call)
    sar.match(myStr)

    validate("123", res[0])
    validate("abc", res[1])
    validate("456", res[2])
    validate(3, len(res))


def test13():
    myStr = "123abc"
    res = []
    sar = RegexpSar()

    def call(from_pos, to_pos):
        res.append(myStr[from_pos:to_pos])
        sar.continue_from(to_pos)

    sar.add_regexp("\\d+", call)
    sar.add_regexp("\\w+", call)
    sar.match(myStr)

    validate("123", res[0])
    # TODO check if this is a real error?
    validate("123abc", res[1])
    validate(2, len(res))


def test14():
    myStr = "backup:x:34:34:backup:/var/backups:/usr/sbin/nologin"
    elemNum = 6
    sar = RegexpSar()
    matchCount = VarPointer()
    strt = VarPointer()
    end = VarPointer()

    def sar_callback(from_pos, to_pos):
        matchCount.append(1)
        sar.continue_from(to_pos)
        if matchCount.val == elemNum - 1:
            strt.set(to_pos)
        elif matchCount.val == elemNum:
            end.set(from_pos)
            sar.stop_match()

    sar.add_regexp(":", sar_callback)
    sar.match(myStr)
    if end.val > 0:
        matchStr = myStr[strt.val:end.val]
        validate("/var/backups", matchStr)
    else:
        # fail on purpuse
        validate(0 == 1, "no match found")

# TODO: check what this test is for?


def test15():
    myStr = "123abc"
    obj = VarPointer()
    sar = RegexpSar()

    def call(from_pos, to_pos):
        obj.set(1)

    sar.add_regexp("\\d+", call)
    sar.add_regexp("\\w+", call)
    sar.match(myStr)

    validate(1, obj.val)


def test16():
    myStr = "123abc"
    res = []
    sar = RegexpSar()

    def call(from_pos, to_pos):
        res.append(myStr[from_pos:to_pos])
        sar.continue_from(to_pos + 1)

    sar.add_regexp("\\d+", call)
    sar.add_regexp("\\a+", call)
    sar.match(myStr)

    validate("123", res[0])
    validate("bc", res[1])
    validate(2, len(res))


def test17():
    myStr = "mm abc nn"
    obj = VarPointer()
    sar = RegexpSar()

    def call(from_pos, to_pos):
        obj.set(1)

    sar.add_regexp("\\d+", call)
    sar.add_regexp("\\a+", call)
    sar.match(myStr)

    if obj.val != 1:
        validate(0 == 1, "regexp should match")


def test18():
    myStr = "first abbbbc second 123 end"
    matches = []
    regexps = [
        ["ab+c", "First Match"],
        ["\\d+", "Second Match"],
    ]

    sar = RegexpSar()
    for re in regexps:
        reStr = re[0]
        reTitle = re[1]

        def gen_call(reTitleInternal):
            def call(from_pos, to_pos):
                matchStr = myStr[from_pos:to_pos]
                matches.append(reTitleInternal + ": " + matchStr)
                sar.continue_from(to_pos)
            return call

        sar.add_regexp(reStr, gen_call(reTitle))

    sar.match(myStr)
    validate(2, len(matches))
    validate("First Match: abbbbc", matches[0])
    validate("Second Match: 123", matches[1])


def test19():
    # get third match and stop
    myStr = "aa11 bb22 cc33 dd44"
    matchCount = VarPointer()
    matchedStr3 = VarPointer()
    sar = RegexpSar()

    def call(from_pos, to_pos):
        matchCount.append(1)
        if matchCount.val == 3:
            matchedStr3.val = myStr[from_pos:to_pos]
            sar.stop_match()
        else:
            sar.continue_from(to_pos)

    sar.add_regexp("\\w+", call)
    sar.match(myStr)

    validate(3, matchCount.val)
    validate("cc33", matchedStr3.val)


def test20():
    # get match only at certain position
    myStr = "aa11 bb22 cc33 dd44"
    matchedStr4 = VarPointer()
    sar = RegexpSar()

    def call(from_pos, to_pos):
        matchedStr4.val = myStr[from_pos:to_pos]
    sar.add_regexp("\\w+", call)
    sar.match_at(myStr, 5)

    validate("bb22", matchedStr4.val)


def test21():
    # get match only at certain position
    myStr = "aaaaaaaaaaaaaaaaaaaaaaaaaabbbc"
    matched = VarPointer()
    sar1 = RegexpSar()
    sar2 = RegexpSar()

    def sar1_callback(from_pos, to_pos):
        sar2.match_at(myStr, to_pos)
        sar1.stop_match()

    sar2.add_regexp("b+c", lambda from_pos, to_pos: matched.append(1))
    sar1.add_regexp("a+", sar1_callback)
    sar1.match(myStr)

    validate(1, matched.val)


def test22():
    sar = RegexpSar()
    alpha_pos = VarPointer()
    alpha_pos.val = -1
    anchorPos = VarPointer()

    def sar_digit_callback(digit_pos, to_pos):
        if alpha_pos.val != -1:
            dist = digit_pos - alpha_pos.val
            if (dist == 1):
                anchorPos.set(digit_pos)

    sar.add_regexp("\\a", lambda from_pos, to_pos: alpha_pos.set(from_pos))
    sar.add_regexp("\\d", sar_digit_callback)
    sar.match("aa bb2cc dd")
    validate(5, anchorPos.val)
