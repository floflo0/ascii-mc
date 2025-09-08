export const copyUint8Array = (dest: Uint8Array, src: Uint8Array): void => {
    for (let i = 0; i < dest.length; ++i)
        dest[i] = src[i];
};
