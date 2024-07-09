/*
 * Helper function used in browser tests to simulate HTML5 events
 */

function simulateKeyEvent(eventType, code, key) {
  if (key) {
    charCode = key.charCodeAt(0)
  } else {
    key = code;
    charCode = 0;
  }
  var event = new KeyboardEvent(eventType, { code, key, charCode, view: window, bubbles: true, cancelable: true });
  return document.dispatchEvent(event);
}

function simulateKeyDown(code, key) {
  var doDefault = simulateKeyEvent('keydown', code, key);
  // As long as not handler called `preventDefault` we also send a keypress
  // event.
  if (doDefault) {
    simulateKeyEvent('keypress', code, key);
  }
}

function simulateKeyUp(code, key) {
  simulateKeyEvent('keyup', code, key);
}

function simulateMouseEvent(x, y, button, absolute) {
  if (!absolute) {
    x += Module['canvas'].offsetLeft;
    y += Module['canvas'].offsetTop;
  }
  var event = document.createEvent("MouseEvents");
  if (button >= 0) {
    var event1 = document.createEvent("MouseEvents");
    event1.initMouseEvent('mousedown', true, true, window,
               1, x, y, x, y,
               0, 0, 0, 0,
               button, null);
    Module['canvas'].dispatchEvent(event1);
    var event2 = document.createEvent("MouseEvents");
    event2.initMouseEvent('mouseup', true, true, window,
               1, x, y, x, y,
               0, 0, 0, 0,
               button, null);
    Module['canvas'].dispatchEvent(event2);
  } else {
    var event1 = document.createEvent("MouseEvents");
    event1.initMouseEvent('mousemove', true, true, window,
               1, x, y, x, y,
               0, 0, 0, 0,
               0, null);
    Module['canvas'].dispatchEvent(event1);
  }
}
