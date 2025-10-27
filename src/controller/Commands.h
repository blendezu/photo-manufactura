#pragma once

#include <QImage>
#include <QRect>

#include "ICommand.h"

// Forward declaration
class ApplicationController;

/**
 * @brief Command to open a file
 */
class OpenFileCommand : public ICommand {
   public:
    explicit OpenFileCommand(ApplicationController* controller);
    bool execute(const QVariantMap& parameters = {}) override;
    QString getDescription() const override {
        return "Open File";
    }

   private:
    ApplicationController* m_controller;
};

/**
 * @brief Command to save current file
 */
class SaveFileCommand : public ICommand {
   public:
    explicit SaveFileCommand(ApplicationController* controller);
    bool execute(const QVariantMap& parameters = {}) override;
    QString getDescription() const override {
        return "Save File";
    }

   private:
    ApplicationController* m_controller;
};

/**
 * @brief Command to save file with new name
 */
class SaveAsFileCommand : public ICommand {
   public:
    explicit SaveAsFileCommand(ApplicationController* controller);
    bool execute(const QVariantMap& parameters = {}) override;
    QString getDescription() const override {
        return "Save As File";
    }

   private:
    ApplicationController* m_controller;
};

/**
 * @brief Undoable command to apply image filter
 */
class ApplyFilterCommand : public UndoableCommand {
   public:
    explicit ApplyFilterCommand(ApplicationController* controller);
    bool execute(const QVariantMap& parameters = {}) override;
    bool undo() override;
    QString getDescription() const override {
        return "Apply Filter";
    }

   protected:
    void saveState() override;
    void restoreState() override;

   private:
    ApplicationController* m_controller;
    QImage m_originalImage;
    QString m_filterName;
};

/**
 * @brief Undoable command to adjust brightness
 */
class AdjustBrightnessCommand : public UndoableCommand {
   public:
    explicit AdjustBrightnessCommand(ApplicationController* controller);
    bool execute(const QVariantMap& parameters = {}) override;
    bool undo() override;
    QString getDescription() const override {
        return "Adjust Brightness";
    }

   protected:
    void saveState() override;
    void restoreState() override;

   private:
    ApplicationController* m_controller;
    QImage m_originalImage;
    int m_brightnessValue;
};

/**
 * @brief Undoable command to crop image
 */
class CropImageCommand : public UndoableCommand {
   public:
    explicit CropImageCommand(ApplicationController* controller);
    bool execute(const QVariantMap& parameters = {}) override;
    bool undo() override;
    QString getDescription() const override {
        return "Crop Image";
    }

   protected:
    void saveState() override;
    void restoreState() override;

   private:
    ApplicationController* m_controller;
    QImage m_originalImage;
    QRect m_cropArea;
};