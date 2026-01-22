import { Exit } from './exit';
import type { Exports, RunCallback, RunCallbackUint32, WasmMain } from './exports';
import type { Imports } from './imports';
import { Terminal } from './terminal';
import type { Ptr, CharPtr } from './types';

const MEMORY_SIZE: number = 15625;  // 1024 MB

const NULL: Ptr = 0;

export class Wasm {
    private readonly memory: WebAssembly.Memory;
    private readonly memoryView: Uint8Array;
    private readonly terminal: Terminal = new Terminal();
    private readonly textDecoder: TextDecoder = new TextDecoder();
    private readonly textEncoder: TextEncoder = new TextEncoder();

    private mainWasm: WasmMain = () => {
        console.assert(false, "mainWasm was not loaded");
    };
    private runCallback: RunCallback = (_callback) => {
        console.assert(false, "runCallback was not loaded");
    };
    private runCallbackUint32: RunCallbackUint32 = (_callback) => {
        console.assert(false, "runCallbackUint32 was not loaded");
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
            run_callback_uint32,
        } = instance.exports as Exports;
        this.mainWasm = wasm_main;
        this.runCallback = run_callback;
        this.runCallbackUint32 = run_callback_uint32;
    }

    private buildEnv(): Imports {
        return {
            memory: this.memory,
            JS_console_log: (ptr) => {
                console.log(this.str(ptr));
            },
            JS_console_error: (ptr) => {
                console.error(this.str(ptr));
            },
            JS_requestAnimationFrame: (callback) => {
                console.assert(callback != NULL);
                requestAnimationFrame(() => this.runCallback(callback));
            },
            JS_Date_now: () => BigInt(Date.now()),
            JS_performance_now: () => performance.now(),
            pow: Math.pow,
            powf: Math.pow,
            cosf: Math.cos,
            sinf: Math.sin,
            tanf: Math.tan,
            atanf: Math.atan,
            roundf: Math.round,
            isnanf: isNaN,
            fmod: (x, y) => x % y,
            fmodf: (x, y) => x % y,
            JS_write: (bufferPtr, count) => {
                this.terminal.setContent(
                    this.textDecoder.decode(this.memoryView.subarray(
                        bufferPtr,
                        bufferPtr + count,
                    )),
                );
            },
            JS_read_char: () => this.terminal.readChar(),
            JS_get_memory_size: () => this.memory.buffer.byteLength,
            JS_get_terminal_width: () => this.terminal.getWidth(),
            JS_get_terminal_height: () => this.terminal.getHeight(),
            _exit: (status) => {
                throw new Exit(status);
            },
            JS_ongamepadconnected: (callback) => {
                if (callback === NULL) {
                    window.ongamepadconnected = null;
                    return;
                }

                window.ongamepadconnected = (event) => {
                    this.runCallbackUint32(callback, event.gamepad.index);
                }
            },
            JS_ongamepaddisconnected: (callback) => {
                if (callback === NULL) {
                    window.ongamepaddisconnected = null;
                    return;
                }

                window.ongamepaddisconnected = (event) => {
                    this.runCallbackUint32(callback, event.gamepad.index);
                }
            },
            JS_gameapad_rumble: (
                gamepadIndex,
                lowFrequency,
                highFrequency,
                durationMiliseconds,
            ) => {
                console.assert(0 <= lowFrequency && lowFrequency <= 0xffff);
                console.assert(0 <= highFrequency && highFrequency <= 0xffff);
                console.assert(0.0 < durationMiliseconds);
                const gamepad = navigator.getGamepads()[gamepadIndex];
                console.assert(gamepad !== null);
                if (gamepad!.hapticActuators !== undefined) {
                    if (!gamepad!.hapticActuators.length) return;

                    this.hapticActuatorsRumble(
                        gamepad!.hapticActuators[0],
                        lowFrequency / 0xffff,
                        highFrequency / 0xffff,
                        durationMiliseconds,
                    );
                    return;
                }

                if (gamepad!.vibrationActuator === undefined) return;

                this.hapticActuatorsRumble(
                    gamepad!.vibrationActuator,
                    lowFrequency / 0xffff,
                    highFrequency / 0xffff,
                    durationMiliseconds,
                );
            },
            JS_get_gamepad_buttons_count: (gamepadIndex) => {
                const gamepad = navigator.getGamepads()[gamepadIndex];
                console.assert(gamepad !== null);
                return gamepad!.buttons.length;
            },
            JS_get_gamepad_axis_count: (gamepadIndex) => {
                const gamepad = navigator.getGamepads()[gamepadIndex];
                console.assert(gamepad !== null);
                return gamepad!.axes.length;
            },
            JS_get_gamepad_id: (gamepadIndex, gamepadId, gamepadIdSize) => {
                console.assert(gamepadId !== NULL);
                console.assert(gamepadIdSize > 0);
                const gamepad = navigator.getGamepads()[gamepadIndex];
                console.assert(gamepad !== null);
                const encodedId = this.textEncoder.encode(gamepad!.id + "\0");
                if (gamepadIdSize < encodedId.length) {
                    throw new Error(
                        `buffer too small to contain gamepad id '${gamepadId}'`,
                    );
                }
                this.memoryView.set(encodedId, gamepadId);
            },
            JS_Gamepad_get_axis: (gamepadIndex, axisIndex) => {
                const gamepad = navigator.getGamepads()[gamepadIndex];
                console.assert(gamepad !== null);
                return gamepad!.axes[axisIndex];
            },
            JS_get_gamepad_button: (gamepadIndex, button) => {
                const gamepad = navigator.getGamepads()[gamepadIndex];
                console.assert(gamepad !== null);
                return gamepad!.buttons[button].pressed;
            },
        };
    }

    private str(ptr: CharPtr): string {
        let end = ptr;
        while (this.memoryView[end]) ++end;
        return this.textDecoder.decode(this.memoryView.subarray(ptr, end));
    };

    private hapticActuatorsRumble(
        hapticActuators: GamepadHapticActuator,
        lowFrequency: number,
        highFrequency: number,
        durationMiliseconds: number,
    ): void {
        console.assert(hapticActuators !== undefined);
        console.assert(0.0 <= lowFrequency && lowFrequency <= 1.0);
        console.assert(0.0 <= highFrequency && highFrequency <= 1.0);
        console.assert(0.0 < durationMiliseconds);
        if (hapticActuators.playEffect !== undefined) {
            let effect: GamepadHapticEffectType;
            if (hapticActuators.effects !== undefined &&
                hapticActuators.effects.length) {
                effect = hapticActuators.effects[0];
            } else if (hapticActuators.type !== undefined) {
                effect = hapticActuators.type;
            } else {
                return;
            }

            hapticActuators.playEffect(effect, {
                duration: durationMiliseconds,
                strongMagnitude: lowFrequency,
                weakMagnitude: highFrequency,
            });
            return;
        }

        if (hapticActuators.pulse === undefined) return;

        hapticActuators.pulse(
            (lowFrequency + highFrequency) * 0.5,
            durationMiliseconds,
        );
    }
}
