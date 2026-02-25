# NeoBot — Mod de Macros para Geometry Dash Android

**NeoBot** es un mod de macros frame-perfect para Geometry Dash 2.2081 en Android, construido con el framework [Geode](https://geode-sdk.org/). Permite grabar y reproducir secuencias de inputs táctiles con precisión total.

**Canal de YouTube:** [youtube.com/@neomc12](https://youtube.com/@neomc12?si=5fEbcEq5Fk7wjEy3)

---

## Cómo Funciona

El sistema se basa en tres pilares fundamentales. En primer lugar, la **interceptación de inputs**: el hook en `GJBaseGameLayer::handleButton` captura todos los eventos de toque procesados por Cocos2d-x antes de que lleguen al jugador. En segundo lugar, la **reproducción directa**: en lugar de simular eventos del sistema Android (lo cual sería poco confiable), el mod modifica directamente el estado interno del jugador llamando a `PlayerObject::pushButton` y `releaseButton` en el frame exacto correspondiente. En tercer lugar, la **sincronización de frames**: el contador de frames de `PlayLayer` (`m_currentFrame`) se usa como referencia temporal absoluta, garantizando que los inputs se reproduzcan en el momento exacto.

---

## Estructura del Proyecto

```
NeoBot/
├── CMakeLists.txt          # Configuración de compilación para Geode
├── mod.json                # Metadatos del mod (ID, versión, compatibilidad)
├── about.md                # Descripción para la tienda de Geode
├── changelog.md            # Historial de versiones
├── README.md               # Este archivo
└── src/
    ├── main.cpp                    # Punto de entrada e inicialización
    ├── manager/
    │   └── MacroManager.hpp        # Lógica central: grabación, reproducción, serialización
    ├── hooks/
    │   ├── PlayLayerHook.hpp       # Hooks de PlayLayer y GJBaseGameLayer
    │   ├── PauseLayerHook.hpp      # Botón de NeoBot en el menú de pausa
    │   └── SafeModeHook.hpp        # Bloqueo de envío al leaderboard
    └── ui/
        ├── NeoBotUI.hpp            # Indicador visual REC/PLAY en PlayLayer
        ├── NeoBotMenuLayer.hpp     # Menú principal con toggles y botones
        ├── SaveMacroLayer.hpp      # Popup para guardar macros con nombre
        └── LoadMacroLayer.hpp      # Popup para cargar macros guardadas
```

---

## Requisitos de Compilación

| Requisito | Versión mínima |
|-----------|---------------|
| Geode SDK | 4.4.0 |
| Android NDK | r26+ |
| CMake | 3.21+ |
| C++ Standard | C++20 |
| Geometry Dash (Android) | 2.2081 |

---

## Instrucciones de Compilación

### 1. Instalar Geode CLI

Descarga e instala el [Geode CLI](https://github.com/geode-sdk/cli/releases) para tu sistema operativo de desarrollo (Windows, macOS o Linux).

### 2. Configurar el SDK de Geode

```bash
geode sdk install
geode sdk update
```

Asegúrate de que la variable de entorno `GEODE_SDK` apunta al directorio del SDK instalado.

### 3. Configurar el NDK de Android

Descarga el Android NDK r26+ desde [developer.android.com/ndk](https://developer.android.com/ndk/downloads) y configura `ANDROID_NDK_HOME`.

### 4. Compilar el mod

```bash
cd NeoBot
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
         -DANDROID_ABI=arm64-v8a \
         -DANDROID_PLATFORM=android-23 \
         -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

El archivo `.geode` resultante se encontrará en el directorio de salida de Geode.

### 5. Instalar en el dispositivo

Copia el archivo `.geode` al directorio de mods de Geode en tu dispositivo Android:
```
/sdcard/Android/data/com.robtopx.geometryjump/files/geode/mods/
```

O usa el Geode Loader directamente desde el juego para instalar el archivo.

---

## Uso del Mod

Una vez instalado, accede al menú de NeoBot desde el botón que aparece en el menú de pausa durante un nivel. Desde allí puedes activar la grabación, la reproducción, el Safe Mode y el auto-guardado, así como guardar y cargar macros.

---

## Notas Técnicas para Android

El mod no utiliza ningún código exclusivo de Windows. Los inputs táctiles son interceptados a nivel del método `GJBaseGameLayer::handleButton`, que es el punto donde Cocos2d-x entrega los eventos táctiles procesados al juego. Esto es más confiable y portable que interceptar los eventos de toque crudos del sistema Android.

La reproducción se realiza llamando directamente a los métodos internos del jugador (`pushButton`/`releaseButton`), lo que garantiza que el estado del juego sea idéntico al de la grabación original, frame por frame.
