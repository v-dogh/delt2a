#!/usr/bin/env python3
"""Run the selection example in a disposable POSIX pseudo-terminal."""
import fcntl
import json
import os
import pathlib
import pty
import select
import struct
import subprocess
import sys
import tempfile
import termios
import time


binary = pathlib.Path(sys.argv[1]).resolve()
master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack('HHHH', 24, 80, 0, 0))
with tempfile.TemporaryDirectory() as directory:
    process = subprocess.Popen([str(binary)], stdin=slave, stdout=slave, stderr=slave,
                               cwd=directory, start_new_session=True,
                               env={**os.environ, 'TERM': 'xterm-256color'})
    os.close(slave)

    def read_until(predicate, timeout):
        output = bytearray()
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if select.select([master], [], [], min(.2, max(0, deadline - time.monotonic())))[0]:
                try:
                    part = os.read(master, 65536)
                except OSError:
                    break
                if not part:
                    break
                output.extend(part)
                if predicate(output):
                    break
            if process.poll() is not None:
                break
        return bytes(output)

    try:
        initial = read_until(lambda data: b'Combining:' in data or b'ASCII mode' in data, 6)
        os.write(master, b'\x1b[<0;2;2M')
        read_until(lambda _: False, .2)
        os.write(master, b'\x1b[<32;5;9M\x1b[<0;5;9m')
        dragged = read_until(lambda data: b'170;170;170' in data, 3)
        os.write(master, b'\x11')
        read_until(lambda _: process.poll() is not None, 3)
        result = {
            'first_paragraph': b'Drag across' in initial,
            'second_paragraph': b'Combining:' in initial or b'ASCII mode' in initial,
            'highlighted': b'170;170;170' in dragged,
            'exit_code': process.wait(timeout=3),
        }
        print(json.dumps(result))
        if not all(result[key] for key in ('first_paragraph', 'second_paragraph', 'highlighted')) or result['exit_code']:
            raise RuntimeError('Selection example smoke test failed')
    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        os.close(master)
