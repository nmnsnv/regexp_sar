
Consider the following: you are tasked with finding in the book Alice In Wonderland all the positions of the sentences in the text, which contains Alice and either the cat or the rabbit.

We may want to break the book into sentences and then look into each sentence into Alice and either cat or rabbit to see if they're in the sentence, but we still wont have the position of those sentences.

Another direction that falls short is the usage of regexps to find Alice, cat and rabbit, unfortunately we can't know when we match either word if they appear in the same sentence.

Although regexps could be a big help they fall short since we cannot apply context into the regexps we write, or even interact with the regexp engine almost at all, which is a shame since it could make our text searching much more powerful then normal regexp engines allow us to.

For this reason I've written a regexp engine that is designed to solve exactly that problem, help us append context into each of our regexps while also allowing us to have a high interactivity with the regexp engine in a very natural way.

SAR is a new regular expression engine designed to take a different approach for writing regular expressions.
It is designed to handle many regular expressions together by uniting them into a single trie while keeping a high performance and keeping track of exactly which regexp was matched.

For example, let's see how we can solve the Alice In Wonderland problem (text is taken from: https://gist.github.com/phillipj/4944029)

```python
from regexp_sar import RegexpSar

# create a SAR instance
sar = RegexpSar()

# initialize lists of all the sentences positions of alice with cat/rabbit
alice_and_cat_position = []
alice_and_rabbit_position = []

# keep track of sentence start position
sentence_start_position = 0

# flags for whether alice/cat/rabbit occurs in the current sentence
is_alice_in_sentence = False
is_cat_in_sentence = False
is_rabbit_in_sentence = False

# mark that the word alice is found in the current sentence

def find_alice(f, t):
    global is_alice_in_sentence
    is_alice_in_sentence = True

# mark that the word cat is found in the current sentence
def find_cat(f, t):
    global is_cat_in_sentence
    is_cat_in_sentence = True

# mark that the word rabbit is found in the current sentence
def find_rabbit(f, t):
    global is_rabbit_in_sentence
    is_rabbit_in_sentence = True

# indicates that the sentece is over,
# append sentence into the right list if found any
def end_of_sentence(from_pos, to_pos):
    global is_cat_in_sentence, is_alice_in_sentence, is_rabbit_in_sentence
    global alice_and_cat_position, alice_and_rabbit_position
    global sentence_start_position

    # check if alice is found with cat/rabbit in current sentence
    if is_alice_in_sentence and is_cat_in_sentence:
        alice_and_cat_position.append([sentence_start_position, to_pos])
    if is_alice_in_sentence and is_rabbit_in_sentence:
        alice_and_rabbit_position.append([sentence_start_position, to_pos])

    # set next sentence start position to end of current sentence position + 1
    sentence_start_position = to_pos + 1

    # reset words in sentence to false
    is_alice_in_sentence = False
    is_cat_in_sentence = False
    is_rabbit_in_sentence = False

# get Alice In Wonderland book content
with open("./alice_in_wonderland.txt", "r") as alice_book:
    text = alice_book.read()

# add alice/cat/rabbit regexps
sar.add_regexp('\\^\\walice\\^\\w', find_alice)
sar.add_regexp('\\^\\wAlice\\^\\w', find_alice)
sar.add_regexp('\\^\\wcat\\^\\w', find_cat)
sar.add_regexp('\\^\\wCat\\^\\w', find_cat)
sar.add_regexp('\\^\\wrabbit\\^\\w', find_rabbit)
sar.add_regexp('\\^\\wRabbit\\^\\w', find_rabbit)

# add end of sentence regexps,
# NOTE that they point to the same callback
for cur_re in ['\\.', '\\?', '!', ';']:
    sar.add_regexp(cur_re, end_of_sentence)

# run the regexps on the text
sar.match(text)

# the matching has completed, show the results:
print("Alice + Cat Matches: " + str(alice_and_cat_position))
print("Alice + Rabbit Matches: " + str(alice_and_rabbit_position))
'''
OUTPUT:
    Alice + Cat Matches: [[32146, 32238], [64026, 64196], ...]
    Alice + Rabbit Matches: [[174, 566], [910, 1004], ...]
'''
```

In the above example you can see that we append a callback for each regexp we append, and it will be called when there will be a match for that specific regexp in the order in which it was found in the text.

Whenever we match Alice/cat/rabbit in the text, we will mark that they are found, then when we match an end of sentence match, we will look what words were matched, and if Alice matched with cat/rabbit, append the current sentence to the required list, then reset the flags that mark the alice/cat/rabbit match since next match of either of those will be in a new sentence, also mark the beginning of the next sentence to be the end of this sentence + 1.

This matching routine will continue until the end of our text, or until we decide to tell the SAR to stop for whatever reason we may find (using ```sar.stop_match()```).

The example above reflects the amount of flexibility that is acquired by using SAR by relative ease, which may allow us to do much more complex pattern matching then we previously could, for instance, SAR has the ability to continue the search after the match from whatever position we may want, which could be useful since sometimes the context of a match is not known until after the word is matched, think about the phrase "3 phones", we cannot append context for the number 3 until we read the word that comes next - "phones", in SAR, we can return to that digit after we've obtained the context - phones, and let the regexp engine match it again (by using ```sar.continue_from(position)```), this is unachievable with current regexp engines


Another problem SAR intended to solve is to give very high performance when matching many regexps at once, which may be needed when we try to search for many regexps on large texts, which SAR is proving to be much faster than python the more we scale.

This, together with the ability to know exactly which regexp was found can prove to be a very powerful tool. Note, that SAR differs then normal regexp alternation by letting us know exactly which regexp was matched, not only that there was a match for one of thousands of regexps/

Let us see how much faster SAR could be by writing a small benchmark:

The following code will match 10,000 random unique regexps (exact words to be specific) of 5 characters each on a random string of length 1,000,000 bytes, which will run on both Python native regexp engine using alternation, and on SAR.

Benchmark:
```python

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
'''
OUTPUT:
    Gen regexps
    Done gen regexps
    Gen match string
    Done gen match string
    Start python match
    Done in 31.657098531723022 seconds 14 matches
    Start SAR Match
    Done Match in: 0.19321727752685547 seconds. 14 matches!
'''
```

As we can see in the results, SAR proved to be much faster then even python built in regexp alternation, by <b>more than x150 times</b> on my PC.

NOTE: The strings used in this benchmark are randomly generated thus results may vary, although after running the benchmark multiple times, the results were rather consistent.

You may note that I've benchmarked only exact matches since generating regexp that can run both on SAR and on python would require a longer script whereas I try to keep the code examples as short as possible, but you are welcome to try more complicated ones on your own.

Looking at the example above, it is good to see another place where SAR may prove more convenient, in normal regexp, you often need to write very long, complicated regexps, as with the alternation we generated for Python regexp engine, while in SAR you write multiple shorter regexps that act together, the regexp ```abc.*def``` could be written in SAR by seperating the two apart, one is ```abc``` and the other ```def``` which may simplify often complicated regexps.

I have some ideas of more features for SAR, for example, add UTF support, also it is possible to add multithreading support for even better performance, or to add support for streams to allow longer text searching.
