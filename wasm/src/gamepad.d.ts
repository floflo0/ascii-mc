export {};

declare global {
    interface Gamepad {
        readonly hapticActuators?: GamepadHapticActuator[];
        readonly vibrationActuator?: GamepadHapticActuator;
    }

    interface GamepadHapticActuator {
        readonly type?: GamepadHapticEffectType;
        readonly effects?: GamepadHapticEffectType[];
        playEffect?(type: GamepadHapticEffectType, params?: GamepadEffectParameters): Promise<GamepadHapticsResult>;
        pulse?(value: number, duration: number): Promise<boolean>;
    }
};
