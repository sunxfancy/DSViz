"""Exercise debugger transport without requiring a running LLDB session."""
import importlib.util
from pathlib import Path
import sys
import types
import unittest

class Error:
    def __init__(self, success=True):
        self.success = success
    def Success(self):
        return self.success

sys.modules['lldb'] = types.SimpleNamespace(SBError=Error, SBExpressionOptions=object)
sys.modules['debugger'] = types.SimpleNamespace(display_html=lambda *a, **kw: None)
spec = importlib.util.spec_from_file_location('dsv_debug', Path(__file__).parents[1] / 'example/debug.py')
debug = importlib.util.module_from_spec(spec)
spec.loader.exec_module(debug)

class Session:
    def __init__(self, data, fail_expression=None, address=4096, size=None, read_ok=True):
        self.data = data
        self.fail_expression = fail_expression
        self.address = address
        self.size = len(data) if size is None else size
        self.read_ok = read_ok
        self.read_size = None
    def __getattr__(self, name):
        if name.startswith('Get'):
            return lambda: self
        raise AttributeError(name)
    def EvaluateExpression(self, expr, options):
        value = self.size if expr == '_dotToDebuggerSize()' else self.address
        return types.SimpleNamespace(GetError=lambda: Error(expr != self.fail_expression), GetValueAsUnsigned=lambda: value)
    def ReadMemory(self, address, size, error):
        self.read_size = size
        error.success = self.read_ok
        return self.data

class TransportTests(unittest.TestCase):
    def test_expression_failure(self):
        debug.mydebugger = Session(b'', fail_expression='graph()')
        with self.assertRaises(RuntimeError): debug.get_result('graph()')
    def test_length_expression_failure(self):
        debug.mydebugger = Session(b'', fail_expression='_dotToDebuggerSize()')
        with self.assertRaises(RuntimeError): debug.get_result('graph()')
    def test_null_pointer(self):
        debug.mydebugger = Session(b'', address=0)
        with self.assertRaisesRegex(RuntimeError, 'null pointer'): debug.get_result('graph()')
    def test_oversized(self):
        debug.mydebugger = Session(b'', size=64 * 1024 * 1024 + 1)
        with self.assertRaisesRegex(RuntimeError, '64 MiB'): debug.get_result('graph()')
        self.assertIsNone(debug.mydebugger.read_size)
    def test_memory_failure(self):
        debug.mydebugger = Session(b'data', read_ok=False)
        with self.assertRaisesRegex(RuntimeError, 'complete visualization'): debug.get_result('graph()')
    def test_short_read(self):
        debug.mydebugger = Session(b'data', size=10)
        with self.assertRaisesRegex(RuntimeError, 'complete visualization'): debug.get_result('graph()')
    def test_invalid_utf8(self):
        debug.mydebugger = Session(b'\xff')
        with self.assertRaises(UnicodeDecodeError): debug.get_result('graph()')
    def test_large_utf8(self):
        payload = ('图' * 10000).encode()
        session = Session(payload)
        debug.mydebugger = session
        self.assertEqual(debug.get_result('graph()'), payload.decode())
        self.assertEqual(session.read_size, len(payload))
    def test_empty(self):
        debug.mydebugger = Session(b'')
        self.assertEqual(debug.get_result('graph()'), '')
    def test_html_text(self):
        debug.mydebugger = Session(b'</pre><script>alert(1)</script>')
        output = []
        debug.debugger.display_html = lambda doc, **kw: output.append(doc)
        debug.plot()
        self.assertIn('&lt;/pre&gt;&lt;script&gt;', output[0])
        self.assertNotIn('<script>alert(1)</script>', output[0])

if __name__ == '__main__':
    unittest.main()
