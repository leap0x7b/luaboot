# luaboot
A fully scriptable UEFI bootloader.

## Why?
1. ~~I'm mentally insane and I have an urge of making cursed stuff~~
2. Why not, I guess?

## Who is this for?
Probably no one. Maybe r/unixporn ricers I guess?

## Configuration
When there's no /boot/luaboot/config.lua, luaboot will boot into a Lua REPL by default. There will be a module
called `luaboot` to control everything and other stuff, such as EFI stuff, ELF parsing, and other low-level
stuff.

## Acknowledgements
Some files are taken and modified from [Limine](https://github.com/limine-bootloader/limine) as well as the
old rewrite branch of [FaruOS (now Kora)](https://github.com/kora/ydin/tree/rewrite-old).

## License
The files included in this repository are licensed under the [MIT license](https://opensource.org/licenses/MIT).

This means you are free to use and modify luaboot and it's source code, even in a proprietary program. You
don't need to open source any modifications to the source code but it's heavily encouraged to do so.

See the [license](LICENSE) for more information.

## Contributions
Anyone is welcome to contribute to this project. Please note that this project is released with a [Contributor
Code of Conduct](CODE_OF_CONDUCT.md). By participating in this project you agree to abide by its terms.