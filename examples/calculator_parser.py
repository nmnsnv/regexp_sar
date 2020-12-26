
from regexp_sar import RegexpSar

# Command to be evaluated
command = "(1 + 1   ) ^ (4 - (2 - 1))"


class StateSar:
    def __init__(self, init_state):
        super().__init__()
        self._sar = RegexpSar()
        self._state = init_state
        self._all_states = set()

    def add_regexp(self, state, regexp, regexp_callback):
        self._all_states.add(state)

        def inner_callback(from_pos, to_pos):
            if self._state == state:
                regexp_callback(from_pos, to_pos)
            self._sar.continue_from(to_pos)
        self._sar.add_regexp(regexp, inner_callback)

    def match(self, match_str):
        self._sar.match(match_str)

    def set_state(self, new_state):
        if new_state not in self._all_states:
            raise Exception("Unknown state: " + str(new_state))
        self._state = new_state

    def get_state(self):
        return self._state


stack = [[]]

state_sar = StateSar("number")


def handle_number(from_pos, to_pos):
    match_str = command[from_pos:to_pos]
    stack[-1].append(float(match_str))
    state_sar.set_state("action")


def handle_action(from_pos, to_pos):
    match_str = command[from_pos:to_pos]
    stack[-1].append(match_str)
    state_sar.set_state("number")


def handle_paren(from_pos, to_pos):
    stack.append([])
    state_sar.set_state("number")


def close_paren(from_pos, to_pos):
    cur_stack = stack.pop()
    stack[-1].append(cur_stack)
    state_sar.set_state("action")


def eval_calc(expr):
    if type(expr) is float:
        return expr

    # expr is list!
    actions = {
        "+": lambda a, b: a + b,
        "-": lambda a, b: a - b,
        "*": lambda a, b: a * b,
        "/": lambda a, b: a / b,
        "^": lambda a, b: a ** b,
    }
    clone_list = expr[:]
    while True:
        arg1_expr = clone_list.pop(0)
        if len(clone_list) == 0:
            return eval_calc(arg1_expr)
        # we ensured before if it contains an action, it'll contain another arg
        cur_action = clone_list.pop(0)
        arg2_expr = clone_list.pop(0)

        clone_list.insert(0, actions[cur_action](
            eval_calc(arg1_expr), eval_calc(arg2_expr)))


state_sar.add_regexp("number", '\d+', handle_number)

action_chars = "+*-/^"
for cur_action_char in action_chars:
    state_sar.add_regexp("action", cur_action_char, handle_action)

state_sar.add_regexp("number", "(", handle_paren)
state_sar.add_regexp("action", ")", close_paren)


state_sar.match(command)
if state_sar.get_state() != "action":
    raise Exception(f"Command is incomplete! missing {state_sar.get_state()}")
if len(stack) > 1:
    raise Exception(
        f"Command is incomplete! didnt close {len(stack)-1} parens")
stack = stack[0]

print("-------")
print(command)
print(stack)
print(f"val = {eval_calc(stack)}")
