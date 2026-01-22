import type { Int, Ptr, SizeT, CharPtr, Uint64, Char, Float, Double, Uint32, Uint8, Uint16 } from './types';

export type Imports = {
    memory: WebAssembly.Memory;
    JS_console_log(ptr: CharPtr): void;
    JS_console_error(ptr: CharPtr): void;
    JS_requestAnimationFrame(callback: Ptr): void;
    JS_Date_now(): Uint64;
    JS_performance_now(): Uint32;
    pow(x: Double, y: Double): Double;
    powf(x: Float, y: Float): Float;
    cosf(x: Float): Float;
    sinf(x: Float): Float;
    tanf(x: Float): Float;
    atanf(x: Float): Float;
    roundf(x: Float): Float;
    isnanf(x: Float): boolean;
    fmod(x: Double, y: Double): Double;
    fmodf(x: Float, y: Float): Float;
    JS_write(bufferPtr: Ptr, count: SizeT): void;
    JS_read_char(): Char;
    JS_get_memory_size(): SizeT;
    JS_get_terminal_width(): Int
    JS_get_terminal_height(): Int
    _exit(status: Int): never;
    JS_ongamepadconnected(callback: Ptr): void;
    JS_ongamepaddisconnected(callback: Ptr): void;
    JS_gameapad_rumble(
        gamepadIndex: Uint32,
        lowFrequency: Uint16,
        highFrequency: Uint16,
        durationMiliseconds: Uint32,
    ): void;
    JS_get_gamepad_buttons_count(gamepadIndex: Uint32): SizeT;
    JS_get_gamepad_axis_count(gamepadIndex: Uint32): SizeT;
    JS_get_gamepad_id(
        gamepadIndex: Uint32,
        gamepadId: CharPtr,
        gamepadIdSize: SizeT,
    ): void;
    JS_Gamepad_get_axis(gamepadIndex: Uint32, axisIndex: Uint8): Float;
    JS_get_gamepad_button(gamepadIndex: Uint32, button: Uint8): boolean,
}
