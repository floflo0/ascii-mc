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
    private pixelWidth: number = -1;
    private pixelHeight: number = -1;
    private fontSize: number = FONT_SIZE;
    private lineHeight: number = LINE_HEIGHT;
    private cellWidth: number = 0;
    private cellHeight: number = this.lineHeight;
    private paddingY: number = 0;
    private paddingX: number = 0;
    private isPointerLocked: boolean = false;
    private handlePointerMove: ((event: MouseEvent) => void) | null = null;
    private handlePointerDown: ((event: MouseEvent) => void) | null = null;
    private handleKeydownCallback: ((keyCode: number) => void) | null = null;
    private handleKeyup: ((event: KeyboardEvent) => void) | null = null;

    public constructor() {
        this.canvas = document.getElementById('terminal') as HTMLCanvasElement;
        console.assert(this.canvas !== null);
        this.context = this.canvas.getContext('2d')!;
        console.assert(this.context !== null);
        this.context.textRendering = 'optimizeSpeed';
        this.setCanvasFont();
        this.handleResize();
        document.addEventListener('keydown', this.handleKeydown.bind(this));
        const handleResize = this.handleResize.bind(this);
        window.addEventListener('resize', handleResize);
        document.fonts.ready.then(handleResize);
        this.canvas.addEventListener('click', this.handleClick.bind(this));
        document.addEventListener(
            'pointerlockchange',
            this.handlePointerLockChange.bind(this),
        );
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
            if (content[i] === '\x1b') {
                console.assert(content[i + 1] === '[');
                i += 2;
                let code = '';
                while (content[i] !== 'm' && content[i] !== 'l' &&
                       content[i] !== 'h' && content[i] !== 'H' &&
                       content[i] !== 'i') {
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

    private async handleClick(): Promise<void>  {
        if (this.isPointerLocked) return;
        this.canvas.tabIndex = 0;
        this.canvas.focus();
        await this.canvas.requestPointerLock();
        this.isPointerLocked = true;
    }

    private handlePointerLockChange(): void {
        this.isPointerLocked = document.pointerLockElement === this.canvas;
    }

    public onPointerMove(
        callback: (movementX: number, movementY: number) => void,
    ): void {
        console.assert(this.handlePointerMove === null);
        // On Firefox, if the use press both left and right click a weird
        // PointerMoveEvent is fired. So we ignore event that contains a button
        // change to ignore those events.
        let previousButtons = 0;
        this.handlePointerMove = (event: MouseEvent) => {
            if (!this.isPointerLocked) return;
            if (event.buttons !== previousButtons) {
                previousButtons = event.buttons;
                return;
            }
            const sensivity =
                navigator.userAgent.includes('Firefox') ? 1.0 : 0.1;
            callback(event.movementX * sensivity, event.movementY * sensivity);
        };
        this.canvas.addEventListener('mousemove', this.handlePointerMove);
    }

    public clearOnPointerMove(): void {
        console.assert(this.handlePointerMove !== null);
        this.canvas.removeEventListener('mousemove', this.handlePointerMove!);
        this.handlePointerMove = null;
    }

    public onPointerDown(callback: (mouseButton: number) => void): void {
        console.assert(this.handlePointerDown === null);
        this.handlePointerDown = (event: MouseEvent) => {
            event.preventDefault();
            if (!this.isPointerLocked) return;
            console.log(event);
            callback(event.button);
        };
        this.canvas.addEventListener('mousedown', this.handlePointerDown);
    }

    public clearOnPointerDown(): void {
        console.assert(this.handlePointerDown !== null);
        this.canvas.removeEventListener('mousedown', this.handlePointerDown!);
        this.handlePointerDown = null;
    }

    public onKeydown(callback: (keyCode: number) => void) {
        console.assert(this.handleKeydownCallback === null);
        this.handleKeydownCallback = callback;
    }

    public clearOnKeydown(): void {
        console.assert(this.handleKeydownCallback !== null);
        this.handleKeydownCallback = null;
    }

    public onKeyup(callback: (keyCode: number) => void) {
        console.assert(this.handleKeyup === null);
        this.handleKeyup = (event: KeyboardEvent) => {
            if (!this.isPointerLocked) return;
            callback(event.keyCode);
        };
        this.canvas.addEventListener('keyup', this.handleKeyup);
    }

    public clearOnKeyup(): void {
        console.assert(this.handleKeyup !== null);
        this.canvas.removeEventListener('keyup', this.handleKeyup!);
        this.handleKeyup = null;
    }

    public printMessage(message: string) {
        this.setContent(message);
    }

    public getWidth(): number {
        return this.width;
    }

    public getHeight(): number {
        return this.height;
    }

    public getPixelWidth(): number {
        return this.pixelWidth;
    }

    public getPixelHeight(): number {
        return this.pixelHeight;
    }

    public readChar(): number {
        return this.stdin.shift() ?? -1;
    }

    private handleKeydown(event: KeyboardEvent) {
        if (!this.isPointerLocked) return;

        if (this.handleKeydownCallback !== null) {
            this.handleKeydownCallback(event.keyCode);
        }

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

    private handleResize() {
        this.pixelWidth = window.innerWidth * window.devicePixelRatio;
        this.pixelHeight = window.innerHeight * window.devicePixelRatio;
        this.canvas.width = this.pixelWidth;
        this.canvas.height = this.pixelHeight;
        this.fontSize = FONT_SIZE * window.devicePixelRatio;
        this.lineHeight = FONT_SIZE * window.devicePixelRatio;
        this.setCanvasFont();
        const measure = this.context.measureText('A');
        this.cellWidth = measure.width;
        this.cellHeight = this.lineHeight;
        this.width = Math.floor(this.pixelWidth / this.cellWidth);
        this.height = Math.floor(this.pixelHeight / this.cellHeight);
        this.paddingX =
            (this.pixelWidth - (this.width * this.cellWidth)) * 0.5;
        this.paddingY =
            (this.pixelHeight - (this.height * this.cellHeight)) * 0.5;
    }
}
