var _globalEval = eval;

function registerUIComponent(sComponent, component)
{
    // extend "global" scope with custom Objects to mock EOS and give controlled access to
    // objects
    (function(sComponent, component) {
        this[sComponent] = component;
    })(sComponent, component);
}

function globalEval(script) {
    return (function(sScript) {
        return eval(sScript);
    })(script);
}

function wrap(script, onerror, type) {
  return "\n"+
  "(function() {try {return _globalEval(" + JSON.stringify(script) + ")}\n"+
    "catch (e) {console.error(\n"+
      "e.message,\n"+
      "e.stack,\n"+
      JSON.stringify('\nIn ' + (type || 'Script EVAL') + ':\n') + ",\n" +
      JSON.stringify(script) +
      ");return " + (onerror || '') + "}\n"+
  "})()";
}

function isolate(script, type) {
  return "\n"+
  "try {_globalEval(" + JSON.stringify(script) + ")}\n"+
    "catch (e) {console.error(\n"+
      "e.message,\n"+
      "e.stack,\n"+
      JSON.stringify('\n\nIn ' + (type || 'Script EVAL') + ':\n') + ",\n" +
      JSON.stringify(script) +
      ");\n"+
      "var hasVarsRxRes = e.message.match(/(.*) is not defined/);\n" +
      "if (hasVarsRxRes.length > 1) {\n" +
        "try{\n"+
          "eval('var ' + hasVarsRxRes[1] + '=null;');\n" +
          "eval(" + JSON.stringify(script) + ");"+
        "} catch(e) {}\n"+
      "}\n" +
    "}";
}

