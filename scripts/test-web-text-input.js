// Drive the shipping EM_JS input bridge without a browser or npm dependencies.
const fs = require('node:fs');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const path = require('node:path');
const source = fs.readFileSync(path.join(__dirname, '../cpp/third_party/cpp_canvas/window.cpp'), 'utf8');
const body = source.match(/EM_JS\(void, web_soft_keyboard, \(int wanted\), \{([\s\S]*?)\n\}\);/)[1];
let node;
const timers = [];
const document = {
    activeElement: null,
    getElementById: () => node,
    createElement: () => ({
        style: {}, listeners: {}, setAttribute() {},
        addEventListener(type, fn) { this.listeners[type] = fn; },
        setSelectionRange(start, end) { this.selectionStart = start; this.selectionEnd = end; },
        focus() { document.activeElement = this; },
        blur() { document.activeElement = null; },
    }),
    body: { appendChild(element) { node = element; } },
};
const Module = {};
vm.runInNewContext(`(function(wanted) {${body}\n})(1)`, {
    document, Module, performance: { now: () => 10 }, setTimeout: fn => timers.push(fn),
});
const input = (value, inputType = 'insertText', data = null) => {
    node.value = value;
    node.listeners.input({ inputType, data });
};
const take = () => {
    const result = [Module.cppSoftErase, Module.cppSoftText];
    Module.cppSoftErase = 0;
    Module.cppSoftText = '';
    return result;
};
input(' abc');
assert.deepEqual(take(), [0, 'abc']);
assert.equal(node.value, ' ');
// A canvas caret move must not turn the old DOM suffix into an insertion.
node.setSelectionRange(0, 0);
node.listeners.beforeinput({ inputType: 'insertText', data: 'X' });
assert.equal(node.selectionStart, 1);
input(' X');
assert.deepEqual(take(), [0, 'X']);
input('');
assert.deepEqual(take(), [1, '']);
input('');
assert.deepEqual(take(), [1, '']);
input(' paste', 'insertFromPaste');
assert.deepEqual(take(), [0, '']);
// Final input before compositionend.
node.listeners.compositionstart();
input(' n', 'insertCompositionText');
input(' ni', 'insertCompositionText');
input(' 你', 'insertCompositionText');
node.listeners.compositionend();
assert.deepEqual(take(), [0, '你']);
timers.splice(0).forEach(fn => fn());
// Final input after compositionend.
node.listeners.compositionstart();
input(' hao', 'insertCompositionText');
node.listeners.compositionend();
input(' 好', 'insertFromComposition');
assert.deepEqual(take(), [0, '好']);
timers.splice(0).forEach(fn => fn());
input(' a');
input('');
assert.deepEqual(take(), [0, '']);
// The C++ key handler owns physical erase keys; DOM must not also delete.
assert(source.includes('if (impl->softKeyboardUp && (key == Key::Backspace || key == Key::Delete)) return EM_TRUE;'));
console.log('Web text input regression checks passed.');
