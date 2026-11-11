#!/usr/bin/env python3
"""Run the editing example in a disposable POSIX pseudo-terminal."""
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
        initial = read_until(lambda data: b'Multi-line input' in data, 6)
        os.write(master, b'\x1b[<0;4;4M')
        read_until(lambda _: False, .2)
        os.write(master, b'\x1b[<0;4;4m')
        read_until(lambda _: False, .2)
        text = 'e\u0301界!' if '--ascii' not in sys.argv else 'abc!'
        os.write(master, text.encode())
        single = read_until(lambda data: b'!' in data, 3)
        os.write(master, b'\x7f')
        read_until(lambda _: False, .2)
        os.write(master, b'\x1b[<0;2;7M')
        read_until(lambda _: False, .2)
        os.write(master, b'\x1b[<0;2;7m')
        read_until(lambda _: False, .2)
        os.write(master, b'first row')
        multi = read_until(lambda data: b'first row' in data, 3)
        os.write(master, b'\r')
        read_until(lambda _: False, .2)
        os.write(master, b'second row')
        multi += read_until(lambda data: b'second row' in data, 3)
        os.write(master, b'\x01')
        highlighted = read_until(lambda data: b'170;170;170' in data, 3)
        os.write(master, b'REPLACED')
        replacement = read_until(lambda data: b'REPLACED' in data, 3)
        os.write(master, b'\x1a')
        undone = read_until(lambda data: b'second row' in data, 3)
        os.write(master, b'\x19')
        redone = read_until(lambda data: b'REPLACED' in data, 3)
        os.write(master, b'\x11')
        read_until(lambda _: process.poll() is not None, 3)
        result = {
            'initial_widgets': b'Single-line input' in initial and b'Multi-line input' in initial,
            'single_typed': text.encode() in single,
            'multi_typed': b'first row' in multi and b'second row' in multi,
            'select_without_clipboard': b'170;170;170' in highlighted,
            'selection_replaced': b'REPLACED' in replacement,
            'undo_without_clipboard': b'second row' in undone,
            'redo_without_clipboard': b'REPLACED' in redone,
            'exit_code': process.wait(timeout=3),
        }
        print(json.dumps(result))
        if not all(value for key, value in result.items() if key != 'exit_code') or result['exit_code']:
            raise RuntimeError('Editing example smoke test failed')
    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        os.close(master)
