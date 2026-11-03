# ReadBlank Qt

A Qt Quick desktop application for reading web content.

## Requirements

- Qt 6.5 or later (Qt Quick, Qt Network, Qt Multimedia)
- C++17
- CMake 3.21 or later
- Ninja
- libxml2
- Piper TTS HTTP server

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Piper TTS Server

ReadBlank Qt uses [Piper](https://github.com/OHF-Voice/piper1-gpl) for speech synthesis. The Piper HTTP server must be running before launching the application.

Install Piper and its HTTP server dependency using pipx:

```bash
pipx install piper-tts
pipx inject piper-tts flask
```

Download the following voice models and place them in `~/Models/Piper`:

- `en_US-lessac-medium`
- `hu_HU-anna-medium`
- `fr_FR-siwis-medium`

### Download Voice Models

ReadBlank Qt currently supports the following Piper voices:

| Language | Voice model |
|---|---|
| English (US) | `en_US-lessac-medium` |
| Hungarian | `hu_HU-anna-medium` |
| French | `fr_FR-siwis-medium` |

Download the voice models using Piper's voice downloader:

```bash
~/.local/pipx/venvs/piper-tts/bin/python \
    -m piper.download_voices \
    --download-dir ~/Models/Piper \
    en_US-lessac-medium \
    hu_HU-anna-medium \
    fr_FR-siwis-medium
```

This downloads the ONNX model files and their corresponding configuration files into `~/Models/Piper`.

Voice models have their own licensing terms, separate from the Piper engine. Check the license information provided with each model before redistribution or use in other projects.

Start the server using the provided script:

```bash
./scripts/start-piper.sh
```

The server listens on `http://127.0.0.1:5000`.

Keep the server running while using speech synthesis.

## Run

```bash
./build/readblank-qt
```

## License

Copyright (C) 2026 Yasuhiro Yamakawa

This project is licensed under the GNU General Public License v3.0 or later.
See [LICENSE](LICENSE) for details.

## Contact

For bug reports and feature requests, please use
[GitHub Issues](https://github.com/kawatab/readblank-qt/issues).
