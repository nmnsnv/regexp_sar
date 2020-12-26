
from regexp_sar import _regexp_sar_c


class RegexpSar(object):
    def __init__(self):
        super().__init__()

        # NOTE: _internal_sar_object is automatically cleaned when has no more references
        self._internal_sar_object = _regexp_sar_c.create_object()

    def add_regexp(self, regexp, callback):
        _regexp_sar_c.add_regexp(self._internal_sar_object, regexp, callback)

    def match_from(self, text, from_position):
        _regexp_sar_c.match_from(self._internal_sar_object, text, from_position)

    def match_at(self, text, at_position):
        _regexp_sar_c.match_at(self._internal_sar_object, text, at_position)

    def match(self, text):
        self.match_from(text, 0)

    def continue_from(self, from_pos):
        _regexp_sar_c.continue_from(self._internal_sar_object, from_pos)

    def stop_match(self):
        _regexp_sar_c.stop_match(self._internal_sar_object)
