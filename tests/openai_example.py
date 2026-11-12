#!/usr/bin/env python3
"""Exercise the headless inference example against a local Chat Completions fixture."""
import http.server
import json
import os
import subprocess
import sys
import threading


requests = []


class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *args):
        pass

    def do_POST(self):
        request = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        requests.append(request)
        if len(requests) == 1:
            delta = {'role': 'assistant', 'tool_calls': [{
                'index': 0, 'id': 'add-1', 'type': 'function',
                'function': {'name': 'add', 'arguments': '{"a":2,"b":3}'}}]}
            finish = 'tool_calls'
        else:
            delta = {'content': 'The answer is 5.' if len(requests) == 2 else 'Checked with another model.'}
            finish = 'stop'
        body = ('data: ' + json.dumps({'id': 'completion-' + str(len(requests)),
            'model': request['model'], 'choices': [{'index': 0, 'delta': delta, 'finish_reason': finish}]}) +
            '\n\ndata: [DONE]\n\n').encode()
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)


with http.server.HTTPServer(('127.0.0.1', 0), Handler) as server:
    worker = threading.Thread(target=server.serve_forever, daemon=True)
    worker.start()
    try:
        env = {key: value for key, value in os.environ.items() if not key.startswith('D2_OPENAI_')}
        result = subprocess.run([sys.argv[1], f'http://127.0.0.1:{server.server_port}/v1/chat/completions',
            'first-model', 'Add 2 and 3', 'second-model'], capture_output=True, text=True, env=env, timeout=10)
        assert result.returncode == 0, result.stderr
        assert len(requests) == 3
        assert requests[1]['messages'][-1]['tool_call_id'] == 'add-1'
        assert requests[1]['messages'][-1]['content'] == '5'
        assert requests[2]['model'] == 'second-model'
        assert 'The answer is 5.' in result.stdout and 'Checked with another model.' in result.stdout
        print(json.dumps({'tool_round_trip': True, 'model_switch': True, 'exit_code': result.returncode}))
    finally:
        server.shutdown()
        worker.join()
