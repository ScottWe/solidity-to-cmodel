/**
 * @date 2020
 * Data and helper functions for generating the harness. This is meant to reduce
 * code duplication.
 */

#include <libsolidity/modelcheck/utils/Harness.h>

#include <libsolidity/modelcheck/codegen/Literals.h>

#include <memory>

using namespace std;

namespace dev
{
namespace solidity
{
namespace modelcheck
{

// -------------------------------------------------------------------------- //

void HarnessUtilities::require(CBlockList & _block, CExprPtr _cond)
{
    auto fn = make_shared<CFuncCall>(
        "sol_require", CArgList{_cond, Literals::ZERO
    });
    _block.push_back(fn->stmt());
}

// -------------------------------------------------------------------------- //

CExprPtr HarnessUtilities::range(uint8_t _l, uint8_t _u, string const& _msg)
{
    return make_shared<CFuncCall>(
        "rt_nd_range", CArgList{
            make_shared<CIntLiteral>(_l),
            make_shared<CIntLiteral>(_u),
            make_shared<CStringLiteral>(_msg)
        }
    );
}

// -------------------------------------------------------------------------- //

CExprPtr HarnessUtilities::byte(std::string const& _msg)
{
    return make_shared<CFuncCall>(
        "rt_nd_byte", CArgList{make_shared<CStringLiteral>(_msg)}
    );
}

// -------------------------------------------------------------------------- //

}
}
}