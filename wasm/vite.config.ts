import { defineConfig } from 'vite';

export default defineConfig(({ mode }) => {
    if (mode === 'dev') {
        return {
            esbuild: {
                drop: ['console'],
            },
        };
    }

    return {};
});
