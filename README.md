# RockOS
It's named after a rock because making this is going to be hard, like a rock

Building
```bash

mkdir -p disk/EFI/BOOT
make
make run

```

OVMF is always found in weird places so just copy it to the root of the project and make sure it's called OVMF.fd