
# SAR - Simple Api for Regexp

Regexp SAR - a python module for multi match event handling regular expression engine

## Description

SAR is a new way of handling regular expression which allows us to run many regular expressions (only limitation being the available memory) at once.
When adding a regexp, there is also a related callback that will be called upon each match in the same order in which they appear on the text 

## Install

```bash
pip install regexp_sar
```

## Import

```python
from regexp_sar import RegexpSar
```

## Example

```python
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
```

# Methods

## constructor

creates a new sar instance with its own regexps and callbacks, many instances can be built at once

## add_regexp

adds a regexp into the sar instance, recieved 2 parameters:
* regexp - the required regexp
* callback - the callback which will be called upon match, the callback receives 2 parameters -
  * from_pos - the start position of the match in the matched string
  * to_pos   - the end position of the match in the matched string (exclude to_pos)

```python
sar = RegexpSar()
sar.add_regexp('abc', lambda from_pos, to_pos: print("Match: " + str(from_pos) + "->" + str(to_pos)))


sar.match("hello abc world") # Match: 6->9
```

## match

begins a match against the previously defined regexps on the received string.
receive 1 parameter:
* string to be matched with

* NOTE: this is syntactic sugar for [match_from](#match_from)(match_str, 0)

## match_from

acts like [match](#match) but starts from a custom position the search.
receive 2 parameters:
* string to be matched with
* start position of the match

## match_at

looks for a match from a specific character only, and will not continue to search for matches in the following characters

## continue_from

Can be called only during a match/match_from, will continue the next match character from the given character index

receive 1 parameter:
* position for next match

## stop_match

Can be called only during a match/match_from, will stop the match after current char matching has ended

# rules

## abbreviations

* .  - matches any character
* \d - matches a digit character (checked by isdigit method)
* \w - matches an alphanumeric character (checked by isalnum method)
* \a - matches an alpha character (checked by isalpha method)
* \s - matches a space character (checked by isspace method)
* \^ - matches a character <b>NOT</b> followed by the match (i.e. ```\^\d+``` will match all non digit strings)

## repetition

* '?' - matches 1 or 0 times
* '*' - matches 0 or more times
* '+' - matches 1 or more times

## backslash ('\\') character

in order to match the '\\' character, it needed to be followed by 3 more backslashes (4 in total) since python string takes 2 backslashes to be represented as one

```python
sar = RegexpSar()
sar.add_regexp('\\\\', lambda from_pos, to_pos: print("Match: " + str(from_pos) + "->" + str(to_pos)))
sar.match('a\\b') # Match: 1->2
```

# examples

Examples may be found in the ```test_oousage.py``` file, and in the examples directory

# Unicode support

Currently not supported. May be added in future update

# Author

Noam Nisanov - ```noam.nisanov@gmail.com```
