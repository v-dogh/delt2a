#!/usr/bin/env python3
"""Run the image example in a disposable POSIX pseudo-terminal."""
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
mode = sys.argv[2] if len(sys.argv) > 2 else 'kitty'
master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack('HHHH', 24, 80, 0, 0))
with tempfile.TemporaryDirectory() as directory:
    process = subprocess.Popen([str(binary), mode, '8', '16'], stdin=slave, stdout=slave, stderr=slave,
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
        marker = b'\x1b_Ga=p' if mode == 'kitty' else b'\x1bP0;1;0q'
        initial = read_until(lambda data: (marker in data and data.endswith(b'\x1b8')) if mode != 'plain'
                             else b'Text clips' in data, 8)
        os.write(master, b'\x12')
        moved = read_until(lambda data: marker in data and data.endswith(b'\x1b8'), 4 if mode != 'plain' else .3)
        os.write(master, b'\x04')
        removed = read_until(lambda data: b'a=d,d=I' in data if mode == 'kitty' else b'Images:' in data,
                             4 if mode != 'plain' else .3)
        os.write(master, b'\x0c')
        unloaded = read_until(lambda _: False, .3)
        os.write(master, b'\x11')
        final = read_until(lambda _: process.poll() is not None, 3)
        result = {
            'labels_visible': b'Images:' in initial and b'Text clips' in initial,
            'initial_graphics': marker in initial if mode != 'plain' else b'\x1b_G' not in initial and b'\x1bP' not in initial,
            'moved_graphics': marker in moved if mode != 'plain' else b'\x1b_G' not in moved and b'\x1bP' not in moved,
            'removed_graphics': b'a=d,d=I' in removed if mode == 'kitty' else marker not in removed,
            'exit_code': process.wait(timeout=3),
        }
        print(json.dumps(result))
        if not all(value for key, value in result.items() if key != 'exit_code') or result['exit_code']:
            raise RuntimeError('Image example smoke test failed')
    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        os.close(master)
