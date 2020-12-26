from regexp_sar import RegexpSar

sar = RegexpSar()

match_word_count = 0
match_word = None
match_num_count = 0
match_num = None
match_str = "hello world 123 abc 456 789"

regexps = [
    ['\w+', 'word'],
    ['\d+', 'number'],
]

for cur_regexp in regexps:
    def find_second_match(description):
        match_count = 0
        match_val = None
        def callback(from_pos, to_pos):
            nonlocal match_count, match_val
            match_count += 1
            if match_count == 2:
                print("Match: " + str(description) + ": " + match_str[from_pos:to_pos])
            sar.continue_from(to_pos)
        return callback
    sar.add_regexp(cur_regexp[0], find_second_match(cur_regexp[1]))

'''
Output:
    Match: word: world
    Match: number: 456
'''
sar.match(match_str)