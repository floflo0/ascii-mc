export type JS_Object = number;

export const JS_NULL: JS_Object = -1;

export class JsObjectsMemory {
    readonly #memory: object[];

    constructor() {
        this.#memory = [];
    }

    add(object_: object | null): JS_Object {
        if (object_ === null) return JS_NULL;
        for (let i = 0; i < this.#memory.length; ++i) {
            if (this.#memory[i] === undefined) {
                this.#memory[i] = object_;
                return i;
            }
        }
        return this.#memory.push(object_) - 1;
    }

    replace(objectIndex: number, newObject: object): void {
        this.#memory[objectIndex] = newObject;
    }

    remove(objectIndex: JS_Object): void {
        console.assert(objectIndex !== JS_NULL);
        delete this.#memory[objectIndex];
    }

    get(objectIndex: JS_Object): object | null {
        if (objectIndex === JS_NULL) return null
        return this.#memory[objectIndex];
    }
}
