import type { Int, Ptr, Uint32 } from './types';

export type WasmMain = () => void;
export type RunCallback = (callback: Ptr) => void;
export type RunCallbackUint32 = (callback: Ptr, param: Uint32) => void;
export type RunCallbackInt = (callback: Ptr, param: Int) => void;
export type RunCallbackIntInt = (
    callback: Ptr,
    param1: Int,
    param2: Int,
) => void;

export interface Exports extends WebAssembly.Exports {
    wasm_main: WasmMain;
    run_callback: RunCallback;
    run_callback_uint32: RunCallbackUint32;
    run_callback_int: RunCallbackInt;
    run_callback_int_int: RunCallbackIntInt;
}
