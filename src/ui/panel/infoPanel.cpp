#include "infoPanel.h"

InfoPanel::InfoPanel(QWidget* parent) : QWidget(parent) {
    // Constructor implementation (if needed)
    layout = new QVBoxLayout(this);
    imageLabel = new QLabel(tr("Image Information will be displayed here."), this);
    layout->addWidget(imageLabel);
    setLayout(layout);
}
InfoPanel::~InfoPanel() {
    // Destructor implementation (if needed)
}

// Additional methods for InfoPanel can be added here
