
from nose.tools import nottest

from regexp_sar import RegexpSar


def test_matching():
    match_test(["abcd"], "qabcdef", 1)
    match_test(["abcd", "abcf"], "zqabcdwabcfr", 3)
    match_test(["abcd", "abc"], "zqabcdwr", 3)

    match_test(["abcd", "amk", "axf", "awe", "aitr"], "zqabcdwamkraxfaz", 7)
    match_test(["a"], "a", 1)
    match_test(["a"], "b", 0)
    match_test(["a"], "ma", 1)
    match_test(["a"], "maa", 2)
    match_test(["asv"], "bbbbasvmm", 1)
    match_test(["asv"], "asv", 1)
    match_test(["asv", "kdf"], "bbasvmkdfm", 3)

    match_test(["as", "ask"], "bbasvmaskm", 4)
    match_test(["as", "ask"], "bbaskmmmm", 3)

    match_test(["ab?cd", "dk?m"], "aaadmabcdn", 3)
    match_test(["ab?cd", "k?m"], "aaadmabcdn", 3)
    match_test(["zzm?"], "iozzj", 1)
    match_test(["zzm?"], "iozzmj", 2)
    match_test(["zzm?n?"], "iozzjzznj", 3)
    match_test(["ab?cde", "acdf"], "mmabcdfkkacdeoo", 1)
    match_test(["ab?cd?e"], "mabcdem macdem macem mabcem nbcden", 4)
    match_test(["ab?c?d?e"], "maem madem mabem", 3)
    match_test(["ac?c?c?e"], "m ace m", 1)
    match_test(["ac?c?c?e", "ace"], "m ace m", 3)
    match_test(["ac?c?c?c?e"], "m acce m", 1)

    match_test(["ab?cd", "acd"], "qacdq", 3)

    match_test(["ab+cd"], "aadmabcdn", 1)
    match_test(["ab+cd"], "aadmabbcdn", 1)
    match_test(["ab+cd"], "aadmabbbbbbbbcdn", 1)

    match_test(["b+"], "abbbc", 3)
    match_test(["b+c"], "abbbcd", 3)

    match_test(["ab+"], "aadmabbbbcdn", 1)
    match_test(["ab+c+d"], "aadmabcccccdn", 1)
    match_test(["ab+c+d"], "aadmabdn", 0)
    match_test(["ab+b?d"], "aadmabdn", 1)
    match_test(["ab+b?d"], "aadmabbbbdn", 1)
    match_test(["ab+c?d"], "aadmabbbbdn", 1)
    match_test(["ab+b+d"], "aadmabbbbdn", 1)
    match_test(["ab+b+d"], "aadmabdn", 0)
    match_test(["ab+cd", "ab?cd"], "aaacdabbbbbbcdn", 3)

    match_test(["ab*cd"], "aaadmabcdn", 1)
    match_test(["ab*cd"], "aaadmabbbbcdn", 1)
    match_test(["ab*cd"], "aaadmacdn", 1)
    match_test(["ab*c*d"], "aamadn", 1)
    match_test(["ab*c*d"], "aamacccccccdn", 1)

    match_test(["ab*c?d"], "aamadn", 1)
    match_test(["ab*c?d"], "aamabcdn", 1)
    match_test(["ab*c?d"], "aamacdn", 1)
    match_test(["ab*c?d"], "aamabdn", 1)

    match_test(["ab?cd", "ab+cd"], "mmabcdmm", 3)
    match_test(["ab?cd", "ab+cd"], "mmacdmm", 1)
    match_test(["ab?cd", "ab+cd"], "mmabbbcdmm", 2)
    match_test(["ab*c?b+e"], "mmabedmm", 1)
    match_test(["ab*c?b+e"], "mmabcbedmm", 1)
    match_test(["ab+b?b+c"], "mmabbcdmm", 1)
    match_test(["ab+c?b+c"], "mmabbbcdmm", 1)

    match_test(["ab\\cd"], "q1abcdef", 1)
    match_test(["ab\\cd"], "q1abcdef", 1)
    match_test(["ab\\\\cd"], "q2abcdef", 0)
    match_test(["ab\\\\cd"], "q3ab\\cdef", 1)
    match_test(["ab\\\\\\\\cd"], "q3ab\\\\cdef", 1)
    match_test(["ab\\?cd"], "qabcdef", 0)
    match_test(["ab\\?cd"], "qab?cdef", 1)

    match_test(["abcd\\?"], "qabcd?ef", 1)
    match_test(["abcd\\"], "qabcdef", 1)
    match_test(["ab?cd"], "qabcdef", 1)
    match_test(["abcd?"], "qabcef", 1)
    match_test(["abcd?"], "qabcdef", 2)
    match_test(["ab?cd"], "qacdef", 1)
    match_test(["ab?cd"], "qab?cdef", 0)
    match_test(["ab\\?cd"], "qab?cdef", 1)
    match_test(["ab?cd", "abce"], "qacem", 0)
    match_test(["abd", "ab?d", "kkk", "abc?d"], "qabdm", 11)
    match_test(["ab?b?b?b?b?cd"], "qacdef", 1)
    match_test(["ab?b?b?cd"], "qabcdef", 1)
    match_test(["ab?b?b?cd"], "qabbcdef", 1)
    match_test(["abc??"], "qabc?def", 1)

    match_test(["ab+cd"], "qabbbbcdef", 1)
    match_test(["ab+cd"], "qabcdq qabbbbcdq", 2)
    match_test(["am+e"], "qammmeq", 1)
    match_test(["ab+c", "am+e"], "qabcq qabbbbbcq qammmeq", 4)
    match_test(["ab+c+d"], "qabbcdq", 1)

    match_test(["ab+cd", "abcf"], "abbcd", 1)
    match_test(["ab+cd", "abcf"], "abbcf", 0)
    match_test(["ab+cd"], "acdf", 0)
    match_test(["ab+"], "qabcdq qabbbbcdq qacdq", 2)
    match_test(["ab+bcd"], "qabcdq", 0)
    match_test(["ab+bcd"], "qabbcdq", 1)
    match_test(["ab+b+cd"], "qabbcdq", 1)
    match_test(["ab+b+cd"], "qabcdq", 0)
    match_test(["ab+b+b+cd"], "qabbcdq", 0)
    match_test(["ab+b+b+cd"], "qabbbcdq", 1)
    match_test(["ab+b+b+cd"], "qabbbbbbbbbbbbbbbbbcdq", 1)
    match_test(["a\\?+cd"], "qa????cdq", 1)
    match_test(["ab*cd"], "qabbbbcdef", 1)
    match_test(["ab*cd"], "qacdef", 1)

    match_test(["ab?cd+d+ef"], "q acddddef q", 1)
    match_test(["ab?cd+d+ef"], "q acdef q", 0)
    match_test(["ab?cd+d+ef"], "q acddddef q", 1)

    match_test(["a\\++b"], "q a+++++++b q", 1)

    # 3 matches: "bbb", "bb", "b"
    match_test(["b+"], "m bbb m", 3)
    match_test(["ab+"], "m abbb m", 1)

    match_test(["a\\+c"], "m a+c ", 1)
    match_test(["a\\+c"], "m aac ", 0)
    match_test(["ab\\?c"], "m aac ", 0)
    match_test(["ab\\?+c"], "m aab??????c ", 1)

    match_test(["a\\dc", "a5c"], "m a5c m", 3)
    match_test(["a\\dc"], "m a5c a8c m", 2)
    match_test(["ab\\d?c"], "m abc m", 1)
    match_test(["ab\\d?c"], "m ab1c m", 1)
    match_test(["ab\\d?c"], "m abdc m", 0)

    match_test(["ab\\d+c"], "m ab7c m", 1)
    match_test(["ab\\d+c"], "m ab7657234c m", 1)
    match_test(["ab\\d*c"], "m abc m", 1)
    match_test(["ab\\d*c"], "m ab234c m", 1)
    match_test(["ab\\d+\\d+c"], "m ab234c m", 1)
    match_test(["ab\\d+\\d+c"], "m ab2c m", 0)

    match_test(["\\d+\\d+c"], "m ab234c m", 2)
    match_test(["\\d+"], "m ab234c m", 3)

    match_test(["a\\sc", "a.c"], "m a c m", 3)
    match_test(["a\\sc"], "m a c a c m", 2)
    match_test(["ab\\s?c"], "m abc m", 1)
    match_test(["a\\s*c"], "m ac a c a      c m", 3)

    match_test(["\\a"], " t ", 1)
    match_test(["\\a"], " 4 ", 0)
    match_test(["\\a+"], " t ", 1)
    match_test(["\\a+"], " 4 ", 0)
    match_test(["\\a+"], " tre ", 3)
    match_test(["\\a+"], " 456 ", 0)
    match_test(["a\\ac", "a.c"], "m afc m", 3)
    match_test(["a\\ac"], "m asc auc m", 2)
    match_test(["a\\ac"], "m a4c m", 0)
    match_test(["ab\\a?c"], "m abc m", 1)
    match_test(["a\\a*c"], "m ac atc asggfc a8c m", 3)

    match_test(["a\\wc"], "m a4c adc m", 2)
    match_test(["ab\\w?c"], "m abtc m", 1)
    match_test(["a\\w*c"], "m ac ahc asd87sc m", 3)

    match_test(["a.c"], "m a5d m", 0)
    match_test(["a.c"], "m akcm alc l ", 2)
    match_test(["a.?c"], "m akcm ac l ", 2)
    match_test(["a.+c"], "m  abbregtjhbc m", 1)
    match_test(["a.+.+c"], "m  ahbc m", 1)
    match_test(["a.+.+c"], "m  ahc m", 0)
    match_test(["a\\.+c"], "m abbc m", 0)
    match_test(["a\\.+c"], "m a..c m", 1)
    match_test(["a\\.c"], "m a.cm", 1)
    match_test(["a\\.c"], "m ahcm", 0)
    match_test(["."], "m", 1)
    match_test(["\\."], "m", 0)
    match_test(["a\\.?b"], "amb", 0)
    match_test(["a\\.?b"], "ab", 1)
    match_test(["a\\.?b"], "a.b", 1)

    match_test(["a\\^bc"], "m adc m", 1)
    match_test(["a\\^bdc"], "m aqdc m", 1)
    match_test(["a\\^bc"], "m abc m", 0)

    match_test(["a\\^b?dc"], "m adc m", 1)
    match_test(["a\\^b?dc"], "m aqdc m", 1)
    match_test(["a\\^b?bc"], "m abc m", 1)
    match_test(["a\\^b?c"], "m ac m", 1)
    match_test(["a\\^b?\\^m?c"], "m attc m", 1)
    match_test(["a\\^b?\\^m?c"], "m ac m", 1)

    match_test(["a\\^bc"], "m adc a c a6c m", 3)
    match_test(["a\\^b\\^dc"], "m affc m", 1)
    match_test(["a\\^b\\^dc"], "m afdc m", 0)
    match_test(["a\\^b\\^dc"], "m abfc m", 0)
    match_test(["a\\^b\\^dc"], "m affa m", 0)
    match_test(["a\\^\\\\c"], "m adc m", 1)
    match_test(["a\\^\\\\c"], "m a\\c m", 0)

    match_test(["a\\^\\sc"], "m a c m", 0)
    match_test(["a\\^\\sc"], "m adc m", 1)
    match_test(["a\\^\\dc"], "m a6c m", 0)
    match_test(["a\\^\\dc"], "m adc m", 1)
    match_test(["a\\^\\ac"], "m adc m", 0)
    match_test(["a\\^\\ac"], "m a6c m", 1)
    match_test(["a\\^\\wc"], "m a6c m", 0)
    match_test(["a\\^\\wc"], "m adc m", 0)
    match_test(["a\\^\\wc"], "m a c m", 1)

    match_test(["a\\^b", "a\\^c"], "ac", 1)

    match_test(["a\\^b+c"], "m adc m", 1)
    match_test(["a\\^b+c"], "m aduic m", 1)
    match_test(["a\\^b+\\^b+c"], "m adfc m", 1)
    match_test(["a\\^b+dc"], "m adghjdc m", 1)
    match_test(["a\\^b+dc"], "m adghbjdc m", 0)
    match_test(["a\\^b*dc"], "m adc m", 1)
    match_test(["a\\^b*c"], "m atc m", 1)
    match_test(["a\\^b*c"], "m atghhgc m", 1)
    match_test(["a\\^\\d+c"], "m adgdc m", 1)
    match_test(["a\\^\\d+c"], "m adghjc m", 1)
    match_test(["a\\^\\d+c"], "m adgh7jc m", 0)
    match_test(["a\\^\\w+c"], "m a-+-c m", 1)
    match_test(["a\\^\\a+c"], "m a-+-c m", 1)
    match_test(["a\\^\\s+c"], "m a-+-c m", 1)


@nottest
def match_test(all_regexp, match_string, match_expected):
    sar = RegexpSar()
    path_res = 0
    for i in range(len(all_regexp)):
        re_num = 2 ** i

        # NOTE: this is done since re_num is not defined every iteration, but only once
        def gen_callback(cur_re_num):
            def match_callback(from_pos, to_pos):
                nonlocal path_res
                path_res += cur_re_num
            return match_callback
        sar.add_regexp(all_regexp[i], gen_callback(re_num))
    sar.match(match_string)
    assert match_expected == path_res, f"Match fail for [{', '.join(all_regexp)}] in >>{match_string}<<"
