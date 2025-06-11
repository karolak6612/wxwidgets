#include "core/assets/MaterialData.h"

namespace RME {
namespace core {
namespace assets {

// Currently, MaterialData and its helper structs have all members public
// and constructors/methods are simple enough to be inline in the header.
// This .cpp file is a placeholder for future non-inline implementations if needed.

// Example of how a more complex method might be moved here later:
/*
MaterialData::MaterialData(QString brushId, QString brushType)
    : id(std::move(brushId)), typeAttribute(std::move(brushType)) {
    // More complex initialization if needed
}
*/

} // namespace assets
} // namespace core
} // namespace RME
