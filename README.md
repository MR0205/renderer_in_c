# Renderer in C
Prototype of a game-engine utilising no external dependencies except WIN32 API

## Build instructions
Currently the projects is not cross-platform and only windows platform layer is implemented. 

To build the project Visual Studio must be installed. Currently the initialisation of VS build tool chain is hard-wired into src/build.bat so you may want to verify that the path to vcvars64.bat is indeed correct.

After fetching the repository and locaing to the root project directory simply launch src/build.bat command with call syntax of your shell.
```
& src/build.bat # syntax for Power Shell

```
## Authors

[Dmitry Morozov](https://github.com/MR0205)


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
