# ImagePipeline V2 & GUI Integration Strategy

Diese Dokumentation beschreibt die Architektur der Bildverarbeitungs-Engine und dient als Leitfaden für die GUI-Entwicklung.

---

## 1. Architektur-Konzept: JIT vs. AOT

Wir verfolgen eine **hybride Strategie**, um Entwicklungsgeschwindigkeit und finale Performance zu vereinen.

### A. Entwicklung: JIT (Just-In-Time) Compilation
Aktuell läuft die Engine im JIT-Modus via `ImagePipeline`.
*   **Funktionsweise:** Die Engine analysiert zur Laufzeit, welche Filter der User aktiviert hat, und baut daraus einen GPU-Shader.
*   **Vorteil:** Maximale Flexibilität. Wir können neue Filter hinzufügen oder Code ändern, ohne einen langen Build-Prozess.
*   **Nachteil:** Der **erste** Aufruf einer Pipeline-Konfiguration dauert lange (~40 Sekunden), weil der Shader erst kompiliert werden muss.
*   **Einsatz:** Während der Entwicklungsphase und für das Prototyping neuer Effekte.

### B. Produktion: AOT (Ahead-of-Time) Compilation
Für das finale Release wird die Pipeline auf AOT umgestellt.
*   **Funktionsweise:** Wir definieren **eine** feste "Super-Pipeline", die alle möglichen Effekte enthält. Diese wird **beim Bauen der Software** einmalig kompiliert.
*   **Vorteil:** **Null Wartezeit** beim Start. Die App startet sofort.
*   **Nachteil:** Die Reihenfolge der Effekte ist statisch festgelegt (was physikalisch aber ohnehin korrekt ist).
*   **Einsatz:** Im Release-Build für den Endkunden.

---

## 2. Die Brücke: Controller-Pattern

Damit die GUI **jetzt** schon entwickelt werden kann, ohne später alles neu schreiben zu müssen, haben wir eine Abstraktionsschicht eingezogen.

Das **Ziel** für den GUI-Entwickler: **Ignoriere die Engine-Details (JIT/AOT). Arbeite nur mit dem Controller.**

### Die Komponenten
1.  **`ImageState` (Das Modell):** Ein einfaches `struct`, das den kompletten Look des Bildes beschreibt (Werte für Helligkeit, Kontrast, Drehung etc.). Es gibt keine Logik, nur Daten.
2.  **`ImageController` (Die Steuerung):** Diese Klasse nimmt den `ImageState` entgegen und übersetzt ihn für die Engine.
    *   *Heute (JIT):* Sie baut die Pipeline-Liste dynamisch zusammen und führt sie aus.
    *   *Morgen (AOT):* Wir tauschen nur das Innere dieser Klasse aus. Sie setzt dann Hardware-Parameter auf dem fertigen Shader.

**Die GUI-Logik bleibt zu 100% identisch, egal ob JIT oder AOT läuft.**

---

## 3. Integrations-Guide für die GUI

### Schritt 1: Initialisierung
Erstelle eine Instanz des Controllers. Dieser kümmert sich um die Engine.

```cpp
#include "image_processing/controller/image_controller.h"

// In deiner Main-Window Klasse:
std::unique_ptr<ImageController> m_controller;
ImageState m_state; // Der aktuelle Zustand des Bildes

void init() {
    m_controller = std::make_unique<ImageController>();
    
    // Bild laden (CV Mat)
    cv::Mat image = cv::imread("photo.jpg");
    m_controller->setImage(image);
}
```

### Schritt 2: Slider-Logik (Data Binding)
Verbinde deine UI-Elemente direkt mit dem `ImageState`.
**Wichtig:** Füge **keine** Operationen "zur Liste hinzu". Ändere nur den Wert im State.

```cpp
// Beispiel: Helligkeits-Slider bewegt
void onBrightnessChanged(float value) {
    // 1. Update State
    m_state.brightness = value;
    
    // 2. Informiere Controller
    m_controller->update(m_state);
    
    // 3. Hole neues Bild & Zeige es an
    cv::Mat result = m_controller->process();
    displayImage(result);
}
```

### Schritt 3: Umgang mit der "Wartezeit" (Dev-Mode)
Da wir noch im JIT-Modus sind, wird der **allererste** Aufruf von `process()` nach dem Start ca. 40 Sekunden dauern.
*   **Tipp:** Zeige beim ersten Laden einen "Initializing Engine..." Spinner an.
*   Sobald die Engine einmal lief ("warm" ist), gehen Änderungen an Slidern (Parameter-Updates) viel schneller, da der JIT-Compiler oft cachen kann oder nur Teile aktualisiert.

---

## 4. Do's and Don'ts

### ✅ DO:
*   Nutze ausschließlich `ImageState` und `ImageController`.
*   Gehe davon aus, dass Effekte eine **feste Reihenfolge** haben (z.B. kommt Weißabgleich immer vor dem Beschnitt). Die Engine kümmert sich darum.
*   Implementiere "Undo/Redo" in der GUI, indem du Kopien von `ImageState` speicherst (Stack). Das ist viel robuster als Engine-seitiges Undo.

### ❌ DON'T:
*   Nutze **nicht** `ImagePipeline` direkt (`addOperation`, `removeOperation`), es sei denn, du entwickelst neue Core-Features.
*   Baue **keine** "Layer-Liste" in der GUI für Standard-Korrekturen (Helligkeit, Kontrast). Das passt nicht zum AOT-Modell.
*   Erwarte nicht, dass Filter-Reihenfolgen vom User geändert werden können.

---

## 5. Zukunfts-Sicherheit
Wenn wir auf AOT umstellen, passiert Folgendes:
1.  Wir tauschen `ImageController.cpp` aus.
2.  Die GUI recompiliert.
3.  **Fertig.** Keine einzige Zeile GUI-Code muss geändert werden.
