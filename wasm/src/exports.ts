import type { Ptr } from './types';

export interface Exports extends WebAssembly.Exports {
    wasm_main(): Promise<void>;
    run_callback(callback: Ptr): void;
    run_callback_int(callback: Ptr, param: number): void;
    run_callback_ptr(callback: Ptr, param: Ptr): void;
    __heap_base: { value: number };
}
