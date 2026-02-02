# Amber Particle SSH

> A GPU-accelerated SSH terminal that renders text as living, shimmering particles

![Amber Particle SSH Demo](assets/demo.gif)

**Amber Particle SSH** transforms your terminal into a mesmerizing display of millions of glowing particles. Every character becomes a cloud of light that pulses, shimmers, and responds to your mouse. Think nixie tubes meets modern GPU compute shaders.

## ✨ Features

- **8 Million Particles** — Each character rendered as hundreds of individual particles
- **GPU Compute Shaders** — OpenGL 4.5 compute pipeline for real-time physics at 120 FPS
- **Force Field Interaction** — Mouse creates physics fields that push particles in real-time
- **Multiple Fonts** — Classic 8x8, HighRes 16x16, Segmented, Vector, and more
- **CRT Effects** — Scanlines, phosphor glow, bloom, vignette, chromatic aberration
- **Themes** — Amber, Green, Synthwave gradient, and customizable color tints
- **Real SSH** — Full terminal emulation via libssh2 + libvterm
- **Multi-Tab Sessions** — Multiple SSH connections in tabbed interface

## 🎬 How It Works

```
SSH Data → ANSI Parser → Terminal Buffer (80×25) → Character Bitmaps 
    → Particle Generation → Physics Compute Shader → GPU Render → Post-FX → Display
```

Each lit pixel in a character's bitmap spawns 5-50 particles in a Gaussian distribution. Dual sine waves create organic pulse and flicker animations. The result is text that feels alive.

## 📦 Dependencies

### Ubuntu/Kubuntu
```bash
sudo apt install build-essential cmake qt6-base-dev libgl-dev \
    libssh2-1-dev libvterm-dev
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake qt6-base libssh2 libvterm
```

## 🔨 Build

```bash
cd antigrav
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

## 🚀 Run

```bash
./build/AmberParticleSSH
```

Or connect directly:
```bash
./build/AmberParticleSSH --host user@server.com
```

## ⚙️ Configuration

| Setting | Description | Range |
|---------|-------------|-------|
| Density | Particles per pixel | 5-50 |
| Spread | Particle distribution radius | 0.1-0.8 |
| Glow Intensity | Particle brightness | 0.5-2.0 |
| Scanline Intensity | CRT scanline effect | 0.0-1.0 |
| Theme | Color scheme | Amber, Green, Synthwave |

## 🎨 Themes

- **Amber** — Classic warm CRT phosphor glow
- **Green** — Retro terminal green
- **Synthwave** — Purple-to-orange gradient with bloom

## 📐 Technical Details

The particle system uses a multi-layer rendering approach:

1. **Layer 0** — Large diffuse particles (background glow)
2. **Layer 1** — Medium particles (main body)
3. **Layer 2** — Small bright particles (sharp detail)
4. **Layer 3** — Tiny HDR particles (sparkle highlights)

Physics simulation runs on GPU compute shaders, allowing real-time interaction with millions of particles while maintaining 120+ FPS on GTX 1080 Ti class hardware.

See [technical_manual.pdf](technical_manual.pdf) for complete documentation.

## 📝 License

MIT License — See [LICENSE](LICENSE)

## 🙏 Credits

Built with:
- [Qt 6](https://www.qt.io/) — UI framework
- [libssh2](https://libssh2.org/) — SSH protocol
- [libvterm](http://www.leonerd.org.uk/code/libvterm/) — Terminal emulation

---

*"Every character is a constellation."*
