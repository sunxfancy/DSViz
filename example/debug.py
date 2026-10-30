import lldb
import debugger
import html

mydebugger = None

def __lldb_init_module(debugger, internal_dict):
    print("__lldb_init_module")
    global mydebugger
    mydebugger = debugger

def get_result(func):
    frame = mydebugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetSelectedFrame()
    options = lldb.SBExpressionOptions()
    result = frame.EvaluateExpression(func, options)
    if not result.GetError().Success():
        raise RuntimeError(str(result.GetError()))
    process = mydebugger.GetSelectedTarget().GetProcess()
    address = result.GetValueAsUnsigned()
    if not address:
        raise RuntimeError("Visualization returned a null pointer")
    size_result = frame.EvaluateExpression('_dotToDebuggerSize()', options)
    if not size_result.GetError().Success():
        raise RuntimeError(str(size_result.GetError()))
    size = size_result.GetValueAsUnsigned()
    if size > 64 * 1024 * 1024:
        raise RuntimeError("Visualization exceeds the 64 MiB debugger limit")
    if not size:
        return ""
    error = lldb.SBError()
    data = process.ReadMemory(address, size, error)
    if not error.Success() or len(data) != size:
        raise RuntimeError("Could not read complete visualization: " + str(error))
    return data.decode("utf-8")


def plot():
    print("plot called")
    data = get_result('_dotToDebugger(b)')
    print(data)
    document = '''
<html>
    <head>
        <script src="https://unpkg.com/@viz-js/viz@3.2.3/lib/viz-standalone.js"></script>
        <script>
            function render() {
                var node = document.getElementById("dot");
                var data = node.textContent;
                node.parentNode.removeChild(node);
                console.log(data);
                Viz.instance().then(function(viz) {
                    var svg = viz.renderSVGElement(data);
                    document.getElementById("graph").appendChild(svg);
                }).catch(error => {
                    console.error(error);
                });
            }
        </script>
    </head>
    <body onLoad='render()'>
        <div id="graph"></div>
        <pre id="dot" hidden>
            ''' + html.escape(data) + '''
        </pre>
    </body>
</html>'''
    
    debugger.display_html(document, position=2)
    return True



