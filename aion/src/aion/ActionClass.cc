#include "ActionClass.hh"

#include "ActionLabel.hh"

using namespace aion;

const std::string &Action::name() const
{
    return label->getName();
}
