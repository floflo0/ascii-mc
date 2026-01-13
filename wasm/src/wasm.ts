import { Exit } from './exit';
import type { Exports, RunCallback, RunCallbackInt, RunCallbackPtr } from './exports';
import { JS_NULL, JsObjectsMemory, type JS_Object } from './js-object-memory';
import { Terminal } from './terminal';
import type { Ptr, SizeT, StringPtr } from './types';

const MEMORY_SIZE: number = 15625;  // 1024 MB

const NULL: Ptr = 0;

export class Wasm {

    private readonly memory: WebAssembly.Memory;
    private readonly memoryView: Uint8Array;
    private readonly terminal: Terminal = new Terminal();
    private readonly jsObjectsMemory: JsObjectsMemory = new JsObjectsMemory();
    private readonly textDecoder = new TextDecoder();
    private readonly textEncoder = new TextEncoder();

    private mainWasm: () => void = () => {
        console.assert(false, "mainWasm was not loaded");
    };
    private runCallback: RunCallback = (_callback) => {
        console.assert(false, "runCallback was not loaded");
    };
    private runCallbackInt: RunCallbackInt = (_callback, _param) => {
        console.assert(false, "runCallbackInt was not loaded");
    }
    private runCallbackPtr: RunCallbackPtr = (_callback, _ptr) => {
        console.assert(false, "runCallbackPtr was not loaded");
    };

    private constructor(memory: WebAssembly.Memory) {
        this.memory = memory;
        this.memoryView = new Uint8Array(this.memory.buffer);
    }

    public static async fromUrl(wasmUrl: string): Promise<Wasm> {
        const memory = new WebAssembly.Memory({
            initial: MEMORY_SIZE,
            maximum: MEMORY_SIZE,
        });
        const wasm = new Wasm(memory);
        const wasmI = await WebAssembly.instantiateStreaming(
            fetch(wasmUrl),
            { env: wasm.buildEnv() },
        );
        wasm.setInstance(wasmI.instance);
        return wasm;
    }

    public run(): void {
        try {
            this.mainWasm();
        } catch (error) {
            if (error instanceof Exit) {
                if (error.status != 0) {
                    this.terminal.printMessage(
                        `Process exit with ${error.status} code.\n\n` +
                        'Refresh the page to restart the game.',
                    );
                }
                return;
            }

            throw error;
        }

        console.assert(false, 'unreachable');
    }

    private setInstance(instance: WebAssembly.Instance): void {
        const {
            wasm_main,
            run_callback,
            run_callback_int,
            run_callback_ptr,
        } = instance.exports as Exports;
        this.mainWasm = wasm_main;
        this.runCallback = run_callback;
        this.runCallbackInt = run_callback_int;
        this.runCallbackPtr = run_callback_ptr;
    }

    private buildEnv() {
        return {
            memory: this.memory,
            JS_Array_foreach: (arrayIndex: JS_Object, callback: Ptr) => {
                console.assert(arrayIndex !== JS_NULL);
                console.assert(callback !== NULL);
                const array =
                    this.jsObjectsMemory.get(arrayIndex) as Array<object>;
                for (let i = 0; i < array.length; ++i) {
                    this.runCallbackInt(
                        callback,
                        this.jsObjectsMemory.add(array[i])
                    )
                }
            },
            JS_console_log: (ptr: StringPtr) => {
                console.log(this.str(ptr));
            },
            JS_console_error: (ptr: StringPtr) => {
                console.error(this.str(ptr));
            },
            JS_requestAnimationFrame: (callback: Ptr): number => {
                console.assert(callback != NULL);
                return requestAnimationFrame(() => this.runCallback(callback));
            },
            JS_requestAnimationFrame_data: (
                callback: Ptr,
                data: Ptr,
            ): number => {
                console.assert(callback != NULL);
                return requestAnimationFrame(() => {
                    this.runCallbackPtr(callback, data);
                });
            },
            JS_cancelAnimationFrame: cancelAnimationFrame,
            JS_Date_now: () => BigInt(Date.now()),
            JS_performance_now: () => Number(performance.now()),
            pow: Math.pow,
            powf: Math.pow,
            cosf: Math.cos,
            sinf: Math.sin,
            tanf: Math.tan,
            atanf: Math.atan,
            roundf: Math.round,
            isnanf: isNaN,
            fmod: (x: number, y: number) => x % y,
            fmodf: (x: number, y: number) => x % y,
            JS_navigator_getGamepads: () => {
                return this.jsObjectsMemory.add(navigator.getGamepads());
            },
            JS_GamepadArray_get: (arrayIndex: JS_Object,
                                  gamepadIndex: number): JS_Object => {
                console.assert(arrayIndex !== JS_NULL);
                console.assert(gamepadIndex !== JS_NULL);
                const array =
                    this.jsObjectsMemory.get(arrayIndex) as (Gamepad | null)[];
                console.assert(array != null);
                return this.jsObjectsMemory.add(array[gamepadIndex]);
            },
            JS_Gamepad_get_id: (gamepadIndex: JS_Object, id: StringPtr,
                                idSize: SizeT) => {
                const gamepad =
                    this.jsObjectsMemory.get(gamepadIndex) as Gamepad | null;
                console.assert(gamepad !== null);
                console.assert(gamepad!.id.length < idSize);
                const gamepadId = gamepad!.id;
                const encodedId = this.textEncoder.encode(gamepadId + "\0");
                if (idSize < encodedId.length) {
                    throw new Error(
                        `buffer too small to contain gamepad id '${gamepadId}'`
                    );
                }
                this.memoryView.set(encodedId, id);
            },
            JS_Gamepad_get_index: (gamepadIndex: JS_Object): number => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.index;
            },
            JS_Gamepad_get_connected: (gamepadIndex: JS_Object): boolean => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.connected;
            },
            JS_Gamepad_get_axe: (
                gamepadIndex: JS_Object,
                axeIndex: number,
            ): number => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.axes[axeIndex];
            },
            JS_Gamepad_get_button: (gamepadIndex: JS_Object,
                buttonIndex: number,
            ): boolean => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.buttons[buttonIndex].pressed;
            },
            JS_Gamepad_get_axes_length: (gamepadIndex: number): number => {
                console.assert(gamepadIndex !== JS_NULL);
                const gamepad =
                    this.jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad.axes.length;
            },
            JS_Object_free: (objectIndex: JS_Object) => {
                console.assert(objectIndex != JS_NULL);
                this.jsObjectsMemory.remove(objectIndex);
            },
            JS_write: (bufferPtr: Ptr, count: SizeT) => {
                this.terminal.setContent(
                    this.textDecoder.decode(this.memoryView.subarray(
                        bufferPtr,
                        bufferPtr + count,
                    )),
                );
            },
            JS_read_char: () => this.terminal.readChar(),
            JS_get_memory_size: (): SizeT => this.memory.buffer.byteLength,
            JS_get_terminal_width: (): number => this.terminal.width,
            JS_get_terminal_height: (): number => this.terminal.height,
            _exit: (status: number) => {
                throw new Exit(status);
            },
        };
    }

    private str(ptr: StringPtr): string {
        let end = ptr;
        while (this.memoryView[end]) ++end;
        return this.textDecoder.decode(this.memoryView.subarray(ptr, end));
    };
}
