import { Wasm } from './wasm';
import mainWasm from './main.wasm?url';
import './style.css'

const wasm = new Wasm(mainWasm);
await wasm.init();
wasm.run();
