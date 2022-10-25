#ifndef PATHFINDER_D3D11_DATA_H
#define PATHFINDER_D3D11_DATA_H

#include <memory>

#include "../../gpu/buffer.h"

using std::shared_ptr;

//! CPU data.

namespace Pathfinder {

struct ClipBufferIDs {
    /// Optional
    shared_ptr<Buffer> metadata;

    shared_ptr<Buffer> tiles;
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D11_DATA_H
