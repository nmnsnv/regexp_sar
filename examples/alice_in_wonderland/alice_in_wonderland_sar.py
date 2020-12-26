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
