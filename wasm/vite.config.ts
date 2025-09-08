import { defineConfig } from 'vite';

export default defineConfig(({ mode }) => {
    if (mode === 'production') {
        return {
            esbuild: {
                drop: ['console'],
            },
            base: '/ascii-mc',
        };
    }

    return {};
});
