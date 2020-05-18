/**
 * First-pass visitor for generating Solidity the first part of main function,
 * which consist of the decalaration of contract, globalstate, nextGS
 * and every input parameters of functions in main function.
 * @date 2019
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/modelcheck/analysis/ContractDependance.h>
#include <libsolidity/modelcheck/codegen/Details.h>
#include <libsolidity/modelcheck/harness/ActorModel.h>
#include <libsolidity/modelcheck/harness/AddressSpace.h>
#include <libsolidity/modelcheck/harness/StateGenerator.h>

#include <vector>
#include <ostream>

namespace dev
{
namespace solidity
{
namespace modelcheck
{

class CallState;
class FunctionSpecialization;
class MapIndexSummary;
class NewCallGraph;
class TypeConverter;

// -------------------------------------------------------------------------- //

/**
 * Prints a forward declaration for the variable decalarations of contract,
 * globalstate and nextGS in main function.
 */
class MainFunctionGenerator: public ASTConstVisitor
{
public:
    // Constructs a printer for all function forward decl's required by the ast.
    MainFunctionGenerator(
        bool _lockstep_time,
	    MapIndexSummary const& _addrdata,
        ContractDependance const& _dependance,
        NewCallGraph const& _new_graph,
        CallState const& _statedata,
        TypeConverter const& _converter
    );

    // Prints the main function.
    void print(std::ostream& _stream);

private:
    // Stores data required to handle addresses.
    AddressSpace m_addrspace;

    // Stores data required to generate or repopulate the global state.
    StateGenerator m_stategen;

    // Stores data required to handle contract instances.
    ActorModel m_actors;

    // Stores all parameters over the address space.
    MapIndexSummary const& m_addrdata;

    CallState const& m_statedata;
	TypeConverter const& m_converter;

    // For each method on each contract, this will generate a case for the
    // switch block. Note that _args have been initialized first by
    // analyze_decls.
    CBlockList build_case(
        FunctionSpecialization const& _spec,
        std::map<VariableDeclaration const*, std::shared_ptr<CVarDecl>> & _args,
        std::shared_ptr<const CVarDecl> _id,
        bool uses_maps
    );
};

// -------------------------------------------------------------------------- //

}
}
}
