/**
 * @date 2020
 * Data and helper functions for generating the harness. This is meant to reduce
 * code duplication.
 */

#include <libsolidity/modelcheck/utils/LibVerify.h>

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

void LibVerify::assertion(CBlockList & _block, CExprPtr _cond)
{
    assert_impl("sol_assert", _block, _cond);
}

void LibVerify::require(CBlockList & _block, CExprPtr _cond)
{
    assert_impl("sol_require", _block, _cond);
}

CExprPtr LibVerify::range(uint8_t _l, uint8_t _u, string const& _msg)
{
    // Determines if there is more than one solution.
    auto lower = make_shared<CIntLiteral>(_l);
    if (_l + 1 == _u)
    {
        return lower;
    }
    else
    {
        auto msg = make_shared<CStringLiteral>(_msg);
        return make_shared<CFuncCall>(
            "nd_range", CArgList{ lower, make_shared<CIntLiteral>(_u), msg }
        );
    }
}

CExprPtr LibVerify::byte(string const& _msg)
{
    return make_shared<CFuncCall>(
        "nd_byte", CArgList{ make_shared<CStringLiteral>(_msg) }
    );
}

void LibVerify::log(CBlockList & _block, string _msg)
{
    auto fn = make_shared<CFuncCall>(
        "smartace_log", CArgList{ make_shared<CStringLiteral>(_msg)
    });
    _block.push_back(fn->stmt());
}

CExprPtr LibVerify::increase(CExprPtr _curr, bool _strict, string _msg)
{
    CFuncCallBuilder call("nd_increase");
    call.push(_curr);
    call.push(_strict ? Literals::ONE : Literals::ZERO);
    call.push(make_shared<CStringLiteral>(_msg));
    return call.merge_and_pop();
}

void LibVerify::assert_impl(string _op, CBlockList & _block, CExprPtr _cond)
{
    auto fn = make_shared<CFuncCall>(_op, CArgList{_cond, Literals::ZERO});
    _block.push_back(fn->stmt());
}

// -------------------------------------------------------------------------- //

}
}
}
