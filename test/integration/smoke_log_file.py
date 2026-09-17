"""Does --log-file put the log where it was told to?

The name is validated and the path built without the game running, and that part
has unit tests. What only a real run shows is the other half: that the sink opens
under logs/, that it makes the directories nobody created, and that lines land in
it rather than in the rotating file.

    $ python smoke_log_file.py
"""

import pathlib
import sys
import time

import machines

NAME = "events/smoke-{}.log".format(time.strftime("%Y%m%d-%H%M%S"))


def main():
    logs = pathlib.Path(machines.data_dir()) / "logs"
    wanted = logs / NAME
    rotating = logs / "machines.txt"

    #  The subdirectory is left out on purpose: making it is the game's job.
    if wanted.exists():
        wanted.unlink()

    rotating_before = rotating.stat().st_mtime if rotating.exists() else None

    print(f"launching with --log-file={NAME}")
    with machines.Machines(extra_args=[f"--log-file={NAME}"]) as game:
        game.console("gfx_preset low")

    if not wanted.exists():
        print(f"FAIL: nothing at {wanted}")
        return 1

    text = wanted.read_text(errors="replace")
    if "Starting" not in text:
        print(f"FAIL: {wanted} has no startup line; {len(text)} bytes")
        return 1

    print(f"ok: {wanted} holds {len(text)} bytes, {len(text.splitlines())} lines")

    #  A named log that quietly also rotated the shared one would still destroy
    #  the batch it was meant to protect.
    rotating_after = rotating.stat().st_mtime if rotating.exists() else None
    if rotating_after != rotating_before:
        print(f"FAIL: {rotating} was written too")
        return 1

    print(f"ok: {rotating.name} untouched")

    return 0


if __name__ == "__main__":
    sys.exit(main())
