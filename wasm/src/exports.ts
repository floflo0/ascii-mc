import type { Ptr, Uint32 } from './types';

export type WasmMain = () => void;
export type RunCallback = (callback: Ptr) => void;
export type RunCallbackUint32 = (callback: Ptr, param: Uint32) => void;

export interface Exports extends WebAssembly.Exports {
    wasm_main: WasmMain;
    run_callback: RunCallback;
    run_callback_uint32: RunCallbackUint32;
}
