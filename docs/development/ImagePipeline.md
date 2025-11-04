# ImagePipeline - Technische Dokumentation

## 📋 ÜBERBLICK
Die ImagePipeline ist das Kernsystem für non-destructive Bildbearbeitung. Sie verwaltet eine Historie von Bearbeitungsschritten und wendet diese in Echtzeit auf das Originalbild an.

## 🏗️ ARCHITEKTUR

### Kernkomponenten:
- originalImage: Unverändertes Ausgangsbild (cv::Mat)
- operations: Liste aller Bearbeitungsoperationen (Vector von ImageOperation)
- undoneOperations: Undo-Historie (Vector von ImageOperation) 
- cachedResult: Zwischengespeichertes Ergebnis (cv::Mat)
- cacheValid: Cache-Gültigkeitsflag (bool)
- liveOperation: Temporäre Operation für Echtzeit-Vorschau (ImageOperation)

## 🔧 FUNKTIONEN

### Bildverwaltung:
- setImage(): Lädt Bild in Pipeline (macht Kopie)
- getOriginalImage(): Gibt Originalbild zurück
- hasImage(): Prüft ob Bild geladen

### Operations-Management:
- addOperation(): Fügt Operation hinzu (löscht Undo-Historie)
- insertOperation(): Fügt Operation an Position ein
- removeOperation(): Entfernt Operation an Position
- clearOperations(): Löscht alle Operationen
- getOperation(): Gibt Operation an Index zurück

### Verarbeitung:
- process(): Wendet alle Operationen an (mit Cache)
- processUpTo(): Verarbeitet bis zu bestimmter Operation

### Echtzeit-Vorschau:
- setLiveOperation(): Setzt temporäre Live-Operation
- clearLiveOperation(): Entfernt Live-Operation

### Undo/Redo:
- undo(): Macht letzte Operation rückgängig
- redo(): Stellt letzte Operation wieder her
- canUndo()/canRedo(): Prüft ob Aktion möglich

### Cache-Management:
- invalidateCache(): Markiert Cache als ungültig
- isCacheValid(): Prüft Cache-Gültigkeit

## 🔄 WORKFLOWS

### Normaler Bearbeitungsflow:
1. setImage() - Bild laden
2. addOperation() - Bearbeitungsschritte hinzufügen
3. process() - Ergebnis berechnen und anzeigen

### Echtzeit-Slider Flow:
1. setLiveOperation() - Temporäre Operation setzen
2. process() - Sofortige Vorschau anzeigen
3. Bei Loslassen: addOperation() + clearLiveOperation()

### Undo/Redo Flow:
1. undo() - Operation in Undo-Historie verschieben
2. redo() - Operation zurück in aktive Liste

## 🎯 BESONDERHEITEN

### Non-destructive Editing:
- Originalbild bleibt immer unverändert
- Alle Bearbeitungen sind reproduzierbar
- Volle Bearbeitungshistorie verfügbar

### Performance-Optimierungen:
- Cache-System vermeidet wiederholte Berechnungen
- Live-Operations für flüssige Echtzeit-Vorschau
- Exception-Safety bei Fehlern in Operationen

### Farbraum-Behandlung:
- Pipeline kümmert sich nicht um Farbraum
- Jede Operation entscheidet selbst (BGR/HSL)
- Geometrische Operationen: Direkt auf BGR
- Farboperationen: Automatische HSL-Konvertierung

## 💡 VERWENDUNG IN GUI

### Für HSL-Slider (Helligkeit, Kontrast, Sättigung):
- Während Bewegung: setLiveOperation() für Echtzeit-Vorschau
- Bei Loslassen: addOperation() für dauerhafte Änderung

### Für geometrische Operationen (Crop, Rotate, Flip):
- Direkt addOperation() - keine Live-Operation nötig

### Für Undo/Redo:
- undo()/redo() aufrufen
- UI-Status mit canUndo()/canRedo() aktualisieren

## Beispiele für Brightness Slider
```bash
// Verbindung im Konstruktor
connect(ui->brightnessSlider, &QSlider::valueChanged, 
        this, &MainWindow::onBrightnessChanged);
connect(ui->brightnessSlider, &QSlider::sliderReleased,
        this, &MainWindow::onBrightnessReleased);

void MainWindow::onBrightnessChanged(int value) {
    // Live-Operation für Echtzeit-Vorschau
    if (!liveBrightness) {
        liveBrightness = std::make_shared<BrightnessAdjust>();
    }
    liveBrightness->setBrightness(value);
    
    // Kombinierte Live-Operation setzen
    auto combinedOp = createCombinedHSLOperation();
    pipeline.setLiveOperation(combinedOp);
    
    updatePreview(); // Sofortige Vorschau
}

void MainWindow::onBrightnessReleased() {
    // Auto-Apply: Finale Operation zur Pipeline
    int finalValue = ui->brightnessSlider->value();
    auto finalOp = std::make_shared<BrightnessAdjust>(finalValue);
    pipeline.addOperation(finalOp);
    pipeline.clearLiveOperation();
    
    // Undo verfügbar machen
    ui->undoButton->setEnabled(true);
    updatePreview();
}
```


## ⚠️ FEHLERBEHANDLUNG

- Leere Bilder werden abgefangen
- Fehlende Operationen werden übersprungen
- Bei Verarbeitungsfehlern: Fallback zum vorherigen Zustand
- Exception-Safety in process()-Methoden

## 🚀 PERFORMANCE-TIPPS

- Cache optimal nutzen (bei unveränderten Operationen)
- Live-Operations für flüssige Slider-Bewegungen
- Frame-Rate Limiting bei häufigen Updates
- processUpTo() für Historie-Anzeige statt process()

## 📈 ZUSTANDSMANAGEMENT

Pipeline-Zustand bei:
- Neues Bild: operations leer, cache invalid
- Operation hinzugefügt: operations wächst, cache invalid
- Live-Operation: cache invalid, temporäre Vorschau
- Undo: Operation von operations → undoneOperations
- Redo: Operation von undoneOperations → operations

Diese Architektur ermöglicht professionelle Bildbearbeitung mit voller Historie und Echtzeit-Vorschau.