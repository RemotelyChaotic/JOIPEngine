var _globalEval = eval;

function evalInContext(script, context) {
    return function() {
        return eval(script);
    }.call(context);
}

function evalInScope(scipt, contextAsScope) {
    return function() {
        with(contextAsScope) {
            return eval(scipt);
        }
    }.call(contextAsScope);
}

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
  "(function() {"+
    "try {return _globalEval(" + JSON.stringify(script) + ")}\n"+
    "catch (e) {"+
      "console.error(\n"+
        "e.message, e.stack,\n"+
        JSON.stringify('\nIn ' + (type || 'Script EVAL') + ':\n') + ",\n" +
        JSON.stringify(script) +
        ");"+
      "return " + (onerror || '') + "}\n"+
  "})()";
}

function isolate(script, type) {
  return "\n"+
  "try {_globalEval(" + JSON.stringify(script) + ")}\n"+
  "catch (e) {\n"+
    "console.error(\n"+
      "e.message,\n"+
      "e.stack,\n"+
      JSON.stringify('\n\nIn ' + (type || 'Script EVAL') + ':\n') + ",\n" +
      JSON.stringify(script) +
    ");\n"+
  "}";
}

