# demoos
**demoOS** è un sistema operativo **bare‑metal** progettato per il Raspberry Pi 3B, scritto in C e Assembly. Non dipende da alcun sistema operativo o libreria standard: viene eseguito direttamente sull’hardware, gestendo periferiche e risorse in modo autonomo.

## Installazione
Prima di eseguire il sistema operativo, assicurati di avere installato i seguenti pacchetti:
``` bash
sudo apt-get install build-essential

sudo apt-get install gcc-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-gcc) /usr/local/bin/aarch64-elf-gcc

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-ld) /usr/local/bin/aarch64-elf-ld

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-objcopy) /usr/local/bin/aarch64-elf-objcopy

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-ld) /usr/local/bin/aarch64-elf-ld

sudo apt-get install qemu-system-arm
```

Inoltre, per usare il filesystem è necessario creare un disco virtuale usando i seguenti comandi. 
``` bash
sudo apt-get install dosfstools

dd if=/dev/zero of=disk.img bs=1M count=64

parted disk.img --script mklabel msdos

parted disk.img --script mkpart primary fat32 1MiB 100%

sudo losetup -fP disk.img

lsblk # cerca il device in cui è stato montato il disco

sudo mkfs.fat -F 32 /dev/loop0p1 # sostituisci loop0p1 con quello in cui è stato montato

sudo losetup -d /dev/loop0
```

## Compilazione ed Esecuzione

### Comandi principali
``` bash
make          # compila il tutto e genera il file kernel8.img
make run      # esegue il kernel in QEMU (emulazione Raspberry Pi 3B)
make clean    # rimuove file oggetto e binari generati
```

### Primo avvio
Se è la prima volta che compili ed esegui DemoOS in QEMU:
``` bash
make
make run
```

### Avvio successivo
Se il file `kernel8.img` è già presente e non hai modificato il codice:
``` bash
make run
```
