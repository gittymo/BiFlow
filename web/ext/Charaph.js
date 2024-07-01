import { CharaphTable, CharaphColumn, CharaphRow, CharaphCell } from "./CharaphTable.js"
import { CharaphPaint } from "./CharaphPaint.js"

export var Charaph = Charaph || {};

Charaph.Table = CharaphTable;
Charaph.Table.Column = CharaphColumn;
Charaph.Table.Row = CharaphRow;
Charaph.Table.Cell = CharaphCell;

Charaph.Paint = CharaphPaint;

class DataEntry {
    #label;
    #value;
    #startIndex;
    #endIndex;
    #fillStyle;
    #strokeStyle;

    static #defaultFillStyleHue = 0;

    constructor(label, value) {
        if (typeof label !== 'string' || !label instanceof String) throw new Error("Label must be a string.");
        if (label.trim().length == 0) throw new Error("Label cannot be empty.");
        if (isNaN(value)) throw new Error("Value must be numeric.");
        this.#label = label;
        this.#value = value;
        this.#strokeStyle = new CharaphPaint("black");
        this.#fillStyle = CharaphPaint.FromHSV(DataEntry.#defaultFillStyleHue, 1.0, 1.0);
        DataEntry.#defaultFillStyleHue += 30;
        if (DataEntry.#defaultFillStyleHue == 360) DataEntry.#defaultFillStyleHue = 0;
    }

    get Label() {
        return this.#label;
    }

    get Value() {
        return this.#value;
    }

    set StartIndex(index) {
        if (isNaN(index)) throw new Error("Index value must be numeric.");
        this.#startIndex = index;
    }

    set EndIndex(index) {
        if (isNaN(index)) throw new Error("Index value must be numeric.");
        this.#endIndex = index;
    }

    set FillStyle(style) {
        if (!style instanceof CharaphPaint) throw new Error("Style must be a CanvasPaintStyle value.");
        this.#fillStyle = style.StyleString;
    }

    get FillStyle() {
        return this.#fillStyle;
    }

    set StrokeStyle(style) {
        if (!style instanceof CharaphPaint) throw new Error("Style must be a CanvasPaintStyle value.");
        this.#strokeStyle = style.StyleString;
    }

    get StrokeStyle() {
        return this.#strokeStyle;
    }
}

class DataSet {
    #dataentries;
    #maxValue = Number.MIN_VALUE;
    #minValue = Number.MAX_VALUE;
    constructor() {
        this.#dataentries = [];
    }

    AddDataEntry(dataEntry) {
        if (!dataEntry instanceof DataEntry) throw new Error("Can only add instances of DataEntry.");
        for (var i = 0; i < this.#dataentries.length; i++) {
            if (dataEntry == this.#dataentries[i]) return;
            if (dataEntry.Label.toLowerCase() == this.#dataentries[i].Label.toLowerCase()) throw new Error("Duplicate data label.");
        }
        
        var dataIndex = this.#dataentries.length;
        dataEntry.StartIndex = dataEntry.EndIndex = dataIndex;
        this.#dataentries[dataIndex] = dataEntry;

        if (dataEntry.Value < this.#minValue) this.#minValue = dataEntry.Value;
        if (dataEntry.Value > this.#maxValue) this.#maxValue = dataEntry.Value;
    }

    SortByValue(ascending = true) {
        if (typeof ascending !== 'boolean') throw new Error("Ascending value must be boolean.");
        for (var i = 0; i < this.#dataentries.length; i++) {
            for (var j = this.#dataentries.length - 1; j > i; j--) {
                if (ascending) {
                    if (this.#dataentries[i].Value > this.#dataentries[j].Value) {
                        var t = this.#dataentries[i];
                        this.#dataentries[i] = this.#dataentries[j];
                        this.#dataentries[j] = t;
                    } else if (this.#dataentries[i].Value == this.#dataentries[j].Value) {
                        if (this.#dataentries[i].Label.toLowerCase() > this.#dataentries[j].Label.toLowerCase()) {
                            var t = this.#dataentries[i];
                            this.#dataentries[i] = this.#dataentries[j];
                            this.#dataentries[j] = t;
                        }
                    }
                } else {
                    if (this.#dataentries[i].Value < this.#dataentries[j].Value) {
                        var t = this.#dataentries[i];
                        this.#dataentries[i] = this.#dataentries[j];
                        this.#dataentries[j] = t;
                    } else if (this.#dataentries[i].Value == this.#dataentries[j].Value) {
                        if (this.#dataentries[i].Label.toLowerCase() < this.#dataentries[j].Label.toLowerCase()) {
                            var t = this.#dataentries[i];
                            this.#dataentries[i] = this.#dataentries[j];
                            this.#dataentries[j] = t;
                        }
                    }
                }
            }
        }
    }

    get MinimumValue() {
        return this.#minValue;
    }
    
    get MaximumValue() {
        return this.#maxValue;
    }

    get Size() {
        return this.#dataentries.length;
    }

    get Entries() {
        return this.#dataentries;
    }

    SortByLabel(ascending = true) {
        if (typeof ascending !== 'boolean') throw new Error("Ascending value must be boolean.");
        for (var i = 0; i < this.#dataentries.length; i++) {
            for (var j = this.#dataentries.length - 1; j > i; j--) {
                if (ascending) {
                    if (this.#dataentries[i].Label.toLowerCase() > this.#dataentries[j].Label.toLowerCase()) {
                        var t = this.#dataentries[i];
                        this.#dataentries[i] = this.#dataentries[j];
                        this.#dataentries[j] = t;
                    }
                } else {
                    if (this.#dataentries[i].Label.toLowerCase() < this.#dataentries[j].Label.toLowerCase()) {
                        var t = this.#dataentries[i];
                        this.#dataentries[i] = this.#dataentries[j];
                        this.#dataentries[j] = t;
                    }
                }
            }
        }
    }
}

class AbstractCharaph {
    _dataset;
    _intervalHandle;
    _canvas;
    _width;
    _height;

    constructor(dataset) {
        if (!dataset instanceof DataSet) throw new Error("Can only create using a DataSet instance.");
        this._dataset = dataset;
        this._width = this._height = 0;
        this._canvas = document.createElement('canvas');
    }

    get Canvas() {
        return this._canvas;
    }

    set Width(width) {
        if (isNaN(width)) throw new Error("Width value must be numeric.");
        if (width <= 0) throw new Error("Width must be greater than zero (0).");
        this._width = width;
        this._canvas.width = width;
        this._canvas.style.width = width + "px";
    }

    get Width() {
        return this._width;
    }

    set Height(height) {
        if (isNaN(height)) throw new Error("Height value must be numeric.");
        if (height <= 0) throw new Error("Height must be greater than zero (0).");
        this._height = height
        this._canvas.height = height;
        this._canvas.style.height = height + "px";
    }

    get Height() {
        return this._height;
    }

    CreateComponent() {
        throw new Error("Cannot instantiate objects from abstract class.");
    }
}

class CharaphBarChart extends AbstractCharaph {
    #vertical;
    #padding = 8;
    #barThicknessPercent = 0.8;
    #labelsVisible = true;
    #labelHeight = 32;
    #sectionWidth;
    #availableHeight;
    #availableWidth;

    constructor(dataset, vertical = true) {
        if (typeof vertical !== 'boolean') throw new error("Vertical value must be boolean.");
        super(dataset);
        this.#vertical = vertical;
    }

    set Width(width) {
        super.Width = width;
        this.#sectionWidth = this._width / (this._dataset.Entries.length + 1);
    }

    set Height(height) {
        super.Height = height;
        this.#availableHeight = this._height - (this.#padding * 2);
    }

    get IsVertical() {
        return this.#vertical;
    }

    set IsVertical(vertical) {
        if (typeof vertical !== 'boolean') throw new Error("Vertical value must be boolean.");
        this.#vertical = vertical;
    }

    DrawBar(dataset_index) {
        if (dataset_index < 0 || dataset_index > this._dataset.Entries.length) return;

        var dataentry = this._dataset.Entries[dataset_index];

        const HALF_SECTION_WIDTH = this.#sectionWidth / 2;
        const BAR_WIDTH = Math.floor(HALF_SECTION_WIDTH * this.#barThicknessPercent);
        const BAR_XPOS_OFFSET = (this.#sectionWidth - BAR_WIDTH) / 2;
        const MAX_BAR_HEIGHT = this.#availableHeight - (this.#labelsVisible ? this.#labelHeight + this.#padding : 0);

        const BAR_HEIGHT = this.#availableHeight * (dataentry.Value / this._dataset.MaximumValue);

        var xpos = this.#padding + HALF_SECTION_WIDTH + (this.#sectionWidth * dataset_index) + BAR_XPOS_OFFSET;
        var ypos = this.#padding + (this.#availableHeight - BAR_HEIGHT);

        var ctx = this._canvas.getContext("2d");
        ctx.fillStyle = dataentry.FillStyle.StyleString;
        ctx.fillRect(xpos, ypos, BAR_WIDTH, BAR_HEIGHT);
        ctx.strokeStyle = dataentry.StrokeStyle.StyleString;
        ctx.strokeRect(xpos, ypos, BAR_WIDTH, BAR_HEIGHT);


    }
    
    Paint() {
        if (this._dataset.length == 0) return;
        if (this._width == 0 || this._height == 0) {
            if (!this._canvas.offsetParent) return;
            var clientRect = this._canvas.getBoundingClientRect();
            this.Width = clientRect.width;
            this.Height = clientRect.height;
        }

        /*var LABEL_WIDTH = SECTION_WIDTH * 0.8;
        var LABEL_X_OFFSET = (SECTION_WIDTH - LABEL_WIDTH) / 2;
        var LABEL_HEIGHT = 32;

            
        // var LABEL_YPOS = 16 + AVAILABLE_HEIGHT;*/

        
        
        var ctx = this._canvas.getContext("2d");
        for (var i = 0; i < this._dataset.Entries.length; i++) {
            
            this.DrawBar(i);
            /*ctx.fillStyle = "black";
            ctx.strokeRect(xpos + LABEL_X_OFFSET, LABEL_YPOS, LABEL_WIDTH, LABEL_HEIGHT);
            ctx.textBaseline = 'middle'; 
            ctx.textAlign = 'center';
            ctx.fillText(this._dataset.Entries[i].Label, xpos + LABEL_X_OFFSET + (LABEL_WIDTH / 2), LABEL_YPOS + (LABEL_HEIGHT / 2), LABEL_WIDTH - 16);*/
        }
    }
}

Charaph.BarChart = CharaphBarChart;