var _globalEval = eval;

function init(storage) {
    // extend "global" scope with custom Objects to mock EOS and give controlled access to
    // objects
    (function(storage) {
        this.teaseStorage = storage;
    })(storage);
}

function globalEval(script) {
    return eval(script);
}

function wrap(script, onerror, type) {
  return "\n"+
  "(function() {try {return _globalEval(" + JSON.stringify(script) + ")}\n"+
    "catch (e) {console.error(\n"+
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
      "e.stack,\n"+
      JSON.stringify('\n\nIn ' + (type || 'Script EVAL') + ':\n') + ",\n" +
      JSON.stringify(script) +
      ")}";
}

