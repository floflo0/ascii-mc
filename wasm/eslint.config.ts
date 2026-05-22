import js from '@eslint/js';
import stylistic from '@stylistic/eslint-plugin';
import { defineConfig } from 'eslint/config';
import importPlugin from 'eslint-plugin-import';
import simpleImportSort from 'eslint-plugin-simple-import-sort';
import tseslint from 'typescript-eslint';

export default defineConfig([
    { ignores: ['dist/**', 'node_modules/**'] },
    { files: ['eslint.config.ts', 'vite.config.ts', 'src/**/*.ts'] },
    js.configs.all,
    tseslint.configs.strictTypeChecked,
    tseslint.configs.stylisticTypeChecked,
    {
        languageOptions: {
            parserOptions: {
                projectService: {
                    allowDefaultProject: ['eslint.config.ts', 'vite.config.ts'],
                },
            },
        },
    },
    importPlugin.flatConfigs.recommended,
    importPlugin.flatConfigs.typescript,
    stylistic.configs.customize({
        indent: 4,
        semi: true,
        jsx: false,
        braceStyle: '1tbs',
    }),
    {
        plugins: {
            'simple-import-sort': simpleImportSort,
        },
        rules: {
            'sort-keys': 'off',
            'one-var': ['error', 'never'],
            'no-inline-comments': 'off',
            'capitalized-comments': 'off',
            'no-magic-numbers': 'off',
            'max-statements': 'off',
            'no-console': ['error', { allow: ['assert'] }],
            'id-length': ['error', { exceptions: ['x', 'y', 'i'] }],
            'no-plusplus': 'off',
            'no-continue': 'off',
            'curly': ['error', 'multi-line'],
            'no-ternary': 'off',
            'max-lines': 'off',
            'max-lines-per-function': 'off',
            'max-len': ['error', {
                code: 80,
                tabWidth: 4,
            }],
            'no-undefined': 'off',
            'sort-imports': 'off',
            'max-params': 'off',
            'yoda': ['error', 'never', { onlyEquality: true }],
            'no-warning-comments': 'off',
            'no-void': ['error', { allowAsStatement: true }],
            'camelcase': ['error', {
                ignoreDestructuring: true,
            }],
            'init-declarations': 'off',
            'no-negated-condition': 'off',
            '@typescript-eslint/consistent-type-definitions': 'off',
            '@typescript-eslint/no-misused-promises': ['error', {
                checksVoidReturn: false,
            }],
            '@typescript-eslint/restrict-template-expressions': ['error', {
                allowNumber: true,
            }],
            '@typescript-eslint/no-unnecessary-condition': 'off',
            // Already covered by Typescript.
            '@typescript-eslint/no-unused-vars': 'off',
            '@typescript-eslint/no-non-null-assertion': 'off',
            'simple-import-sort/imports': 'error',
            '@typescript-eslint/only-throw-error': 'off',
            'simple-import-sort/exports': 'error',
            'import/no-unresolved': 'off',
            'import/first': 'error',
            'import/newline-after-import': 'error',
            'import/no-duplicates': 'error',
        },
    },
]);
