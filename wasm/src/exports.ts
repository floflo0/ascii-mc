import type { Ptr } from './types';

export type WasmMain = () => void;
export type RunCallback = (callback: Ptr) => void;
export type RunCallbackInt = (callback: Ptr, param: number) => void;
export type RunCallbackPtr = (callback: Ptr, param: Ptr) => void;

export interface Exports extends WebAssembly.Exports {
    wasm_main: WasmMain;
    run_callback: RunCallback;
    run_callback_int: RunCallbackInt;
    run_callback_ptr: RunCallbackPtr;
}
