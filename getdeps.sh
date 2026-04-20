# Git hates me.

mkdir deps
git clone 'https://github.com/osdev0/cc-runtime.git' deps/cc-runtime --branch packed --depth 1
git clone 'https://github.com/osdev0/freestnd-c-hdrs-0bsd.git' deps/freestanding-c-headers --depth 1
git clone 'https://github.com/Limine-Bootloader/limine-protocol.git' deps/limine-protocol --depth 1
git clone 'https://github.com/Limine-Bootloader/Limine.git' deps/limine --branch v11.3.1-binary --depth 1
git clone 'https://github.com/Mintsuki/Flanterm' --depth 1
