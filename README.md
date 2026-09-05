# UnityKernel

This is a simple C++ kernel designed for X86-64(AMD64) architecture.

> **Current status:** This is early alpha version. Work in progress.


## Features

- Freestanding C++ kernel code without the host standard library
- Multiboot2 boot protocol and GRUB boot menu

## Run the ISO with QEMU

Docker is only needed to build the ISO. After the build, run the exported
`out/kernel.iso` with QEMU installed on the host operating system.

### Install QEMU

Linux (Debian or Ubuntu):

```sh
sudo apt update
sudo apt install qemu-system-x86
```

macOS (Homebrew):

```sh
brew install qemu
```

Windows (Windows Package Manager):

```powershell
winget install SoftwareFreedomConservancy.QEMU
```

### Build the ISO with Docker

Export the ISO from the build container:

```sh
mkdir -p out
docker build -t unitykernel .
docker run --rm -v "$PWD/out:/out" unitykernel \
	sh -c 'cp build/kernel.iso /out/kernel.iso'
```

### Run on Any System

Run this command from the project directory. QEMU opens its normal graphical
window and sends the serial output to the terminal:

```sh
qemu-system-x86_64 \
	-cdrom out/kernel.iso \
	-serial stdio
```

On Windows PowerShell, use the same command on one line or replace `\` line
continuations with the PowerShell backtick.

### Optional Acceleration

The portable command above works without hardware virtualization. Use the
following platform-specific options when acceleration is available:

Linux with KVM:

```sh
qemu-system-x86_64 -cdrom out/kernel.iso \
	-enable-kvm -cpu host -serial stdio
```

macOS with Hypervisor.framework:

```sh
qemu-system-x86_64 -cdrom out/kernel.iso \
	-accel hvf -cpu host -serial stdio
```

Windows with WHPX:

```powershell
qemu-system-x86_64 -cdrom out/kernel.iso -accel whpx -cpu max -serial stdio
```

## Docker

The commands below use POSIX shell syntax and work in Linux, macOS, Git Bash,
or WSL. In PowerShell, run a command on one line or replace `\` line
continuations with the PowerShell backtick.

### Build the ISO

Build the kernel and create `kernel.iso` in a reproducible container:

```sh
docker build -t unitykernel .
docker run --rm -v "$PWD/out:/out" unitykernel \
	sh -c 'cp build/kernel.iso /out/kernel.iso'
```

The ISO is also available inside the image at
`/workspace/build/kernel.iso`.

### Run on Linux

Run QEMU with hardware acceleration and a graphical display over VNC:

```sh
docker run --rm -it --device /dev/kvm \
	-p 127.0.0.1:5900:5900 \
	unitykernel qemu-system-x86_64 \
	-cdrom /workspace/build/kernel.iso \
	-enable-kvm -cpu host -vnc :0 -serial stdio
```

Connect a VNC client to `127.0.0.1:5900`:

- Linux: TigerVNC, Remmina, or another VNC client
- macOS: Screen Sharing, TigerVNC, or RealVNC
- Windows: TigerVNC or RealVNC

For macOS Screen Sharing, use:

```text
vnc://127.0.0.1:5900
```

### Run on macOS or Windows

Docker Desktop usually does not provide `/dev/kvm`. Use QEMU's software
emulation and connect with a VNC client as above:

```sh
docker run --rm -it \
	-p 127.0.0.1:5900:5900 \
	unitykernel qemu-system-x86_64 \
	-cdrom /workspace/build/kernel.iso \
	-accel tcg -cpu qemu64 -vnc :0 -serial stdio
```

Keep this terminal running while using the VNC client. Press `Ctrl+C` to stop
QEMU.

### Run without a graphical display

To use only the serial console:

```sh
docker run --rm -it \
	unitykernel qemu-system-x86_64 \
	-cdrom /workspace/build/kernel.iso \
	-display none -serial stdio
```

## Disclaimer

**UnityKernel is an independent open-source project and is not affiliated with, endorsed by, sponsored by, or otherwise associated with Unity Technologies.**

“Unity” and “Unity Technologies” are trademarks of their respective owners. The name “UnityKernel” does not imply any affiliation with Unity Technologies or its products and services.

## License

UnityKernel is licensed under the GNU General Public License v3.0. See
[LICENSE](LICENSE) for the full text.
