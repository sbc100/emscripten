/*
 * Helper function used in browser tests to simulate HTML5 events
 */

const codeToKeyCode = {
  'Backspace': 8,
  'Tab': 9,
  'ShiftLeft': 16,
  'CtrlLeft': 17,
  'AltLeft': 18,
  'Escape': 27,
  'KeyA': 65,
  'KeyB': 66,
  'ArrowLeft': 37,
  'ArrowUp': 38,
  'ArrowRight': 39,
  'ArrowDown': 40,
  'Numpad4': 100,
  'F1': 112,
};

const codeToKey = {
  'KeyA': 'a',
  'KeyB': 'b',
  'AltLeft': 'Alt',
  'CtrlLeft': 'Ctrl',
  'ShiftLeft': 'Shift',
  'Backspace': 'Backspace',
  'Tab': 'Tab',
  'Numpad4': 'Numpad4',
  'ArrowUp': 'ArrowUp',
  'ArrowDown': 'ArrowDown',
  'ArrowLeft': 'ArrowLeft',
  'ArrowRight': 'ArrowRight',
  'F1': 'F1',
  'Escape': 'Escape',
};


function simulateKeyEvent(eventType, code, target) {
  const keyCode = codeToKeyCode[code];
  if (!keyCode) throw new Error("unhandled code: " + code);
  const key = codeToKey[code];
  if (!key) throw new Error("unhandled code: " + code);
  var props = { code, key, keyCode, charCode: keyCode, view: window, bubbles: true, cancelable: true };
  console.error("simulateKeyEvent", eventType, props);
  var event = new KeyboardEvent(eventType, props);
  if (!target) target = document;
  return target.dispatchEvent(event);
}

function simulateKeyDown(code, target = undefined) {
  var doDefault = simulateKeyEvent('keydown', code, target);
  // As long as not handler called `preventDefault` we also send a keypress
  // event.
  if (doDefault) {
    simulateKeyEvent('keypress', code, target);
  }
}

function simulateKeyUp(code, target = undefined) {
  simulateKeyEvent('keyup', code, target);
}

function simulateKeyDownUp(code, target = undefined) {
  simulateKeyDown(code, target);
  simulateKeyUp(code, target);
}

function simulateMouseEvent(eventType, x, y, button, absolute) {
  if (!absolute) {
    x += Module['canvas'].offsetLeft;
    y += Module['canvas'].offsetTop;
  }
  var event = document.createEvent("MouseEvents");
  event.initMouseEvent(eventType, true, true, window,
             1, x, y, x, y,
             0, 0, 0, 0,
             button, null);
  Module['canvas'].dispatchEvent(event);
}

function simulateMouseDown(x, y, button, absolute) {
  simulateMouseEvent('mousedown', x, y, button, absolute);
}

function simulateMouseUp(x, y, button, absolute) {
  simulateMouseEvent('mouseup', x, y, button, absolute);
}

function simulateMouseMove(x, y, absolute) {
  simulateMouseEvent('mousemove', x, y, 0, absolute);
}

function simulateMouseClick(x, y, button, absolute) {
  simulateMouseDown(x, y, button, absolute);
  simulateMouseUp(x, y, button, absolute);
}
