
from regexp_sar import RegexpSar

with open("./c_code.c", "r") as c_fh:
    c_content = c_fh.read()

required_method = "malloc"

sar = RegexpSar()

last_found_word = None


def found_name(from_pos, to_pos):
    global last_found_word
    last_found_word = c_content[from_pos: to_pos]
    sar.continue_from(to_pos)


sar.add_regexp("\\w+", found_name)

# name of last encountered function
last_found_function = None


def found_function(from_pos, to_pos):
    global last_found_function
    last_found_function = last_found_word


sar.add_regexp("(", found_function)

# name of function we are currently in its body (with its body)
inside_function_name = None
curly_bracket_count = 0


def handle_open_curly_bracket(from_pos, to_pos):
    global curly_bracket_count, inside_function_name
    if curly_bracket_count == 0:
        inside_function_name = last_found_function
    curly_bracket_count += 1


sar.add_regexp("{", handle_open_curly_bracket)


def handle_close_curly_bracket(from_pos, to_pos):
    global curly_bracket_count
    curly_bracket_count -= 1


sar.add_regexp("}", handle_close_curly_bracket)


def handle_required_method_found(from_pos, to_pos):
    print(f"found at: {inside_function_name}")


sar.add_regexp(f"\\^\\w{required_method}\\^\\w", handle_required_method_found)

sar.match(c_content)
