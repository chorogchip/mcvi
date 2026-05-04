# mcvi

`mcvi` is a terminal, vi-like editor for sparse 3D Minecraft block data.

The visible editing plane is a fixed `y` slice:

- horizontal screen axis: `x`
- vertical screen axis: `z`
- layer axis: `y`

Blocks are edited as single display characters. Minecraft block states are assigned with aliases such as `:bl d minecraft:dirt`.

## Build

```sh
make
```

## Run

```sh
./mcvi
./mcvi file.json
./mcvi file.schem
make run
```

The editor needs an interactive terminal for normal use.

## Auto Mode

`-auto` replays keys without entering raw terminal mode. This is useful for smoke tests.

```sh
./mcvi -auto "iaaaaaa\eDacccHHH\e:w /tmp/test.json\n:q!"
./mcvi -auto ":bl a minecraft:stone\niaaa\e:w /tmp/test.schem\n:q!"
./mcvi -auto ":meta name Test\n:meta author mkkim\n:w /tmp/test.schem\n:q!" examples/simple_column.json
```

Auto escapes:

- `\n`, `\r`: Enter
- `\e`: Escape
- `\b`: Backspace
- `\t`: Tab
- `\\`: literal backslash

## Files

- `.json`: mcvi JSON, stores blocks and aliases
- `.schem`: Sponge Schematic v3, GZip-compressed NBT export
- `.nbt`, `.mca`: detected but not implemented yet

Example files:

- `examples/simple_column.json`
- `examples/simple_column.schem`

## Movement

- `h`, `j`, `k`, `l`: move left, down, up, right in the current `y` slice
- arrow keys: same as `h`, `j`, `k`, `l`
- `0`: move to `x=0`
- `$`: move to last occupied block on the current row
- `^`: move to first occupied block on the current row
- `H`: move one layer up, `y+1`
- `L`: move one layer down, `y-1`

## Direction

Insert mode advances in the active direction.

- `Ctrl-f`: `+x`
- `Ctrl-b`: `-x`
- `Ctrl-u`: `+y`
- `Ctrl-d`: `-y`
- `Ctrl-e`: `+z`
- `Ctrl-y`: `-z`

## Editing

- `i`: insert at cursor
- `a`: append after cursor
- `I`: insert at first occupied block on row
- `A`: append after end of row
- `Esc`: return to Normal mode
- `Enter` in Insert mode: new row, `z+1`, `x=0`
- `Backspace` in Insert mode: delete previous block in active direction
- `x`: delete block at cursor
- `X`: delete block before cursor
- `D`: delete from cursor to end of row
- `C`: delete from cursor to end of row, then enter Insert mode
- `s`: delete block at cursor, then enter Insert mode
- `S`: delete current row, then enter Insert mode

## Commands

- `:w`: write current file
- `:w file`: write to file
- `:q`: quit if clean
- `:q!`: quit without saving
- `:wq`, `:x`: write and quit
- `:bl`: show block alias usage
- `:bl all`: show all aliases
- `:bl emp`: show unused alias letters
- `:bl a`: show alias `a`
- `:bl a minecraft:stone`: set alias `a`
- `:meta`: show schematic metadata
- `:meta all`: show schematic metadata
- `:meta name VALUE`: set schematic name
- `:meta author VALUE`: set schematic author
- `:meta version VALUE`: set Minecraft data version
- `:meta data_version VALUE`: same as `version`
- `:meta mc_version VALUE`: same as `version`

## Schematic Export Notes

`.schem` export writes Sponge Schematic v3:

- root compound contains `Schematic`
- block data is stored in `Schematic.Blocks.Data`
- palette is stored in `Schematic.Blocks.Palette`
- metadata is stored in `Schematic.Metadata`
- every non-space block character must have an alias before export
