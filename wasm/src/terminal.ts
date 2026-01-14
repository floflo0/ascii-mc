import { COLOR_BLACK, COLOR_WHITE, colors } from './color-codes';

const CTRL_C: number = 0x03;
const KEY_BACKSPACE: number = 0x7f;
const KEY_ENTER: number = 0x0d;
const KEY_ESCAPE: number = 0x1b;

const FONTS = 'Terminus, \'Ubuntu Mono\', monospace';
const FONT_SIZE = 16;  // px
const LINE_HEIGHT = 16;  // px

export class Terminal {

    private readonly canvas: HTMLCanvasElement;
    private readonly context: CanvasRenderingContext2D;
    private readonly stdin: number[] = [];
    private width: number = -1;
    private height: number = -1;
    private fontSize: number = FONT_SIZE;
    private lineHeight: number = LINE_HEIGHT;
    private cellWidth: number = 0;
    private cellHeight: number = this.lineHeight;
    private paddingY: number = 0;
    private paddingX: number = 0;

    public constructor() {
        this.canvas = document.getElementById('terminal') as HTMLCanvasElement;
        console.assert(this.canvas !== null);
        this.context = this.canvas.getContext('2d')!;
        console.assert(this.context !== null);
        this.context.textRendering = 'optimizeSpeed';
        this.setCanvasFont();
        this.#handleResize();
        document.addEventListener('keydown', this.#handleKeydown.bind(this));
        const handleResize = this.#handleResize.bind(this);
        window.addEventListener('resize', handleResize);
        document.fonts.ready.then(handleResize);
    }

    public setContent(content: string): void {
        this.context.fillStyle = colors[COLOR_BLACK];
        this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
        this.setCanvasFont();
        this.context.fillStyle = colors[COLOR_WHITE];
        let lineVisibleCharatersCount = 0;
        let x = this.paddingX;
        let y = this.cellHeight + this.paddingY;
        let text = '';
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
                    this.context.fillText(text, x, y);
                    x += text.length * this.cellWidth;
                    text = '';
                    this.context.fillStyle = colors[code];
                }
                continue;
            }

            text += content[i];
            ++lineVisibleCharatersCount;
            if (lineVisibleCharatersCount === this.width) {
                this.context.fillText(text, x, y);
                x = this.paddingX;
                y += this.cellHeight;
                text = '';
                lineVisibleCharatersCount = 0;
            }
        }
    }

    private setCanvasFont() {
        this.context.font = `bold ${this.fontSize}px ${FONTS}`;
    }

    printMessage(message: string) {
        this.setContent(message);
    }

    getWidth(): number {
        return this.width;
    }

    getHeight(): number {
        return this.height;
    }

    readChar(): number {
        return this.stdin.shift() ?? -1;
    }

    #handleKeydown(event: KeyboardEvent) {
        if (event.key == 'Shift') return;

        if (event.key == 'Backspace') {
            this.stdin.push(KEY_BACKSPACE);
            return;
        }

        if (event.key == 'Enter') {
            this.stdin.push(KEY_ENTER);
            return;
        }

        if (event.key == 'Escape') {
            this.stdin.push(KEY_ESCAPE);
            return;
        }

        if (event.ctrlKey) {
            if (event.key == 'c') {
                this.stdin.push(CTRL_C);
                return;
            }
            return;
        }

        this.stdin.push(event.key.charCodeAt(0));
    }

    #handleResize() {
        const width = window.innerWidth * window.devicePixelRatio;
        const height = window.innerHeight * window.devicePixelRatio;
        this.canvas.width = width;
        this.canvas.height = height;
        this.fontSize = FONT_SIZE * window.devicePixelRatio;
        this.lineHeight = FONT_SIZE * window.devicePixelRatio;
        this.setCanvasFont();
        const measure = this.context.measureText('A');
        this.cellWidth = measure.width;
        this.cellHeight = this.lineHeight;
        this.width = Math.floor(width / this.cellWidth);
        this.height = Math.floor(height / this.cellHeight);
        this.paddingX = (width - (this.width * this.cellWidth)) * 0.5;
        this.paddingY = (height - (this.height * this.cellHeight)) * 0.5;
    }
}
