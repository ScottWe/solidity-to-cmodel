/**
 * Decouples address logic from the scheduler. This allows for address models to
 * be interchanged or maintained without analysis of the entire scheduler.
 * 
 * @date 2020
 */

#pragma once

#include <libsolidity/modelcheck/codegen/Details.h>

#include <cstdint>
#include <memory>

namespace dev
{
namespace solidity
{
namespace modelcheck
{

class MapIndexSummary;
class NondetSourceRegistry;

// -------------------------------------------------------------------------- //

/**
 * Describes the possible addresses, maintains their allocations, and provides a
 * utility to allocate distinct addresses.
 */
class AddressSpace
{
public:
    AddressSpace(
        std::shared_ptr<MapIndexSummary const> _address_data,
        std::shared_ptr<NondetSourceRegistry> _nd_reg
    );

    // Returns a unique address. If all possible addresses have been expended,
    // an exception is raised.
    uint64_t reserve();

    // Generates statements in _block to map all constants to distinct values.
    void map_constants(CBlockList & _block) const;

private:
    // Stores the minimum allocatable address.
    const uint64_t MIN_ADDR;

    // The maximum allocated address.
    const uint64_t MAX_ADDR;

    // Stores all parameters over the address space.
    std::shared_ptr<MapIndexSummary const> m_address_data;

    std::shared_ptr<NondetSourceRegistry> m_nd_reg;

    // The last allocated address.
    uint64_t m_next_addr;

    // Computes the value for `MIN_ADDR` from the input `_address_data`.
    static uint64_t compute_min_addr(MapIndexSummary const& _address_data);
};

// -------------------------------------------------------------------------- //

}
}
}
