# OperationRegistry - Filter Management System

## 📋 ÜBERSICHT
Die `OperationRegistry` ist ein Factory-System zur dynamischen Verwaltung von Bildfiltern. Sie fungiert als zentrale Registrierungsstelle für alle Filter-Operationen.

## 🎯 WARUM OPERATIONREGISTRY?

### Problem ohne Registry:
- Jeder neue Filter erfordert manuelle UI-Änderungen
- Filter müssen hard-coded im GUI-Code referenziert werden
- Keine einheitliche Verwaltung der Filter
- Schwer erweiterbar für Plugins

### Lösung mit Registry:
- Filter werden automatisch in der UI angezeigt
- Neue Filter erfordern keine GUI-Änderungen
- Zentrale Verwaltung aller Filter
- Einfache Erweiterbarkeit

## 🏗️ ARCHITEKTUR

### Kernkonzept: Factory Pattern
Die Registry verwaltet eine Liste von Factory-Funktionen, die Filter-Instanzen erstellen können.

### Kategorien-System:
Filter werden in Kategorien organisiert:
- COLOR_EFFECTS: Farbeffekte
- VINTAGE: Vintage/Looks  
- BLACK_WHITE: Schwarz-Weiß Filter
- DETAIL: Detail-Verbesserung
- CREATIVE: Kreative Effekte

### Singleton Pattern:
Die Registry ist als Singleton implementiert - es gibt nur eine Instanz im gesamten Programm.

### 💡 VORTEILE
Für Entwickler:
- Einheitliche Schnittstelle: Alle Filter haben dieselbe API
- Einfache Erweiterung: Neue Filter ohne GUI-Änderungen
- Plugins möglich: Externe Filter können registriert werden
- Konsistente Kategorisierung: Filter sind logisch organisiert

Für Benutzer:
- Vollständige Filter-Liste: Keine versteckten Filter
- Organisierte Oberfläche: Filter nach Kategorien gruppiert
- Schneller Zugriff: Ein Klick zum Anwenden
- Beschreibungen: Tooltips erklären jeden Filter


### 🔧 INTEGRATION MIT PIPELINE
Die OperationRegistry erstellt Filter, die direkt mit der ImagePipeline kompatibel sind:

1. Filter wird aus Registry erstellt
2. Filter wird zur Pipeline hinzugefügt
3. Pipeline wendet Filter auf das Bild an
4. Ergebnis wird in der GUI angezeigt

🚀 ZUKUNFTSERWEITERUNGEN
- Filter-Presets: Voreinstellungen für Filter
- Benutzer-Filter: Custom Filter speichern
- Filter-Stärke: Intensität einstellbar
- Filter-Stapel: Mehrere Filter kombinieren

## 🔧 VERWENDUNG IN DER GUI

### 1. Filter-Menü erstellen
```cpp
// Filter-Menü dynamisch aus Registry füllen
void setupFilterMenu() {
    auto& registry = OperationRegistry::getInstance();
    
    // Für jede Kategorie ein Untermenü erstellen
    for (auto category : categories) {
        auto filters = registry.getFiltersByCategory(category);
        QMenu* categoryMenu = filterMenu->addMenu(categoryName);
        
        // Jeden Filter als Menu-Eintrag hinzufügen
        for (const auto& filterName : filters) {
            QAction* action = new QAction(filterName);
            connect(action, &QAction::triggered, [filterName]() {
                applyFilter(filterName);
            });
            categoryMenu->addAction(action);
        }
    }
}
```
### 2. Filter anwenden 

```cpp

void applyFilter(const std::string& filterName) {
    auto& registry = OperationRegistry::getInstance();
    auto filter = registry.createFilter(filterName);
    
    if (filter) {
        // Filter zur Pipeline hinzufügen
        pipeline.addOperation(filter);
        updatePreview();
    }
}
```


## 3. Filter-Panel mit Vorschau
```cpp
// Filter-Buttons mit Vorschau erstellen
void createFilterPanel() {
    auto& registry = OperationRegistry::getInstance();
    auto filters = registry.getAvailableFilters();
    
    for (const auto& filterName : filters) {
        // Button für jeden Filter erstellen
        QPushButton* btn = new QPushButton(filterName);
        
        // Tooltip mit Beschreibung anzeigen
        auto info = registry.getFilterInfo(filterName);
        btn->setToolTip(info.description);
        
        connect(btn, &QPushButton::clicked, [filterName]() {
            applyFilter(filterName);
        });
    }
}
```