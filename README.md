# PacketSniffer
PacketSniffer is a tool for capturing and analyzing network packets, designed using `libpcap`, `ncurses`, and `libxlsxwriter`.

---

## Prerequisites
Before building the project, make sure you have the following libraries installed:

1. **libpcap**: For network packet capture.
2. **ncurses**: For the terminal-based user interface.
3. **libxlsxwriter**: For generating reports in Excel format.

### Installing dependencies

#### On Debian/Ubuntu-based distributions:
```bash
sudo apt update
sudo apt install -y libpcap-dev libncurses-dev libxlsxwriter-dev
```

#### On Arch Linux-based distributions:
libxlsxwriter is not available in the official Arch Linux repositories, but it can be installed from the AUR (Arch User Repository). You can use an AUR helper like yay or paru to install it.
```bash
sudo pacman -Syu
sudo pacman -S libpcap ncurses
yay -S libxlsxwriter
```

#### On Fedora/RHEL-based distributions:
```bash
sudo dnf install -y libpcap-devel ncurses-devel libxlsxwriter-devel
```

If you are using a different Linux distribution, consult the corresponding package manager to install these libraries.

---

## Building and Installation
Follow these steps to build and run the program:

### 1. Clone the repository
Clone the repository to your local machine:
```bash
git clone https://github.com/Andrew869/PacketSniffer.git
cd PacketSniffer
```

### 2. Create the build directory
Create a directory for the build files and navigate to it:
```bash
mkdir build
cd build
```

### 3. Configure with CMake
Configure the project using CMake:
```bash
cmake ..
```
This will generate the necessary build files.

### 4. Build the project
Build the program by running:
```bash
make
```

### 5. Install (optional)
If you want to install the executable to `/usr/local/bin` to access it from anywhere, run:
```bash
sudo make install
```
This will generate the following files:
`/usr/local/bin/packetsniffer`
`/usr/share/applications/packetsniffer.desktop`

---

### Additional Notes
- If you want to uninstall the program, you can run `make uninstall` in the build directory, provided that you've configured the uninstall target in your CMakeLists.txt as described in the setup.
