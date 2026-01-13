import { COLOR_WHITE, colorCodesClasses } from './color-codes';

const CTRL_C: number = 0x03;
const KEY_BACKSPACE: number = 0x7f;
const KEY_ENTER: number = 0x0d;
const KEY_ESCAPE: number = 0x1b;

export class Terminal {

    readonly #pre: HTMLPreElement;
    readonly #terminalContentElement: HTMLDivElement;
    readonly #measureElement: HTMLDivElement;
    readonly #stdin: number[] = [];
    #width: number = -1;
    #height: number = -1;

    constructor() {
        this.#pre = document.querySelector('pre.terminal') as HTMLPreElement;
        console.assert(this.#pre !== null);
        this.#terminalContentElement = document.createElement('div');
        this.#pre.appendChild(this.#terminalContentElement);
        this.#measureElement = document.createElement('div');
        this.#measureElement.textContent = 'A';
        this.#measureElement.classList.add('measure');
        this.#pre.appendChild(this.#measureElement);
        this.#handleResize();
        document.addEventListener('keydown', this.#handleKeydown.bind(this));
        const handleResize = this.#handleResize.bind(this);
        window.addEventListener('resize', handleResize);
        document.fonts.ready.then(handleResize);
       }

    setContent(content: string) {
        let lineVisibleCharatersCount = 0;
        let terminalContent =
            `<span class="${colorCodesClasses[COLOR_WHITE]}">`;
        for (let i = 0; i < content.length; ++i) {
            if (content[i] == '\x1b') {
                console.assert(content[i + 1] == '[');
                i += 2;
                let code = '';
                while (content[i] != 'm' && content[i] != 'l' &&
                       content[i] != 'h' && content[i] != 'H' &&
                       content[i] != 'i') {
                    code += content[i++];
                }
                if (content[i] === 'm') {
                    terminalContent +=
                        `</span><span class="${colorCodesClasses[code]}">`;
                }
                continue;
            }
            terminalContent += content[i];
            ++lineVisibleCharatersCount;
            if (lineVisibleCharatersCount === this.#width) {
                lineVisibleCharatersCount = 0;
                terminalContent += '\n';
            }
        }
        this.#terminalContentElement.innerHTML = terminalContent;
    }

    printMessage(message: string) {
        this.#terminalContentElement.textContent = message;
    }

    get width(): number {
        return this.#width;
    }

    get height(): number {
        return this.#height;
    }

    readChar(): number {
        return this.#stdin.shift() ?? -1;
    }

    #handleKeydown(event: KeyboardEvent) {
        if (event.key == 'Shift') return;

        if (event.key == 'Backspace') {
            this.#stdin.push(KEY_BACKSPACE);
            return;
        }

        if (event.key == 'Enter') {
            this.#stdin.push(KEY_ENTER);
            return;
        }

        if (event.key == 'Escape') {
            this.#stdin.push(KEY_ESCAPE);
            return;
        }

        if (event.ctrlKey) {
            if (event.key == 'c') {
                this.#stdin.push(CTRL_C);
                return;
            }
            return;
        }

        this.#stdin.push(event.key.charCodeAt(0));
    }

    #handleResize() {
        const {
            width: preWidth,
            height: preHeight,
        } = this.#pre.getBoundingClientRect();
        const {
            width: measureWidth,
            height: measureHeight,
        } = this.#measureElement.getBoundingClientRect();
        this.#width = Math.floor(preWidth / measureWidth);
        this.#height = Math.floor(preHeight / measureHeight);

        const paddingX = (preWidth - (this.#width * measureWidth)) / 2.0;
        const paddingY = (preHeight - (this.#height * measureHeight)) / 2.0;
        this.#pre.style.padding = `${paddingY}px ${paddingX}px`;
    }
}
