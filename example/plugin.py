import re
from clang.cindex import Cursor

messages = []


def end_object(cursor: Cursor, name: str):
    comment = cursor.raw_comment
    if not comment:
        return

    comment = re.sub(r"(?://)|(?:/\*)|(?:\*/)", "", comment)
    comment = comment.strip()

    match_obj = re.search(r"cmd +?= +?(\S+)", comment)
    if match_obj:
        cmd = match_obj[1]
        messages.append((cmd, name))


CASE_TPL = """  case $cmd: return $name;"""
CODE_TPL = """
/* plugin output */
const struct clColumn* get_message_def(int cmd)
{
  switch (cmd)
  {
$cases

  default: return NULL;
  }
}
"""
HEADER_STR = """
/* plugin output */
const struct clColumn* get_message_def(int cmd);
"""


def render_tpl(tpl: str, data: dict[str, str]) -> str:
    result = tpl

    for k, v in data.items():
        result = re.sub(f"\\${k}\\r?\\n?", v, result)

    return result


def complete():
    cases = []

    for cmd, name in messages:
        print(f"cmd: {cmd}, {name}")

        line = render_tpl(
            CASE_TPL,
            {
                "cmd": cmd,
                "name": name,
            },
        )

        cases.append(line)

    code = render_tpl(
        CODE_TPL,
        {
            "cases": "\n".join(cases),
        },
    )

    return (HEADER_STR, code)
