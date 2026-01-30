# emu_ape

A self-contained emulator server built as an [Actually Portable Executable](https://justine.lol/ape.html). Run a single binary on any OS and get a browser-based retro game emulator -- no dependencies, no installation.

The server embeds [EmulatorJS](https://github.com/EmulatorJS/EmulatorJS) and all its cores directly inside the APE binary using Cosmopolitan's zip filesystem. Drop ROM files into a `roms/` folder and they appear in the web UI automatically.

## Usage

1. Download `emu_ape` from the [releases page](https://github.com/MattCruikshank/emu_ape/releases)
2. Create a `roms/` directory next to the binary
3. Drop your ROM files into `roms/`
4. Run `./emu_ape`
5. Open http://localhost:6711

ROMs are listed in the browser. Click one to play, or drag and drop a ROM file directly onto the page.

## Building from source

Requires [Cosmopolitan](https://github.com/jart/cosmopolitan) (`fatcosmocc` and `zip`) and `7z`.

```
make
```

This will automatically download EmulatorJS v4.2.3, compile the server, and embed all assets into the binary.

```
make run
```

## Acknowledgments

Many thanks to the [EmulatorJS](https://github.com/EmulatorJS/EmulatorJS) project for providing a fantastic browser-based emulation platform. This project would not be possible without their work packaging RetroArch cores for the web.
