import lldb
import debugger
import json
import re

mydebugger = None

def __lldb_init_module(debugger, internal_dict):
    print("__lldb_init_module")
    global mydebugger
    mydebugger = debugger

def get_result(func):
    frame = mydebugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetSelectedFrame()
    options = lldb.SBExpressionOptions()
    result = frame.EvaluateExpression(func, options)
    cstring = mydebugger.GetSelectedTarget().GetProcess().ReadCStringFromMemory(result.GetValueAsUnsigned(), 1024 * 16, lldb.SBError())
    return cstring


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
        <xmp id="dot">
            ''' + data + '''
        </xmp>
    </body>
</html>'''
    
    debugger.display_html(document, position=2)
    return True



