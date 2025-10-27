#pragma once

#include <QString>
#include <QVariantMap>

/**
 * @brief Command Interface for Command Pattern
 *
 * All user actions are implemented as commands to enable undo/redo,
 * macro recording, and consistent action handling.
 */
class ICommand {
   public:
    virtual ~ICommand() = default;

    /**
     * @brief Execute the command
     * @param parameters Command parameters
     * @return true if successful, false otherwise
     */
    virtual bool execute(const QVariantMap& parameters = {}) = 0;

    /**
     * @brief Undo the command (if supported)
     * @return true if successful, false otherwise
     */
    virtual bool undo() {
        return false;
    }

    /**
     * @brief Check if command supports undo
     * @return true if undoable
     */
    virtual bool isUndoable() const {
        return false;
    }

    /**
     * @brief Redo the command (if supported)
     * @return true if successful, false otherwise
     */
    virtual bool redo() {
        return false;
    }

    /**
     * @brief Check if command supports redo
     * @return true if redoable
     */
    virtual bool isRedoable() const {
        return false;
    }

    /**
     * @brief Get command name/description
     * @return Command description
     */
    virtual QString getDescription() const = 0;

    /**
     * @brief Get command parameters schema (if any)
     * @return Parameters schema as QVariantMap
     */
    virtual QVariantMap getParametersSchema() const {
        return {};
    }

    /**
     * @brief Validate command parameters
     * @param parameters Parameters to validate
     * @return true if valid, false otherwise
     */
    virtual bool validateParameters(const QVariantMap& parameters) const {
        return true;
    }
};

/**
 * @brief Base class for undoable commands
 */
class UndoableCommand : public ICommand {
   public:
    bool isUndoable() const override {
        return true;
    }

   protected:
    virtual void saveState() = 0;
    virtual void restoreState() = 0;
};

/**
 * @brief Base class for redoable commands
 */
class RedoableCommand : public ICommand {
   public:
    bool isRedoable() const override {
        return true;
    }

   protected:
    virtual void saveState() = 0;
    virtual void restoreState() = 0;
};