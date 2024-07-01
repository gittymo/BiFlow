export class CharaphPaint {
    #styleString;
    constructor(styleString) {
        if (typeof styleString !== 'string' || !styleString instanceof String) throw new Error("styleString must be a string.");
        this.#styleString = styleString;
    }

    set StyleString(style) {
        if (typeof styleString !== 'string' || !styleString instanceof String) throw new Error("styleString must be a string.");
        this.#styleString = styleString;
    }

    get StyleString() {
        return this.#styleString;
    }

    static RoundRGBValue(value) {
        if (value < 0) value = 0;
        if (value > 255) value = 255;
    }

    static RoundHue(value) {
        var neg = value < 0;
        value = value % 360;
        if (neg) value += 180;
        return value > 1 ? value / 360 : value;
    }

    static RoundSatVal(value) {
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        return value > 1 ? value / 100 : value;
    }

    static FromRGB(red, green, blue) {
        if (isNaN(red) || isNaN(green) || isNaN(blue)) throw new Error("RGB values must be numeric.");
        var cs = new CharaphPaint(`rgb(${RoundRGBValue(red)},${RoundRGBValue(green)}, ${RoundRGBValue(blue)})`);
        return cs;
    }

    static HSVtoRGB(h, s, v) {
        var r, g, b, i, f, p, q, t;
        if (arguments.length === 1) {
            s = h.s, v = h.v, h = h.h;
        }
        i = Math.floor(h * 6);
        f = h * 6 - i;
        p = v * (1 - s);
        q = v * (1 - f * s);
        t = v * (1 - (1 - f) * s);
        switch (i % 6) {
            case 0: r = v, g = t, b = p; break;
            case 1: r = q, g = v, b = p; break;
            case 2: r = p, g = v, b = t; break;
            case 3: r = p, g = q, b = v; break;
            case 4: r = t, g = p, b = v; break;
            case 5: r = v, g = p, b = q; break;
        }
        return {
            r: Math.round(r * 255),
            g: Math.round(g * 255),
            b: Math.round(b * 255)
        };
    }

    static FromHSV(hue, saturation, value) {
        if (isNaN(hue) || isNaN(saturation) || isNaN(value)) throw new Error("HSV values must be numeric.");
        hue = CharaphPaint.RoundHue(hue);
        saturation = CharaphPaint.RoundSatVal(saturation);
        value = CharaphPaint.RoundSatVal(value);
        var rgb = CharaphPaint.HSVtoRGB(hue, saturation, value);
        var cs = new CharaphPaint(`rgb(${rgb.r},${rgb.g},${rgb.b})`);
        return cs;
    }
}
