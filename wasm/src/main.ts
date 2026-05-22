import './style.css';

import mainWasm from './main.wasm?url';
import { Wasm } from './wasm';

document.addEventListener('DOMContentLoaded', async () => {
    const wasm = await Wasm.fromUrl(mainWasm);
    wasm.run();
});
