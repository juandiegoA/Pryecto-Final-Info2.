# Ultimate en TRON

Base inicial de un videojuego 2D en C++17 y Qt 6.

## Estructura

- `include/logic` y `src/logic`: entidades y contratos de la logica del juego, sin dependencias Qt.
- `include/gui` y `src/gui`: interfaz Qt minima.
- `src/main.cpp`: arranque de la aplicacion.
- `CMakeLists.txt`: configuracion de compilacion con CMake.

## Compilacion

```powershell
$env:Path = 'C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;' + $env:Path
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:\Qt\6.10.2\mingw_64 -DCMAKE_CXX_COMPILER=C:\Qt\Tools\mingw1310_64\bin\g++.exe -DCMAKE_MAKE_PROGRAM=C:\Qt\Tools\Ninja\ninja.exe
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build
```

El kit concreto de Qt puede variar segun la instalacion local.
