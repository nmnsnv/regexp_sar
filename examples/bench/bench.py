import random
import re
import time
from regexp_sar import RegexpSar

# generates a random string of length "size"
def str_gen(size):
    return "".join(random.choices("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", k=size))

# list contains all the regexps
regexp_list = []
print("Gen regexps")
# define each regexp length
re_len = 5

# define amount of regexps required
regexp_amount = 10_000

for i in range(regexp_amount):
    cur_re = str_gen(re_len)
    while cur_re in regexp_list:
        cur_re = str_gen(re_len)
    regexp_list.append(cur_re)
print("Done gen regexps")

print("Gen match string")
# generate the text on which we will run
match_str = str_gen(1_000_000)
print("Done gen match string")

# counts amount of matches for SAR matchingg
match_count = 0
# this will be called every time any regexp was matched in the text
def count_matches(from_pos, to_pos):
    global match_count
    match_count += 1


# benchmark native Python regexp engine performance
print("Start python match")
# generate Python regexp string using alternation
python_native_re = f"({'|'.join(regexp_list)})"
start = time.time()

# find amount of matches found with native Python regexp engine
python_re_match_count = len(re.findall(python_native_re, match_str))
print(f"Done in {time.time() - start} seconds {python_re_match_count} matches")


# create a SAR instance
sar = RegexpSar()

# append all the regexps for the SAR instance
for cur_re in regexp_list:
    sar.add_regexp(cur_re, count_matches)

# benchmark SAR performance
print("Start SAR Match")
start = time.time()
sar.match(match_str)
print(f"Done Match in: {time.time() - start} seconds. {match_count} matches!")
