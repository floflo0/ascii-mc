import { Wasm } from './wasm';
import mainWasm from './main.wasm?url';
import './style.css'

document.addEventListener('DOMContentLoaded', async () => {
    const wasm = await Wasm.fromUrl(mainWasm);
    wasm.run();
});
