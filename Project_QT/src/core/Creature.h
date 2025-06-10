#ifndef RME_CREATURE_H
#define RME_CREATURE_H

#include <QString>
#include <memory> // For std::unique_ptr

namespace RME {

class Creature {
public:
    Creature(const QString& name = "Unnamed Creature");
    virtual ~Creature() = default;

    // Virtual deep copy
    virtual std::unique_ptr<Creature> deepCopy() const;

    QString getName() const;
    void setName(const QString& newName);

    // Add other basic members or methods as needed for Tile interaction later
    // For example, position might be stored here or managed externally
    // Position position;

private:
    QString name;
    // Other creature-specific data will be added in a dedicated task
};

} // namespace RME

#endif // RME_CREATURE_H
