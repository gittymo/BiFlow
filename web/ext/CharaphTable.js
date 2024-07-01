// var Charaph = Charaph || {};

export class CharaphTable {
    #columns = [];
    #rows = [];
    #longestColumn = Number.MIN_VALUE;
    #longestRow = Number.MIN_VALUE;
    #name;

    set Name(name) {
        if (name != null && (typeof name !== 'string' || !name instanceof String)) throw new Error("Name must be string or not defined.");
        this.#name = name;
    }

    get Name() {
        return this.#name;
    }

    constructor(name) {
        this.Name = name;

    }

    get Columns() {
        return this.#columns;
    }

    get Rows() {
        return this.#rows;
    }

    AddRow(rowIndex) {
        if (isNaN(rowIndex)) throw new Error("RowIndex must be a numerical value.");
        if (rowIndex > this.#longestColumn) this.#longestColumn = rowIndex;
        for (var i = 0; i < this.#rows.length; i++) {
            if (this.#rows[i].RowIndex == rowIndex) throw new Error(`Table already has a row with index ${rowIndex}`);
        }
        this.#rows[this.#rows.length] = new CharaphRow(this)
    }

    set LongestRow(index) {
        if (isNaN(index)) throw new Error("Index value must be numerical.");
        if (index > this.#longestRow) this.#longestRow = index;
    }

    get LongestRow() {
        return this.#longestRow;
    }

    AddColumn(column) {
        if (!column instanceof Charaph.Column) throw new Error("Must be an instance of Charaph.Column");
        for (var i = 0; i < this.#columns.length; i++) {
            if (name.toLowerCase() == this.#columns[i].Name.toLowerCase()) throw new Error(`Column with name ${name} already exists in parent table.`);
        }
        this.#columns[this.#columns.length] = column;
        for (var i = 0; i < this.#columns.length; i++) {
            for (var j = this.#columns.length - 1; j > i; j--) {
                if (this.#columns[i].CellIndex > this.#columns[j].CellIndex) {
                    var t = this.#columns[i];
                    this.#columns[i] = this.#columns[j];
                    this.#columns[j] = t;
                }
            }
        }
        if (column.CellIndex > this.#longestRow) this.#longestRow = column.CellIndex;
    }
}

// Charaph.Table = CharaphTable;

export class CharaphColumn {
    #name;
    #cellIndex;
    constructor(table, name, cellIndex) {
        if (table === undefined) throw new Error("Cannot create a column without a parent table.");
        if (!table instanceof Charaph.Table) throw new Error("Table must be an instance of Charaph.Table");
        if (name == null) throw new Error("Cannot create columns with undefined names.");
        if (typeof name !== 'string' || !name instanceof String) throw new Error("Name must be a string or be left undefined.");
        if ((typeof name === 'string' || name instanceof String)) {
            name = name.trim();
            if (name.length == 0) throw new Error("Name value cannot be empty.");
        }
        if (isNaN(cellIndex)) throw new Error("Cell index must be a numeric value.");
        if (cellIndex < 0) throw new Error("Cell index must be greater than or equal to zero.");

        this.#name = name;
        this.#cellIndex = cellIndex;
        table.AddColumn(this);
    }

    get CellIndex() {
        return this.#cellIndex;
    }
}

// Charaph.Column = CharaphColumn;

export class CharaphRow {
    #cells;
    #rowIndex;
    #biggestIndex = Number.MIN_VALUE;
    constructor(table, rowIndex) {
        if (!table instanceof CharaphTable) throw new Error("Table must be an instance of Charaph.Table");
        if (isNaN(rowIndex)) throw new Error("RowIndex value must be numeric.");
        if (rowIndex < 0) throw new Error("Row index must be zero (0) or greater.");
        this.#cells = [];
        this.#rowIndex = rowIndex;
    }

    AddCell(cell) {
        if (!cell instanceof Charaph.Cell) throw new Error("Must be an instance of Charaph.Cell");
        for (var i = 0; i < this.#cells.length; i++) {
            if (this.#cells[i].CellIndex == cell.CellIndex) throw new Error("Row already has cell at the same index.");
        }
        this.#cells[this.#cells.length] = cell;
        for (var i = 0; i < this.#cells.length; i++) {
            for (var j = this.#cells.length - 1; j > i; j--) {
                if (this.#cells[i].CellIndex > this.#cells[j].CellIndex) {
                    var t = this.#cells[i];
                    this.#cells[i] = this.#cells[j];
                    this.#cells[j] = t;
                }
            }
        }
        if (cell.CellIndex > this.#biggestIndex) this.#biggestIndex = cell.CellIndex;
    }

    get RowIndex() {
        return this.#rowIndex;
    }

    get Cells() {
        return this.#cells;
    }

    get Length() {
        return this.#biggestIndex;
    }
}

// Charaph.Row = CharaphRow;

export class CharaphCell {
    value;
    #cellIndex;
    constructor(value, table, rowIndex, cellIndex) {
        if (isNaN(cellIndex)) throw new Error("Index value must be numerical.");
        if (!table instanceof CharaphTable) throw new Error("Table must be an instance of Charaph.Table");

        this.#cellIndex = cellIndex;
    }

    get CellIndex() {
        return this.#cellIndex;
    }
}

// Charaph.Cell = CharaphCell;