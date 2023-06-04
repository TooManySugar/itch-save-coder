# About

Itch! Save Coder is a tool to decode and encode back save files of game Itch!

# Usage
```shell
itch-save-coder [OPTIONS]... [INPUT_FILE]
```

#### OPTIONS
`-h`    `--help`            Print this usage info to output  
`-v`    `--version`         Print program version info to output  
`-o`    `--output <FILE>`   Specify output file, console(stdout) by default  
If neither 'help' or 'version' options are specified INPUT_FILE required

#### INPUT_FILE
Path to input file either save data or decoded xml data

# Save file format

Itch! stores save data in file
```shell
%userprofile%\Documents\JoWood\Itch\GameData.dat
```
Contents of this file is 34 [XOR](https://en.wikipedia.org/wiki/Exclusive_or)ed bytes of [UTF-16LE](https://en.wikipedia.org/wiki/UTF-16#Byte-order_encoding_schemes) encoded XML document.

So essentially <b>Itch! Save Coder</b> does XOR of each byte of the input file with 34 and stores result to output, this works in both ways to encode and decode save files.

### XML Schema
See [GameData.xsd](GameData.xsd)

# Compiling

```shell
make
```