
# DLP (Data Loss Prevention) in SAR

Data Loss Prevention (DLP) systems are systems that are tasked with detecting data leakage or destruction of confidential data.
For DLP systems to work they are required to handle huge amounts of data in search of possible leakage.

A basic way to try and identify sensitive data is to look for sensitive information in all the content.
For instance:

```
employee names, id numbers, customer contracts, employee phone numbers, salaries, passwords, addresses, etc...
```

Let's take a look at how we can enforce employees names. We want to detect our employee Noam Nisanov, how might we search for his name?
We could make an exact match for all of our employees, but this approach quickly appears flawed. What if the name is written differently?
For instance:

```
Noam Nisanov
Noam-Nisanov
NoamNisanov
Noam     Nisanov
Noam
Nisanov
Noam, Nisanov
```

It could even be written vice versa - ```Nisanov Noam, Nisanov-Noam, NisanovNoam``` etc...

It's very clear that there are infinite different ways to write employee names, so no exact matching could work.
A much better way to detect names would be with the help of regexps, with that, we could detect names rather easily: ```\b(Noam[^\a]*Nisanov)\b``` or ```\b(Nisanov[^\a]*Noam)\b```

To detect sensitive content we could automatically generate regexps for all instances of sensitive information that we've mentioned before.
For instance:
```
# Employee Noam Nisanov
\b(Noam[^\a]*Nisanov)\b
\b(Nisanov[^\a]*Noam)\b

# Employee Adam Smith
\b(Smith[^\a]*Adam)\b
\b(Adam[^\a]*Smith)\b

# Employee X phone number
012-?1234567

# Contractor address
221C[^\w]*Baker[^\w]*Street

# Employee Id
ID_112
ID_113
ID_114

...
```
After we generated all the regexps, we can join them together with alternation and start searching documents to match them.

There are a few problems that arise from that issue:

* Python Regex Engine performs rather poorly using alternations
* No way to tell which regex was found, or to what the match relates to (Employee name, phone number, etc...)
* No ability to interact with Python Regex Engine during matching process - 0 ability to add logic to the matching process:
    * Say we want to only find up to N matches before we stop matching to avoid wasting CPU time.
    * Assuming we deal with very large documents, we cannot update on the fly that a match was made during the regex matching process - High latency

## SAR - Event Driven Regex Engine

SAR (Simple Api for Regexps) is a Regex Engine that is designed to provide:

* **NOTE:** SAR api is different then Python API. Please take a look at [SAR homepage](https://github.com/nmnsnv/regexp_sar/) for documantation

* High performance
* Match many regexps on a single run
* Highly interactive matching process - Interact with the Regex Engine while its running
* Event Driven regexps:
    * Every regex is added with it's unique handler
    * The handler will be called on every match of that handler
    * The handlers are raised in the same order they matched on the text

With SAR, we're able to deal with the flaws presented before:

* Very high performance compared to Python native Regex Engine (more than x150 times on exact match):
    * Code found in [introduction post](https://github.com/nmnsnv/regexp_sar/blob/master/docs/introduction.md)

* Every match, the regex which caused the match is known
    * We can stop matching after N matches
    * Notify that a match is made after each match is made

```python
from regexp_sar import RegexpSar

sar = RegexpSar()

sar.add_regexp("Noam\\s*Nisanov", lambda from_pos, to_pos: print("Match 'Noam Nisanov'"))
sar.add_regexp("Adam\\s*Smith"  , lambda from_pos, to_pos: print("Match 'Adam Smith'"))

# prints: Match 'Noam Nisanov'
sar.match("This text contains Noam Nisanov")

# prints: Match 'Adam Smith'
sar.match("This text contains Adam Smith")

# NOTE: print order - same order as found in text
# prints: Match 'Adam Smith'
# prints: Match 'Noam Nisanov'
sar.match("This text contains Adam Smith first, then Noam Nisanov")

# NOTE: print order - same order as found in text
# prints: Match 'Noam Nisanov'
# prints: Match 'Adam Smith'
sar.match("This text contains Noam Nisanov first, then Adam Smith")
```

* We can interact with the Regex Engine while it's running

```python
from regexp_sar import RegexpSar

def alert_data_leak(regexp_name):
    print(f"CRITICAL MATCH: {regexp_name}")

max_match_amount = 3
sar = RegexpSar()

regexps = [
    # Employee name
    "Noam\\^\\w*Nisanov", # \^\w* means not \w* (\^ is not)

    # Employee phone number
    "012\\^\\w*1234567",

    # Employee address
    "221C^\\w*Baker\\^\\w*Street",

    # All employee Ids
    "ID_112",
    "ID_113",
    "ID_114",
]

match_amount = 0
def gen_callback(regexp_name):
    def count_match(from_pos, to_pos):
        global match_amount
        match_amount += 1
        alert_data_leak(regexp_name)
        if match_amount == max_match_amount:
            print("Too many matches, stopping")
            sar.stop_match()
    return count_match

for r in regexps:
    sar.add_regexp(r, gen_callback(r))

match_str = """
Full Name: Noam Nisanov
Phone number: 012-1234567
Address: 221C, Baker-Street, London
Employee Id: ID_113

colleagues: ID_112, ID_114
"""

sar.match(match_str)
# prints
# CRITICAL MATCH: Noam\^\w*Nisanov
# CRITICAL MATCH: 012\^\w*1234567
# CRITICAL MATCH: ID_113
# Too many matches, stopping
```
