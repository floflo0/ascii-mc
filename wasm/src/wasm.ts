import { instantiateStreaming } from 'asyncify-wasm';

import { Exit } from './exit';
import type { Exports } from './exports';
import { JS_NULL, JsObjectsMemory, type JS_Object } from './js-object-memory';
import { Terminal } from './terminal';
import type { Ptr, SizeT, StringPtr } from './types';
import { copyUint8Array } from './utils';

const MEMORY_SIZE: number = 15625;  // 1024 MB

const NULL: Ptr = 0;

export class Wasm {

    #wasm: WebAssembly.WebAssemblyInstantiatedSource | null = null;
    readonly #wasmPath: string;
    readonly #memory: WebAssembly.Memory;
    readonly #terminal: Terminal;
    readonly #jsObjectsMemory: JsObjectsMemory;

    constructor(wasmPath: string) {
        this.#wasmPath = wasmPath;
        this.#memory = new WebAssembly.Memory({
            initial: MEMORY_SIZE,
            maximum: MEMORY_SIZE,
        });
        this.#terminal = new Terminal();
        this.#jsObjectsMemory = new JsObjectsMemory();
    }

    async init() {
        this.#wasm = await instantiateStreaming(
            fetch(this.#wasmPath),
            { env: this.#buildEnv() },
        );
    }

    async run() {
        console.assert(this.#wasm !== null);
        const { wasm_main } = this.#wasm!.instance.exports as Exports;

        try {
            await wasm_main();
        } catch (error) {
            if (error instanceof Exit) {
                this.#terminal.printMessage(
                    `Process exit with ${error.status} code.\n\n` +
                    'Refresh the page to restart the game.',
                );
                return;
            }

            throw error;
        }

        console.assert(false, 'unreachable');
    }

    #buildEnv() {
        return {
            memory: this.#memory,
            JS_Array_foreach: (arrayIndex: JS_Object, callback: Ptr) => {
                console.assert(this.#wasm !== null);
                console.assert(arrayIndex !== JS_NULL);
                console.assert(callback !== NULL);
                const { run_callback_int } =
                    this.#wasm!.instance.exports as Exports;
                const array =
                    this.#jsObjectsMemory.get(arrayIndex) as Array<object>;
                array.forEach(value => run_callback_int(
                    callback,
                    this.#jsObjectsMemory.add(value)
                ));
            },
            JS_console_log: (ptr: StringPtr) => {
                console.log(this.#str(ptr));
            },
            JS_console_error: (ptr: StringPtr) => {
                console.error(this.#str(ptr));
            },
            JS_requestAnimationFrame: (callback: Ptr): number => {
                console.assert(callback != NULL);
                return requestAnimationFrame(() => {
                    console.assert(this.#wasm !== null);
                    const { run_callback } =
                        this.#wasm!.instance.exports as Exports;
                    run_callback(callback);
                });
            },
            JS_requestAnimationFrame_data: (
                callback: Ptr,
                data: Ptr,
            ): number => {
                console.assert(callback != NULL);
                return requestAnimationFrame(() => {
                    console.assert(this.#wasm !== null);
                    const { run_callback_ptr } =
                        this.#wasm!.instance.exports as Exports;
                    run_callback_ptr(callback, data);
                });
            },
            JS_cancelAnimationFrame: cancelAnimationFrame,
            JS_Date_now: () => BigInt(Date.now()),
            JS_performance_now: () => Number(performance.now()),
            JS_Math_pow: Math.pow,
            JS_Math_cos: Math.cos,
            JS_Math_sin: Math.sin,
            JS_Math_tan: Math.tan,
            JS_Math_atan: Math.atan,
            JS_Math_round: Math.round,
            JS_isNaN: isNaN,
            JS_fmod: (x: number, y: number) => x % y,
            JS_navigator_getGamepads: () => {
                return this.#jsObjectsMemory.add(navigator.getGamepads());
            },
            JS_GamepadArray_get: (arrayIndex: JS_Object,
                                  gamepadIndex: number): JS_Object => {
                console.assert(arrayIndex !== JS_NULL);
                console.assert(gamepadIndex !== JS_NULL);
                const array =
                    this.#jsObjectsMemory.get(arrayIndex) as (Gamepad | null)[];
                console.assert(array != null);
                return this.#jsObjectsMemory.add(array[gamepadIndex]);
            },
            JS_Gamepad_get_id: (gamepadIndex: JS_Object, id: StringPtr,
                                idSize: SizeT) => {
                const gamepad =
                    this.#jsObjectsMemory.get(gamepadIndex) as Gamepad | null;
                console.assert(gamepad !== null);
                console.assert(gamepad!.id.length < idSize);
                const idBuffer = this.#getUint8Array(id, idSize);
                const encodedId =
                    (new TextEncoder()).encode(gamepad!.id + "\0");
                copyUint8Array(idBuffer, encodedId);
            },
            JS_Gamepad_get_index: (gamepadIndex: JS_Object): number => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.#jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.index;
            },
            JS_Gamepad_get_connected: (gamepadIndex: JS_Object): boolean => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.#jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.connected;
            },
            JS_Gamepad_get_axe: (
                gamepadIndex: JS_Object,
                axeIndex: number,
            ): number => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.#jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.axes[axeIndex];
            },
            JS_Gamepad_get_button: ( gamepadIndex: JS_Object,
                buttonIndex: number,
            ): boolean => {
                console.assert(gamepadIndex != JS_NULL);
                const gamepad =
                    this.#jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad!.buttons[buttonIndex].pressed;
            },
            JS_Gamepad_get_axes_length: (gamepadIndex: number): number => {
                console.assert(gamepadIndex !== JS_NULL);
                const gamepad =
                    this.#jsObjectsMemory.get(gamepadIndex) as Gamepad;
                console.assert(gamepad !== null);
                return gamepad.axes.length;
            },
            JS_Object_free: (objectIndex: JS_Object) => {
                console.assert(objectIndex != JS_NULL);
                this.#jsObjectsMemory.remove(objectIndex);
            },
            JS_usleep: (usec: number): Promise<void> => {
                return new Promise((resolve, _reject) => {
                    setTimeout(resolve, usec / 1000);
                });
            },
            JS_write: (bufferPtr: Ptr, count: SizeT) => {
                const buffer = this.#str(bufferPtr, count);
                this.#terminal.setContent(buffer);
            },
            JS_read_char: () => this.#terminal.readChar(),
            JS_get_memory_size: (): SizeT => {
                return this.#memory.buffer.byteLength;
            },
            JS_get_terminal_width: (): number => {
                return this.#terminal.width;
            },
            JS_get_terminal_height: (): number => {
                return this.#terminal.height;
            },
            JS_exit: (status: number) => {
                throw new Exit(status);
            },
            JS_wait_for_next_frame: (): Promise<void> => {
                return new Promise((resolve, _reject) => {
                    requestAnimationFrame(() => resolve());
                });
            },
        };
    }

    #getUint8Array(ptr: Ptr, length: number): Uint8Array {
        return new Uint8Array(this.#memory.buffer, ptr, length);
    };

    #str(ptr: StringPtr, size?: number): string {
        if (size === undefined) {
            const mem = new Uint8Array(this.#memory.buffer, ptr);
            size = 0;
            while (mem[size]) ++size;
        }
        return new TextDecoder().decode(this.#getUint8Array(ptr, size));
    };
}
