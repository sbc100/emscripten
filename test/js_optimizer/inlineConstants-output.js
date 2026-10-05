const NUM = 42;

const LONG_STR = "this string is longer than its identifier";

export const EXPORTED_CONST = 77;

function testExpressions(cmd) {
  if (cmd === 42) return true;
  if (cmd < 3.14) return false;
  if (cmd === -1) return -1n;
  if (cmd === "hi") return LONG_STR;
  switch (cmd) {
   case 1:
    return 10;

   case 2:
    return 20;
  }
  return 42 + 100n + 1024 * 1024;
}

const obj = {
  NUM: 42,
  prop: 3.14,
  [-1]: "computed",
  NUM: "uncomputed key should not be changed"
};

function testMember(x) {
  var a = x[42];
  var b = x.NUM;
  return a + b;
}

function testParamShadow(NUM) {
  return NUM;
}

function testLocalConstShadow() {
  return 100;
}

function testLocalVarShadow() {
  if (true) {
    var NUM = 200;
  }
  return NUM;
}

function testCatchShadow() {
  try {
    throw 1;
  } catch (NUM) {
    return NUM;
  }
}

const arrow = x => x + 42 + 77;

export { NUM };
